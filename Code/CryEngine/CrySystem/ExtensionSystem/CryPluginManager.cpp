// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "CryPluginManager.h"

#include "System.h"

#include <CryExtension/ICryFactory.h>
#include <CryExtension/ICryFactoryRegistry.h>
#include <CryExtension/CryCreateClassInstance.h>

#include <CryMono/IMonoRuntime.h>

#include <CryCore/Platform/CryLibrary.h>

CCryPluginManager* CCryPluginManager::s_pThis = 0;

// Descriptor for the C++ binary file of a plugin.
// This is separate since a plugin does not necessarily have to come from a binary, for example if static linking is used.
struct SNativePluginModule
{
	SNativePluginModule() {}

	SNativePluginModule(const char* path)
		: m_engineModulePath(path)
	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "LoadPlugin");
		MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "%s", path);

		m_pFactory = gEnv->pSystem->LoadModuleWithFactory(path, cryiidof<ICryPlugin>());

		if (m_pFactory == nullptr)
		{
			CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Plugin load failed - valid ICryPlugin implementation was not found in plugin %s!", path);

			MarkUnloaded();
			return;
		}
	}

	SNativePluginModule(SNativePluginModule& other)
	{
		m_engineModulePath = other.m_engineModulePath;
		
		other.MarkUnloaded();
	}

	SNativePluginModule(SNativePluginModule&& other)
	{
		m_engineModulePath = other.m_engineModulePath;
		
		other.MarkUnloaded();
	}

	SNativePluginModule& operator=(const SNativePluginModule&& other)
	{
		m_engineModulePath = other.m_engineModulePath;
		
		return *this;
	}

	~SNativePluginModule()
	{
		Shutdown();
	}

	bool Shutdown()
	{
		bool bSuccess = false;
		if (m_engineModulePath.size() > 0)
		{
			bSuccess = GetISystem()->UnloadEngineModule(m_engineModulePath);

			// Prevent Shutdown twice
			MarkUnloaded();
		}

		return bSuccess;
	}

	void MarkUnloaded()
	{
		m_engineModulePath.clear();
	}

	bool IsLoaded()
	{
		return m_engineModulePath.size() > 0;
	}

	ICryFactory* GetFactory() const { return m_pFactory; }

protected:
	string m_engineModulePath;
	ICryFactory* m_pFactory;
};

struct SPluginContainer
{
	// Constructor for native plug-ins
	SPluginContainer(const std::shared_ptr<ICryPlugin>& plugin, SNativePluginModule&& module)
		: m_pPlugin(plugin)
		, m_module(module)
		, m_pluginClassId(plugin->GetFactory()->GetClassID())
	{
	}

	// Constructor for managed (Mono) plug-ins, or statically linked ones
	SPluginContainer(const std::shared_ptr<ICryPlugin>& plugin)
		: m_pPlugin(plugin) {}

	bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
	{
		bool bSuccess = false;

		if (m_pPlugin)
		{
			m_pPlugin->SetUpdateFlags(IPluginUpdateListener::EUpdateType_NoUpdate);
			bSuccess = m_pPlugin->Initialize(env, initParams);
		}

		return bSuccess;
	}

	bool Shutdown()
	{
		m_pPlugin->UnregisterFlowNodes();
		m_pPlugin.reset();

		return m_module.Shutdown();
	}

	ICryPlugin* GetPluginPtr() const
	{
		if (m_pPlugin)
		{
			return m_pPlugin.get();
		}

		return nullptr;
	}

	friend bool operator==(const SPluginContainer& left, const SPluginContainer& right)
	{
		return (left.GetPluginPtr() == right.GetPluginPtr());
	}

	CryClassID                     m_pluginClassId;
	string                         m_pluginAssetDirectory;

	ICryPluginManager::EPluginType m_pluginType;

	std::shared_ptr<ICryPlugin>    m_pPlugin;

	SNativePluginModule            m_module;
};

CCryPluginManager::CCryPluginManager(const SSystemInitParams& initParams)
	: m_systemInitParams(initParams)
{
	s_pThis = this;

	GetISystem()->GetISystemEventDispatcher()->RegisterListener(this);
}

CCryPluginManager::~CCryPluginManager()
{
	GetISystem()->GetISystemEventDispatcher()->RemoveListener(this);

	UnloadAllPlugins();

	CRY_ASSERT(m_pluginContainer.empty());

	if (gEnv->pConsole)
	{
		gEnv->pConsole->RemoveCommand("sys_reload_plugin");
		gEnv->pConsole->UnregisterVariable("sys_debug_plugin", true);
	}
}

void CCryPluginManager::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_REGISTER_FLOWNODES:
		{
			for (auto it = m_pluginContainer.cbegin(); it != m_pluginContainer.cend(); ++it)
			{
				it->GetPluginPtr()->RegisterFlowNodes();
			}
		}
		break;
	}
}

void CCryPluginManager::Initialize()
{
	// Start with loading the default engine plug-ins
	LoadPluginFromDisk(EPluginType::EPluginType_CPP, "CryDefaultEntities");
	//Schematyc + Schematyc Standard Enviroment
	LoadPluginFromDisk(EPluginType::EPluginType_CPP, "CrySchematycCore");
	LoadPluginFromDisk(EPluginType::EPluginType_CPP, "CrySchematycSTDEnv");

	LoadPluginFromDisk(EPluginType::EPluginType_CPP, "CrySensorSystem");
}

//--- UTF8 parse helper routines

static const char* Parser_NextChar(const char* pStart, const char* pEnd)
{
	CRY_ASSERT(pStart != nullptr && pEnd != nullptr);

	if (pStart < pEnd)
	{
		CRY_ASSERT(0 <= *pStart && *pStart <= SCHAR_MAX);
		pStart++;
	}

	CRY_ASSERT(pStart <= pEnd);
	return pStart;
}

static const char* Parser_StrChr(const char* pStart, const char* pEnd, int c)
{
	CRY_ASSERT(pStart != nullptr && pEnd != nullptr);
	CRY_ASSERT(pStart <= pEnd);
	CRY_ASSERT(0 <= c && c <= SCHAR_MAX);
	const char* it = (const char*)memchr(pStart, c, pEnd - pStart);
	return (it != nullptr) ? it : pEnd;
}

static bool Parser_StrEquals(const char* pStart, const char* pEnd, const char* szKey)
{
	size_t klen = strlen(szKey);
	return (klen == pEnd - pStart) && memcmp(pStart, szKey, klen) == 0;
}

bool CCryPluginManager::LoadPluginFromDisk(EPluginType type, const char* path)
{
	CryLogAlways("Loading plugin %s", path);

	std::shared_ptr<ICryPlugin> pPlugin;

	switch (type)
	{
	case EPluginType::EPluginType_CPP:
		{
			// Load the module, note that this calls ISystem::InitializeEngineModule
			// Automatically unloads in destructor
			SNativePluginModule module(path);

			ICryUnknownPtr pUnk = module.GetFactory()->CreateClassInstance();
			pPlugin = cryinterface_cast<ICryPlugin>(pUnk);
			if (!pPlugin)
			{
				CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Plugin load failed - Could not create an instance of %s in plugin %s!", module.GetFactory()->GetName(), path);
				return false;
			}

			m_pluginContainer.emplace_back(pPlugin, std::move(module));
			module.MarkUnloaded();

			break;
		}

	case EPluginType::EPluginType_CS:
		{
			if (gEnv->pMonoRuntime == nullptr)
			{
				CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Tried to load Mono plugin %s without having loaded the CryMono module!", path);

				return false;
			}

			pPlugin = gEnv->pMonoRuntime->LoadBinary(path);
			if (!pPlugin)
			{
				CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Plugin load failed - Could not load Mono binary %s", path);

				return false;
			}

			m_pluginContainer.emplace_back(pPlugin);
			break;
		}
	}

	auto &containedPlugin = m_pluginContainer.back();

	if (!containedPlugin.Initialize(*gEnv, m_systemInitParams))
	{
		CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Plugin load failed - Failed to initialize plugin %s!", path);

		m_pluginContainer.pop_back();

		return false;
	}

	// Notification to listeners, that plugin got initialized
	NotifyEventListeners(containedPlugin.m_pluginClassId, IPluginEventListener::EPluginEvent::Initialized);
	return true;
}

bool CCryPluginManager::UnloadAllPlugins()
{
	bool bError = false;
	for (auto it = m_pluginContainer.begin(); it != m_pluginContainer.end(); ++it)
	{
		if (!it->Shutdown())
		{
			bError = true;
		}

		// notification to listeners, that plugin got un-initialized
		NotifyEventListeners(it->m_pluginClassId, IPluginEventListener::EPluginEvent::Unloaded);
	}

	m_pluginContainer.clear();

	return !bError;
}

void CCryPluginManager::NotifyEventListeners(const CryClassID& classID, IPluginEventListener::EPluginEvent event)
{
	for (auto it = m_pluginListenerMap.cbegin(); it != m_pluginListenerMap.cend(); ++it)
	{
		if (std::find(it->second.begin(), it->second.end(), classID) != it->second.end())
		{
			it->first->OnPluginEvent(classID, event);
		}
	}
}

void CCryPluginManager::Update(IPluginUpdateListener::EPluginUpdateType updateFlags)
{
	for (auto it = m_pluginContainer.cbegin(); it != m_pluginContainer.cend(); ++it)
	{
		if (it->GetPluginPtr()->GetUpdateFlags() & updateFlags)
		{
			it->GetPluginPtr()->OnPluginUpdate(updateFlags);
		}
	}
}

std::shared_ptr<ICryPlugin> CCryPluginManager::QueryPluginById(const CryClassID& classID) const
{
	for (auto it = m_pluginContainer.cbegin(); it != m_pluginContainer.cend(); ++it)
	{
		ICryFactory* pFactory = it->GetPluginPtr()->GetFactory();
		if (pFactory)
		{
			if (pFactory->GetClassID() == classID || pFactory->ClassSupports(classID))
			{
				return it->m_pPlugin;
			}
		}
	}

	return nullptr;
}


std::shared_ptr<ICryPlugin> CCryPluginManager::AcquirePluginById(const CryClassID& classID)
{
	std::shared_ptr<ICryPlugin> pPlugin;
	CryCreateClassInstance(classID, pPlugin);

	if (pPlugin == nullptr)
	{
		return nullptr;
	}

	m_pluginContainer.emplace_back(pPlugin);
	if (!m_pluginContainer.back().Initialize(*gEnv, m_systemInitParams))
	{
		m_pluginContainer.back().Shutdown();
		m_pluginContainer.pop_back();

		return nullptr;
	}

	return pPlugin;
}

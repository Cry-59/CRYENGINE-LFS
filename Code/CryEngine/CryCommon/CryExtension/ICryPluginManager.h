// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CrySystem/ICryPlugin.h>
#include <CryCore/Containers/CryListenerSet.h>

struct IPluginEventListener
{
	enum class EPluginEvent
	{
		Initialized,
		Unloaded,
	};

	virtual ~IPluginEventListener() {}
	virtual void OnPluginEvent(const CryClassID& pluginClassId, EPluginEvent event) = 0;
};

struct ICryPluginManager
{
	enum class EPluginType
	{
		EPluginType_CPP = 0,
		EPluginType_CS
	};

	ICryPluginManager() {}
	virtual ~ICryPluginManager() {}

	virtual void RegisterEventListener(const CryClassID& pluginClassId, IPluginEventListener* pListener) = 0;
	virtual void RemoveEventListener(const CryClassID& pluginClassId, IPluginEventListener* pListener) = 0;

	virtual bool LoadPluginFromDisk(EPluginType type, const char* path) = 0;

	template<typename T>
	T* QueryPlugin() const
	{
		if (ICryUnknownPtr pExtension = QueryPluginById(cryiidof<T>()))
		{
			return cryinterface_cast<T>(pExtension.get());
		}

		return nullptr;
	}

	// Queries whether a plugin exists, and if not tries to create a new instance
	template<typename T>
	T* AcquirePlugin()
	{
		if (ICryUnknownPtr pExtension = AcquirePluginById(cryiidof<T>()))
		{
			return cryinterface_cast<T>(pExtension.get());
		}

		return nullptr;
	}

protected:
	virtual std::shared_ptr<ICryPlugin> QueryPluginById(const CryClassID& classID) const = 0;
	virtual std::shared_ptr<ICryPlugin> AcquirePluginById(const CryClassID& classID) = 0;
};

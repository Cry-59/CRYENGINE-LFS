// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "CryMovie.h"
#include "Movie.h"
#include <CryCore/CrtDebugStats.h>

// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>
#include <CrySystem/IEngineModule.h>
#include <CryExtension/ICryFactory.h>
#include <CryExtension/ClassWeaver.h>

#undef GetClassName

struct CSystemEventListner_Movie : public ISystemEventListener
{
public:
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
			{
				CLightAnimWrapper::ReconstructCache();
				break;
			}
		}
	}
};

static CSystemEventListner_Movie g_system_event_listener_movie;

class CEngineModule_CryMovie : public IMovieEngineModule
{
	CRYINTERFACE_BEGIN()
		CRYINTERFACE_ADD(IEngineModule)
		CRYINTERFACE_ADD(IMovieEngineModule)
	CRYINTERFACE_END()

	CRYGENERATE_SINGLETONCLASS(CEngineModule_CryMovie, "EngineModule_CryMovie", 0xdce26beebdc6400f, 0xa0e9b42839f2dd5b)

	virtual ~CEngineModule_CryMovie() {}

	virtual const char* GetName() override { return "CryMovie"; };
	virtual const char* GetCategory() override { return "CryEngine"; };

	virtual bool        Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;
		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_movie);

		env.pMovieSystem = new CMovieSystem(pSystem);
		return true;
	}
};

CRYREGISTER_SINGLETON_CLASS(CEngineModule_CryMovie)
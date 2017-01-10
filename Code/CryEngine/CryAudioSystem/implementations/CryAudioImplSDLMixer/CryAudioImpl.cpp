// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "AudioImpl.h"
#include "AudioImplCVars.h"
#include <CryAudio/IAudioSystem.h>
#include <CryCore/Platform/platform_impl.inl>
#include <CrySystem/IEngineModule.h>
#include <CryExtension/ClassWeaver.h>

using namespace CryAudio::Impl::SDL_mixer;

// Define global objects.
CSoundAllocator<2*1024*1024> g_audioImplMemoryPool;
CAudioLogger g_audioImplLogger;
CAudioImplCVars CryAudio::Impl::SDL_mixer::g_audioImplCVars;

//////////////////////////////////////////////////////////////////////////
class CEngineModule_CryAudioImplSDLMixer : public IAudioSystemImplementationModule
{
	CRYINTERFACE_BEGIN()
		CRYINTERFACE_ADD(IEngineModule)
		CRYINTERFACE_ADD(IAudioSystemImplementationModule)
	CRYINTERFACE_END()
	
	CRYGENERATE_SINGLETONCLASS(CEngineModule_CryAudioImplSDLMixer, "EngineModule_AudioImpl", 0x8030c0d1905b4031, 0xa3785a8b53125f3f)

	CEngineModule_CryAudioImplSDLMixer();
	virtual ~CEngineModule_CryAudioImplSDLMixer() {}

	//////////////////////////////////////////////////////////////////////////
	virtual char const* GetName()     override { return "CryAudioImplSDLMixer"; }
	virtual char const* GetCategory() override { return "CryAudio"; }

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override
	{
		// Initialize memory pools.
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "SDL Mixer Audio Implementation Memory Pool Primary");
		size_t const poolSize = g_audioImplCVars.m_primaryMemoryPoolSize << 10;
		uint8* const pPoolMemory = new uint8[poolSize];
		g_audioImplMemoryPool.InitMem(poolSize, pPoolMemory, "SDL Mixer Implementation Audio Pool");

		POOL_NEW_CREATE(CAudioImpl, pImpl);

		if (pImpl != nullptr)
		{
			g_audioImplLogger.Log(eAudioLogType_Always, "CryAudioImplSDLMixer loaded");

			SAudioRequest request;
			request.flags = eAudioRequestFlags_PriorityHigh | eAudioRequestFlags_ExecuteBlocking;

			SAudioManagerRequestData<eAudioManagerRequestType_SetAudioImpl> requestData(pImpl);
			request.pData = &requestData;

			gEnv->pAudioSystem->AddRequestListener(&CEngineModule_CryAudioImplSDLMixer::OnAudioEvent, nullptr, eAudioRequestType_AudioManagerRequest, eAudioManagerRequestType_SetAudioImpl);
			env.pAudioSystem->PushRequest(request);
			gEnv->pAudioSystem->RemoveRequestListener(&CEngineModule_CryAudioImplSDLMixer::OnAudioEvent, nullptr);
		}
		else
		{
			g_audioImplLogger.Log(eAudioLogType_Always, "CryAudioImplSDLMixer failed to load");
		}

		return m_bSuccess;
	}

	//////////////////////////////////////////////////////////////////////////
	static void OnAudioEvent(SAudioRequestInfo const* const pAudioRequestInfo)
	{
		m_bSuccess = pAudioRequestInfo->requestResult == eAudioRequestResult_Success;
	}

	static bool m_bSuccess;
};

CRYREGISTER_SINGLETON_CLASS(CEngineModule_CryAudioImplSDLMixer)
bool CEngineModule_CryAudioImplSDLMixer::m_bSuccess = false;

CEngineModule_CryAudioImplSDLMixer::CEngineModule_CryAudioImplSDLMixer()
{
	g_audioImplCVars.RegisterVariables();
}

#include <CryCore/CrtDebugStats.h>

// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "IAudioInterfacesCommonData.h"
#include <CryCore/Platform/platform.h>

#include <CrySystem/IEngineModule.h>

// General macros.
//#define ENABLE_AUDIO_PORT_MESSAGES

#if defined(ENABLE_AUDIO_PORT_MESSAGES) && defined(_MSC_VER)
	#define AUDIO_STRINGANIZE2(x) # x
	#define AUDIO_STRINGANIZE1(x) AUDIO_STRINGANIZE2(x)
	#define TODO(y)               __pragma(message(__FILE__ "(" AUDIO_STRINGANIZE1(__LINE__) ") : " "[AUDIO] TODO >>> " AUDIO_STRINGANIZE2(y)))
	#define REINST_FULL(y)        __pragma(message(__FILE__ "(" AUDIO_STRINGANIZE1(__LINE__) ") : " "[AUDIO] REINST " __FUNCSIG__ " >>> " AUDIO_STRINGANIZE2(y)))
	#define REINST(y)             __pragma(message(__FILE__ "(" AUDIO_STRINGANIZE1(__LINE__) ") : " "[AUDIO] REINST " __FUNCTION__ " >>> " AUDIO_STRINGANIZE2(y)))
#else
	#define TODO(y)
	#define REINST_FULL(y)
	#define REINST(y)
#endif

//! \note We need this explicit here to prevent circular includes to IEntity.
//! Unique identifier for each entity instance.
typedef unsigned int EntityId;

// Forward declarations.
struct IVisArea;
struct ICVar;
class CAudioRayInfo;

namespace CryAudio
{
namespace Impl
{
struct IAudioImpl;
}
}

enum EAudioDataScope : AudioEnumFlagsType
{
	eAudioDataScope_None,
	eAudioDataScope_Global,
	eAudioDataScope_LevelSpecific,
	eAudioDataScope_All,
};

enum EAudioManagerRequestType : AudioEnumFlagsType
{
	eAudioManagerRequestType_None                   = 0,
	eAudioManagerRequestType_SetAudioImpl           = BIT(0),
	eAudioManagerRequestType_ReleaseAudioImpl       = BIT(1),
	eAudioManagerRequestType_RefreshAudioSystem     = BIT(2),
	eAudioManagerRequestType_ReserveAudioObjectId   = BIT(3),
	eAudioManagerRequestType_LoseFocus              = BIT(4),
	eAudioManagerRequestType_GetFocus               = BIT(5),
	eAudioManagerRequestType_MuteAll                = BIT(6),
	eAudioManagerRequestType_UnmuteAll              = BIT(7),
	eAudioManagerRequestType_StopAllSounds          = BIT(8),
	eAudioManagerRequestType_ParseControlsData      = BIT(9),
	eAudioManagerRequestType_ParsePreloadsData      = BIT(10),
	eAudioManagerRequestType_ClearControlsData      = BIT(11),
	eAudioManagerRequestType_ClearPreloadsData      = BIT(12),
	eAudioManagerRequestType_PreloadSingleRequest   = BIT(13),
	eAudioManagerRequestType_UnloadSingleRequest    = BIT(14),
	eAudioManagerRequestType_UnloadAFCMDataByScope  = BIT(15),
	eAudioManagerRequestType_DrawDebugInfo          = BIT(16), //!< Only used internally!
	eAudioManagerRequestType_AddRequestListener     = BIT(17),
	eAudioManagerRequestType_RemoveRequestListener  = BIT(18),
	eAudioManagerRequestType_ChangeLanguage         = BIT(19),
	eAudioManagerRequestType_RetriggerAudioControls = BIT(20),
	eAudioManagerRequestType_ReleasePendingRays     = BIT(21), //!< Only used internally!
	eAudioManagerRequestType_ReloadControlsData     = BIT(22),
	eAudioManagerRequestType_GetAudioFileData       = BIT(23), //!< Only used internally!
};

enum EAudioCallbackManagerRequestType : AudioEnumFlagsType
{
	eAudioCallbackManagerRequestType_None                          = 0,
	eAudioCallbackManagerRequestType_ReportStartedEvent            = BIT(0), //!< Only relevant for delayed playback.
	eAudioCallbackManagerRequestType_ReportFinishedEvent           = BIT(1), //!< Only used internally!
	eAudioCallbackManagerRequestType_ReportFinishedTriggerInstance = BIT(2), //!< Only used internally!
	eAudioCallbackManagerRequestType_ReportStartedFile             = BIT(3), //!< Only used internally!
	eAudioCallbackManagerRequestType_ReportStoppedFile             = BIT(4), //!< Only used internally!
	eAudioCallbackManagerRequestType_ReportVirtualizedEvent        = BIT(5), //!< Only used internally!
	eAudioCallbackManagerRequestType_ReportPhysicalizedEvent       = BIT(6), //!< Only used internally!
};

enum EAudioListenerRequestType : AudioEnumFlagsType
{
	eAudioListenerRequestType_None              = 0,
	eAudioListenerRequestType_SetTransformation = BIT(0),
};

enum EAudioObjectRequestType : AudioEnumFlagsType
{
	eAudioObjectRequestType_None                 = 0,
	eAudioObjectRequestType_PrepareTrigger       = BIT(0),
	eAudioObjectRequestType_UnprepareTrigger     = BIT(1),
	eAudioObjectRequestType_PlayFile             = BIT(2),
	eAudioObjectRequestType_StopFile             = BIT(3),
	eAudioObjectRequestType_ExecuteTrigger       = BIT(4),
	eAudioObjectRequestType_StopTrigger          = BIT(5),
	eAudioObjectRequestType_StopAllTriggers      = BIT(6),
	eAudioObjectRequestType_SetTransformation    = BIT(7),
	eAudioObjectRequestType_SetRtpcValue         = BIT(8),
	eAudioObjectRequestType_SetSwitchState       = BIT(9),
	eAudioObjectRequestType_SetVolume            = BIT(10),
	eAudioObjectRequestType_SetEnvironmentAmount = BIT(11),
	eAudioObjectRequestType_ResetEnvironments    = BIT(12),
	eAudioObjectRequestType_ReleaseObject        = BIT(13),
	eAudioObjectRequestType_ProcessPhysicsRay    = BIT(14), //!< Only used internally!
};

enum EAudioOcclusionType : AudioEnumFlagsType
{
	eAudioOcclusionType_None,
	eAudioOcclusionType_Ignore,
	eAudioOcclusionType_Adaptive,
	eAudioOcclusionType_Low,
	eAudioOcclusionType_Medium,
	eAudioOcclusionType_High,

	eAudioOcclusionType_Count,
};
AUTO_TYPE_INFO(EAudioOcclusionType);

//////////////////////////////////////////////////////////////////////////
struct SAudioManagerRequestDataBase : public SAudioRequestDataBase
{
	explicit SAudioManagerRequestDataBase(EAudioManagerRequestType const _type)
		: SAudioRequestDataBase(eAudioRequestType_AudioManagerRequest)
		, type(_type)
	{}

	virtual ~SAudioManagerRequestDataBase() override = default;

	EAudioManagerRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioManagerRequestType T>
struct SAudioManagerRequestData : public SAudioManagerRequestDataBase
{
	SAudioManagerRequestData()
		: SAudioManagerRequestDataBase(T)
	{}

	virtual ~SAudioManagerRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_SetAudioImpl> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(CryAudio::Impl::IAudioImpl* const _pImpl)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_SetAudioImpl)
		, pImpl(_pImpl)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	CryAudio::Impl::IAudioImpl* const pImpl;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_ReserveAudioObjectId> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(CATLAudioObject** const _ppAudioObject, char const* const _szAudioObjectName)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_ReserveAudioObjectId)
		, ppAudioObject(_ppAudioObject)
		, szAudioObjectName(_szAudioObjectName)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	CATLAudioObject** const ppAudioObject;
	char const* const       szAudioObjectName;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_AddRequestListener> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(
	  void const* const _pObjectToListenTo,
	  void(*_func)(SAudioRequestInfo const* const),
	  EAudioRequestType const _type,
	  AudioEnumFlagsType _specificRequestMask)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_AddRequestListener)
		, pObjectToListenTo(_pObjectToListenTo)
		, func(_func)
		, type(_type)
		, specificRequestMask(_specificRequestMask)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	void const* const        pObjectToListenTo;
	void                     (* func)(SAudioRequestInfo const* const);
	EAudioRequestType const  type;
	AudioEnumFlagsType const specificRequestMask;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_RemoveRequestListener> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(void const* const _pObjectToListenTo, void(*_func)(SAudioRequestInfo const* const))
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_RemoveRequestListener)
		, pObjectToListenTo(_pObjectToListenTo)
		, func(_func)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	void const* const pObjectToListenTo;
	void              (* func)(SAudioRequestInfo const* const);
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_ParseControlsData> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(char const* const _szFolderPath, EAudioDataScope const _dataScope)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_ParseControlsData)
		, szFolderPath(_szFolderPath)
		, dataScope(_dataScope)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	char const* const     szFolderPath;
	EAudioDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_ParsePreloadsData> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(char const* const _szFolderPath, EAudioDataScope const _dataScope)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_ParsePreloadsData)
		, szFolderPath(_szFolderPath)
		, dataScope(_dataScope)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	char const* const     szFolderPath;
	EAudioDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_ClearControlsData> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(EAudioDataScope const _dataScope)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_ClearControlsData)
		, dataScope(_dataScope)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	EAudioDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_ClearPreloadsData> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(EAudioDataScope const _dataScope)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_ClearPreloadsData)
		, dataScope(_dataScope)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	EAudioDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_PreloadSingleRequest> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(AudioPreloadRequestId const _audioPreloadRequestId, bool const _bAutoLoadOnly)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_PreloadSingleRequest)
		, audioPreloadRequestId(_audioPreloadRequestId)
		, bAutoLoadOnly(_bAutoLoadOnly)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	AudioPreloadRequestId const audioPreloadRequestId;
	bool const                  bAutoLoadOnly;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_UnloadSingleRequest> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(AudioPreloadRequestId const _audioPreloadRequestId)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_UnloadSingleRequest)
		, audioPreloadRequestId(_audioPreloadRequestId)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	AudioPreloadRequestId const audioPreloadRequestId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_UnloadAFCMDataByScope> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(EAudioDataScope const _dataScope)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_UnloadAFCMDataByScope)
		, dataScope(_dataScope)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	EAudioDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_RefreshAudioSystem> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(char const* const _szLevelName)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_RefreshAudioSystem)
		, szLevelName(_szLevelName)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	char const* const szLevelName;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioManagerRequestData<eAudioManagerRequestType_ReloadControlsData> : public SAudioManagerRequestDataBase
{
	explicit SAudioManagerRequestData(char const* const _szFolderPath, char const* const _szLevelName)
		: SAudioManagerRequestDataBase(eAudioManagerRequestType_ReloadControlsData)
		, szFolderPath(_szFolderPath)
		, szLevelName(_szLevelName)
	{}

	virtual ~SAudioManagerRequestData() override = default;

	char const* const szFolderPath;
	char const* const szLevelName;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioCallbackManagerRequestDataBase : public SAudioRequestDataBase
{
	explicit SAudioCallbackManagerRequestDataBase(EAudioCallbackManagerRequestType const _type)
		: SAudioRequestDataBase(eAudioRequestType_AudioCallbackManagerRequest)
		, type(_type)
	{}

	virtual ~SAudioCallbackManagerRequestDataBase() override = default;

	EAudioCallbackManagerRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioCallbackManagerRequestType T>
struct SAudioCallbackManagerRequestData : public SAudioCallbackManagerRequestDataBase
{
	virtual ~SAudioCallbackManagerRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportStartedEvent> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(AudioEventId const _audioEventId)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportStartedEvent)
		, audioEventId(_audioEventId)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioEventId const audioEventId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportFinishedEvent> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(AudioEventId const _audioEventId, bool const _bSuccess)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportFinishedEvent)
		, audioEventId(_audioEventId)
		, bSuccess(_bSuccess)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioEventId const audioEventId;
	bool const         bSuccess;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportVirtualizedEvent> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(AudioEventId const _audioEventId)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportVirtualizedEvent)
		, audioEventId(_audioEventId)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioEventId const audioEventId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportPhysicalizedEvent> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(AudioEventId const _audioEventId)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportPhysicalizedEvent)
		, audioEventId(_audioEventId)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioEventId const audioEventId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportFinishedTriggerInstance> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(AudioControlId const _audioTriggerId)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportFinishedTriggerInstance)
		, audioTriggerId(_audioTriggerId)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioControlId const audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportStartedFile> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(
	  AudioStandaloneFileId const _audioStandaloneFileInstanceId,
	  char const* const _szFile, bool _bSuccess)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportStartedFile)
		, audioStandaloneFileInstanceId(_audioStandaloneFileInstanceId)
		, szFile(_szFile)
		, bSuccess(_bSuccess)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioStandaloneFileId const audioStandaloneFileInstanceId;
	char const* const           szFile;
	bool const                  bSuccess;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportStoppedFile> : public SAudioCallbackManagerRequestDataBase
{
	explicit SAudioCallbackManagerRequestData(
	  AudioStandaloneFileId const _audioStandaloneFileInstanceId,
	  char const* const _szFile)
		: SAudioCallbackManagerRequestDataBase(eAudioCallbackManagerRequestType_ReportStoppedFile)
		, audioStandaloneFileInstanceId(_audioStandaloneFileInstanceId)
		, szFile(_szFile)
	{}

	virtual ~SAudioCallbackManagerRequestData() override = default;

	AudioStandaloneFileId const audioStandaloneFileInstanceId;
	char const* const           szFile;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioObjectRequestDataBase : public SAudioRequestDataBase
{
	explicit SAudioObjectRequestDataBase(EAudioObjectRequestType const _type)
		: SAudioRequestDataBase(eAudioRequestType_AudioObjectRequest)
		, type(_type)
	{}

	virtual ~SAudioObjectRequestDataBase() override = default;

	EAudioObjectRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioObjectRequestType T>
struct SAudioObjectRequestData : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(T)
	{}

	virtual ~SAudioObjectRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_PrepareTrigger> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_PrepareTrigger)
		, audioTriggerId(INVALID_AUDIO_CONTROL_ID)
	{}

	explicit SAudioObjectRequestData(AudioControlId const _audioTriggerId)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_PrepareTrigger)
		, audioTriggerId(_audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioControlId audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_UnprepareTrigger> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_UnprepareTrigger)
		, audioTriggerId(INVALID_AUDIO_CONTROL_ID)
	{}

	explicit SAudioObjectRequestData(AudioControlId const _audioTriggerId)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_UnprepareTrigger)
		, audioTriggerId(_audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioControlId audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_PlayFile> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_PlayFile)
		, szFile(nullptr)
		, bLocalized(true)
		, usedAudioTriggerId(INVALID_AUDIO_CONTROL_ID)
	{}

	explicit SAudioObjectRequestData(char const* const _szFile, bool _bLocalized, AudioControlId _usedAudioTriggerId)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_PlayFile)
		, szFile(_szFile)
		, bLocalized(_bLocalized)
		, usedAudioTriggerId(_usedAudioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	char const*    szFile;
	bool           bLocalized;
	AudioControlId usedAudioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_StopFile> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_StopFile)
		, szFile(nullptr)
	{}

	explicit SAudioObjectRequestData(char const* const _szFile)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_StopFile)
		, szFile(_szFile)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	char const* szFile;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_ExecuteTrigger> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_ExecuteTrigger)
		, audioTriggerId(INVALID_AUDIO_CONTROL_ID)
		, timeUntilRemovalInMS(0.0f)
	{}

	explicit SAudioObjectRequestData(AudioControlId const _audioTriggerId, float const fPassedTimeUntilRemovalInMS)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_ExecuteTrigger)
		, audioTriggerId(_audioTriggerId)
		, timeUntilRemovalInMS(fPassedTimeUntilRemovalInMS)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioControlId audioTriggerId;
	float          timeUntilRemovalInMS;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_StopTrigger> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_StopTrigger)
		, audioTriggerId(INVALID_AUDIO_CONTROL_ID)
	{}

	explicit SAudioObjectRequestData(AudioControlId const _audioTriggerId)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_StopTrigger)
		, audioTriggerId(_audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioControlId audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_SetTransformation> : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(CAudioObjectTransformation const& _transformation)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetTransformation)
		, transformation(_transformation)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	// We opt for copying the transformation instead of storing a reference in order to prevent a potential dangling-reference bug.
	// Callers might pass a vector or matrix to the constructor, which implicitly convert to CAudioObjectTransformation.
	// Implicit conversion introduces a temporary object, and a reference could potentially dangle, as the temporary gets destroyed before this request gets passed to CAudioSystem::PushRequest,
	// where it gets ultimately copied for internal processing.
	CAudioObjectTransformation const transformation;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_SetRtpcValue> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetRtpcValue)
		, audioRtpcId(INVALID_AUDIO_CONTROL_ID)
		, value(0.0f)
	{}

	explicit SAudioObjectRequestData(AudioControlId const _audioRtpcId, float const fPassedValue)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetRtpcValue)
		, audioRtpcId(_audioRtpcId)
		, value(fPassedValue)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioControlId audioRtpcId;
	float          value;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_SetSwitchState> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetSwitchState)
		, audioSwitchId(INVALID_AUDIO_CONTROL_ID)
		, audioSwitchStateId(INVALID_AUDIO_SWITCH_STATE_ID)
	{}

	explicit SAudioObjectRequestData(AudioControlId const _audioSwitchId, AudioSwitchStateId const _audioSwitchStateId)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetSwitchState)
		, audioSwitchId(_audioSwitchId)
		, audioSwitchStateId(_audioSwitchStateId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioControlId     audioSwitchId;
	AudioSwitchStateId audioSwitchStateId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_SetVolume> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetVolume)
		, volume(1.0f)
	{}

	explicit SAudioObjectRequestData(float const _volume)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetVolume)
		, volume(_volume)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	float const volume;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_SetEnvironmentAmount> : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetEnvironmentAmount)
		, audioEnvironmentId(INVALID_AUDIO_ENVIRONMENT_ID)
		, amount(1.0f)
	{}

	explicit SAudioObjectRequestData(AudioEnvironmentId const _audioEnvironmentId, float const _amount)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_SetEnvironmentAmount)
		, audioEnvironmentId(_audioEnvironmentId)
		, amount(_amount)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	AudioEnvironmentId audioEnvironmentId;
	float              amount;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<eAudioObjectRequestType_ProcessPhysicsRay> : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(CAudioRayInfo* const _pAudioRayInfo)
		: SAudioObjectRequestDataBase(eAudioObjectRequestType_ProcessPhysicsRay)
		, pAudioRayInfo(_pAudioRayInfo)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	CAudioRayInfo* const pAudioRayInfo;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioListenerRequestDataBase : public SAudioRequestDataBase
{
	explicit SAudioListenerRequestDataBase(EAudioListenerRequestType const _type)
		: SAudioRequestDataBase(eAudioRequestType_AudioListenerRequest)
		, type(_type)
	{}

	virtual ~SAudioListenerRequestDataBase() override = default;

	EAudioListenerRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioListenerRequestType T>
struct SAudioListenerRequestData : public SAudioListenerRequestDataBase
{
	virtual ~SAudioListenerRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioListenerRequestData<eAudioListenerRequestType_SetTransformation> : public SAudioListenerRequestDataBase
{
	explicit SAudioListenerRequestData(CAudioObjectTransformation const& _transformation, CATLListener* const _pListener = nullptr)
		: SAudioListenerRequestDataBase(eAudioListenerRequestType_SetTransformation)
		, transformation(_transformation)
		, pListener(_pListener)
	{}

	virtual ~SAudioListenerRequestData() override = default;

	// We opt for copying the transformation instead of storing a reference in order to prevent a potential dangling-reference bug.
	// Callers might pass a vector or matrix to the constructor, which implicitly convert to CAudioObjectTransformation.
	// Implicit conversion introduces a temporary object, and a reference could potentially dangle, as the temporary gets destroyed before this request gets passed to CAudioSystem::PushRequest,
	// where it gets ultimately copied for internal processing.
	CAudioObjectTransformation const transformation;
	CATLListener* const              pListener;
};

//////////////////////////////////////////////////////////////////////////
struct IAudioProxy
{
	// <interfuscator:shuffle>
	virtual ~IAudioProxy() = default;

	virtual void             Initialize(char const* const szAudioObjectName, bool const bInitAsync = true) = 0;
	virtual void             Release() = 0;
	virtual void             Reset() = 0;
	virtual void             PlayFile(SAudioPlayFileInfo const& _playbackInfo, SAudioCallBackInfo const& _callBackInfo = SAudioCallBackInfo::GetEmptyObject()) = 0;
	virtual void             StopFile(char const* const szFile) = 0;
	virtual void             ExecuteTrigger(AudioControlId const audioTriggerId, SAudioCallBackInfo const& callBackInfo = SAudioCallBackInfo::GetEmptyObject()) = 0;
	virtual void             StopTrigger(AudioControlId const audioTriggerId) = 0;
	virtual void             SetSwitchState(AudioControlId const audioSwitchId, AudioSwitchStateId const audioSwitchStateId) = 0;
	virtual void             SetRtpcValue(AudioControlId const audioRtpcId, float const value) = 0;
	virtual void             SetOcclusionType(EAudioOcclusionType const occlusionType) = 0;
	virtual void             SetTransformation(Matrix34 const& transformation) = 0;
	virtual void             SetPosition(Vec3 const& position) = 0;
	virtual void             SetEnvironmentAmount(AudioEnvironmentId const audioEnvironmentId, float const amount) = 0;
	virtual void             SetCurrentEnvironments(EntityId const entityToIgnore = 0) = 0;
	virtual CATLAudioObject* GetAudioObject() const = 0;
	// </interfuscator:shuffle>
};

struct IAudioSystemEngineModule : public IEngineModule
{
	CRYINTERFACE_DECLARE(IAudioSystemEngineModule, 0x6C7BA422375B4325, 0xAE00918679610D2E);
};

struct IAudioSystemImplementationModule : public IEngineModule
{
	CRYINTERFACE_DECLARE(IAudioSystemImplementationModule, 0x5C4ADBECA34349CE, 0xB7992A856CDD553B);
};

//////////////////////////////////////////////////////////////////////////
struct IAudioSystem
{
	// <interfuscator:shuffle>
	virtual ~IAudioSystem() = default;

	virtual bool          Initialize() = 0;
	virtual void          Release() = 0;
	virtual void          PushRequest(SAudioRequest const& audioRequest) = 0;
	virtual void          AddRequestListener(void (* func)(SAudioRequestInfo const* const), void* const pObjectToListenTo, EAudioRequestType const requestType = eAudioRequestType_AudioAllRequests, AudioEnumFlagsType const specificRequestMask = ALL_AUDIO_REQUEST_SPECIFIC_TYPE_FLAGS) = 0;
	virtual void          RemoveRequestListener(void (* func)(SAudioRequestInfo const* const), void* const pObjectToListenTo) = 0;
	virtual void          ExternalUpdate() = 0;
	virtual bool          GetAudioTriggerId(char const* const szAudioTriggerName, AudioControlId& audioTriggerId) const = 0;
	virtual bool          GetAudioRtpcId(char const* const szAudioRtpcName, AudioControlId& audioRtpcId) const = 0;
	virtual bool          GetAudioSwitchId(char const* const szAudioSwitchName, AudioControlId& audioSwitchId) const = 0;
	virtual bool          GetAudioSwitchStateId(AudioControlId const audioSwitchId, char const* const szSwitchStateName, AudioSwitchStateId& audioSwitchStateId) const = 0;
	virtual bool          GetAudioPreloadRequestId(char const* const szAudioPreloadRequestName, AudioPreloadRequestId& audioPreloadRequestId) const = 0;
	virtual bool          GetAudioEnvironmentId(char const* const szAudioEnvironmentName, AudioEnvironmentId& audioEnvironmentId) const = 0;
	virtual CATLListener* CreateAudioListener() = 0;
	virtual void          ReleaseAudioListener(CATLListener* pListener) = 0;
	virtual void          OnCVarChanged(ICVar* const pCvar) = 0;
	virtual char const*   GetConfigPath() const = 0;
	virtual IAudioProxy*  GetFreeAudioProxy() = 0;
	virtual void          GetAudioFileData(char const* const szFilename, SAudioFileData& audioFileData) = 0;
	virtual void          GetAudioTriggerData(AudioControlId const audioTriggerId, SAudioTriggerData& audioFileData) = 0;
	virtual void          SetAllowedThreadId(threadID id) = 0;
	// </interfuscator:shuffle>
};

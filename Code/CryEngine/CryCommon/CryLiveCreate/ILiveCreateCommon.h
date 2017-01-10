// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef _H_ILIVECREATECOMMON_H_
#define _H_ILIVECREATECOMMON_H_

#include <CrySystem/IEngineModule.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// LiveCreate should not be enabled or compiled in server or release mode.
#if (defined(DEDICATED_SERVER) || defined(_RELEASE)) && !defined(NO_LIVECREATE)
	#define NO_LIVECREATE
#else
	#if CRY_PLATFORM_DESKTOP
		#define LIVECREATE_FOR_PC
	#endif
#endif

struct ILiveCreateEngineModule : public IEngineModule
{
	CRYINTERFACE_DECLARE(ILiveCreateEngineModule, 0xB93B314C06B24660, 0xADF0C6A1CB3EAF26);
};

namespace LiveCreate
{
struct IHost;
struct IHostInfo;
struct IManager;
struct IPlatformHandler;
struct IPlatformHandlerFactory;

enum ELogMessageType
{
	eLogType_Normal,
	eLogType_Warning,
	eLogType_Error
};

struct IManagerListenerEx
{
	// <interfuscator:shuffle>
	virtual ~IManagerListenerEx(){};

	//! Manager was successfully connected to a remote host.
	virtual void OnHostConnected(IHostInfo* pHostInfo) {};

	//! Manager was disconnected from a remote host.
	virtual void OnHostDisconnected(IHostInfo* pHostInfo) {};

	//! Manager connection was confirmed and we are ready to send LiveCreate data.
	virtual void OnHostReady(IHostInfo* pHost) {};

	//! LiveCreate host is busy (probably loading the level).
	virtual void OnHostBusy(IHostInfo* pHost) {};

	//! Manager wide sending status flag has changed.
	virtual void OnSendingStatusChanged(bool bCanSend) {};

	//! Internal message logging.
	virtual void OnLogMessage(ELogMessageType aType, const char* pMessage) {};

	// </interfuscator:shuffle>
};
}

#endif

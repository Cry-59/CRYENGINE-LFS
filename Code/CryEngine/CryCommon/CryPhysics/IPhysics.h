// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.
#pragma once

#include <CrySystem/IEngineModule.h>

#if defined (_LIB)
#define CRYPHYSICS_API
#elif defined (PHYSICS_EXPORTS)
#define CRYPHYSICS_API DLL_EXPORT
#else
#define CRYPHYSICS_API DLL_IMPORT
#endif

#if !defined (_LIB)
extern "C" 
#endif
CRYPHYSICS_API IPhysicalWorld * CreatePhysicalWorld(struct ISystem* pLog);

//! IDs that can be used for foreign id.
enum EPhysicsForeignIds
{
	PHYS_FOREIGN_ID_TERRAIN                  = 0,
	PHYS_FOREIGN_ID_STATIC                   = 1,
	PHYS_FOREIGN_ID_ENTITY                   = 2,
	PHYS_FOREIGN_ID_FOLIAGE                  = 3,
	PHYS_FOREIGN_ID_ROPE                     = 4,
	PHYS_FOREIGN_ID_SOUND_OBSTRUCTION        = 5,
	PHYS_FOREIGN_ID_SOUND_PROXY_OBSTRUCTION  = 6,
	PHYS_FOREIGN_ID_SOUND_REVERB_OBSTRUCTION = 7,
	PHYS_FOREIGN_ID_WATERVOLUME              = 8,
	PHYS_FOREIGN_ID_BREAKABLE_GLASS          = 9,
	PHYS_FOREIGN_ID_BREAKABLE_GLASS_FRAGMENT = 10,
	PHYS_FOREIGN_ID_RIGID_PARTICLE           = 11,
	PHYS_FOREIGN_ID_RESERVED1                = 12,
	PHYS_FOREIGN_ID_RAGDOLL                  = 13,

	PHYS_FOREIGN_ID_USER                     = 100, //!< All user defined foreign ids should start from this enum.
};

struct IPhysicsEngineModule : public IEngineModule
{
	CRYINTERFACE_DECLARE(IPhysicsEngineModule, 0x0B6197F33C68, 0xA13408BB46801741);
};

#include <CryMemory/CrySizer.h>
#include <CryPhysics/physinterface.h>

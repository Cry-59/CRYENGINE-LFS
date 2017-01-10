#pragma once

#include "../Interface/IPhysicsSyncComponent.h"

//#include "PhysicalEntityServerHistory.h"

#include <IGameObject.h>

class CPhysicsSyncComponent final 
	: public IPhysicsSyncComponent
	, public IGameObjectProfileManager
{
	CRY_ENTITY_COMPONENT_CLASS(CPhysicsSyncComponent, IPhysicsSyncComponent, "Physics Network Sync", 0xFD4E4314B56347B2, 0x948217AF3897D686);

	virtual ~CPhysicsSyncComponent() {}

	// IEntityComponent
	virtual void Initialize() override;

	virtual void NetworkSerialize(TSerialize ser, uint32 aspect, uint8 profile, int flags) override;

	virtual	void ProcessEvent(SEntityEvent& event) override;
	virtual uint64 GetEventMask() const { return BIT64(ENTITY_EVENT_PREPHYSICSUPDATE) | BIT64(ENTITY_EVENT_PHYS_POSTSTEP) | BIT64(ENTITY_EVENT_ENABLE_PHYSICS) | BIT64(ENTITY_EVENT_PHYSICS_CHANGE_STATE); }
	// ~IEntityComponent

	// IPhysicsSyncComponent
	virtual void BecomeLocalUser() override 
	{ 
		//m_history.BecomeLocalUser();
		m_bIsLocalUser = true;
	}

	virtual void UpdateDefaultPhysicalEntityType() override;
	// ~IPhysicsSyncComponent

	// IGameObjectProfileManager
	virtual bool SetAspectProfile(EEntityAspects aspect, uint8 profile) override;
	virtual uint8 GetDefaultProfile(EEntityAspects aspect) override;
	// ~IGameObjectProfileManager

protected:
	const EEntityAspects GetNetAspect() const { return eEA_GameClientE; }

	void DelegateControlToClient(bool bDelegate);

protected:
	//CPhysicalEntityServerHistory m_history;

	pe_type m_defaultPhysicsType;
	pe_type m_currentPhysicsType;

	bool m_bRemoteEntityPhysicalized : 1;
	bool m_bIsLocalUser : 1;
};

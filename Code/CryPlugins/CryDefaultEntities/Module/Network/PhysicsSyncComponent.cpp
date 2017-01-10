#include "StdAfx.h"
#include "PhysicsSyncComponent.h"

#include <IGameObject.h>

CRYREGISTER_CLASS(CPhysicsSyncComponent);

const bool g_useCryPhysicsSynchronization = true;

void CPhysicsSyncComponent::Initialize()
{
	// Game objects are currently required for networking functionality
	IGameObject* pGameObject = gEnv->pGameFramework->GetIGameObjectSystem()->CreateGameObjectForEntity(GetEntityId());

	m_pEntity->Activate(true);
	pGameObject->EnablePrePhysicsUpdate(EPrePhysicsUpdate::ePPU_Always);
	m_pEntity->PrePhysicsActivate(true);

	m_pEntity->PhysicsNetSerializeEnable(g_useCryPhysicsSynchronization);

	UpdateDefaultPhysicalEntityType();

	// We need to physicalize before binding to network or the physics won't be synced.
	if (m_defaultPhysicsType == PE_NONE)
	{
		SEntityPhysicalizeParams physParams;
		physParams.type = PE_NONE;
		m_pEntity->Physicalize(physParams);
	}

	pGameObject->CaptureProfileManager(this);

	// Disable aspect delegation since the server is always in control of physics.
	pGameObject->EnableDelegatableAspect(eEA_Physics, false);
	pGameObject->SetAspectProfile(eEA_Physics, m_currentPhysicsType);

	pGameObject->ChangedNetworkState(GetNetAspect());

	m_bRemoteEntityPhysicalized = false;
	m_bIsLocalUser = false;
}

void CPhysicsSyncComponent::ProcessEvent(SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_PREPHYSICSUPDATE)
	{
		if (!g_useCryPhysicsSynchronization)
		{
			if (IPhysicalEntity* pPhysicalEntity = m_pEntity->GetPhysicalEntity())
			{
				//m_history.OnPreStep(*pPhysicalEntity);
			}
		}
	}
	else if (event.event == ENTITY_EVENT_PHYS_POSTSTEP)
	{
		// Only handle post step events on the main thread
		if (event.nParam[0] == 0)
		{
			return;
		}

		if (IPhysicalEntity* pPhysicalEntity = m_pEntity->GetPhysicalEntity())
		{
			/*if (!g_useCryPhysicsSynchronization)
			{
				m_history.OnPostStep(*pPhysicalEntity);
			}

			if (m_history.ShouldSendLocalSequenceToServer())
			{
				gEnv->pGameFramework->GetGameObject(GetEntityId())->ChangedNetworkState(GetNetAspect());
			}*/
		}
	}
	else if (event.event == ENTITY_EVENT_ENABLE_PHYSICS)
	{
		// This is the first point at which we can actually make sure the remote character exists and is physicalized.
		// Since this is tied to an actor we need to call it once on the first physicalization to make sure all future 
		// Control switches are handled properly.
		if (!m_bRemoteEntityPhysicalized && gEnv->bServer)
		{
			DelegateControlToClient(false);

			m_bRemoteEntityPhysicalized = true;
		}
	}
	else if (event.event == ENTITY_EVENT_PHYSICS_CHANGE_STATE)
	{
		pe_type currentPhysProfile = PE_NONE;

		if (IPhysicalEntity* pPhysicalEntity = GetEntity()->GetPhysicalEntity())
		{
			currentPhysProfile = pPhysicalEntity->GetType();
		}

		if (currentPhysProfile != m_currentPhysicsType)
		{
			IGameObject* pGameObject = gEnv->pGameFramework->GetGameObject(GetEntityId());

			pGameObject->SetAspectProfile(eEA_Physics, currentPhysProfile);

			m_currentPhysicsType = currentPhysProfile;
		}
	}
}

void CPhysicsSyncComponent::NetworkSerialize(TSerialize ser, uint32 aspect, uint8 profile, int flags)
{
	if (aspect == GetNetAspect())
	{
		//m_history.OnSerializeLocalSequence(ser);
	}
	else if (aspect == eEA_Physics)
	{
		auto remotePhysicsType = (pe_type)profile;

		if (remotePhysicsType != PE_NONE)
		{
			if (ser.IsReading())
			{
				if (!g_useCryPhysicsSynchronization)
				{
					//m_history.OnSerializeClientFromNetwork(ser);
				}
			}
			else
			{
				if (g_useCryPhysicsSynchronization)
				{
					if (m_pEntity->GetPhysicalEntity() == nullptr || m_pEntity->GetPhysicalEntity()->GetType() != remotePhysicsType)
					{
						gEnv->pPhysicalWorld->SerializeGarbageTypedSnapshot(ser, remotePhysicsType, 0);
						return;
					}
				}
				else
				{
					//m_history.OnSerializeClientFromNetwork(ser);
				}
			}

			if (g_useCryPhysicsSynchronization)
			{
				GetEntity()->PhysicsNetSerializeTyped(ser, remotePhysicsType, flags);
			}
		}
	}
}

void CPhysicsSyncComponent::DelegateControlToClient(bool bDelegate)
{
	if (!gEnv->bServer)
		return;

	INetContext* pNetContext = gEnv->pGameFramework->GetNetContext();

	//Disable / enable control of the remote actor. Dont touch the local client.
	if (pNetContext != nullptr && !m_bIsLocalUser)
	{
		IGameObject* pGameObject = gEnv->pGameFramework->GetGameObject(GetEntityId());
		
		auto pNetChannel = bDelegate ? gEnv->pGameFramework->GetNetChannel(pGameObject->GetChannelId()) : nullptr;

		pNetContext->DelegateAuthority(GetEntityId(), pNetChannel);
	}
}

bool CPhysicsSyncComponent::SetAspectProfile(EEntityAspects aspect, uint8 profile)
{
	//Switch the actors physics state to the correct profile.
	if (aspect == eEA_Physics)
	{
		if (m_currentPhysicsType != profile)
		{
			// Delegate control when not ragdoll or physicalized
			DelegateControlToClient(profile != PE_ARTICULATED && profile != PE_NONE);

			m_currentPhysicsType = (pe_type)profile;

			for (IListener* pListener : m_listeners)
			{
				pListener->OnServerPhysicalEntityTypeChanged(m_currentPhysicsType);
			}
		}

		return true;
	}

	return false;
}

void CPhysicsSyncComponent::UpdateDefaultPhysicalEntityType()
{
	if (IPhysicalEntity* pPhysicalEntity = m_pEntity->GetPhysicalEntity())
	{
		pe_params_flags flags;
		flags.flagsOR = pef_log_poststep;
		pPhysicalEntity->SetParams(&flags);

		m_defaultPhysicsType = m_currentPhysicsType = pPhysicalEntity->GetType();
	}
	else
	{
		m_defaultPhysicsType = m_currentPhysicsType = PE_NONE;
	}
}

uint8 CPhysicsSyncComponent::GetDefaultProfile(EEntityAspects aspect)
{
	return aspect == eEA_Physics ? PE_NONE: 0;
}
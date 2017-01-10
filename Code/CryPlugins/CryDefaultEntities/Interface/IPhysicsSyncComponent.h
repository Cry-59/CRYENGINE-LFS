#pragma once

#include <CryEntitySystem/IEntityComponent.h>

struct IPhysicsSyncComponent : public IEntityComponent
{
	CRY_ENTITY_COMPONENT_INTERFACE(IPhysicsSyncComponent, 0xE791D859883241C9, 0xAF7E9D51F5832F82)

	struct IListener
	{
		virtual void OnServerPhysicalEntityTypeChanged(pe_type newType) = 0;
	};

	// To be called when the physics object is used locally, in order to trigger movement prediction
	virtual void BecomeLocalUser() = 0;
	// Called when the default physics type of the entity is changed
	virtual void UpdateDefaultPhysicalEntityType() = 0;

	void AddListener(IListener& listener) { stl::push_back_unique(m_listeners, &listener); }
	void RemoveListener(IListener& listener) { stl::find_and_erase(m_listeners, &listener); }

protected:
	std::vector<IListener*> m_listeners;
};
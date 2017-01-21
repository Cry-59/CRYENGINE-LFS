#pragma once

#include "Helpers/DesignerEntityComponent.h"

#include <CrySerialization/Decorators/Resources.h>

////////////////////////////////////////////////////////
// Sample entity for creating a particle entity
////////////////////////////////////////////////////////
class CDefaultParticleEntity final
	: public CDesignerEntityComponent<IParticleEntityComponent>
	, public IEntityPropertyGroup
{
	CRY_ENTITY_COMPONENT_CLASS(CDefaultParticleEntity, IParticleEntityComponent, "Particle", 0x31B3EAD4C34442F7, 0xB794B33746D4232B);

	virtual ~CDefaultParticleEntity();

public:
	// CDesignerEntityComponent
	virtual IEntityPropertyGroup* GetPropertyGroup() final { return this; }

	virtual void OnResetState() override;

	virtual void SetLocalTransform(const Matrix34& tm) override;
	// ~CDesignerEntityComponent

	// IParticleEntityComponent
	virtual void SetParticleEffectName(cstr effectName) override;
	// ~IParticleEntityComponent

	// IEntityPropertyGroup
	virtual void SerializeProperties(Serialization::IArchive& archive) override
	{
		archive(m_bActive, "Active", "Active");
		archive(Serialization::ParticleName(m_particleEffectPath), "ParticleEffect", "ParticleEffect");

		if (archive.isInput() && m_pEntity != nullptr)
		{
			OnResetState();
		}
	}
	// ~IEntityPropertyGroup

protected:
	int m_particleSlot = -1;

	bool m_bActive = true;
	string m_particleEffectPath;
};

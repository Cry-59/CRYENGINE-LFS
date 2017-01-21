#include "StdAfx.h"
#include "ParticleEntity.h"

#include <Cry3DEngine/I3DEngine.h>
#include <CryParticleSystem/IParticlesPfx2.h>

class CParticleRegistrator
	: public IEntityRegistrator
{
	virtual void Register() override
	{
		if (gEnv->pEntitySystem->GetClassRegistry()->FindClass("ParticleEffect") != nullptr)
		{
			// Skip registration of default engine class if the game has overridden it
			CryLog("Skipping registration of default engine entity class ParticleEffect, overridden by game");
			return;
		}

		RegisterEntityWithDefaultComponent<CDefaultParticleEntity>("ParticleEffect", "Particles", "Particles.bmp");
	}
};

CParticleRegistrator g_particleRegistrator;

CRYREGISTER_CLASS(CDefaultParticleEntity);

CDefaultParticleEntity::~CDefaultParticleEntity()
{
	if (m_particleSlot != -1)
	{
		m_pEntity->FreeSlot(m_particleSlot);
	}
}

void CDefaultParticleEntity::SetParticleEffectName(cstr effectName)
{
	m_particleEffectPath = effectName;
	OnResetState();
}

void CDefaultParticleEntity::OnResetState()
{
	IEntity& entity = *GetEntity();

	// Check if we have to unload first
	if (m_particleSlot != -1)
	{
		entity.FreeSlot(m_particleSlot);
		m_particleSlot = -1;
	}

	// Check if the light is active
	if (!m_bActive)
		return;

	if (IParticleEffect* pEffect = gEnv->pParticleManager->FindEffect(m_particleEffectPath, "ParticleEntity"))
	{
		m_particleSlot = entity.LoadParticleEmitter(m_particleSlot, pEffect);
	}
}

void CDefaultParticleEntity::SetLocalTransform(const Matrix34& tm)
{
	if (m_particleSlot != -1)
	{
		GetEntity()->SetSlotLocalTM(m_particleSlot, tm);
	}
}
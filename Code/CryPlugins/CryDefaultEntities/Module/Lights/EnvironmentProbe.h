#pragma once

#include "Helpers/DesignerEntityComponent.h"

#include <CryRenderer/IRenderer.h>
#include <CryRenderer/IShader.h>

#include <CrySerialization/Math.h>
#include <CrySerialization/Decorators/Resources.h>

////////////////////////////////////////////////////////
// Sample entity for creating an environment probe
////////////////////////////////////////////////////////
class CEnvironmentProbeEntity final 
	: public CDesignerEntityComponent<>
	, public IEntityPropertyGroup
{
	CRY_ENTITY_COMPONENT_INTERFACE_AND_CLASS(CEnvironmentProbeEntity, "Environment Probe", 0x0D3D1840D239411E, 0x873814C56CCCEE2C);

	virtual ~CEnvironmentProbeEntity() {}

public:
	// CDesignerEntityComponent
	virtual IEntityPropertyGroup* GetPropertyGroup() final { return this; }

	virtual void OnResetState() override;

	virtual void SetLocalTransform(const Matrix34& tm) override;
	// ~CDesignerEntityComponent

	// IEntityPropertyGroup
	virtual void SerializeProperties(Serialization::IArchive& archive) override
	{
		archive(m_bActive, "Active", "Active");
		archive(m_light.m_ProbeExtents, "BoxSize", "BoxSize");

		if (archive.openBlock("Options", "Options"))
		{
			archive(m_bIgnoreVisAreas, "IgnoreVisAreas", "IgnoreVisAreas");
			archive(m_light.m_nSortPriority, "SortPriority", "SortPriority");
			archive(m_attentuationFalloffMax, "AttentuationFalloffMax", "AttentuationFalloffMax");

			archive.closeBlock();
		}

		if (archive.openBlock("OptionsAdvanced", "Advanced"))
		{
			archive(Serialization::TextureFilename(m_cubemapPath), "Cubemap", "Cubemap");

			archive.closeBlock();
		}

		if (archive.isInput() && m_pEntity != nullptr)
		{
			OnResetState();
		}
	}
	// ~IEntityPropertyGroup

protected:
	void GetCubemapTextures(const char* path, ITexture** pSpecular, ITexture** pDiffuse);

	// Specifies the entity geometry slot in which the light is loaded, -1 if not currently loaded
	int m_lightSlot = -1;

	// Light parameters, updated in the OnResetState function
	CDLight m_light;

	bool m_bActive = true;
	string m_cubemapPath;

	bool m_bIgnoreVisAreas = false;
	float m_attentuationFalloffMax = 1.f;
};

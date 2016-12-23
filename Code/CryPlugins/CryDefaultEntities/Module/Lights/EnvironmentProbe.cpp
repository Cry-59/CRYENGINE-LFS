#include "StdAfx.h"
#include "EnvironmentProbe.h"

class CProbeRegistrator : public IEntityRegistrator
{
	virtual void Register() override
	{
		if (gEnv->pEntitySystem->GetClassRegistry()->FindClass("EnvironmentLight") != nullptr)
		{
			// Skip registration of default engine class if the game has overridden it
			CryLog("Skipping registration of default engine entity class EnvironmentLight, overridden by game");
			return;
		}

		RegisterEntityWithDefaultComponent<CEnvironmentProbeEntity>("EnvironmentLight", "Lights", "Light.bmp");
	}
};

CProbeRegistrator g_probeRegistrator;

CRYREGISTER_CLASS(CEnvironmentProbeEntity);

void CEnvironmentProbeEntity::OnResetState()
{
	IEntity& entity = *GetEntity();

	// Check if we have to unload first
	if (m_lightSlot != -1)
	{
		entity.FreeSlot(m_lightSlot);
		m_lightSlot = -1;
	}

	if (!m_bActive)
		return;

	m_light.m_nLightStyle = 0;
	m_light.SetPosition(ZERO);
	m_light.m_fLightFrustumAngle = 45;
	m_light.m_Flags = DLF_POINT | DLF_THIS_AREA_ONLY | DLF_DEFERRED_CUBEMAPS;
	m_light.m_LensOpticsFrustumAngle = 255;

	m_light.m_fRadius = pow(m_light.m_ProbeExtents.GetLengthSquared(), 0.5f);

	m_light.SetLightColor(ColorF(1.f, 1.f, 1.f, 1.f));
	m_light.SetSpecularMult(1.f);

	m_light.m_fHDRDynamic = 0.f;

	if (m_bIgnoreVisAreas)
		m_light.m_Flags |= DLF_IGNORES_VISAREAS;

	/*if (GetPropertyBool(eProperty_BoxProject))
		m_light.m_Flags |= DLF_BOX_PROJECTED_CM;

	m_light.m_fBoxWidth = GetPropertyFloat(eProperty_BoxSizeX);
	m_light.m_fBoxLength = GetPropertyFloat(eProperty_BoxSizeY);
	m_light.m_fBoxHeight = GetPropertyFloat(eProperty_BoxSizeZ);*/

	m_light.SetFalloffMax(m_attentuationFalloffMax);

	m_light.m_fFogRadialLobe = 0.f;

	m_light.SetAnimSpeed(0.f);
	m_light.m_fProjectorNearPlane = 0.f;

	ITexture* pSpecularTexture, * pDiffuseTexture;
	GetCubemapTextures(m_cubemapPath, &pSpecularTexture, &pDiffuseTexture);

	m_light.SetSpecularCubemap(pSpecularTexture);
	m_light.SetDiffuseCubemap(pDiffuseTexture);

	if (!m_light.GetSpecularCubemap())
	{
		m_light.SetDiffuseCubemap(nullptr);
		m_light.SetSpecularCubemap(nullptr);

		m_light.m_Flags &= ~DLF_DEFERRED_CUBEMAPS;

		CRY_ASSERT_MESSAGE(false, "Failed to load specular cubemap for environment probe!");
		return;
	}
	else if (!m_light.GetDiffuseCubemap())
	{
		m_light.SetDiffuseCubemap(nullptr);
		m_light.SetSpecularCubemap(nullptr);

		m_light.m_Flags &= ~DLF_DEFERRED_CUBEMAPS;

		CRY_ASSERT_MESSAGE(false, "Failed to load diffuse cubemap for environment probe!");
		return;
	}

	// Acquire resources, as the cubemap will be releasing when it goes out of scope
	m_light.AcquireResources();

	// Load the light source into the entity
	m_lightSlot = entity.LoadLight(m_lightSlot, &m_light);
}

void CEnvironmentProbeEntity::SetLocalTransform(const Matrix34& tm)
{
	if (m_lightSlot != -1)
	{
		GetEntity()->SetSlotLocalTM(m_lightSlot, tm);
	}
}

void CEnvironmentProbeEntity::GetCubemapTextures(const char* path, ITexture** pSpecular, ITexture** pDiffuse)
{
	stack_string specularCubemap = path;

	stack_string sSpecularName(specularCubemap);
	int strIndex = sSpecularName.find("_diff");
	if (strIndex >= 0)
	{
		sSpecularName = sSpecularName.substr(0, strIndex) + sSpecularName.substr(strIndex + 5, sSpecularName.length());
		specularCubemap = sSpecularName.c_str();
	}

	char diffuseCubemap[ICryPak::g_nMaxPath];
	cry_sprintf(diffuseCubemap, "%s%s%s.%s", PathUtil::AddSlash(PathUtil::GetPathWithoutFilename(specularCubemap)).c_str(),
	          PathUtil::GetFileName(specularCubemap).c_str(), "_diff", PathUtil::GetExt(specularCubemap));

	// '\\' in filename causing texture duplication
	stack_string specularCubemapUnix = PathUtil::ToUnixPath(specularCubemap.c_str());
	stack_string diffuseCubemapUnix = PathUtil::ToUnixPath(diffuseCubemap);

	*pSpecular = gEnv->pRenderer->EF_LoadTexture(specularCubemapUnix, FT_DONT_STREAM);
	*pDiffuse = gEnv->pRenderer->EF_LoadTexture(diffuseCubemapUnix, FT_DONT_STREAM);
}

// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "AutomaticEntityComponent.h"

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>
#include <Schematyc/Entity/EntityUtils.h>
#include <Schematyc/Entity/EntityClasses.h>
#include <Schematyc/Types/ResourceTypes.h>

#include "AutoRegister.h"
#include "STDModules.h"

#include <CryExtension/ICryFactoryRegistry.h>
#include <CryExtension/ICryFactory.h>

#include <array>

namespace Schematyc
{
	CAutomaticEntityComponent::CAutomaticEntityComponent(const CryClassID& guid)
		: m_guid(guid)
		, m_pEntityComponent(nullptr)
	{
	}

	static bool SerializePropertiesWrapper(void* rawPointer, yasli::Archive& ar)
	{
		static_cast<IEntityPropertyGroup*>(rawPointer)->SerializeProperties(ar);
		return true;
	}

	bool CAutomaticEntityComponent::Init()
	{
		IEntity& entity = EntityUtils::GetEntity(*this);
		if (entity.GetComponentByTypeId(m_guid) == nullptr)
		{
			if (m_pEntityComponent = entity.AddComponent(m_guid, std::shared_ptr<IEntityComponent>(), false))
			{
				if (auto* pPropertyGroup = static_cast<IEntityPropertyGroup*>(CComponent::GetProperties()))
				{
					DynArray<char> propertyBuffer;
					gEnv->pSystem->GetArchiveHost()->SaveBinaryBuffer(propertyBuffer, Serialization::SStruct(yasli::TypeID::get<IEntityPropertyGroup>(), pPropertyGroup, sizeof(IEntityPropertyGroup), &SerializePropertiesWrapper));

					gEnv->pSystem->GetArchiveHost()->LoadBinaryBuffer(Serialization::SStruct(yasli::TypeID::get<IEntityPropertyGroup>(), m_pEntityComponent->GetPropertyGroup(), sizeof(IEntityPropertyGroup), &SerializePropertiesWrapper), propertyBuffer.data(), propertyBuffer.size());
				}
			}
		}

		return true;
	}

	void CAutomaticEntityComponent::Run(ESimulationMode simulationMode)
	{
		if (simulationMode == ESimulationMode::Preview)
		{
			IEntity& entity = EntityUtils::GetEntity(*this);
			entity.SendEvent(SEntityEvent(ENTITY_EVENT_RESET));
		}

		if (m_pEntityComponent != nullptr)
		{
			m_pEntityComponent->SetLocalTransform(CComponent::GetTransform().ToMatrix34());
		}
	}

	void CAutomaticEntityComponent::Register(IEnvRegistrar& registrar)
	{
		CEnvRegistrationScope scope = registrar.Scope(g_entityClassGUID);

		std::array<ICryFactory*, 50> factories;
		size_t numFactories = factories.size();

		gEnv->pSystem->GetCryFactoryRegistry()->IterateFactories(cryiidof<IEntityComponent>(), factories.data(), numFactories);

		for (auto it = factories.begin(), end = it + numFactories; it != end; ++it)
		{
			class CAutomaticEntityComponentFactory final : public CEnvComponent<CAutomaticEntityComponent>
			{
			public:
				inline CAutomaticEntityComponentFactory(const CryClassID& guid, const char* szName, const SSourceFileInfo& sourceFileInfo)
					: CEnvComponent((const SGUID&)guid, szName, sourceFileInfo)
					, m_guid(guid)
				{}

				virtual CComponentPtr Create() const override
				{
					return std::allocate_shared<CAutomaticEntityComponent>(m_allocator, m_guid);
				}

			protected:
				CryClassID m_guid;
			};

			class CAutomaticEntityProperties : public IProperties
			{
			public:
				CAutomaticEntityProperties(std::shared_ptr<IEntityComponent>& pComponent)
					: m_pDummyComponent(pComponent)
				{
				}

				// IProperties
				virtual void* GetValue() override
				{
					return m_pDummyComponent->GetPropertyGroup();
				}

				virtual const void* GetValue() const override
				{
					return m_pDummyComponent->GetPropertyGroup();
				}

				virtual IPropertiesPtr Clone() const override
				{
					return std::make_shared<CAutomaticEntityProperties>(*this);
				}

				virtual void Serialize(Serialization::IArchive& archive) override
				{
					m_pDummyComponent->GetPropertyGroup()->SerializeProperties(archive);
				}
				// IProperties

			protected:
				std::shared_ptr<IEntityComponent> m_pDummyComponent;
			};

			if ((*it)->GetClassID() == cryiidof<CEntityObjectComponent>())
			{
				continue;
			}

			std::shared_ptr<IEntityComponent> pTempInstance = std::static_pointer_cast<IEntityComponent>((*it)->CreateClassInstance());
			if (pTempInstance == nullptr)
			{
				continue;
			}

			auto pComponent = std::make_shared<CAutomaticEntityComponentFactory>((*it)->GetClassID(), (*it)->GetName(), SCHEMATYC_SOURCE_FILE_INFO);
			pComponent->SetAuthor(g_szCrytek);

			if (IEntityPropertyGroup* pProperties = pTempInstance->GetPropertyGroup())
			{
				pComponent->SetPropertiesPointer(std::make_shared<CAutomaticEntityProperties>(pTempInstance));
			}

			//pComponent->SetDescription("Automatically exposed entity component"); // TODO: Add descriptions to registered entity components
			//pComponent->SetIcon("icons:schematyc/entity_audio_component.ico"); // TODO: Add icons to entity components
			pComponent->SetFlags({ Schematyc::EEnvComponentFlags::Transform, Schematyc::EEnvComponentFlags::Attach });
			scope.Register(pComponent);

			CEnvRegistrationScope componentScope = registrar.Scope(pComponent->GetGUID());
		}
	}

	SCHEMATYC_AUTO_REGISTER(&Schematyc::CAutomaticEntityComponent::Register)
}
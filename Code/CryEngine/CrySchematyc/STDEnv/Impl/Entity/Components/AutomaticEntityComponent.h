// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <Schematyc/Component.h>
#include <Schematyc/Types/MathTypes.h>
#include <CrySerialization/Forward.h>

namespace Schematyc
{
	// Forward declare interfaces.
	struct IEnvRegistrar;

	class CAutomaticEntityComponent final : public CComponent
	{
	public:
		CAutomaticEntityComponent() {}
		CAutomaticEntityComponent(const CryClassID& guid);

		// CComponent
		virtual bool Init() override;
		virtual void Run(ESimulationMode simulationMode) override;
		// ~CComponent

		static void Register(IEnvRegistrar& registrar);

	private:
		CryClassID m_guid;
	};
} // Schematyc

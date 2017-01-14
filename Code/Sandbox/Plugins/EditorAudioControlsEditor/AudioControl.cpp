// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "AudioControl.h"
#include "ATLControlsModel.h"
#include "IEditor.h"
#include <IAudioSystemItem.h>
#include <ACETypes.h>
#include "AudioControlsEditorPlugin.h"
#include "ImplementationManager.h"
#include <CrySerialization/StringList.h>
#include <CrySerialization/Decorators/Range.h>
#include <CrySerialization/Decorators/ActionButton.h>
#include <Serialization/Decorators/ToggleButton.h>
#include <CryMath/Cry_Geo.h>
#include <Util/Math.h>
#include <ConfigurationManager.h>

namespace ACE
{
CATLControl::CATLControl()
	: m_id(ACE_INVALID_ID)
	, m_type(eACEControlType_RTPC)
	, m_scope(0)
	, m_pParent(nullptr)
	, m_radius(0.0f)
	, m_occlusionFadeOutDistance(0.0f)
	, m_bAutoLoad(true)
	, m_bModified(false)
	, m_pModel(nullptr)
	, m_modifiedSignalEnabled(true)
	, m_bMatchRadiusAndAttenuation(true)
{
}

CATLControl::CATLControl(const string& controlName, CID id, EACEControlType eType, CATLControlsModel* pModel)
	: m_id(id)
	, m_type(eType)
	, m_pParent(nullptr)
	, m_radius(0.0f)
	, m_occlusionFadeOutDistance(0.0f)
	, m_bAutoLoad(true)
	, m_bModified(false)
	, m_pModel(pModel)
	, m_bMatchRadiusAndAttenuation(true)
{
	m_modifiedSignalEnabled = false;
	SetName(controlName);
	m_scope = pModel->GetGlobalScope();
	m_modifiedSignalEnabled = true;
}

CATLControl::~CATLControl()
{
	m_connectedControls.clear();
}

CID CATLControl::GetId() const
{
	return m_id;
}

EACEControlType CATLControl::GetType() const
{
	return m_type;
}

string CATLControl::GetName() const
{
	return m_name;
}

void CATLControl::SetName(const string& name)
{
	if (name != m_name)
	{
		SignalControlAboutToBeModified();
		m_name = m_pModel->GenerateUniqueName(this, name);
		SignalControlModified();
	}
}

Scope CATLControl::GetScope() const
{
	return m_scope;
}

void CATLControl::SetScope(Scope scope)
{
	if (m_scope != scope)
	{
		if (m_pModel->IsChangeValid(this, GetName(), scope))
		{
			SignalControlAboutToBeModified();
			m_scope = scope;
			SignalControlModified();
		}
	}
}

bool CATLControl::IsAutoLoad() const
{
	return m_bAutoLoad;
}

void CATLControl::SetAutoLoad(bool bAutoLoad)
{
	if (bAutoLoad != m_bAutoLoad)
	{
		SignalControlAboutToBeModified();
		m_bAutoLoad = bAutoLoad;
		SignalControlModified();
	}
}

size_t CATLControl::GetConnectionCount()
{
	return m_connectedControls.size();
}

ConnectionPtr CATLControl::GetConnectionAt(int index)
{
	if (index < m_connectedControls.size())
	{
		return m_connectedControls[index];
	}
	return nullptr;
}

ConnectionPtr CATLControl::GetConnection(CID id)
{
	if (id >= 0)
	{
		const size_t size = m_connectedControls.size();
		for (int i = 0; i < size; ++i)
		{
			ConnectionPtr pConnection = m_connectedControls[i];
			if (pConnection && pConnection->GetID() == id)
			{
				return pConnection;
			}
		}
	}
	return nullptr;
}

ConnectionPtr CATLControl::GetConnection(IAudioSystemItem* pAudioSystemControl)
{
	return GetConnection(pAudioSystemControl->GetId());
}

void CATLControl::AddConnection(ConnectionPtr pConnection)
{
	if (pConnection)
	{
		m_connectedControls.push_back(pConnection);

		if (m_bMatchRadiusAndAttenuation)
		{
			MatchRadiusToAttenuation();
		}

		pConnection->signalConnectionChanged.Connect(this, &CATLControl::SignalControlModified);

		IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
		if (pAudioSystemImpl)
		{
			IAudioSystemItem* pAudioSystemControl = pAudioSystemImpl->GetControl(pConnection->GetID());
			if (pAudioSystemControl)
			{
				pAudioSystemImpl->EnableConnection(pConnection);
				SignalConnectionAdded(pAudioSystemControl);
			}
		}

		SignalControlModified();
	}
}

void CATLControl::RemoveConnection(ConnectionPtr pConnection)
{
	if (pConnection)
	{
		auto it = std::find(m_connectedControls.begin(), m_connectedControls.end(), pConnection);
		if (it != m_connectedControls.end())
		{
			IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
			if (pAudioSystemImpl)
			{
				IAudioSystemItem* pAudioSystemControl = pAudioSystemImpl->GetControl(pConnection->GetID());
				if (pAudioSystemControl)
				{

					pAudioSystemImpl->DisableConnection(pConnection);
					m_connectedControls.erase(it);

					if (m_bMatchRadiusAndAttenuation)
					{
						MatchRadiusToAttenuation();
					}

					SignalConnectionRemoved(pAudioSystemControl);
					SignalControlModified();

				}
			}
		}
	}
}

void CATLControl::ClearConnections()
{
	if (!m_connectedControls.empty())
	{
		IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
		if (pAudioSystemImpl)
		{
			for (ConnectionPtr& connection : m_connectedControls)
			{
				pAudioSystemImpl->DisableConnection(connection);
				IAudioSystemItem* pAudioSystemControl = pAudioSystemImpl->GetControl(connection->GetID());
				if (pAudioSystemControl)
				{
					SignalConnectionRemoved(pAudioSystemControl);
				}
			}
		}
		m_connectedControls.clear();

		if (m_bMatchRadiusAndAttenuation)
		{
			MatchRadiusToAttenuation();
		}
		SignalControlModified();
	}
}

void CATLControl::RemoveConnection(IAudioSystemItem* pAudioSystemControl)
{
	if (pAudioSystemControl)
	{
		const CID id = pAudioSystemControl->GetId();
		auto it = m_connectedControls.begin();
		auto end = m_connectedControls.end();
		for (; it != end; ++it)
		{
			if ((*it)->GetID() == id)
			{
				IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
				if (pAudioSystemImpl)
				{
					pAudioSystemImpl->DisableConnection(*it);
				}
				m_connectedControls.erase(it);

				if (m_bMatchRadiusAndAttenuation)
				{
					MatchRadiusToAttenuation();
				}

				SignalConnectionRemoved(pAudioSystemControl);
				SignalControlModified();

				return;
			}
		}
	}
}

void CATLControl::SignalControlModified()
{
	if (m_modifiedSignalEnabled && m_pModel)
	{
		m_pModel->OnControlModified(this);
	}
}

void CATLControl::SignalControlAboutToBeModified()
{
	if (m_modifiedSignalEnabled && m_pModel)
	{
		m_pModel->OnControlAboutToBeModified(this);
	}
}

void CATLControl::SignalConnectionAdded(IAudioSystemItem* pMiddlewareControl)
{
	if (m_pModel)
	{
		m_pModel->OnConnectionAdded(this, pMiddlewareControl);
	}
}

void CATLControl::SignalConnectionRemoved(IAudioSystemItem* pMiddlewareControl)
{
	if (m_pModel)
	{
		m_pModel->OnConnectionRemoved(this, pMiddlewareControl);
	}
}

void CATLControl::ReloadConnections()
{
	IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
	if (pAudioSystemImpl)
	{
		std::map<int, XMLNodeList> connectionNodes;
		std::swap(connectionNodes, m_connectionNodes);
		for (auto& connectionPair : connectionNodes)
		{
			for (auto& connection : connectionPair.second)
			{
				LoadConnectionFromXML(connection.xmlNode, connectionPair.first);
			}
		}
	}
}

void CATLControl::LoadConnectionFromXML(XmlNodeRef xmlNode, int platformIndex)
{
	IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
	if (pAudioSystemImpl)
	{
		ConnectionPtr pConnection = pAudioSystemImpl->CreateConnectionFromXMLNode(xmlNode, GetType());
		if (pConnection)
		{
			if (GetType() == eACEControlType_Preload)
			{
				// The connection could already exist but using a different platform
				ConnectionPtr pPreviousConnection = GetConnection(pConnection->GetID());
				if (pPreviousConnection == nullptr)
				{
					if (platformIndex != -1)
					{
						pConnection->ClearPlatforms();
					}
					AddConnection(pConnection);
				}
				else
				{
					pConnection = pPreviousConnection;
				}
				if (platformIndex != -1)
				{
					pConnection->EnableForPlatform(platformIndex, true);
				}
			}
			else
			{
				AddConnection(pConnection);
			}
		}
		else if (GetType() == eACEControlType_Preload && platformIndex == -1)
		{
			// If it's a preload connection from another middleware and the platform
			// wasn't found (old file format) fall back to adding them to all the platforms
			const std::vector<dll_string>& platforms = GetIEditor()->GetConfigurationManager()->GetPlatformNames();
			const size_t count = platforms.size();
			for (size_t i = 0; i < count; ++i)
			{
				AddRawXMLConnection(xmlNode, false, i);
			}
			return;
		}

		AddRawXMLConnection(xmlNode, pConnection != nullptr, platformIndex);
	}
}

void CATLControl::Serialize(Serialization::IArchive& ar)
{
	if (ar.openBlock("properties", "+Properties"))
	{
		// Name
		string newName = m_name;
		ar(newName, "name", "Name");

		// Scope
		Serialization::StringList scopeList;
		ScopeInfoList scopeInfoList;
		m_pModel->GetScopeInfoList(scopeInfoList);
		for (auto& scope : scopeInfoList)
		{
			scopeList.push_back(scope.name);
		}
		Serialization::StringListValue selectedScope(scopeList, m_pModel->GetScopeInfo(m_scope).name);
		Scope newScope = m_scope;
		if (m_type != eACEControlType_State)
		{
			ar(selectedScope, "scope", "Scope");
			newScope = m_pModel->GetScope(scopeList[selectedScope.index()]);
		}

		// Auto Load
		bool bAutoLoad = m_bAutoLoad;
		if (m_type == eACEControlType_Preload)
		{
			ar(bAutoLoad, "auto_load", "Auto Load");
		}

		// Max Radius
		float maxRadius = m_radius;
		float fadeOutDistance = m_occlusionFadeOutDistance;
		if (m_type == eACEControlType_Trigger)
		{

			if (ar.openBlock("activity_radius", "Activity Radius"))
			{
				bool hasPlaceholderConnections = false;
				float attenuationRadius = 0.0f;
				IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
				if (pAudioSystemImpl)
				{
					float radius = 0.0f;
					for (auto pConnection : m_connectedControls)
					{
						IAudioSystemItem* pItem = pAudioSystemImpl->GetControl(pConnection->GetID());
						if (pItem && !pItem->IsPlaceholder())
						{
							attenuationRadius = std::max(attenuationRadius, pItem->GetRadius());
						}
						else
						{
							// If control has placeholder connection we cannot enforce the link between activity radius
							// and attenuation as the user could be missing the middleware project
							hasPlaceholderConnections = true;
						}
					}
				}

				if (m_bMatchRadiusAndAttenuation && !hasPlaceholderConnections)
				{
					ar(Serialization::Range<float>(maxRadius, 0.0f, std::numeric_limits<float>::max()), "max_radius", "!^");
				}
				else
				{
					ar(Serialization::Range<float>(maxRadius, 0.0f, std::numeric_limits<float>::max()), "max_radius", "^");
				}

				if (!hasPlaceholderConnections)
				{
					if (ar.openBlock("attenuation", "Attenuation"))
					{
						ar(attenuationRadius, "attenuation", "!^");
						ar(Serialization::ToggleButton(m_bMatchRadiusAndAttenuation, "icons:Navigation/Tools_Link.ico", "icons:Navigation/Tools_Link_Unlink.ico"), "link", "^");
						if (m_bMatchRadiusAndAttenuation)
						{
							maxRadius = attenuationRadius;
						}

						ar.closeBlock();
					}
				}
				ar(Serialization::Range<float>(fadeOutDistance, 0.0f, maxRadius), "fadeOutDistance", "Occlusion Fade-Out Distance");
			}
		}

		if (ar.isInput())
		{
			SignalControlAboutToBeModified();
			m_modifiedSignalEnabled = false; // we are manually sending the signals and don't want the other SetX functions to send anymore
			SetName(newName);
			SetScope(newScope);
			SetAutoLoad(bAutoLoad);
			SetRadius(maxRadius);
			SetOcclusionFadeOutDistance(fadeOutDistance);
			m_modifiedSignalEnabled = true;
			SignalControlModified();
		}
	}

}

void CATLControl::SetParent(CATLControl* pParent)
{
	m_pParent = pParent;
	if (pParent)
	{
		SetScope(pParent->GetScope());
	}
}
void CATLControl::SetRadius(float radius)
{
	if (radius != m_radius)
	{
		SignalControlAboutToBeModified();
		m_radius = radius;
		SignalControlModified();
	}
}

void CATLControl::SetOcclusionFadeOutDistance(float fadeOutDistance)
{
	if (fadeOutDistance != m_occlusionFadeOutDistance)
	{
		SignalControlAboutToBeModified();
		m_occlusionFadeOutDistance = fadeOutDistance;
		SignalControlModified();
	}
}

void CATLControl::AddRawXMLConnection(XmlNodeRef xmlNode, bool bValid, int platformIndex /*= -1*/)
{
	m_connectionNodes[platformIndex].push_back(SRawConnectionData(xmlNode, bValid));
}

XMLNodeList& CATLControl::GetRawXMLConnections(int platformIndex /*= -1*/)
{
	return m_connectionNodes[platformIndex];
}

void CATLControl::MatchRadiusToAttenuation()
{
	IAudioSystemEditor* pAudioSystemImpl = CAudioControlsEditorPlugin::GetImplementationManger()->GetImplementation();
	if (pAudioSystemImpl)
	{
		float radius = 0.0f;
		for (auto pConnection : m_connectedControls)
		{
			IAudioSystemItem* pItem = pAudioSystemImpl->GetControl(pConnection->GetID());
			if (pItem)
			{
				if (pItem->IsPlaceholder())
				{
					// We don't match controls that have placeholder
					// connections as we don't know what the real values should be
					return;
				}
				else
				{
					radius = std::max(radius, pItem->GetRadius());
				}
			}
		}

		SetRadius(radius);
	}
}

}

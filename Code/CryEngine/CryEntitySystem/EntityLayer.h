// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*************************************************************************
   -------------------------------------------------------------------------
   Description: Entity layer container.

   -------------------------------------------------------------------------
   History:
   - 11:2:2010   10:50 : Created by Sergiy Shaykin

*************************************************************************/
#ifndef __ENTITYLAYER_H__
#define __ENTITYLAYER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <CryCore/StlUtils.h>
#include <CryEntitySystem/IEntityLayer.h>

struct SEntityLayerGarbage
{
	SEntityLayerGarbage(IGeneralMemoryHeap* pHeap, const string& layerName)
		: pHeap(pHeap)
		, layerName(layerName)
		, nAge(0)
	{
	}

	IGeneralMemoryHeap* pHeap;
	string              layerName;
	int                 nAge;
};
class CEntityLayer;

//////////////////////////////////////////////////////////////////////////
// Structure for deferred layer activation/deactivation processing
// The operations are queued during serialization, then sorted to ensure deactivation
// happens prior to activation.
struct SPostSerializeLayerActivation
{
	typedef void (CEntityLayer::* ActivationFunc)(bool);
	CEntityLayer*  m_layer;
	ActivationFunc m_func;
	bool           enable;
};

typedef std::vector<SPostSerializeLayerActivation> TLayerActivationOpVec;

class CEntityLayer : public IEntityLayer
{
	struct EntityProp
	{
		EntityProp() : m_id(0), m_bIsNoAwake(false), m_bIsHidden(false)
		{
		}

		EntityProp(EntityId id, bool bIsNoAwake, bool bIsHidden)
			: m_id(id)
			, m_bIsNoAwake(bIsNoAwake)
			, m_bIsHidden(bIsHidden)
		{
		}

		bool operator==(const EntityProp& other) const
		{
			return (m_id == other.m_id);
		}

		EntityId m_id;
		bool     m_bIsNoAwake : 1;
		bool     m_bIsHidden  : 1;
	};

	struct EntityPropFindPred
	{
		EntityPropFindPred(EntityId _idToFind) : idToFind(_idToFind) {}
		bool operator()(const EntityProp& entityProp) { return entityProp.m_id == idToFind; }
		EntityId idToFind;
	};

public:
	typedef std::vector<SEntityLayerGarbage> TGarbageHeaps;

public:
	CEntityLayer(const char* name, uint16 id, bool havePhysics, int specs, bool defaultLoaded, TGarbageHeaps& garbageHeaps);
	virtual ~CEntityLayer();

	virtual void          SetParentName(const char* szParent) override { if (szParent) m_parentName = szParent; }
	virtual void          AddChild(IEntityLayer* pLayer) override      { return m_childs.push_back(static_cast<CEntityLayer*>(pLayer)); }
	virtual int           GetNumChildren() const override              { return m_childs.size(); }
	virtual CEntityLayer* GetChild(int idx) const override             { return m_childs[idx]; }
	virtual void          AddObject(EntityId id) override;
	virtual void          RemoveObject(EntityId id) override;
	virtual void          Enable(bool bEnable, bool bSerialize = true, bool bAllowRecursive = true) override;
	virtual bool          IsEnabled() const override                 { return (m_isEnabled | m_isEnabledBrush); }
	virtual bool          IsEnabledBrush() const override            { return m_isEnabledBrush; }
	virtual bool          IsSerialized() const override              { return m_isSerialized; }
	virtual bool          IsDefaultLoaded() const override           { return m_defaultLoaded; }
	virtual bool          IncludesEntity(EntityId id) const override { return m_entities.find(id) != m_entities.end(); }
	virtual const char*   GetName() const override                   { return m_name.c_str(); }
	virtual const char*   GetParentName() const override             { return m_parentName.c_str(); }
	virtual const uint16  GetId() const override                     { return m_id; }

	void                  GetMemoryUsage(ICrySizer* pSizer, int* pOutNumEntities);
	void                  Serialize(TSerialize ser, TLayerActivationOpVec& deferredOps);
	virtual bool          IsSkippedBySpec() const override;

private:

	void EnableBrushes(bool isEnable);
	void EnableEntities(bool isEnable);
	void ReEvalNeedForHeap();

private:
	typedef std::unordered_map<EntityId, EntityProp, stl::hash_uint32> TEntityProps;

	int                        m_specs;
	string                     m_name;
	string                     m_parentName;
	bool                       m_isEnabled;
	bool                       m_isEnabledBrush;
	bool                       m_isSerialized;
	bool                       m_havePhysics;
	bool                       m_defaultLoaded;
	bool                       m_wasReEnabled;
	uint16                     m_id;
	std::vector<CEntityLayer*> m_childs;
	TEntityProps               m_entities;

	TGarbageHeaps*             m_pGarbageHeaps;
	IGeneralMemoryHeap*        m_pHeap;
};

#endif //__ENTITYLAYER_H__

// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   Entity.h
//  Version:     v1.00
//  Created:     18/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once


#include <CryEntitySystem/IEntity.h>
#include <CryCore/Containers/CryListenerSet.h>

//////////////////////////////////////////////////////////////////////////
// This is the order in which the proxies are serialized
const static EEntityProxy ProxySerializationOrder[] = {
	ENTITY_PROXY_SCRIPT,
	ENTITY_PROXY_AUDIO,
	ENTITY_PROXY_AREA,
	ENTITY_PROXY_BOIDS,
	ENTITY_PROXY_BOID_OBJECT,
	ENTITY_PROXY_CAMERA,
	ENTITY_PROXY_FLOWGRAPH,
	ENTITY_PROXY_SUBSTITUTION,
	ENTITY_PROXY_TRIGGER,
	ENTITY_PROXY_USER,
	ENTITY_PROXY_ROPE,
	ENTITY_PROXY_ENTITYNODE,
	ENTITY_PROXY_CLIPVOLUME,
	ENTITY_PROXY_DYNAMICRESPONSE,
	ENTITY_PROXY_LAST
};

//////////////////////////////////////////////////////////////////////////
// These headers cannot be replaced with forward references.
// They are needed for correct up casting from IEntityComponent to real proxy class.
	#include "RenderProxy.h"
	#include "PhysicsProxy.h"
	#include "ScriptProxy.h"
	#include "SubstitutionProxy.h"
	#include "EntityComponentsVector.h"
//////////////////////////////////////////////////////////////////////////

// forward declarations.
struct IEntitySystem;
struct IEntityArchetype;
class CEntitySystem;
struct AIObjectParams;
struct SGridLocation;
struct SProximityElement;

// (MATT) This should really live in a minimal AI include, which right now we don't have  {2009/04/08}
	#ifndef INVALID_AIOBJECTID
typedef uint32 tAIObjectID;
		#define INVALID_AIOBJECTID ((tAIObjectID)(0))
	#endif

//////////////////////////////////////////////////////////////////////
class CEntity : public IEntity
{
	// Entity constructor.
	// Should only be called from Entity System.
	CEntity(SEntitySpawnParams& params);

public:
	// Entity destructor.
	// Should only be called from Entity System.
	virtual ~CEntity();

	IEntitySystem* GetEntitySystem() const { return g_pIEntitySystem; };

	// Called by entity system to complete initialization of the entity.
	bool Init(SEntitySpawnParams& params);
	// Called by EntitySystem every frame for each pre-physics active entity.
	void PrePhysicsUpdate(SEntityEvent& event);
	// Called by EntitySystem every frame for each active entity.
	void Update(SEntityUpdateContext& ctx);
	// Called by EntitySystem before entity is destroyed.
	void ShutDown();

	//////////////////////////////////////////////////////////////////////////
	// IEntity interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EntityId   GetId() const final   { return m_nID; }
	virtual EntityGUID GetGuid() const final { return m_guid; }

	//////////////////////////////////////////////////////////////////////////
	virtual IEntityClass*     GetClass() const final     { return m_pClass; }
	virtual IEntityArchetype* GetArchetype() const final { return m_pArchetype; }

	//////////////////////////////////////////////////////////////////////////
	virtual void   SetFlags(uint32 flags) final;
	virtual uint32 GetFlags() const final                      { return m_flags; }
	virtual void   AddFlags(uint32 flagsToAdd) final           { SetFlags(m_flags | flagsToAdd); }
	virtual void   ClearFlags(uint32 flagsToClear) final       { SetFlags(m_flags & (~flagsToClear)); }
	virtual bool   CheckFlags(uint32 flagsToCheck) const final { return (m_flags & flagsToCheck) == flagsToCheck; }

	virtual void   SetFlagsExtended(uint32 flags) final;
	virtual uint32 GetFlagsExtended() const final                            { return m_flagsExtended; }

	virtual bool   IsGarbage() const final                                   { return m_bGarbage; }
	virtual bool   IsLoadedFromLevelFile() const final                       { return m_bLoadedFromLevelFile; }
	ILINE void     SetLoadedFromLevelFile(const bool bIsLoadedFromLevelFile) { m_bLoadedFromLevelFile = bIsLoadedFromLevelFile; }

	//////////////////////////////////////////////////////////////////////////
	virtual void        SetName(const char* sName) final;
	virtual const char* GetName() const final { return m_szName.c_str(); }
	virtual string      GetEntityTextDescription() const final;

	//////////////////////////////////////////////////////////////////////////
	virtual void     AttachChild(IEntity* pChildEntity, const SChildAttachParams& attachParams) final;
	virtual void     DetachAll(int nDetachFlags = 0) final;
	virtual void     DetachThis(int nDetachFlags = 0, int nWhyFlags = 0) final;
	virtual int      GetChildCount() const final;
	virtual IEntity* GetChild(int nIndex) const final;
	virtual void     EnableInheritXForm(bool bEnable);
	virtual IEntity* GetParent() const final { return m_hierarchy.pParent; }
	virtual Matrix34 GetParentAttachPointWorldTM() const final;
	virtual bool     IsParentAttachmentValid() const final;
	virtual IEntity* GetAdam()
	{
		IEntity* pParent, * pAdam = this;
		while (pParent = pAdam->GetParent()) pAdam = pParent;
		return pAdam;
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void SetWorldTM(const Matrix34& tm, int nWhyFlags = 0) final;
	virtual void SetLocalTM(const Matrix34& tm, int nWhyFlags = 0) final;

	// Set position rotation and scale at once.
	virtual void            SetPosRotScale(const Vec3& vPos, const Quat& qRotation, const Vec3& vScale, int nWhyFlags = 0) final;

	virtual Matrix34        GetLocalTM() const final;
	virtual const Matrix34& GetWorldTM() const final { return m_worldTM; }

	virtual void            GetWorldBounds(AABB& bbox) const final;
	virtual void            GetLocalBounds(AABB& bbox) const final;
	virtual void            SetLocalBounds(const AABB& bounds, bool bDoNotRecalculate) final;
	virtual void            InvalidateLocalBounds() final;

	//////////////////////////////////////////////////////////////////////////
	virtual void        SetPos(const Vec3& vPos, int nWhyFlags = 0, bool bRecalcPhyBounds = false, bool bForce = false) final;
	virtual const Vec3& GetPos() const final { return m_vPos; }

	virtual void        SetRotation(const Quat& qRotation, int nWhyFlags = 0) final;
	virtual const Quat& GetRotation() const final { return m_qRotation; }

	virtual void        SetScale(const Vec3& vScale, int nWhyFlags = 0) final;
	virtual const Vec3& GetScale() const final    { return m_vScale; }

	virtual Vec3        GetWorldPos() const final { return m_worldTM.GetTranslation(); }
	virtual Ang3        GetWorldAngles() const final;
	virtual Quat        GetWorldRotation() const final;
	virtual const Vec3& GetForwardDir() const final { ComputeForwardDir(); return m_vForwardDir; }
	//////////////////////////////////////////////////////////////////////////

	virtual void Activate(bool bActive) final;
	virtual bool IsActive() const final { return m_bActive; }

	virtual void PrePhysicsActivate(bool bActive) final;
	virtual bool IsPrePhysicsActive() final;

	//////////////////////////////////////////////////////////////////////////
	virtual void Serialize(TSerialize ser, int nFlags) final;

	virtual bool SendEvent(SEntityEvent& event) final;
	//////////////////////////////////////////////////////////////////////////

	virtual void SetTimer(int nTimerId, int nMilliSeconds) final;
	virtual void KillTimer(int nTimerId) final;

	virtual void Hide(bool bHide) final;
	virtual bool IsHidden() const final { return m_bHidden; }

	virtual void Invisible(bool bInvisible) final;
	virtual bool IsInvisible() const final { return m_bInvisible; }

	//////////////////////////////////////////////////////////////////////////
	virtual IAIObject*  GetAI() final                       { return (m_aiObjectID ? GetAIObject() : NULL); }
	virtual bool        HasAI() const final                 { return m_aiObjectID != 0; }
	virtual tAIObjectID GetAIObjectID() const final         { return m_aiObjectID; }
	virtual void        SetAIObjectID(tAIObjectID id) final { m_aiObjectID = id; }
	//////////////////////////////////////////////////////////////////////////
	virtual bool        RegisterInAISystem(const AIObjectParams& params) final;
	//////////////////////////////////////////////////////////////////////////
	// reflect changes in position or orientation to the AI object
	void UpdateAIObject();
	//////////////////////////////////////////////////////////////////////////

	virtual void                SetUpdatePolicy(EEntityUpdatePolicy eUpdatePolicy) final;
	virtual EEntityUpdatePolicy GetUpdatePolicy() const final { return (EEntityUpdatePolicy)m_eUpdatePolicy; }

	//////////////////////////////////////////////////////////////////////////
	// Entity Proxies Interfaces access functions.
	//////////////////////////////////////////////////////////////////////////
	virtual IEntityComponent* GetProxy(EEntityProxy proxy) const final;
	virtual IEntityComponent* CreateProxy(EEntityProxy proxy) final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	virtual IEntityComponent* AddComponent(CryInterfaceID typeId, std::shared_ptr<IEntityComponent> pComponent,bool bAllowDuplicate) final;
	virtual void              RemoveComponent(IEntityComponent* pComponent) final;
	virtual IEntityComponent* GetComponentByTypeId(const CryInterfaceID& interfaceID) const final;

	virtual void CloneComponentsFrom(IEntity& otherEntity) final;

	//////////////////////////////////////////////////////////////////////////
	// Physics.
	//////////////////////////////////////////////////////////////////////////
	virtual void             Physicalize(SEntityPhysicalizeParams& params) final;
	virtual void             AssignPhysicalEntity(IPhysicalEntity* pPhysEntity, int nSlot = -1) final;
	virtual void             EnablePhysics(bool enable) final;
	virtual bool             IsPhysicsEnabled() const final;
	virtual IPhysicalEntity* GetPhysics() const final;

	virtual int              PhysicalizeSlot(int slot, SEntityPhysicalizeParams& params) final;
	virtual void             UnphysicalizeSlot(int slot) final;
	virtual void             UpdateSlotPhysics(int slot) final;

	virtual void             SetPhysicsState(XmlNodeRef& physicsState) final;
	virtual void             GetPhysicsWorldBounds(AABB& bounds) const final;

	virtual void             AddImpulse(int ipart, const Vec3& pos, const Vec3& impulse, bool bPos, float fAuxScale, float fPushScale = 1.0f) final;

	virtual bool             PhysicalizeFoliage(int iSlot) final;
	virtual void             DephysicalizeFoliage(int iSlot) final;

	//! retrieve starting partid for a given slot.
	virtual int  GetPhysicalEntityPartId0(int islot = 0) final;

	virtual void PhysicsNetSerializeEnable(bool enable) final;
	virtual void PhysicsNetSerializeTyped(TSerialize ser, int type, int flags) final;
	virtual void PhysicsNetSerialize(TSerialize ser) final;
	virtual void NetworkSerialize(TSerialize ser, uint32 aspect, uint8 profile, int flags) final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Custom entity material.
	//////////////////////////////////////////////////////////////////////////
	virtual void       SetMaterial(IMaterial* pMaterial) final;
	virtual IMaterial* GetMaterial() final;

	//////////////////////////////////////////////////////////////////////////
	// Entity Slots interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool                       IsSlotValid(int nSlot) const final;
	virtual int                        AllocateSlot() final;
	virtual void                       FreeSlot(int nSlot) final;
	virtual int                        GetSlotCount() const final;
	virtual void                       ClearSlots() final;
	virtual bool                       GetSlotInfo(int nSlot, SEntitySlotInfo& slotInfo) const final;
	virtual const Matrix34&            GetSlotWorldTM(int nIndex) const final;
	virtual const Matrix34&            GetSlotLocalTM(int nIndex, bool bRelativeToParent) const final;
	virtual void                       SetSlotLocalTM(int nIndex, const Matrix34& localTM, int nWhyFlags = 0) final;
	virtual void                       SetSlotCameraSpacePos(int nIndex, const Vec3& cameraSpacePos) final;
	virtual void                       GetSlotCameraSpacePos(int nSlot, Vec3& cameraSpacePos) const final;
	virtual bool                       SetParentSlot(int nParentSlot, int nChildSlot) final;
	virtual void                       SetSlotMaterial(int nSlot, IMaterial* pMaterial) final;
	virtual IMaterial*                 GetSlotMaterial(int nSlot) const final;
	virtual IMaterial*                 GetRenderMaterial(int nSlot = -1) const final;
	virtual void                       SetSlotFlags(int nSlot, uint32 nFlags) final;
	virtual uint32                     GetSlotFlags(int nSlot) const final;
	virtual bool                       ShouldUpdateCharacter(int nSlot) const final;
	virtual ICharacterInstance*        GetCharacter(int nSlot) final;
	virtual int                        SetCharacter(ICharacterInstance* pCharacter, int nSlot) final;
	virtual IStatObj*                  GetStatObj(int nSlot) final;
	virtual int                        SetStatObj(IStatObj* pStatObj, int nSlot, bool bUpdatePhysics, float mass = -1.0f) final;
	virtual IParticleEmitter*          GetParticleEmitter(int nSlot) final;
	virtual IGeomCacheRenderNode*      GetGeomCacheRenderNode(int nSlot) final;
	virtual IRenderNode*               GetRenderNode(int nSlot = -1) const final;
	virtual bool                       IsRendered() const final;
	virtual void                       PreviewRender( SPreviewRenderParams &params) final;

	virtual void                       MoveSlot(IEntity* targetIEnt, int nSlot) final;

	virtual int                        LoadGeometry(int nSlot, const char* sFilename, const char* sGeomName = NULL, int nLoadFlags = 0) final;
	virtual int                        LoadCharacter(int nSlot, const char* sFilename, int nLoadFlags = 0) final;
	#if defined(USE_GEOM_CACHES)
	virtual int                        LoadGeomCache(int nSlot, const char* sFilename) final;
	#endif
	virtual int                        SetParticleEmitter(int nSlot, IParticleEmitter* pEmitter, bool bSerialize = false) final;
	virtual int                        LoadParticleEmitter(int nSlot, IParticleEffect* pEffect, SpawnParams const* params = NULL, bool bPrime = false, bool bSerialize = false) final;
	virtual int                        LoadLight(int nSlot, CDLight* pLight) final;
	int                                LoadLightImpl(int nSlot, CDLight* pLight);

	virtual bool                       UpdateLightClipBounds(CDLight& light);
	virtual int                        LoadCloud(int nSlot, const char* sFilename) override;
	virtual int                        SetCloudMovementProperties(int nSlot, const SCloudMovementProperties& properties) override;
	int                                LoadCloudBlocker(int nSlot, const SCloudBlockerProperties& properties);
	int                                LoadFogVolume(int nSlot, const SFogVolumeProperties& properties);

	int                                FadeGlobalDensity(int nSlot, float fadeTime, float newGlobalDensity);
	int                                LoadVolumeObject(int nSlot, const char* sFilename);
	int                                SetVolumeObjectMovementProperties(int nSlot, const SVolumeObjectMovementProperties& properties);

	virtual void                       SetSubObjHideMask(int nSlot, hidemask nSubObjHideMask) final        { GetEntityRender()->SetSubObjHideMask(nSlot, nSubObjHideMask); };
	virtual hidemask                   GetSubObjHideMask(int nSlot) const final                            { return GetEntityRender()->GetSubObjHideMask(nSlot); };

	virtual void                       SetRenderNodeParams(const IEntity::SRenderNodeParams& params) final { GetEntityRender()->SetRenderNodeParams(params); };
	virtual IEntity::SRenderNodeParams GetRenderNodeParams() const final                                   { return GetEntityRender()->GetRenderNodeParams(); };

	virtual void                       InvalidateTM(int nWhyFlags = 0, bool bRecalcPhyBounds = false) final;

	// Inline implementation.
	virtual IScriptTable* GetScriptTable() const final;

	// Load/Save entity parameters in XML node.
	virtual void         SerializeXML(XmlNodeRef& node, bool bLoading, bool bIncludeScriptProxy) final;

	virtual void SerializeProperties(Serialization::IArchive& ar) final;

	virtual IEntityLink* GetEntityLinks() final;
	virtual IEntityLink* AddEntityLink(const char* sLinkName, EntityId entityId, EntityGUID entityGuid = 0) final;
	virtual void         RemoveEntityLink(IEntityLink* pLink) final;
	virtual void         RemoveAllEntityLinks() final;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	virtual IEntity* UnmapAttachedChild(int& partId) final;

	virtual void     GetMemoryUsage(ICrySizer* pSizer) const final;

	//////////////////////////////////////////////////////////////////////////
	// Local methods.
	//////////////////////////////////////////////////////////////////////////

	// Returns true if entity was already fully initialized by this point.
	virtual bool IsInitialized() const final { return m_bInitialized; }

	virtual void DebugDraw(const SGeometryDebugDrawInfo& info) final;
	//////////////////////////////////////////////////////////////////////////

	// Get fast access to the slot, only used internally by entity components.
	class CEntitySlot* GetSlot(int nSlot) const;

	// Get render proxy object.
	ILINE const CEntityRender* GetEntityRender() const { return static_cast<const CEntityRender*>(&m_render); }
	ILINE CEntityRender*       GetEntityRender()       { return static_cast<CEntityRender*>(&m_render); }

	// Get physics proxy object.
	ILINE const CEntityPhysics* GetPhysicalProxy() const { return static_cast<const CEntityPhysics*>(&m_physics); }
	ILINE CEntityPhysics*       GetPhysicalProxy()       { return static_cast<CEntityPhysics*>(&m_physics); }

	// Get script proxy object.
	ILINE CEntityComponentLuaScript* GetScriptProxy() const { return (CEntityComponentLuaScript*)CEntity::GetProxy(ENTITY_PROXY_SCRIPT); }
	//////////////////////////////////////////////////////////////////////////

	// For internal use.
	CEntitySystem* GetCEntitySystem() const { return g_pIEntitySystem; }

	//////////////////////////////////////////////////////////////////////////
	// Activates entity only for specified number of frames.
	// numUpdates must be a small number from 0-15.
	void ActivateForNumUpdates(int numUpdates);

	void ActivateEntityIfNecessary();
	bool ShouldActivate();

	//////////////////////////////////////////////////////////////////////////
	// Description:
	//   Invalidates entity and childs cached world transformation.
	void CalcLocalTM(Matrix34& tm) const;

	void Hide(bool bHide, EEntityEvent eEvent1, EEntityEvent eEvent2);

	// for ProximityTriggerSystem
	SProximityElement* GetProximityElement()         { return m_pProximityEntity; }

	virtual void       IncKeepAliveCounter() final   { m_nKeepAliveCounter++; }
	virtual void       DecKeepAliveCounter() final   { assert(m_nKeepAliveCounter > 0); m_nKeepAliveCounter--; }
	virtual void       ResetKeepAliveCounter() final { m_nKeepAliveCounter = 0; }
	virtual bool       IsKeptAlive() const final     { return m_nKeepAliveCounter > 0; }

	// LiveCreate entity interface
	virtual bool  HandleVariableChange(const char* szVarName, const void* pVarData) final;

	virtual void  OnRenderNodeVisibilityChange(bool bBecomeVisible) final;
	virtual float GetLastSeenTime() const final;

	#ifdef SEG_WORLD
	virtual unsigned int GetSwObjDebugFlag() const             { return m_eSwObjDebugFlag; };
	virtual void         SetSwObjDebugFlag(unsigned int uiVal) { m_eSwObjDebugFlag = uiVal; };

	virtual void         SetLocalSeg(bool bLocalSeg)           { m_bLocalSeg = bLocalSeg; }
	virtual bool         IsLocalSeg() const                    { return m_bLocalSeg; }
	#endif //SEG_WORLD

	void SetCloneLayerId(int cloneLayerId) { m_cloneLayerId = cloneLayerId; }
	int  GetCloneLayerId() const           { return m_cloneLayerId; }

	//////////////////////////////////////////////////////////////////////////
	void AddEntityEventListener(EEntityEvent event, IEntityEventListener* pListener);
	void RemoveEntityEventListener(EEntityEvent event, IEntityEventListener* pListener);
	void RemoveAllEventListeners();

	uint32 GetEditorObjectID() const final override;
	void   SetObjectID(uint32 ID) final override;
	void   GetEditorObjectInfo(bool& bSelected, bool& bHighlighted) const final override;
	void   SetEditorObjectInfo(bool bSelected, bool bHighlighted) final override;

protected:
	//////////////////////////////////////////////////////////////////////////
	// Attachment.
	//////////////////////////////////////////////////////////////////////////
	void OnRellocate(int nWhyFlags);
	void LogEvent(SEntityEvent& event, CTimeValue dt);
	//////////////////////////////////////////////////////////////////////////

private:
	void ComputeForwardDir() const;
	bool IsScaled(float threshold = 0.0003f) const
	{
		return (fabsf(m_vScale.x - 1.0f) + fabsf(m_vScale.y - 1.0f) + fabsf(m_vScale.z - 1.0f)) >= threshold;
	}
	// Fetch the IA object from the AIObjectID, if any
	IAIObject* GetAIObject();

private:
	friend class CEntitySystem;
	friend class CPhysicsEventListener; // For faster access to internals.
	friend class CEntitySlot;           // For faster access to internals.
	friend class CEntityRender;         // For faster access to internals.
	friend class CEntityComponentTriggerBounds;
	friend class CEntityPhysics;

	enum class EBindingType
	{
		eBT_Pivot,
		eBT_GeomCacheNode,
		eBT_CharacterBone,
	};

	// Childs structure, only allocated when any child entity is attached.
	struct STransformHierarchy
	{
		std::vector<CEntity*> childs;
		CEntity*              pParent = nullptr;
		EBindingType          parentBindingType = EBindingType::eBT_Pivot;
		int                   attachId = -1;
		attachMask            childrenAttachIds = 0; // bitmask of used attachIds of the children
	};

	struct SEventListeners
	{
		struct SListener
		{
			EEntityEvent          event;
			IEntityEventListener* pListener;
			SListener() : event(ENTITY_EVENT_LAST), pListener(nullptr) {}
			SListener(EEntityEvent e, IEntityEventListener* l) : event(e), pListener(l) {}
			SListener(IEntityEventListener* l) : event(ENTITY_EVENT_LAST), pListener(l) {}
			operator bool() const { return pListener != 0; }
			bool operator==(const SListener& l) const { return event == l.event && pListener == l.pListener; }
			bool operator!=(const SListener& l) const { return !(*this == l); }

		};
		SEventListeners(size_t reserve) : m_listeners(reserve), haveListenersMask(0) {}
		// Bit mask of the events where listener was registered
		// Clearing the bit is not required, it will only cause slightly lower performance of the send event
		uint64                  haveListenersMask;
		CListenerSet<SListener> m_listeners;
	};

private:
	//////////////////////////////////////////////////////////////////////////
	// Member variables
	//////////////////////////////////////////////////////////////////////////

	// Name of the entity.
	string m_szName;

	//////////////////////////////////////////////////////////////////////////
	// Internal entity flags (must be first member var of CEntity) (Reduce cache misses on access to entity data).
	//////////////////////////////////////////////////////////////////////////
	unsigned int         m_bActive        : 1;      // Active Entities are updated every frame.
	unsigned int         m_bInActiveList  : 1;      // Added to entity system active list.
	unsigned int         m_bRequiresComponentUpdate  : 1; // Whether or not any components require update callbacks
	mutable unsigned int m_bBoundsValid   : 1;      // Set when the entity bounding box is valid.
	unsigned int         m_bInitialized   : 1;      // Set if this entity already Initialized.
	unsigned int         m_bHidden        : 1;      // Set if this entity is hidden.
	unsigned int         m_bGarbage       : 1;      // Set if this entity is garbage and will be removed soon.

	unsigned int         m_bTrigger       : 1;      // Set if entity is proximity trigger itself.
	unsigned int         m_bWasRelocated  : 1;      // Set if entity was relocated at least once.
	unsigned int         m_nUpdateCounter : 4;      // Update counter is used to activate the entity for the limited number of frames.
	                                                // Usually used for Physical Triggers.
	                                                // When update counter is set, and entity is activated, it will automatically
	                                                // deactivate after this counter reaches zero
	unsigned int m_eUpdatePolicy        : 4;        // Update policy defines in which cases to call
	                                                // entity update function every frame.
	unsigned int m_bInvisible           : 1;        // Set if this entity is invisible.
	unsigned int m_bNotInheritXform     : 1;        // Inherit or not transformation from parent.
	unsigned int m_bInShutDown          : 1;        // Entity is being shut down.

	mutable bool m_bDirtyForwardDir     : 1;    // Cached world transformed forward vector
	unsigned int m_bLoadedFromLevelFile : 1;    // Entity was loaded from level file
	#ifdef SEG_WORLD
	unsigned int m_eSwObjDebugFlag      : 2;
	bool         m_bLocalSeg            : 1;
	#endif //SEG_WORLD

	// Unique ID of the entity.
	EntityId m_nID;

	// Optional entity guid.
	EntityGUID m_guid;

	// Entity flags. any combination of EEntityFlags values.
	uint32 m_flags;

	// Entity extended flags. any combination of EEntityFlagsExtended values.
	uint32 m_flagsExtended;

	// Pointer to the class that describe this entity.
	IEntityClass* m_pClass;

	// Pointer to the entity archetype.
	IEntityArchetype* m_pArchetype;

	// Position of the entity in local space.
	Vec3             m_vPos;
	// Rotation of the entity in local space.
	Quat             m_qRotation;
	// Scale of the entity in local space.
	Vec3             m_vScale;
	// World transformation matrix of this entity.
	mutable Matrix34 m_worldTM;

	mutable Vec3     m_vForwardDir;

	// Pointer to hierarchical binding structure.
	// It contains array of child entities and pointer to the parent entity.
	// Because entity attachments are not used very often most entities do not need it,
	// and space is preserved by keeping it separate structure.
	STransformHierarchy m_hierarchy;

	// The representation of this entity in the AI system.
	tAIObjectID m_aiObjectID;

	// Custom entity material.
	_smart_ptr<IMaterial> m_pMaterial;

	// Array of components, linear search in a small array is almost always faster then a more complicated set or map containers.
	CEntityComponentsVector m_components;

	// Entity Links.
	IEntityLink* m_pEntityLinks;

	// For tracking entity in the partition grid.
	SGridLocation*     m_pGridLocation;
	// For tracking entity inside proximity trigger system.
	SProximityElement* m_pProximityEntity;

	// counter to prevent deletion if entity is processed deferred by for example physics events
	uint32 m_nKeepAliveCounter;

	// If this entity is part of a layer that was cloned at runtime, this is the cloned layer
	// id (not related to the layer id)
	int                              m_cloneLayerId;

	std::unique_ptr<SEventListeners> m_pEventListeners;

	CEntityRender                    m_render;
	CEntityPhysics                   m_physics;
	uint32                           m_objectID;
};

// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryExtension/ICryFactoryRegistryImpl.h>
#include <CryExtension/ICryFactory.h>

#include <vector>

class CCryFactoryRegistryImpl : public ICryFactoryRegistryImpl
{
public:
	virtual ICryFactory* GetFactory(const CryClassID& cid) const;
	virtual void         IterateFactories(const CryInterfaceID& iid, ICryFactory** pFactories, size_t& numFactories) const;

	virtual void         RegisterCallback(ICryFactoryRegistryCallback* pCallback);
	virtual void         UnregisterCallback(ICryFactoryRegistryCallback* pCallback);

	virtual void         RegisterFactories(const SRegFactoryNode* pFactories);
	virtual void         UnregisterFactories(const SRegFactoryNode* pFactories);

	virtual void         UnregisterFactory(ICryFactory* const pFactory);

public:
	static CCryFactoryRegistryImpl& Access();

private:
	struct FactoryByCID
	{
		CryClassID   m_cid;
		ICryFactory* m_ptr;

		FactoryByCID(const CryClassID& cid) : m_cid(cid), m_ptr(0) {}
		FactoryByCID(ICryFactory* ptr) : m_cid(ptr ? ptr->GetClassID() : MAKE_CRYGUID(0, 0)), m_ptr(ptr) { assert(m_ptr); }
		bool operator<(const FactoryByCID& rhs) const { return m_cid < rhs.m_cid; }
	};
	typedef std::vector<FactoryByCID>      FactoriesByCID;
	typedef FactoriesByCID::iterator       FactoriesByCIDIt;
	typedef FactoriesByCID::const_iterator FactoriesByCIDConstIt;

	struct FactoryByIID
	{
		CryInterfaceID m_iid;
		ICryFactory*   m_ptr;

		FactoryByIID(CryInterfaceID iid, ICryFactory* pFactory) : m_iid(iid), m_ptr(pFactory) {}
		bool operator<(const FactoryByIID& rhs) const { if (m_iid != rhs.m_iid) return m_iid < rhs.m_iid; return m_ptr < rhs.m_ptr; }
	};
	typedef std::vector<FactoryByIID>      FactoriesByIID;
	typedef FactoriesByIID::iterator       FactoriesByIIDIt;
	typedef FactoriesByIID::const_iterator FactoriesByIIDConstIt;
	struct LessPredFactoryByIIDOnly
	{
		bool operator()(const FactoryByIID& lhs, const FactoryByIID& rhs) const { return lhs.m_iid < rhs.m_iid; }
	};

	typedef std::vector<ICryFactoryRegistryCallback*> Callbacks;
	typedef Callbacks::iterator                       CallbacksIt;
	typedef Callbacks::const_iterator                 CallbacksConstIt;

private:
	virtual ~CCryFactoryRegistryImpl() {}

	bool GetInsertionPos(ICryFactory* pFactory, FactoriesByCIDIt& itPosForCID);
	void UnregisterFactoryInternal(ICryFactory* const pFactory);

private:
	static CCryFactoryRegistryImpl s_registry;

private:
	mutable CryReadModifyLock m_guard;

	FactoriesByCID            m_byCID;
	FactoriesByIID            m_byIID;

	Callbacks                 m_callbacks;
};

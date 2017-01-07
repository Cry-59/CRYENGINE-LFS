// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#if !defined(AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_)
#define AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <CryCore/Project/CryModuleDefs.h>
#define eCryModule eCryM_Physics
#define PHYSICS_EXPORTS
#include <CryCore/Platform/platform.h>

//#define MEMSTREAM_DEBUG 1
#define MEMSTREAM_DEBUG_TAG (0xcafebabe)

#if defined(MEMSTREAM_DEBUG)
#define MEMSTREAM_DEBUG_ASSERT(x) if (!(x)) { __debugbreak(); }
#else
#define MEMSTREAM_DEBUG_ASSERT(x)
#endif

// Entity profiling only possible in non-release builds
#if !defined(_RELEASE)
# define ENTITY_PROFILER_ENABLED
# define PHYSWORLD_SERIALIZATION
#endif

#if CRY_COMPILER_MSVC
#pragma warning (disable : 4554 4305 4244 4996)
#pragma warning (disable : 6326) //Potential comparison of a constant with another constant
#elif CRY_COMPILER_CLANG
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // MSVC equivalent C4996
#elif CRY_COMPILER_GCC
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // MSVC equivalent C4996
#endif

// C6326: potential comparison of a constant with another constant
#define CONSTANT_COMPARISON_OK PREFAST_SUPPRESS_WARNING(6326)
// C6384: dividing sizeof a pointer by another value
#define SIZEOF_ARRAY_OK				 PREFAST_SUPPRESS_WARNING(6384)
// C6246: Local declaration of <variable> hides declaration of same name in outer scope.
#define LOCAL_NAME_OVERRIDE_OK PREFAST_SUPPRESS_WARNING(6246)
// C6201: buffer overrun for <variable>, which is possibly stack allocated: index <name> is out of valid index range <min> to <max>
#if defined(__clang__)
#define INDEX_NOT_OUT_OF_RANGE _Pragma("clang diagnostic ignored \"-Warray-bounds\"")
#else
#define INDEX_NOT_OUT_OF_RANGE PREFAST_SUPPRESS_WARNING(6201)
#endif
// C6385: invalid data: accessing <buffer name>, the readable size is <size1> bytes, but <size2> bytes may be read
#define NO_BUFFER_OVERRUN			 PREFAST_SUPPRESS_WARNING(6385 6386)
// C6255: _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
#define STACK_ALLOC_OK				 PREFAST_SUPPRESS_WARNING(6255)


#include <vector>
#include <map>
#include <algorithm>
#include <float.h>

#include <CryThreading/CryThread.h>

#include <CryMath/Cry_Math.h>
#include <CryMath/Cry_XOptimise.h>

#if CRY_PLATFORM_WINDOWS && CRY_PLATFORM_64BIT
#undef min
#undef max
#endif


#define ENTGRID_2LEVEL

// TODO: reference additional headers your program requires here
#include <CryMemory/CrySizer.h>
#include <CryPhysics/primitives.h>
#include "utils.h"
#include <CryPhysics/physinterface.h>


#if MAX_PHYS_THREADS<=1
extern threadID g_physThreadId;
inline int IsPhysThread() { return iszero((int)(CryGetCurrentThreadId()-g_physThreadId)); }
inline void MarkAsPhysThread() { g_physThreadId = CryGetCurrentThreadId(); }
inline void MarkAsPhysWorkerThread(int*) {}
inline int get_iCaller() { return IsPhysThread()^1; }
inline int get_iCaller_int() { return 0; }
#else // MAX_PHYS_THREADS>1
TLS_DECLARE(int*,g_pidxPhysThread)
inline int IsPhysThread() {
	int dummy = 0;
	INT_PTR ptr = (INT_PTR)TLS_GET(INT_PTR, g_pidxPhysThread);
	ptr += (INT_PTR)&dummy-ptr & (ptr-1>>sizeof(INT_PTR)*8-1 ^ ptr>>sizeof(INT_PTR)*8-1);
	return *(int*)ptr;
}
void MarkAsPhysThread();
void MarkAsPhysWorkerThread(int*);
inline int get_iCaller() {
	int dummy = MAX_PHYS_THREADS;
	INT_PTR ptr = (INT_PTR)TLS_GET(INT_PTR,g_pidxPhysThread);
	ptr += (INT_PTR)&dummy-ptr & (ptr-1>>sizeof(INT_PTR)*8-1 ^ ptr>>sizeof(INT_PTR)*8-1);
	return *(int*)ptr;
}
#define get_iCaller_int get_iCaller
#endif


#ifndef MAIN_THREAD_NONWORKER
#define FIRST_WORKER_THREAD 1
#else
#define FIRST_WORKER_THREAD 0
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_)

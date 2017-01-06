// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef NP_READ_CHECK_H
#define NP_READ_CHECK_H

#include "foundation/PxSimpleTypes.h"

namespace physx
{

class NpScene;

// RAII wrapper around the PxScene::startRead() method, note that this
// object does not acquire any scene locks, it is an error checking only mechanism
class NpReadCheck
{
public: 
	NpReadCheck(const NpScene* scene, const char* functionName);
	~NpReadCheck();
private:
	const NpScene* mScene;
	const char* mName;
	PxU32 mErrorCount;
};

#if (PX_DEBUG || PX_CHECKED)
	// Creates a scoped read check object that detects whether appropriate scene locks
	// have been acquired and checks if reads/writes overlap, this macro should typically
	// be placed at the beginning of any const API methods that are not multi-thread safe, 
	// the error conditions checked can be summarized as:
	
	// 1. PxSceneFlag::eREQUIRE_RW_LOCK was specified but PxScene::lockRead() was not yet called
	// 2. Other threads were already writing, or began writing during the object lifetime
	#define NP_READ_CHECK(npScenePtr) NpReadCheck npReadCheck(static_cast<const NpScene*>(npScenePtr), __FUNCTION__);
#else
	#define NP_READ_CHECK(npScenePtr)
#endif

}

#endif // NP_Read_CHECK_H

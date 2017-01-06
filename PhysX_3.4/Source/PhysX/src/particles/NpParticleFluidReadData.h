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


#ifndef PX_PHYSICS_NP_PARTICLE_FLUID_READ_DATA
#define PX_PHYSICS_NP_PARTICLE_FLUID_READ_DATA

#include "PxParticleFluidReadData.h"
#include "PsFoundation.h"

namespace physx
{

class NpParticleFluidReadData : public PxParticleFluidReadData, public Ps::UserAllocated
{
public:

	NpParticleFluidReadData() :
		mIsLocked(false),
		mFlags(PxDataAccessFlag::eREADABLE)
	{
		strncpy(mLastLockedName, "UNDEFINED", sBufferLength);
	}

	virtual	~NpParticleFluidReadData()
	{}

	// implementation for PxLockedData
	virtual void setDataAccessFlags(PxDataAccessFlags flags) { mFlags = flags; }
	virtual PxDataAccessFlags getDataAccessFlags() { return mFlags; }

	virtual void unlock() { unlockFast(); }

	// internal methods
	void unlockFast() { mIsLocked = false; }

	void lock(const char* callerName)
	{
		if (mIsLocked)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxParticleReadData access through %s while its still locked by last call of %s.", callerName, mLastLockedName);
			PX_ALWAYS_ASSERT_MESSAGE("PxParticleReadData access violation");
		}
		strncpy(mLastLockedName, callerName, sBufferLength); 
		mLastLockedName[sBufferLength-1]=0;
		mIsLocked = true;
	}

private:

	static const PxU32 sBufferLength = 128;
	bool mIsLocked;
	char mLastLockedName[sBufferLength];
	PxDataAccessFlags mFlags;
};

}

#endif

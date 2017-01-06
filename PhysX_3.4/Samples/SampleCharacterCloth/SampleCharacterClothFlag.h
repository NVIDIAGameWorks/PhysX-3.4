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

#ifndef SAMPLE_CHARACTER_CLOTH_FLAG_H
#define SAMPLE_CHARACTER_CLOTH_FLAG_H

#include "cloth/PxCloth.h"
#include "PhysXSample.h"

class SampleCharacterCloth;
class RenderClothActor;

class SampleCharacterClothFlag
{
public:
    SampleCharacterClothFlag(SampleCharacterCloth& sample, const PxTransform &pose, PxU32 resX, PxU32 resY, PxReal sizeX, PxReal sizeY, PxReal height);

	void     setWind(const PxVec3 &dir, PxReal strength, const PxVec3& range);
    void     update(PxReal dtime);
	void     release();
	PxCloth* getCloth() { return mCloth; }
    
protected:
	RenderClothActor*		mRenderActor;
	SampleCharacterCloth&	mSample;
    PxCloth*                mCloth;
    SampleArray<PxVec2>     mUVs;
    PxVec3                  mWindDir;
    PxVec3                  mWindRange;
	PxReal                  mWindStrength;
    PxReal                  mTime;
	PxRigidDynamic*			mCapsuleActor;
private:
	SampleCharacterClothFlag& operator=(const SampleCharacterClothFlag&);
};

#endif

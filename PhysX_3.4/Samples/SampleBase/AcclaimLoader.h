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

#ifndef ACCLAIM_LOADER
#define ACCLAIM_LOADER

#include "foundation/PxFlags.h"
#include "foundation/PxTransform.h"
#include "SampleAllocator.h"
#include "SampleArray.h"

namespace Acclaim
{

#define MAX_BONE_NAME_CHARACTER_LENGTH 100
#define MAX_BONE_NUMBER 32

///////////////////////////////////////////////////////////////////////////////
struct BoneDOFFlag
{
	enum Enum
	{
		eRX	= (1<<0), 
		eRY	= (1<<1), 
		eRZ	= (1<<2),
		eLENGTH	= (1<<3)
	};
};

typedef PxFlags<BoneDOFFlag::Enum,PxU16> BoneDOFFlags;
PX_FLAGS_OPERATORS(BoneDOFFlag::Enum, PxU16)

///////////////////////////////////////////////////////////////////////////////
struct Bone
{
	int              mID;
	char             mName[MAX_BONE_NAME_CHARACTER_LENGTH];
	PxVec3           mDirection;
	PxReal           mLength;
	PxVec3           mAxis;
	BoneDOFFlags     mDOF; 
	Bone*	         mParent;

public:
	Bone() :
		mID(-1),
		mDirection(0.0f),
		mLength(0.0f),
		mAxis(0.0f, 0.0f, 1.0f),
		mDOF(0),
		mParent(NULL)
		{
		}	
};

///////////////////////////////////////////////////////////////////////////////
struct ASFData
{
	struct Header
	{
		PxReal           mMass;
		PxReal           mLengthUnit;
		bool             mAngleInDegree;
	};

	struct Root
	{
		PxVec3           mPosition;
		PxVec3           mOrientation;
	};

	Header               mHeader;
	Root	             mRoot;
	Bone*                mBones;
	PxU32                mNbBones;

public:
	void release() { if (mBones) free(mBones); }
};

///////////////////////////////////////////////////////////////////////////////
struct FrameData
{
	PxVec3   mRootPosition;
	PxVec3   mRootOrientation;

	PxVec3   mBoneFrameData[MAX_BONE_NUMBER];
	PxU32    mNbBones;
};

///////////////////////////////////////////////////////////////////////////////
struct AMCData
{
	FrameData*    mFrameData;
	PxU32         mNbFrames;

public:
	void release() { if (mFrameData) free(mFrameData); }
};

///////////////////////////////////////////////////////////////////////////////
bool readASFData(const char* filename, ASFData& data);
bool readAMCData(const char* filename, ASFData& asfData, AMCData& amcData);
}

#endif // ACCLAIM_LOADER

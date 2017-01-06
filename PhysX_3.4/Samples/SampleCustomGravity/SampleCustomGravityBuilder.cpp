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


#include "PxPhysicsAPI.h"
#include "SampleCustomGravity.h"
#include "Planet.h"
#include "PsMathUtils.h"

using namespace PxToolkit;

static const PxReal gPlatformSpeed = 4.0f;
static const PxReal gGlobalScale = 1.0f;

void SampleCustomGravity::buildScene()
{
	if(1)
	{
		importRAWFile("planet.raw", 2.0f);
	}

	if(0)
	{
//		const PxU32 tesselationLevel = 10;
		const PxU32 tesselationLevel = 32;
		PlanetMesh planet;
		planet.generate(mPlanet.mPos, mPlanet.mRadius, tesselationLevel, tesselationLevel);

		RAWMesh data;
		data.mName			= "planet";
		data.mTransform		= PxTransform(PxIdentity);
		data.mNbVerts		= planet.getNbVerts();
		data.mVerts			= planet.getVerts();
		data.mUVs			= 0;
		data.mMaterialID	= 0;
		data.mNbFaces		= planet.getNbTris();
		data.mIndices		= planet.getIndices();
		char buffer[256];
		sprintf(buffer, "PlanetMesh%.2f", mPlanet.mRadius);

//		char srcMetaFile[512];	srcMetaFile[0]=0;
//		GetOutputFileManager().getFilePath(buffer, srcMetaFile);
//		setFilename(srcMetaFile);

		setFilename(getSampleMediaFilename(buffer));
		newMesh(data);
	}

	if(0)
	{

	const PxF32 rotationSpeed = 1.0f;
	BasicRandom random(42);
	const PxU32 nbPlatforms = 20;
	for(PxU32 i=0;i<nbPlatforms;i++)
	{
		PxVec3 up;
		random.unitRandomPt(up);
		up.normalize();

		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), up);

		PxVec3 pts[2];
		pts[0] = up * mPlanet.mRadius;
		pts[1] = up * (mPlanet.mRadius + 5.0f);
		KinematicPlatform* platform = createPlatform(2, pts, q, gGlobalScale * PxVec3(0.1f, 4.0f, 1.0f), gPlatformSpeed, rotationSpeed);

		platform->setT(PxF32(i)/PxF32(nbPlatforms-1));
	}

	}

	if(0)
	{
		const PxF32 radius = 25.0f;
		const PxF32 platformSpeed = 4.0f;
//		const PxF32 platformSpeed = 10.0f;

		const PxU32 nbPts = 100;
		PxVec3 pts[100];
		for(PxU32 i=0;i<nbPts;i++)
		{
			const float angle = 6.283185f * float(i)/float(nbPts-1);
			const float s = sinf(angle);
			const float c = cosf(angle);
			pts[i] = radius * PxVec3(s, c, 0.0f);
		}
		const PxReal rotationSpeed = 0.0f;

		const PxVec3 up(0.0f, 0.0f, 1.0f);
		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), up);

		KinematicPlatform* platform = createPlatform(nbPts, pts, q, gGlobalScale * PxVec3(1.0f, 1.0f, 1.0f), platformSpeed, rotationSpeed);
		const PxF32 travelTime = platform->getTravelTime();
		platform->setRotationSpeed(-6.283185f/travelTime);
	}

}

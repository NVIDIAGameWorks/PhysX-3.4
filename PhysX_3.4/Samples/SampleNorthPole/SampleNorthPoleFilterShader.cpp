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
#include "SampleNorthPole.h"

void SampleNorthPole::customizeSceneDesc(PxSceneDesc& sceneDesc)
{
	sceneDesc.gravity					= PxVec3(0,-9.81,0);
	sceneDesc.filterShader				= filter;
	sceneDesc.simulationEventCallback	= this;
	sceneDesc.flags						|= PxSceneFlag::eENABLE_CCD;
	sceneDesc.flags						|= PxSceneFlag::eREQUIRE_RW_LOCK;
}

void SampleNorthPole::setSnowball(PxShape& shape)
{
	PxFilterData fd = shape.getSimulationFilterData();
	fd.word3 |= SNOWBALL_FLAG;
	shape.setSimulationFilterData(fd);
}

bool SampleNorthPole::needsContactReport(const PxFilterData& filterData0, const PxFilterData& filterData1)
{
	const PxU32 needsReport = PxU32(DETACHABLE_FLAG | SNOWBALL_FLAG);
	PxU32 flags = (filterData0.word3 | filterData1.word3);
	return (flags & needsReport) == needsReport;
}

void SampleNorthPole::setDetachable(PxShape& shape)
{
	PxFilterData fd = shape.getSimulationFilterData();
	fd.word3 |= PxU32(DETACHABLE_FLAG);
	shape.setSimulationFilterData(fd);
}

bool SampleNorthPole::isDetachable(PxFilterData& filterData)
{
	return filterData.word3 & PxU32(DETACHABLE_FLAG) ? true : false;
}

void SampleNorthPole::setCCDActive(PxShape& shape, PxRigidBody* rigidBody)
{
	rigidBody->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	PxFilterData fd = shape.getSimulationFilterData();
	fd.word3 |= CCD_FLAG;
	shape.setSimulationFilterData(fd);
	
}

bool SampleNorthPole::isCCDActive(PxFilterData& filterData)
{
	return filterData.word3 & CCD_FLAG ? true : false;
}

PxFilterFlags SampleNorthPole::filter(	PxFilterObjectAttributes attributes0,
						PxFilterData filterData0, 
						PxFilterObjectAttributes attributes1,
						PxFilterData filterData1,
						PxPairFlags& pairFlags,
						const void* constantBlock,
						PxU32 constantBlockSize)
{

	if (isCCDActive(filterData0) || isCCDActive(filterData1))
	{
		pairFlags |= PxPairFlag::eSOLVE_CONTACT;
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
	}

	if (needsContactReport(filterData0, filterData1))
	{
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}

	pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
	return PxFilterFlags();
}

void SampleNorthPole::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	for(PxU32 i=0; i < nbPairs; i++)
	{
		PxU32 n = 2;
		const PxContactPairFlag::Enum delShapeFlags[] = { PxContactPairFlag::eREMOVED_SHAPE_0, PxContactPairFlag::eREMOVED_SHAPE_1 };
		const PxContactPair& cp = pairs[i];
		while(n--)
		{
			if(!(cp.flags & delShapeFlags[n]))
			{
				PxShape* shape = cp.shapes[n];
				PxFilterData fd = shape->getSimulationFilterData();
				if(isDetachable(fd))
				{
					mDetaching.push_back(shape);
				}
			}
		}
	}
}

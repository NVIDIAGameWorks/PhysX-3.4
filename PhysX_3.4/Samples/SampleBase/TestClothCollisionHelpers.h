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

#ifndef WRAP_CLOTH_H
#define WRAP_CLOTH_H

//----------------------------------------------------------------------------//
#include "PxPhysicsAPI.h"
#include "TestArray.h"
#include "TestMotionGenerator.h"

#include "cloth/PxCloth.h"

#include "PhysXSample.h"

//----------------------------------------------------------------------------//

namespace physx
{
	class PxClothMeshDesc;
	class PxClothFabric;
}

namespace Test
{
    // wrapper for collision data for cloth
    class ClothCollision 
    {
    public:
		friend class ::RenderClothActor;
        ClothCollision();

        // Test::ClothCollision
        virtual PxU32 addSphere(const PxVec3& position, PxReal radius);
        virtual PxU32 addCapsuleIndex(PxU32 i0, PxU32 i1);
		virtual PxU32 addCapsule(const PxVec3& position1, PxReal radius1, const PxVec3& position2, PxReal radius2);
		virtual PxU32 getNbSpheres() const;
		virtual PxU32 getNbCapsules() const;

		virtual PxBounds3 getWorldBounds() const; 
		virtual const PxClothCollisionSphere* getSpheres() const;
		virtual const PxU32* getCapsules() const;

		virtual void setClothPose(const PxTransform &clothPose);
        virtual void setCapsuleMotion(PxU32 id, const PxVec3 &linear, const PxVec3 &angular);
		virtual void setSphereMotion(PxU32 id, const PxVec3 &linear);
        virtual void updateMotion(PxReal time, PxReal timestep);

		virtual void release();

        
		virtual ~ClothCollision();
    protected:
        // convert sphere positions to local pose of cloth
        void applyLocalTransform();

    protected:
		PxTransform mClothPose;
		Test::Array<PxVec3> mOrigPositions;
		Test::Array<PxVec3> mWorldPositions;
		Test::Array<PxClothCollisionSphere> mSpheres;
		Test::Array<MotionGenerator> mSphereMotion;
		Test::Array<MotionGenerator> mCapsuleMotion;
		Test::Array<PxU32> mCapsuleIndices;
	};
}


//----------------------------------------------------------------------------//

#endif // WRAP_CLOTH_H


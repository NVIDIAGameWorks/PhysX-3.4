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

#ifndef TEST_CLOTH
#define TEST_CLOTH

#include "PxShape.h"
#include "cloth/PxCloth.h"

using namespace physx;

namespace Test
{
    class ClothCollision
    {
    public:
        virtual PxU32 addSphere(const PxVec3& position, PxReal radius) = 0;
        virtual PxU32 addCapsuleIndex(PxU32 i0, PxU32 i1) = 0;
		virtual PxU32 addCapsule(const PxVec3& position1, PxReal radius1, const PxVec3& position2, PxReal radius2) = 0;
		virtual PxU32 getNbSpheres() const = 0;
		virtual const PxClothCollisionSphere* getSpheres() const = 0;
		virtual void setClothPose(const PxTransform &pose) = 0;
		virtual void setCapsuleMotion(PxU32 id, const PxVec3 &linear, const PxVec3 &angular) = 0;
        virtual void setSphereMotion(PxU32 id, const PxVec3 &linear) = 0;
		virtual void updateMotion(PxReal time, PxReal timestep) = 0;
		virtual bool generateClothCollisionData(PxClothCollisionData &) const = 0;
		virtual ~ClothCollision() {};
    };

	class Cloth
	{
	public:

		// border flags
		enum 
		{
			NONE = 0,
			BORDER_TOP		= (1 << 0),
			BORDER_BOTTOM	= (1 << 1),
			BORDER_LEFT		= (1 << 2),
			BORDER_RIGHT	= (1 << 3)
		};

		// solver types
		enum SolverType 
        {
            eMIXED = 1 << 0, // eSTIFF for vertical fiber, eFAST for everything else
            eFAST = 1 << 1,  // eFAST for everything
            eSTIFF = 1 << 2, // eSTIFF for everything
			eZEROSTRETCH = 1 << 3 // eZEROSTRETCH for zero stretch fiber, eFAST for everything else
        };

		virtual void detachVertex(PxU32 vertexId) = 0;
		virtual void attachVertexToWorld(PxU32 vertexId) = 0;
		virtual void attachVertexToWorld(PxU32 vertexId, const PxVec3& pos) = 0;
		virtual void attachBorderToWorld(PxU32 borderFlags) = 0;
		virtual void attachOverlapToShape(PxShape* shape, PxReal radius = 0.1f) = 0;
		virtual void createVirtualParticles(int numSamples) = 0;
		virtual ClothCollision& getCollision() = 0;
		virtual const PxTransform& getClothPose() const = 0;
		virtual void release() = 0;
		virtual PxCloth& getCloth() = 0;
		virtual void setCloth(PxCloth&) = 0;
		virtual PxU32 getNbParticles() const = 0;
		virtual PxClothParticle* getParticles() = 0;
		virtual PxBounds3 getWorldBounds(bool includeColliders = false) = 0;
		virtual void setClothPose(const PxTransform &pose, bool keepIntertia = true) = 0;
		virtual void setAnimationSpeed(PxReal) = 0;
		virtual void setDampingCoefficient(PxReal d) = 0;
		virtual void setMassScalingCoefficient(PxReal s) = 0;
		virtual void setMotion(const PxVec3 &linear, const PxVec3 &angular) = 0;
		virtual void setSolverFrequency(PxReal v) = 0;
		virtual void setSolverType(Cloth::SolverType solverType) = 0;
		virtual void setStiffness(PxReal v) = 0;
		virtual void setSweptContact(bool val) = 0;
		virtual void setUseGPU(bool val) = 0;
        virtual void updateMotion(PxReal time, PxReal timestep, bool keepInertia = true) = 0;
		virtual ~Cloth() {};
	};
};

#endif // TEST_CLOTH

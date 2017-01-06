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


#ifndef DY_ARTICULATION_HELPER_H
#define DY_ARTICULATION_HELPER_H


#include "DyArticulation.h"

namespace physx
{
struct PxsBodyCore;

class PxcConstraintBlockStream;
class PxcRigidBody;
class PxsConstraintBlockManager;
struct PxSolverConstraintDesc;

namespace Dy
{
	struct FsInertia;
	struct SolverConstraint1DExt;
	struct ArticulationJointCore;
	struct ArticulationSolverDesc;


struct ArticulationJointTransforms
{
	PxTransform		cA2w;				// joint parent frame in world space 
	PxTransform		cB2w;				// joint child frame in world space
	PxTransform		cB2cA;				// joint relative pose in world space
};

class ArticulationHelper
{
public:
	static PxU32	computeUnconstrainedVelocities(const ArticulationSolverDesc& desc,
												   PxReal dt,
												   PxcConstraintBlockStream& stream,
												   PxSolverConstraintDesc* constraintDesc,
												   PxU32& acCount,
												   PxsConstraintBlockManager& constraintBlockManager,
												   const PxVec3& gravity, PxU64 contextID);

	static void		updateBodies(const ArticulationSolverDesc& desc,
							 	 PxReal dt);


	static void		getImpulseResponse(const FsData& matrix, 
									   PxU32 linkID, 
									   const Cm::SpatialVectorV& impulse,
									   Cm::SpatialVectorV& deltaV);


	static PX_FORCE_INLINE 
			void	getImpulseResponse(const FsData& matrix, 
									   PxU32 linkID, 
									   const Cm::SpatialVector& impulse,
									   Cm::SpatialVector& deltaV)
	{
		getImpulseResponse(matrix, linkID, reinterpret_cast<const Cm::SpatialVectorV&>(impulse), reinterpret_cast<Cm::SpatialVectorV&>(deltaV));
	}

	static void		getImpulseSelfResponse(const FsData& matrix, 
										   PxU32 linkID0, 
										   const Cm::SpatialVectorV& impulse0,
										   Cm::SpatialVectorV& deltaV0,
										   PxU32 linkID1,
										   const Cm::SpatialVectorV& impulse1,
										   Cm::SpatialVectorV& deltaV1);

	static void		flushVelocity(FsData& matrix);

	static void		saveVelocity(const ArticulationSolverDesc& m);

	static void		getDataSizes(PxU32 linkCount, PxU32 &solverDataSize, PxU32& totalSize, PxU32& scratchSize);

	static void		initializeDriveCache(FsData &data,
										 PxU16 linkCount,
										 const ArticulationLink* links,
										 PxReal compliance,
										 PxU32 iterations,
										 char* scratchMemory,
										 PxU32 scratchMemorySize);

	static PxU32	getDriveCacheLinkCount(const FsData& cache);

	static void		applyImpulses(const FsData& matrix,
								  Cm::SpatialVectorV* Z,
								  Cm::SpatialVectorV* V);

private:
	static PxU32	getLtbDataSize(PxU32 linkCount);
	static PxU32	getFsDataSize(PxU32 linkCount);

	static void		prepareDataBlock(FsData& fsData,
									 const ArticulationLink* links,
									 PxU16 linkCount,	
									 PxTransform* poses,
								 	 FsInertia *baseInertia,
									 ArticulationJointTransforms* jointTransforms,
									 PxU32 expectedSize);

	static void		setInertia(FsInertia& inertia,
							   const PxsBodyCore& body,
							   const PxTransform& pose);

	static void		setJointTransforms(ArticulationJointTransforms& transforms,
									   const PxTransform& parentPose,
									   const PxTransform& childPose,
									   const ArticulationJointCore& joint);

	static void		prepareLtbMatrix(FsData& fsData,
									 const FsInertia* baseInertia,
									 const PxTransform* poses,
									 const ArticulationJointTransforms* jointTransforms,
									 PxReal recipDt);

	static void		prepareFsData(FsData& fsData,
								  const ArticulationLink* links);

	static PX_FORCE_INLINE PxReal getResistance(PxReal compliance);


	static void		createHardLimit(const FsData& fsData,
									const ArticulationLink* links,
									PxU32 linkIndex,
									SolverConstraint1DExt& s, 
									const PxVec3& axis, 
									PxReal err,
									PxReal recipDt);

	static void		createTangentialSpring(const FsData& fsData,
										   const ArticulationLink* links,
										   PxU32 linkIndex,
										   SolverConstraint1DExt& s, 
										   const PxVec3& axis, 
										   PxReal stiffness,
										   PxReal damping,
										   PxReal dt);

	static PxU32 setupSolverConstraints(FsData& fsData, PxU32 solverDataSize,
													PxcConstraintBlockStream& stream,
													PxSolverConstraintDesc* constraintDesc,
													const ArticulationLink* links,
													const ArticulationJointTransforms* jointTransforms,
													PxReal dt,
													PxU32& acCount,
													PxsConstraintBlockManager& constraintBlockManager);

	static void		computeJointDrives(FsData& fsData,
									   Ps::aos::Vec3V* drives, 
									   const ArticulationLink* links,
									   const PxTransform* poses, 
									   const ArticulationJointTransforms* transforms, 
									   const Ps::aos::Mat33V* loads, 
									   PxReal dt);

};

}

}

#endif //DY_ARTICULATION_HELPER_H

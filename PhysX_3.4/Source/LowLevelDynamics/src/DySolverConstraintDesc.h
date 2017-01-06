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


#ifndef DY_SOLVERCONSTRAINTDESC_H
#define DY_SOLVERCONSTRAINTDESC_H

#include "PxvConfig.h"
#include "DySolverConstraintTypes.h"
#include "PsUtilities.h"
#include "PxConstraintDesc.h"
#include "solver/PxSolverDefs.h"

namespace physx
{

struct PxcNpWorkUnit;

struct PxsContactManagerOutput;

namespace Cm
{
	class SpatialVector;
}

struct PxSolverBody;
struct PxSolverBodyData;

namespace Dy
{

struct FsData;




// dsequeira: moved this articulation stuff here to sever a build dep on Articulation.h through DyThreadContext.h and onward

struct SelfConstraintBlock
{
	PxU32	startId;				
	PxU32	numSelfConstraints;	
	PxU16	fsDataLength;		
	PxU16	requiredSolverProgress;
	uintptr_t eaFsData;
};

//This class rolls together multiple contact managers into a single contact manager.
struct CompoundContactManager
{
	PxU32 mStartIndex;
	PxU16 mStride;
	PxU16 mReducedContactCount;

	PxcNpWorkUnit* unit;			//This is a work unit but the contact buffer has been adjusted to contain all the contacts for all the subsequent pairs
	PxsContactManagerOutput* cmOutput;
	PxU8* originalContactPatches;	//This is the original contact buffer that we replaced with a combined buffer	
	PxU8* originalContactPoints;
	PxU8 originalContactCount;
	PxU8 originalPatchCount;
	PxU8 originalStatusFlags;
	PxReal* originalForceBuffer;	//This is the original force buffer that we replaced with a combined force buffer
	PxU16* forceBufferList;			//This is a list of indices from the reduced force buffer to the original force buffers - we need this to fix up the write-backs from the solver	
};

struct SolverConstraintPrepState
{
enum Enum 
{
	eOUT_OF_MEMORY,
	eUNBATCHABLE,
	eSUCCESS
};
};

PX_FORCE_INLINE bool isArticulationConstraint(const PxSolverConstraintDesc& desc)
{
	return desc.linkIndexA != PxSolverConstraintDesc::NO_LINK || 
		desc.linkIndexB != PxSolverConstraintDesc::NO_LINK;
}


PX_FORCE_INLINE void setConstraintLength(PxSolverConstraintDesc& desc, const PxU32 constraintLength)
{
	PX_ASSERT(0==(constraintLength & 0x0f));
	PX_ASSERT(constraintLength <= PX_MAX_U16 * 16);
	desc.constraintLengthOver16 = Ps::to16(constraintLength >> 4);
}

PX_FORCE_INLINE void setWritebackLength(PxSolverConstraintDesc& desc, const PxU32 writeBackLength)
{
	PX_ASSERT(0==(writeBackLength & 0x03));
	PX_ASSERT(writeBackLength <= PX_MAX_U16 * 4);
	desc.writeBackLengthOver4 = Ps::to16(writeBackLength >> 2);
}

PX_FORCE_INLINE PxU32 getConstraintLength(const PxSolverConstraintDesc& desc)
{
	return PxU32(desc.constraintLengthOver16 << 4);
}

PX_FORCE_INLINE PxU32 getWritebackLength(const PxSolverConstraintDesc& desc)
{
	return PxU32(desc.writeBackLengthOver4 << 2);
}

PX_COMPILE_TIME_ASSERT(0 == (0x0f & sizeof(PxSolverConstraintDesc)));

#define MAX_PERMITTED_SOLVER_PROGRESS 0xFFFF

}

}

#endif //DY_SOLVERCONSTRAINTDESC_H

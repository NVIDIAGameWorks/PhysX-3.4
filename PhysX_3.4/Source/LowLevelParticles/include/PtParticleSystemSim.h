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

#ifndef PT_PARTICLE_SYSTEM_SIM_H
#define PT_PARTICLE_SYSTEM_SIM_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "CmPhysXCommon.h"
#include "PtParticleSystemCore.h"
#include "PtParticleShape.h"

namespace physx
{

class PxParticleDeviceExclusiveAccess;

namespace Pt
{

struct ParticleSystemSimDataDesc;
class ParticleSystemSim;

/*!
\file
ParticleSystemSim interface.
*/

/************************************************************************/
/* ParticleSystemsSim                                                      */
/************************************************************************/

/**
Descriptor for the batched shape update pipeline stage
*/
struct ParticleShapesUpdateInput
{
	ParticleShape** shapes;
	PxU32 shapeCount;
};

/**
Descriptor for the batched collision update pipeline stage
*/
struct ParticleCollisionUpdateInput
{
	PxU8* contactManagerStream;
};

/*!
Descriptor for updated particle packet shapes
*/
struct ParticleShapeUpdateResults
{
	ParticleShape* const* createdShapes;   //! Handles of newly created particle packet shapes
	PxU32 createdShapeCount;               //! Number of newly created particle packet shapes
	ParticleShape* const* destroyedShapes; //! Handles of particle packet shapes to delete
	PxU32 destroyedShapeCount;             //! Number of particle packet shapes to delete
};

class ParticleSystemSim
{
  public:
	virtual ParticleSystemState& getParticleStateV() = 0;
	virtual void getSimParticleDataV(ParticleSystemSimDataDesc& simParticleData, bool devicePtr) const = 0;
	virtual void getShapesUpdateV(ParticleShapeUpdateResults& updateResults) const = 0;

	virtual void setExternalAccelerationV(const PxVec3& v) = 0;
	virtual const PxVec3& getExternalAccelerationV() const = 0;

	virtual void setSimulationTimeStepV(PxReal value) = 0;
	virtual PxReal getSimulationTimeStepV() const = 0;

	virtual void setSimulatedV(bool) = 0;
	virtual Ps::IntBool isSimulatedV() const = 0;

	// gpuBuffer specifies that the interaction was created asynchronously to gpu execution (for rb ccd)
	virtual void addInteractionV(const ParticleShape& particleShape, ShapeHandle shape, BodyHandle body, bool isDynamic,
	                             bool gpuBuffer) = 0;

	// gpuBuffer specifies that the interaction was created asynchronously to gpu execution (for rb ccd)
	virtual void removeInteractionV(const ParticleShape& particleShape, ShapeHandle shape, BodyHandle body,
	                                bool isDynamic, bool isDyingRb, bool gpuBuffer) = 0;

	virtual void onRbShapeChangeV(const ParticleShape& particleShape, ShapeHandle shape) = 0;

	// applies the buffered interaction updates.
	virtual void flushBufferedInteractionUpdatesV() = 0;

	// passes the contact manager stream needed for collision - the callee is responsible for releasing it
	virtual void passCollisionInputV(ParticleCollisionUpdateInput input) = 0;

#if PX_SUPPORT_GPU_PHYSX
	virtual Ps::IntBool isGpuV() const = 0;
	virtual void enableDeviceExclusiveModeGpuV() = 0;
	virtual PxParticleDeviceExclusiveAccess* getDeviceExclusiveAccessGpuV() const = 0;
#endif

  protected:
	virtual ~ParticleSystemSim()
	{
	}
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_SYSTEM_SIM_H

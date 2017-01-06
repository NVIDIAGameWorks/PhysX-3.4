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
  
#include "PxPhysXConfig.h"

#if PX_USE_PARTICLE_SYSTEM_API
#if PX_SUPPORT_GPU_PHYSX

#include "PxParticleDeviceExclusive.h"
#include "NpParticleSystem.h"
#include "NpParticleFluid.h"

using namespace physx;

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::enable(PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->enableDeviceExclusiveModeGpu();
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->enableDeviceExclusiveModeGpu();
}

//-------------------------------------------------------------------------------------------------------------------//

bool PxParticleDeviceExclusive::isEnabled(PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		return static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->isDeviceExclusiveModeEnabledGpu();
	else if (particleBase.is<PxParticleFluid>())
		return static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->isDeviceExclusiveModeEnabledGpu();

	return false;
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::getReadWriteCudaBuffers(PxParticleBase& particleBase, PxCudaReadWriteParticleBuffers& buffers)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->getReadWriteCudaBuffersGpu(buffers);
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->getReadWriteCudaBuffersGpu(buffers);
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::setValidParticleRange(PxParticleBase& particleBase, PxU32 validParticleRange)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->setValidParticleRangeGpu(validParticleRange);
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->setValidParticleRangeGpu(validParticleRange);
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::setFlags(PxParticleBase& particleBase, PxParticleDeviceExclusiveFlags flags)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->setDeviceExclusiveModeFlagsGpu(reinterpret_cast<PxU32&>(flags));
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->setDeviceExclusiveModeFlagsGpu(reinterpret_cast<PxU32&>(flags));
}

//-------------------------------------------------------------------------------------------------------------------//

class physx::PxBaseTask* PxParticleDeviceExclusive::getLaunchTask(class PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		return static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->getLaunchTaskGpu();
	else if (particleBase.is<PxParticleFluid>())
		return static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->getLaunchTaskGpu();

	return NULL;
}

//-------------------------------------------------------------------------------------------------------------------//

void PxParticleDeviceExclusive::addLaunchTaskDependent(class PxParticleBase& particleBase, class physx::PxBaseTask& dependent)
{
	if (particleBase.is<PxParticleSystem>())
		static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->addLaunchTaskDependentGpu(dependent);
	else if (particleBase.is<PxParticleFluid>())
		static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->addLaunchTaskDependentGpu(dependent);
}

//-------------------------------------------------------------------------------------------------------------------//

CUstream PxParticleDeviceExclusive::getCudaStream(class PxParticleBase& particleBase)
{
	if (particleBase.is<PxParticleSystem>())
		return (CUstream)static_cast<NpParticleSystem*>(particleBase.is<PxParticleSystem>())->getCudaStreamGpu();
	else if (particleBase.is<PxParticleFluid>())
		return (CUstream)static_cast<NpParticleFluid*>(particleBase.is<PxParticleFluid>())->getCudaStreamGpu();

	return NULL;
}

//-------------------------------------------------------------------------------------------------------------------//

#endif // PX_SUPPORT_GPU_PHYSX
#endif // PX_USE_PARTICLE_SYSTEM_API

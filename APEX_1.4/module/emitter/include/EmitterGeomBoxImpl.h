/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_BOX_IMPL_H__
#define __EMITTER_GEOM_BOX_IMPL_H__

#include "EmitterGeomBase.h"
#include "PsUserAllocated.h"
#include "EmitterGeomBoxParams.h"

namespace NvParameterized
{
class Interface;
};

namespace nvidia
{
namespace emitter
{

class EmitterGeomBoxImpl : public EmitterGeomBox, public EmitterGeomBase
{
public:
	EmitterGeomBoxImpl(NvParameterized::Interface* params);

	/* Asset callable methods */
	EmitterGeom*				getEmitterGeom();
	const EmitterGeomBox* 	isBoxGeom() const
	{
		return this;
	}
	EmitterType::Enum		getEmitterType() const
	{
		return mType;
	}
	void						setEmitterType(EmitterType::Enum t);
	void						setExtents(const PxVec3& extents)
	{
		*mExtents = extents;
	}
	PxVec3						getExtents() const
	{
		return *mExtents;
	}
	void						destroy()
	{
		delete this;
	}

	/* AssetPreview methods */
	void                        drawPreview(float scale, RenderDebugInterface* renderDebug) const;

	/* Actor callable methods */
	void						visualize(const PxTransform& pose, RenderDebugInterface& renderDebug);
	float				computeNewlyCoveredVolume(const PxMat44&, const PxMat44&, float, QDSRand&) const;

	void						computeFillPositions(physx::Array<PxVec3>& positions,
	        physx::Array<PxVec3>& velocities,
	        const PxTransform&,
			const PxVec3&,
	        float,
	        PxBounds3& outBounds,
	        QDSRand& rand) const;

	float				computeEmitterVolume() const;
	PxVec3				randomPosInFullVolume(
	    const PxMat44& pose,
	    QDSRand& rand) const;
	PxVec3				randomPosInNewlyCoveredVolume(
	    const PxMat44& pose,
	    const PxMat44& oldPose,
	    QDSRand& rand) const;
	bool						isInEmitter(
	    const PxVec3& pos,
	    const PxMat44& pose) const;

protected:
	EmitterType::Enum		mType;
	PxVec3*				mExtents;
	EmitterGeomBoxParams*   	mGeomParams;
};

}
} // end namespace nvidia

#endif
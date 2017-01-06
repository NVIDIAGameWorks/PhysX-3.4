/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_SPHERE_IMPL_H__
#define __EMITTER_GEOM_SPHERE_IMPL_H__

#include "EmitterGeomBase.h"
#include "PsUserAllocated.h"
#include "EmitterGeomSphereParams.h"

namespace NvParameterized
{
class Interface;
};

namespace nvidia
{
namespace emitter
{

class EmitterGeomSphereImpl : public EmitterGeomSphere, public EmitterGeomBase
{
public:
	EmitterGeomSphereImpl(NvParameterized::Interface* params);

	/* Asset callable methods */
	EmitterGeom*				getEmitterGeom();
	const EmitterGeomSphere* 	isSphereGeom() const
	{
		return this;
	}
	EmitterType::Enum		getEmitterType() const
	{
		return mType;
	}
	void						setEmitterType(EmitterType::Enum t);
	void	                    setRadius(float radius)
	{
		*mRadius = radius;
	}
	float	                    getRadius() const
	{
		return *mRadius;
	}
	void	                    setHemisphere(float hemisphere)
	{
		*mHemisphere = hemisphere;
	}
	float	                    getHemisphere() const
	{
		return *mHemisphere;
	}
	void						destroy()
	{
		delete this;
	}

	/* AssetPreview methods */
	void                        drawPreview(float scale, RenderDebugInterface* renderDebug) const;

	/* Actor callable methods */
	void						visualize(const PxTransform& pose, RenderDebugInterface& renderDebug);

	void						computeFillPositions(physx::Array<PxVec3>& positions,
	        physx::Array<PxVec3>& velocities,
	        const PxTransform&,
			const PxVec3&,
	        float,
	        PxBounds3& outBounds,
	        QDSRand& rand) const;

	float				computeEmitterVolume() const;
	PxVec3				randomPosInFullVolume(const PxMat44& pose, QDSRand& rand) const;
	bool						isInEmitter(const PxVec3& pos, const PxMat44& pose) const;

protected:
	EmitterType::Enum		mType;
	float*				mRadius;
	float*				mHemisphere;
	EmitterGeomSphereParams*	mGeomParams;
};

}
} // end namespace nvidia

#endif

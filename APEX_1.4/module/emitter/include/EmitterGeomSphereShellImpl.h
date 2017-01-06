/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_SPHERE_SHELL_IMPL_H__
#define __EMITTER_GEOM_SPHERE_SHELL_IMPL_H__

#include "EmitterGeomBase.h"
#include "PsUserAllocated.h"
#include "EmitterGeomSphereShellParams.h"

namespace NvParameterized
{
class Interface;
};

namespace nvidia
{
namespace emitter
{

class EmitterGeomSphereShellImpl : public EmitterGeomSphereShell, public EmitterGeomBase
{
public:
	EmitterGeomSphereShellImpl(NvParameterized::Interface* params);

	/* Asset callable methods */
	EmitterGeom*				getEmitterGeom();
	const EmitterGeomSphereShell* isSphereShellGeom() const
	{
		return this;
	}
	EmitterType::Enum		getEmitterType() const
	{
		return mType;
	}
	void						setEmitterType(EmitterType::Enum t);
	void						setRadius(float radius)
	{
		*mRadius = radius;
	}
	float				getRadius() const
	{
		return *mRadius;
	}
	void						setShellThickness(float thickness)
	{
		*mShellThickness = thickness;
	}
	float				getShellThickness() const
	{
		return *mShellThickness;
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
	PxVec3				randomPointOnUnitSphere(QDSRand& rand) const;

	EmitterType::Enum		mType;
	float*				mRadius;
	float*				mShellThickness;
	float*				mHemisphere;
	EmitterGeomSphereShellParams* mGeomParams;
};

}
} // end namespace nvidia

#endif

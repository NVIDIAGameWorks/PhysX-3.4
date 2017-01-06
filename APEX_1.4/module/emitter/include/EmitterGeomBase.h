/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_BASE_H__
#define __EMITTER_GEOM_BASE_H__

#include "Apex.h"
#include "EmitterGeoms.h"
#include "PsArray.h"
#include "PsUserAllocated.h"
#include <ApexUsingNamespace.h>
#include "ApexRand.h"

namespace nvidia
{
namespace apex
{
class RenderDebugInterface;
}
namespace emitter
{

/* Implementation base class for all EmitterGeom derivations */

class EmitterGeomBase : public UserAllocated
{
public:
	/* Asset callable functions */
	virtual EmitterGeom*				getEmitterGeom() = 0;
	virtual void						destroy() = 0;

	/* ApexEmitterActor runtime access methods */
	virtual float				computeEmitterVolume() const = 0;
	virtual void						computeFillPositions(physx::Array<PxVec3>& positions,
	        physx::Array<PxVec3>& velocities,
	        const PxTransform&,
			const PxVec3&,
	        float,
	        PxBounds3& outBounds,
	        QDSRand& rand) const = 0;

	virtual PxVec3				randomPosInFullVolume(const PxMat44&, QDSRand&) const = 0;

	/* AssetPreview methods */
	virtual void                        drawPreview(float scale, RenderDebugInterface* renderDebug) const = 0;

	/* Optional override functions */
	virtual void						visualize(const PxTransform&, RenderDebugInterface&) { }

	virtual float						computeNewlyCoveredVolume(const PxMat44&, const PxMat44&, float, QDSRand&) const;
	virtual PxVec3				randomPosInNewlyCoveredVolume(const PxMat44&, const PxMat44&, QDSRand&) const;

protected:
	virtual bool						isInEmitter(const PxVec3& pos, const PxMat44& pose) const = 0;
};

}
} // end namespace nvidia

#endif

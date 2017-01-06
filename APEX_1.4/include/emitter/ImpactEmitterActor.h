/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IMPACT_EMITTER_ACTOR_H
#define IMPACT_EMITTER_ACTOR_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class ImpactEmitterAsset;
class RenderVolume;

///Impact emitter actor.  Emits particles at impact places
class ImpactEmitterActor : public Actor
{
protected:
	virtual ~ImpactEmitterActor() {}

public:
	///Gets the pointer to the underlying asset
	virtual ImpactEmitterAsset* 	getEmitterAsset() const = 0;

	/**
	\brief Registers an impact in the queue

	\param hitPos impact position
	\param hitDir impact direction
	\param surfNorm normal of the surface that is hit by the impact
	\param setID - id for the event set which should be spawned. Specifies the behavior. \sa ImpactEmitterAsset::querySetID

	*/
	virtual void registerImpact(const PxVec3& hitPos, const PxVec3& hitDir, const PxVec3& surfNorm, uint32_t setID) = 0;

	///Emitted particles are injected to specified render volume on initial frame.
	///Set to NULL to clear the preferred volume.
	virtual void setPreferredRenderVolume(RenderVolume* volume) = 0;
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // IMPACT_EMITTER_ACTOR_H

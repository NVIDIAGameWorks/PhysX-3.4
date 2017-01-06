/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IOFX_ACTOR_H
#define IOFX_ACTOR_H

#include "Apex.h"
#include "IofxAsset.h"
#include "IofxRenderable.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief IOFX actor public interface
*/
class IofxActor : public Actor
{
public:
	///get the radius of the partice
	virtual float				getObjectRadius() const = 0;
	
	///get the number of particles
	virtual uint32_t			getObjectCount() const = 0;
	
	///get the number of visible particles (if sprite depth sorting is enabled, returns only number of particles in front of camera)
	virtual uint32_t			getVisibleCount() const = 0;
	
	///get the name of the IOS asset used to feed partices to the IOFX
	virtual const char*			getIosAssetName() const = 0;

	///returns AABB covering all objects in this actor, it's updated each frame during Scene::fetchResults().
	virtual physx::PxBounds3	getBounds() const = 0;
	
	/**
	\brief Acquire a pointer to the iofx's renderable proxy and increment its reference count.  
	
	The IofxRenderable will only be deleted when its reference count is zero.  
	Calls to IofxRenderable::release decrement the reference count, as does	a call to IofxActor::release().
	*/
	virtual IofxRenderable*		acquireRenderableReference() = 0;
};

PX_POP_PACK

}
} // namespace nvidia

#endif // IOFX_ACTOR_H

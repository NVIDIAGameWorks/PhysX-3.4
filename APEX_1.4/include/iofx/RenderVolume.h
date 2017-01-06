/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDER_VOLUME_H
#define RENDER_VOLUME_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class IofxAsset;
class IofxActor;

/**
 \brief An object which "owns" a volume of world space.
 
 Any particles which enter the
 owned volume will be migrated to an IOFX actor owned by this object (if the
 particle's IOFX asset is affected by this volume).

 When volumes overlap, their relative priorities break the tie.  If multiple volumes
 have the same priority, the tie breaker goes to the volume that owns the particle.
 */
class RenderVolume : public ApexInterface
{
public:
	/// Returns AABB covering all objects in this render volume, it's updated each frame during Scene::fetchResults().
	virtual physx::PxBounds3 	getBounds() const = 0;

	/// Returns true if the volume affects all IOFX assets
	virtual bool				getAffectsAllIofx() const = 0;

	/// Adds IOFX asset to volume's list of affected IOFX assets, returns false on failure
	virtual bool    			addIofxAsset(IofxAsset& iofx) = 0;

	/// Moves the render volume while maintaining its dimensions
	virtual void				setPosition(const PxVec3& pos) = 0;

	/// Directly assigns a new AABB ownership volume
	virtual void    			setOwnershipBounds(const PxBounds3& b) = 0;

	/// Retrieves the configured AABB bounds of the volume.  Call getBounds() for the "live" bounds.
	virtual PxBounds3 			getOwnershipBounds() const = 0;

	/** \brief Retrieve list of IOFX actors owned by this volume (one per IOFX Asset per IOS actor)
	 *
	 * Returns count of 0 if empty.  Returned pointer is undefined when count is 0.
	 *
	 * The bounds of each of these IOFX is guaranteed to be within the bounds of the volume itself.
	 * Calling the updateRenderResources and dispatchRenderResources() methods of the volume will
	 * implicitly call the same methods of each of these IOFX actors, so there is no need to iterate
	 * over them for rendering purposes, unless you require special logic per IOFX.
	 *
	 * It is not necessary to release these actors, they will be released automatically when their
	 * volume, their IOFX Asset, or their host IOS actor are released.
	 *
	 * This call is not thread-safe.  The returned buffer is only valid until the next APEX API
	 * call that steps the simulation or modifies the number of IOFX actors in a scene.
	 */
	virtual IofxActor* const* 	lockIofxActorList(uint32_t& count) = 0;
	
	/** 
	 \brief Unlock IOFX actors which where locked by calling lockIofxActorList
	 \see RenderVolume::lockIofxActorList
	 */
	virtual void 				unlockIofxActorList() = 0;

	/** \brief Retrieve list of volume's affected IOFX assets.
	 *
	 * Returns count of 0 if empty or volume affects all IOFX assets.  Returned pointer is
	 * undefined when count is 0.
	 *
	 * The returned buffer not thread-safe, and is only valid until the next APEX API
	 * call that steps the simulation or modifies the number of IOFX assets in a scene.
	 */
	virtual IofxAsset* const* 	getIofxAssetList(uint32_t& count) const = 0;

	virtual PxVec3				getPosition() const = 0; ///< Returns center of ownership bounds
	virtual uint32_t			getPriority() const = 0; ///< Returns priority of volume

	/** \brief Returns true if this volume affects the specified IOFX asset
	 *
	 * Callers must acquire render lock of the volume before calling
	 * this function, for thread safety.
	 */
	virtual bool				affectsIofxAsset(const IofxAsset& iofx) const = 0;

protected:
	virtual ~RenderVolume() {}
};

PX_POP_PACK

}
} // namespace nvidia

#endif // RENDER_VOLUME_H

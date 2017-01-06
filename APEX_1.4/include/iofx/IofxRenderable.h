/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IOFX_RENDERABLE_H
#define IOFX_RENDERABLE_H

#include "foundation/Px.h"
#include "ApexInterface.h"
#include "Renderable.h"
#include "IofxRenderCallback.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief IofxSharedRenderData stores common render data shared by several IOFX Renderables.
*/
struct IofxSharedRenderData
{
	uint32_t	maxObjectCount; //!< maximum number of particles
	uint32_t	activeObjectCount; //!< currently active number of particles
};

/**
\brief IofxSpriteSharedRenderData stores sprite render data shared by several IOFX Renderables.
*/
struct IofxSpriteSharedRenderData : IofxSharedRenderData
{
	IofxSpriteRenderLayout	spriteRenderLayout; //!< sprite render layout
	UserRenderBuffer*			spriteRenderBuffer; //!< sprite render buffer
	UserRenderSurface*		spriteRenderSurfaces[IofxSpriteRenderLayout::MAX_SURFACE_COUNT]; //!< sprite render surfaces
};

/**
\brief IofxMeshSharedRenderData stores mesh render data shared by several IOFX Renderables.
*/
struct IofxMeshSharedRenderData : IofxSharedRenderData
{
	IofxMeshRenderLayout		meshRenderLayout; //!< mesh render layout
	UserRenderBuffer*			meshRenderBuffer; //!< mesh render buffer
};

/**
\brief IofxCommonRenderData stores common render data for one IOFX Renderable.
*/
struct IofxCommonRenderData
{
	uint32_t		startIndex; //!< index of the first particle in render buffer
	uint32_t		objectCount; //!< number of particles in render buffer

	const char*		renderResourceNameSpace; //!< render resource name space
	const char*		renderResourceName; //!< render resource name
	void*			renderResource; //!< render resource
};

/**
\brief IofxSpriteRenderData stores sprite render data for one IOFX Renderable.
*/
struct IofxSpriteRenderData : IofxCommonRenderData
{
	uint32_t		visibleCount; //!< number of visible particles (if sprite depth sorting is enabled, includes only particles in front of the view frustrum)

	const IofxSpriteSharedRenderData* sharedRenderData; //!< reference to Sprite shared render data
};

/**
\brief IofxMeshRenderData stores mesh render data for one IOFX Renderable.
*/
struct IofxMeshRenderData : IofxCommonRenderData
{
	const IofxMeshSharedRenderData* sharedRenderData; //!< reference to Mesh shared render data
};

class IofxSpriteRenderable;
class IofxMeshRenderable;

/**
 \brief The IOFX renderable represents a unit of rendering.
		It contains complete information to render a batch of particles with the same material/mesh in the same render volume.
*/
class IofxRenderable : public ApexInterface
{
public:
	/**
	\brief Type of IOFX renderable
	*/
	enum Type
	{
		SPRITE, //!< Sprite particles type
		MESH //!< Mesh particles type
	};

	/**
	\brief Return Type of this renderable.
	*/
	virtual Type						getType() const = 0;

	/**
	\brief Return Sprite render data for Sprite renderable and NULL for other types.
	*/
	virtual const IofxSpriteRenderData*	getSpriteRenderData() const = 0;

	/**
	\brief Return Mesh render data for Mesh renderable and NULL for other types.
	*/
	virtual const IofxMeshRenderData*	getMeshRenderData() const = 0;

	/**
	\brief Return AABB of this renderable.
	*/
	virtual const physx::PxBounds3&		getBounds() const = 0;

protected:
	virtual	~IofxRenderable() {}

};

PX_POP_PACK

}
} // end namespace nvidia

#endif // IOFX_RENDERABLE_H

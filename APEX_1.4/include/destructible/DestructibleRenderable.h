/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DESTRUCTIBLE_RENDERABLE_H
#define DESTRUCTIBLE_RENDERABLE_H

#include "foundation/Px.h"
#include "ApexInterface.h"
#include "Renderable.h"
#include "ModuleDestructible.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT


/**
	Destructible renderable API.  The destructible renderable contains rendering information for an DestructibleActor.
*/
class DestructibleRenderable : public ApexInterface, public Renderable
{
public:
	/**
		Get the render mesh actor for the specified mesh type.
	*/
	virtual RenderMeshActor*	getRenderMeshActor(DestructibleActorMeshType::Enum type = DestructibleActorMeshType::Skinned) const = 0;

protected:
	virtual	~DestructibleRenderable() {}
};

PX_POP_PACK

}
} // end namespace nvidia

#endif // DESTRUCTIBLE_RENDERABLE_H

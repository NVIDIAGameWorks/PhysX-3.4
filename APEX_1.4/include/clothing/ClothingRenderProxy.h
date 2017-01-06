/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_RENDER_PROXY_H
#define CLOTHING_RENDER_PROXY_H

#include "Renderable.h"
#include "ApexInterface.h"

namespace NvParameterized
{
class Interface;
}

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Instance of ClothingRenderProxy. This is the renderable of a clothing actor.
The data in this object is consistent until it is returned to APEX with the release()
call.
*/
class ClothingRenderProxy : public ApexInterface, public Renderable
{
protected:
	virtual ~ClothingRenderProxy() {}

public:
	/**
	\brief True if the render proxy contains simulated data, false if it is purely animated.
	*/
	virtual bool hasSimulatedData() const = 0;
};

PX_POP_PACK

}
} // namespace nvidia

#endif // CLOTHING_RENDER_PROXY_H

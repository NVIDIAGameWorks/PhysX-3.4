/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_BONE_BUFFER_DESC_H
#define USER_RENDER_BONE_BUFFER_DESC_H

/*!
\file
\brief class UserRenderBoneBufferDesc, structs RenderDataFormat and RenderBoneSemantic
*/

#include "RenderDataFormat.h"
#include "UserRenderResourceManager.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief The semantics available for bone buffers
*/
struct RenderBoneSemantic
{
	/**
	\brief Enum of the semantics available for bone buffers
	*/
	enum Enum
	{
		POSE = 0,		//!< A matrix that transforms from object space into animated object space or even world space
		PREVIOUS_POSE,	//!< The corresponding poses from the last frame
		NUM_SEMANTICS	//!< Count of semantics, not a valid semantic.
	};
};



/**
\brief Descriptor to generate a bone buffer

This descriptor is filled out by APEX and helps as a guide how the
bone buffer should be generated.
*/
class UserRenderBoneBufferDesc
{
public:
	UserRenderBoneBufferDesc(void)
	{
		maxBones = 0;
		hint     = RenderBufferHint::STATIC;
		for (uint32_t i = 0; i < RenderBoneSemantic::NUM_SEMANTICS; i++)
		{
			buffersRequest[i] = RenderDataFormat::UNSPECIFIED;
		}
	}

	/**
	\brief Check if parameter's values are correct
	*/
	bool isValid(void) const
	{
		uint32_t numFailed = 0;
		return (numFailed == 0);
	}

public:
	/**
	\brief The maximum amount of bones this buffer will ever hold.
	*/
	uint32_t				maxBones;

	/**
	\brief Hint on how often this buffer is updated.
	*/
	RenderBufferHint::Enum	hint;

	/**
	\brief Array of semantics with the corresponding format.

	RenderDataFormat::UNSPECIFIED is used for semantics that are disabled
	*/
	RenderDataFormat::Enum	buffersRequest[RenderBoneSemantic::NUM_SEMANTICS];
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif

/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_BONE_BUFFER_H
#define USER_RENDER_BONE_BUFFER_H

/*!
\file
\brief class UserRenderBoneBuffer
*/

#include "RenderBufferData.h"
#include "UserRenderBoneBufferDesc.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief type of the bone buffer data
*/
class RenderBoneBufferData : public RenderBufferData<RenderBoneSemantic, RenderBoneSemantic::Enum> {};

/**
\brief Used for storing skeletal bone information used during skinning.

Separated out into its own interface as we don't know how the user is going to implement this
it could be in another vertex buffer, a texture or just an array and skinned on CPU depending
on engine and hardware support.
*/
class UserRenderBoneBuffer
{
public:
	virtual		~UserRenderBoneBuffer() {}

	/**
	\brief Called when APEX wants to update the contents of the bone buffer.

	The source data type is assumed to be the same as what was defined in the descriptor.
	APEX should call this function and supply data for ALL semantics that were originally
	requested during creation every time its called.

	\param [in] data				Contains the source data for the bone buffer.
	\param [in] firstBone			first bone to start writing to.
	\param [in] numBones			number of bones to write.
	*/
	virtual void writeBuffer(const nvidia::RenderBoneBufferData& data, uint32_t firstBone, uint32_t numBones)	= 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif

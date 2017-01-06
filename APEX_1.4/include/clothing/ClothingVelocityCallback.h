/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_VELOCITY_CALLBACK_H
#define CLOTHING_VELOCITY_CALLBACK_H

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

/**
\brief container class for the velocity shader callback.
*/
class ClothingVelocityCallback
{
public:
	/**
	\brief This callback will be fired in Apex threads. It must not address any user data, just operate on the data.
	\param [in,out] velocities  The velocities of the cloth. These can be modified if necessary, but then the method needs to return true!
	\param [in] positions       The positions of the cloth. Must not be modified, only read.
	\param [in] numParticles    Size of the velocities and positions array.

	\return return true if the velocities have been altered, false if they just have been read
	*/
	virtual bool velocityShader(PxVec3* velocities, const PxVec3* positions, uint32_t numParticles) = 0;
};

} // namespace nvidia
} // namespace nvidia

#endif // CLOTHING_VELOCITY_CALLBACK_H

/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLE_ACTOR_JOINT_IMPL_H__
#define __DESTRUCTIBLE_ACTOR_JOINT_IMPL_H__

#include "Apex.h"

namespace nvidia
{
namespace apex
{
class DestructibleActorJointDesc;
}
namespace destructible
{
class DestructibleStructure;
class DestructibleScene;

class DestructibleActorJointImpl : public ApexResource
{
public:
	DestructibleActorJointImpl(const DestructibleActorJointDesc& destructibleActorJointDesc, DestructibleScene& dscene);

	virtual		~DestructibleActorJointImpl();

	PxJoint*	getJoint()
	{
		return joint;
	}

	bool		updateJoint();

protected:

	PxJoint*				joint;
	DestructibleStructure*	structure[2];
	int32_t			attachmentChunkIndex[2];
};

}
} // end namespace nvidia

#endif // __DESTRUCTIBLE_ACTOR_JOINT_IMPL_H__

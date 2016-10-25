/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_ACTOR_GPU_H__
#define __IOFX_ACTOR_GPU_H__

#include "Apex.h"
#include "IofxActor.h"
#include "ApexActor.h"
#include "IofxActorImpl.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
}
namespace iofx
{


class Modifier;
class IofxAssetImpl;
class IofxScene;
class IofxManager;

class IofxActorGPU : public IofxActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	IofxActorGPU(ResID renderResID, IofxScene* iscene, IofxManager& mgr)
		: IofxActorImpl(renderResID, iscene, mgr)
	{
	}
	~IofxActorGPU()
	{
	}
};


}
} // namespace nvidia

#endif // __IOFX_ACTOR_GPU_H__

// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef SCENE_CRAB_H
#define SCENE_CRAB_H

#include "SceneKapla.h"
#include "foundation/Px.h"
#include "foundation/PxSimpleTypes.h"
#include "common/PxPhysXCommonConfig.h"
#include "task/PxTask.h"
#include "CrabManager.h"
#include <vector>


namespace physx
{
	class PxRigidDynamic;
	class PxRevoluteJoint;
	class PxJoint;
}

class SceneCrab : public SceneKapla
{
public:
	SceneCrab(PxPhysics* pxPhysics, PxCooking *pxCooking, bool isGrb, Shader *defaultShader, const char *resourcePath, float slowMotionFactor);
	~SceneCrab();



	virtual void						onInit(PxScene* pxScene);

	virtual void						setScene(PxScene* scene);
	
	virtual void duringSim(float dt);
	virtual void syncAsynchronousWork();

	virtual void getInitialCamera(PxVec3& pos, PxVec3& dir) { pos = PxVec3(-70.f, -5.f, -70.f); dir = PxVec3(1.f, -0.3f, 1.f).getNormalized(); }

	virtual void handleKeyDown(unsigned char key, int x, int y);
	virtual void handleKeyUp(unsigned char key, int x, int y);

private:
	void	createCrabs();

private:

	CrabManager						mCrabManager;
	PxU32							mNbCrabsX;
	PxU32							mNbCrabsZ;
	PxU32							mNbSuperCrabs;
};

#endif

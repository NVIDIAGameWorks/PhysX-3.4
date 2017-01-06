/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef SIM_SCENE
#define SIM_SCENE

#include "SimSceneBase.h"

namespace nvidia
{
namespace destructible
{
	class DestructibleActorImpl;
}

namespace fracture
{

class FracturePattern;
class Actor;

class SimScene : public base::SimScene
{
public:
	static SimScene* createSimScene(PxPhysics *pxPhysics, PxCooking *pxCooking, PxScene *scene, float minConvexSize, PxMaterial* defaultMat, const char *resourcePath);
protected:
	SimScene(PxPhysics *pxPhysics, PxCooking *pxCooking, PxScene *scene, float minConvexSize, PxMaterial* defaultMat, const char *resourcePath);
public:
	virtual ~SimScene();

	virtual void createSingletons();

	virtual base::Actor* createActor(nvidia::destructible::DestructibleActorImpl* actor);
	virtual base::Convex* createConvex();
	virtual base::Compound* createCompound(const base::FracturePattern *pattern, const base::FracturePattern *secondaryPattern = NULL, float contactOffset = 0.005f, float restOffset = -0.001f);
	virtual base::FracturePattern* createFracturePattern();
	virtual void onWake(PxActor** actors, uint32_t count);
	virtual void onSleep(PxActor** actors, uint32_t count);	

	FracturePattern* getDefaultGlass() {return mDefaultGlass;}

protected:
	FracturePattern* mDefaultGlass;
};

}
}

#endif
#endif
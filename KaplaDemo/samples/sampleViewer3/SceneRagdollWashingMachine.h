#ifndef SCENE_RAGDOLL_WASHING_MACHINE_H
#define CENE_RAGDOLL_WASHING_MACHINE_H

#include "SceneKapla.h"

using namespace physx;

// ---------------------------------------------------------------------
class SceneRagdollWashingMachine : public SceneKapla
{
	PxReal mChangeSpeedTime;
	PxU32 mSpeedState;
	PxRigidDynamic* mWashingMachine;
public:
	SceneRagdollWashingMachine(PxPhysics* pxPhysics, PxCooking *pxCooking, bool isGrb,
		Shader *defaultShader, const char *resourcePath, float slowMotionFactor);

	virtual ~SceneRagdollWashingMachine(){}

	virtual void preSim(float dt);

	virtual void onInit(PxScene* pxScene);
};

#endif
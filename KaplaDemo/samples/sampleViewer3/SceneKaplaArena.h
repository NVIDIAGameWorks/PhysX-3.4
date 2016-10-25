#ifndef SCENE_KAPLA_ARENA_H
#define SCENE_KAPLA_ARENA_H

#include "SceneKapla.h"

using namespace physx;

// ---------------------------------------------------------------------
class SceneKaplaArena : public SceneKapla
{
public:
	SceneKaplaArena(PxPhysics* pxPhysics, PxCooking *pxCooking, bool isGrb,
		Shader *defaultShader, const char *resourcePath, float slowMotionFactor);

	virtual ~SceneKaplaArena(){}

	virtual void onInit(PxScene* pxScene);

private:

	

};
#endif  // SCENE_BOXES_H

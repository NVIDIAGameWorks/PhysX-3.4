#ifndef SCENE_KAPLA_TOWER_H
#define SCENE_KAPLA_TOWER_H

#include "SceneKapla.h"

using namespace physx;

// ---------------------------------------------------------------------
class SceneKaplaTower : public SceneKapla
{
public:
	SceneKaplaTower(PxPhysics* pxPhysics, PxCooking *pxCooking, bool isGrb,
		Shader *defaultShader, const char *resourcePath, float slowMotionFactor);

	virtual ~SceneKaplaTower(){}

	virtual void handleKeyDown(unsigned char key, int x, int y);

	virtual void preSim(float dt);
	virtual void postSim(float dt);

	virtual void onInit(PxScene* pxScene);

private:

	

	void createGeometricTower(PxU32 height, PxU32 nbFacets, PxVec3 dims, PxVec3 startPos, PxReal startDensity, PxMaterial* material, ShaderMaterial& mat);
	void createCommunicationWire(PxVec3 startPos, PxVec3 endPos,PxReal connectRadius, PxReal connectHeight, PxReal density, PxReal offset, PxRigidDynamic* startBody, 
		PxRigidDynamic* endBody, PxMaterial* material, ShaderMaterial& mat, PxQuat& rot);
	void createTwistTower(PxU32 height, PxU32 nbBlocksPerLayer, PxVec3 dims, PxVec3 startPos, PxMaterial* material, ShaderMaterial& mat);
	void createRectangularTower(PxU32 nbX, PxU32 nbZ, PxU32 height, PxVec3 dims, PxVec3 centerPos, PxMaterial* material, ShaderMaterial& mat);
	//void createCylindricalTower(PxU32 nbRadialPoints, PxReal maxRadius, PxReal minRadius, PxU32 height, PxVec3 dims, PxVec3 centerPos, PxMaterial* material, ShaderMaterial& mat);

	PxReal mAccumulatedTime;

	bool hadInteraction;
	PxU32 nbProjectiles;

};
#endif  // SCENE_BOXES_H

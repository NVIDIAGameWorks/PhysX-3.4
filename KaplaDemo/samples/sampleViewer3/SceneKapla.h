#ifndef SCENE_FRACTURE_H
#define SCENE_FRACTURE_H


#include "SampleViewerScene.h"
#include "Compound.h"
#include "Shader.h"

#include "foundation/PxVec3.h"

#include "PhysXMacros.h"
#include "PxFiltering.h"

extern int DEFAULT_SOLVER_ITERATIONS;

using namespace physx;

class GLUquadric;
class Particles;
class ShaderShadow;
class SimScene;
class Mesh;
class RegularCell3D;
class TerrainMesh;

// ---------------------------------------------------------------------
class SceneKapla : public SampleViewerScene
{
public:
	SceneKapla(PxPhysics* pxPhysics, PxCooking *pxCooking, bool isGrb,
		Shader *defaultShader, const char *resourcePath, float slowMotionFactor);

	virtual ~SceneKapla();

	virtual void preSim(float dt);
	virtual void postSim(float dt);

	virtual void handleMouseButton(int button, int state, int x, int y);
	virtual void handleMouseMotion(int x, int y);

	virtual void handleKeyDown(unsigned char key, int x, int y);
	virtual void handleKeyUp(unsigned char key, int x, int y);
	virtual void handleSpecialKey(unsigned char key, int x, int y);

	virtual void render(bool useShader);
	virtual void printPerfInfo();
	SimScene* getSimScene() {return mSimScene;}

	ShaderShadow* getSimSceneShader() {return mSimSceneShader;}

	virtual std::string getWeaponName();

	virtual bool isSceneKapla() {return true;}

	void shoot(PxVec3 &orig, PxVec3 &dir);

	void createCylindricalTower(PxU32 nbRadialPoints, PxReal maxRadius, PxReal minRadius, PxU32 height, PxVec3 dims, PxVec3 centerPos, PxMaterial* material, ShaderMaterial& mat,
		PxFilterData& simFilterData, PxFilterData& queryFilterData, PxReal density = 1.f, const bool useSweep = false, bool startAsleep = false);
	void createWall(PxReal radius, PxU32 height, PxVec3 dims, PxVec3 centerPos, PxMaterial* material, ShaderMaterial& mat, PxFilterData& simFilterData, PxFilterData& queryFilterData,
		PxReal density = 1.f);
	void createGroundPlane(PxFilterData simFilterData = PxFilterData(), PxVec3 pose = PxVec3(0,0,0), PxFilterData queryFilterData = PxFilterData());
	void createTerrain(const char* terrainName, const char* textureName, PxReal maxHeight = 30.f, bool invert = false);

	void createRagdoll(PxVec3 pos, PxVec3 vel, ShaderMaterial& mat);

	virtual void onInit(PxScene* pxScene);

	virtual void setScene(PxScene* pxScene);

	ShaderShadow* mSimSceneShader;

	Compound* createObject(const PxVec3 &pos, const PxVec3 &vel, const PxVec3 &omega,
		bool particles, const ShaderMaterial &mat, bool useSecondaryPattern = false, ShaderShadow* shader = NULL, int matID = 0, int surfMatID = 0);

	Compound* createObject(const PxTransform &pose, const PxVec3 &vel, const PxVec3 &omega,
		bool particles, const ShaderMaterial &mat, bool useSecondaryPattern = false, ShaderShadow* shader = NULL, int matID = 0, int surfMatID = 0);

	Compound* createObject(PxRigidDynamic* body, const ShaderMaterial &mat, ShaderShadow* shader = NULL, int matID = 0, int surfMatID = 0);

protected:

	void pickDraw();
	
	GLUquadric *mPickQuadric;

	SimScene *mSimScene;
	//RegularCell3D* mFluidSim;
	

	bool  mGunActive;
	int   mMouseX;
	int   mMouseY;
	bool  mMouseDown;

	int mFrameNr;

	enum Weapon {
		wpMeteor,
		wpBullet,
		wpBall,
		numWeapons,
	};

	static Weapon mWeapon;
	int mWeaponLifeTime;
	Mesh *mBulletMesh;
	Mesh *mAsteroidMesh;
	float mMeteorDistance;
	float mMeteorVelocity;
	float mMeteorSize;
	float mLaserImpulse;
	float mWeaponImpulse;

	PxReal mDefaultContactOffset;
	PxReal mDefaultRestOffset;

	int debugDrawNumConvexes;
	TerrainMesh* mTerrain;

	
};
#endif  // SCENE_BOXES_H

#ifndef SIM_SCENE
#define SIM_SCENE

#include "Shader.h"
#include "PxPhysics.h"
#include "PxCooking.h"
#include "PxSimulationEventCallback.h"
#include <PsArray.h>
#include "ConvexRenderer.h"

#include "SimSceneBase.h"

using namespace physx::fracture;

class TerrainMesh;
struct ParticlesDesc;
class Particles;
class RegularCell3D;
class Actor;

class SimScene : public base::SimScene
{
public:
	static SimScene* createSimScene(PxPhysics *pxPhysics, PxCooking *pxCooking, bool isGrbScene, float minConvexSize, PxMaterial* defaultMat, const char *resourcePath);
protected:
	SimScene(PxPhysics *pxPhysics, PxCooking *pxCooking, bool isGrbScene, float minConvexSize, PxMaterial* defaultMat, const char *resourcePath);
public:
	virtual ~SimScene();

	virtual void createSingletons();

	virtual base::Actor* createActor();
	virtual base::Convex* createConvex();
	virtual base::Compound* createCompound(PxReal contactOffset = 0.005f, PxReal restOffset = -0.001f);

	virtual void clear();

	virtual void profileBegin(const char* name);
	virtual void profileEnd(const char* name);

	bool addXml(const std::string &path, const std::string &filename, bool ignoreVisualMesh, float scale, const PxTransform &trans,
		Shader* defaultShader, const ShaderMaterial &defaultMat);

	void postSim(float dt, RegularCell3D* fluidSim);

	//Particles* getParticles() { return mParticles; }

	ConvexRenderer &getConvexRenderer() { return mConvexRenderer; }

	void draw(bool useShader);
	void setShaderMaterial(Shader* shader, const ShaderMaterial& mat) {this->mShader = shader; this->mShaderMat = mat;}
	void setFractureForceThreshold(float threshold) { mFractureForceThreshold = threshold; }
	void createRenderBuffers();
	void dumpSceneGeometry();

	void loadAndCreateTextureArrays();

	std::vector<std::string> diffuseTexNames;
	std::vector<std::string> bumpTexNames;
	std::vector<std::string> specularTexNames;
	std::vector<std::string> emissiveReflectSpecPowerTexNames;

protected:

	virtual void create3dTexture();
	virtual void updateConvexesTex();

	GLuint diffuseTexArray, bumpTexArray, specularTexArray, emissiveReflectSpecPowerTexArray;

	GLuint loadTextureArray(std::vector<std::string>& names);

	//Particles *mParticles;

	ConvexRenderer mConvexRenderer;

	Shader* mShader;
	ShaderMaterial mShaderMat;
};

#endif

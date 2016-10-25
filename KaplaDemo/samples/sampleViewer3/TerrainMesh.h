
#pragma once

#include "BmpFile.h"
#include "glmesh.h"
#include "Shader.h"
#include "PxCooking.h"
#include <vector>
using namespace std;
class TerrainMesh{
public:
	// Able to render mesh
	// Able to generate physX 
	PxRigidStatic* hfActor;
	PxTransform mHFPose;
	GLuint hfTexture;
	GLMesh* mMesh;
	PxScene* myScene;
	Shader* mShader;
	ShaderMaterial mMaterial;

	~TerrainMesh();
	TerrainMesh(PxPhysics& physics, PxCooking& cooking, PxScene& scene, PxMaterial& material, PxVec3 midPoint, const char* heightMapName, const char* textureName, 
		float hfScale, float maxHeight, Shader* shader, bool invert = false);
	void draw(bool useShader);


	float xStart;
	float zStart;
	float dx, idx;
	vector<float> terrainHeights;
	vector<PxVec3> terrainNormals;

	int width;
	int height;
	float getHeight(float x, float z);
	float getHeightNormal(float x, float z, PxVec3& normal);
	float getHeightNormalCondition(float x, float y, float z, PxVec3& normal);
};


#include "SceneKaplaArena.h"
#include "Convex.h"
#include "PxSimpleFactory.h"
#include "PxRigidStatic.h"
#include "PxShape.h"
#include "foundation/PxMathUtils.h"
#include "foundation/PxMat44.h"

#include <stdio.h>
#include <GL/glut.h>

#include "SimScene.h"
#include "CompoundCreator.h"
#include "TerrainMesh.h"

#include "PhysXMacros.h"

#include "MathUtils.h"

#include "PxRigidBodyExt.h"


extern bool LoadTexture(const char *filename, GLuint &texId, bool createMipmaps, GLuint type = GL_TEXTURE_2D, int *width = NULL, int *height = NULL);

// ----------------------------------------------------------------------------------------------
SceneKaplaArena::SceneKaplaArena(PxPhysics* pxPhysics, PxCooking *pxCooking, bool isGrb,
	Shader *defaultShader, const char *resourcePath, float slowMotionFactor) :
	SceneKapla(pxPhysics, pxCooking, isGrb, defaultShader, resourcePath, slowMotionFactor)
{

	//onInit(pxPhysics);
}

void SceneKaplaArena::onInit(PxScene* pxScene)
{
	SceneKapla::onInit(pxScene);
	createGroundPlane();

	const PxVec3 dims(0.08f, 0.25f, 1.0f);
	PxMaterial* DefaultMaterial = mPxPhysics->createMaterial(0.4f, 0.15f, 0.1f);

	ShaderMaterial mat;
	mat.init();

	const PxU32 nbOuterRadialLayouts = 384;
	const PxReal outerRadius = 40;

	PxFilterData queryFilterData;
	PxFilterData simFilterData;

	createCylindricalTower(nbOuterRadialLayouts, outerRadius, outerRadius, 8, dims, PxVec3(0.f, 0.f, 0.f), DefaultMaterial, mat, simFilterData, queryFilterData);
	createCylindricalTower(nbOuterRadialLayouts, outerRadius - 2.f, outerRadius - 2.f, 8, dims, PxVec3(0.f, 0.f, 0.f), DefaultMaterial, mat, simFilterData, queryFilterData);
}

#ifndef __GLMESH_H__
#define __GLMESH_H__
#include <GL/glew.h>

#include "PxPhysics.h"
#include <vector>
using namespace physx;
class GLMesh{
public:
	GLMesh(GLuint elementTypei = GL_TRIANGLES);
	~GLMesh();
	void draw();

	// For indices
	std::vector<PxU32> indices;
	std::vector<PxVec3> vertices;
	std::vector<PxVec3> normals;
	std::vector<PxVec3> colors;
	std::vector<float> texCoords; // treats as u v
	std::vector<PxVec3> tangents;
	std::vector<PxVec3> bitangents;

	// For raw
	std::vector<PxVec3> rawVertices;
	std::vector<PxVec3> rawNormals;

	void reset();
	void genVBOIBO();
	void updateVBOIBO(bool dynamicVB = true);
	void drawVBOIBO(bool enable = true, bool draw = true, bool disable = true, bool drawpoints = false);

	// For vertex buffer
	GLuint vbo;
	GLuint ibo;
	bool firstTimeBO;
	bool withTexture, withColor, withNormal, withTangent;
	GLuint elementType;


}; 
#endif

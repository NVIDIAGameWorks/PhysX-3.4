#ifndef MESH
#define MESH

#include <foundation/PxVec3.h>
#include <PsArray.h>
#include <PsAllocator.h>

#include "Shader.h"

#include "MeshBase.h"

using namespace physx::fracture;

class Mesh : public base::Mesh
{
public:
	bool loadFromObjFile(const std::string &filename);

	void setShader(Shader* shader, const ShaderMaterial &mat) { mShader = shader; mShaderMat = mat; }
	Shader* getShader() const { return mShader; }
	const ShaderMaterial& getShaderMat() { return mShaderMat; }
	void draw(bool useShader);
protected:
	// shading
	Shader *mShader;
	ShaderMaterial mShaderMat;
	Shader *mWrinkleShader;

	shdfnd::Array<std::string> mNames;
};

#endif
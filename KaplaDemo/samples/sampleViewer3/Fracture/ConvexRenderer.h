#ifndef CONVEX_RENDERER
#define CONVEX_RENDERER

#include "Convex.h"
#include <vector>

using namespace physx;

class Shader;

// ----------------------------------------------------------------------------
class ConvexRenderer
{
public:
	ConvexRenderer();
	~ConvexRenderer();

	void init();

	const static int maxVertsPerGroup = 100000;

	void setActive(bool active) { mActive = active; };

	void add(const base::Convex* convex, Shader* shader);
	void remove(const base::Convex* convex);

	void render();
	void setShaderMaterial(Shader* shader, const ShaderMaterial& mat) {this->mShader = shader; this->mShaderMat = mat;}
	void setTexArrays(unsigned int diffuse, unsigned int bump, unsigned int specular, unsigned int specPower) {
		mDiffuseTexArray = diffuse; mBumpTexArray = bump; 
		mSpecularTexArray = specular; mEmissiveReflectSpecPowerTexArray = specPower; }
	void setVolTex(unsigned int volTexi) { volTex = volTexi;}
private:
	void updateRenderBuffers();
	void updateTransformations();

	Shader* mShader;
	ShaderMaterial mShaderMat;

	struct ConvexGroup {
		void init() {
			numVertices = 0; numIndices = 0;
			VBO = 0; IBO = 0; matTex = 0;
			texSize = 0;
		}
		bool dirty;
		std::vector<const Convex*> convexes;

		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		std::vector<float> texCoords;

		int numVertices, numIndices;
		unsigned int VBO;
		unsigned int IBO;
		unsigned int matTex;
		int texSize;

		Shader* mShader;
	};

	std::vector<ConvexGroup*> mGroups;

	bool mActive;

	float mBumpTextureUVScale;
	float mExtraNoiseScale;
	float mRoughnessScale;

	unsigned int mDiffuseTexArray;
	unsigned int mBumpTexArray;
	unsigned int mSpecularTexArray;
	unsigned int mEmissiveReflectSpecPowerTexArray;
	unsigned int volTex;
};

#endif

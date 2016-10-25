#ifndef COMPOUND
#define COMPOUND

#include <foundation/PxVec3.h>
#include <foundation/PxPlane.h>
#include <foundation/PxBounds3.h>
#include <foundation/PxTransform.h>
#include <PsArray.h>
#include <PxRigidDynamic.h>

#include "Shader.h"

#include "CompoundBase.h"

using namespace physx;

class Convex;
class Particles;
class Mesh;
class SimScene;
class CompoundGeometry;
class XMLParser;

class ShaderShadow;

using namespace physx::fracture;

class Compound : public base::Compound
{
	friend class SimScene;
protected:
	Compound(SimScene* scene, PxReal contactOffset = 0.005f, PxReal restOffset = -0.001f):
		physx::fracture::base::Compound((base::SimScene*)scene,contactOffset,restOffset) {}
public:

	virtual void convexAdded(base::Convex* c, Shader* shader);
	virtual void convexRemoved(base::Convex* c);

	bool createFromXml(XMLParser *p, float scale, const PxTransform &trans, bool ignoreVisualMesh = false);

	void setShader(Shader* shader, const ShaderMaterial &mat) { mShader = shader; mShaderMat = mat; }
	Shader* getShader() const { return mShader; }
	const ShaderMaterial& getShaderMat() { return mShaderMat; }

	virtual void draw(bool useShader, bool debug = false);

	virtual void clear();

	virtual void copyShaders(base::Compound*);

protected:

	virtual float getSleepingThresholdRB();

	Shader *mShader;
	ShaderMaterial mShaderMat;
};

#endif

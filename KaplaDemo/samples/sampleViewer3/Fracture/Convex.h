#ifndef CONVEX
#define CONVEX

#include <PxPhysics.h>
#include <PxCooking.h>
#include <foundation/PxVec3.h>
#include <foundation/PxPlane.h>
#include <foundation/PxBounds3.h>
#include <foundation/PxTransform.h>
#include <PsArray.h>

#include "ConvexBase.h"

using namespace physx;

class physx::PxShape;
class physx::PxActor;
class physx::PxScene;
class physx::PxConvexMesh;

class Compound;
class CompoundGeometry;
class XMLParser;

using namespace physx::fracture;

class Convex : public base::Convex
{
	friend class SimScene;
protected:
	Convex(base::SimScene* scene): base::Convex(scene) {}
public:

	virtual void draw(bool debug = false);

	bool createFromXml(XMLParser *p, float scale, bool ignoreVisualMesh = false);
};

#endif

#include "RTdef.h"
#if RT_COMPILE
#ifndef COMPOUND_GEOMETRY_BASE
#define COMPOUND_GEOMETRY_BASE

#include <foundation/PxVec3.h>
#include <foundation/PxPlane.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

namespace physx
{
namespace fracture
{
namespace base
{

// -----------------------------------------------------------------------------------
class CompoundGeometry : public ::physx::shdfnd::UserAllocated
{
public:
	CompoundGeometry() {}
	virtual ~CompoundGeometry() {}
	bool loadFromFile(const char *filename);
	bool saveFromFile(const char *filename);
	void clear();
	void derivePlanes();

	virtual void debugDraw(int /*maxConvexes*/ = 0) const {}

	struct Convex {			// init using CompoundGeometry::initConvex()
		int firstVert;
		int numVerts;
		int firstNormal;	// one per face index!	If not provided (see face flags) face normal is used
		int firstIndex;
		int numFaces;
		int firstPlane;
		int firstNeighbor;
		int numNeighbors;
		float radius;
		bool isSphere;
	};

	void initConvex(Convex &c);

	shdfnd::Array<Convex> convexes;
	shdfnd::Array<PxVec3> vertices;
	shdfnd::Array<PxVec3> normals;	// one per face and vertex!
	// face size, face flags, id, id, .., face size, face flags, id ..
	shdfnd::Array<int> indices;
	shdfnd::Array<int> neighbors;

	shdfnd::Array<PxPlane> planes;  // derived for faster cuts

	enum FaceFlags {
		FF_OBJECT_SURFACE = 1,
		FF_HAS_NORMALS = 2,
		FF_INVISIBLE = 4,
		FF_NEW = 8,
	};
};

}
}
}

#endif
#endif
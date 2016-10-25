#ifndef COMPOUND_GEOMETRY
#define COMPOUND_GEOMETRY

#include <foundation/PxVec3.h>
#include <foundation/PxPlane.h>
#include <PsArray.h>

#include "CompoundGeometryBase.h"

using namespace physx;

class CompoundGeometry : public physx::fracture::base::CompoundGeometry
{
public:
	virtual void debugDraw(int maxConvexes = 0) const;
};

#endif
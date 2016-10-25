#ifndef POLYGON_TRIANGULATOR_H
#define POLYGON_TRIANGULATOR_H

#include <foundation/PxVec3.h>
#include <PsArray.h>

using namespace physx;

#include <math.h>

#include "PolygonTriangulatorBase.h"

using namespace physx::fracture;

class PolygonTriangulator : public base::PolygonTriangulator
{
	friend class SimScene;
protected:
	PolygonTriangulator(SimScene* scene): base::PolygonTriangulator((base::SimScene*)scene) {}
};

#endif

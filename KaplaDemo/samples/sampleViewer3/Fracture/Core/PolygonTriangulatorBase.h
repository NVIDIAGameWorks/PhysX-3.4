#include "RTdef.h"
#if RT_COMPILE
#ifndef POLYGON_TRIANGULATOR_BASE_H
#define POLYGON_TRIANGULATOR_BASE_H

#include <foundation/PxVec3.h>
#include <foundation/PxMath.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

namespace physx
{
namespace fracture
{
namespace base
{

// ------------------------------------------------------------------------------

class PolygonTriangulator : public ::physx::shdfnd::UserAllocated {
	friend class SimScene;
public: 
	// singleton pattern
	//static PolygonTriangulator* getInstance();
	//static void destroyInstance();

	void triangulate(const PxVec3 *points, int numPoints, const int *indices = NULL, PxVec3 *planeNormal = NULL);
	const shdfnd::Array<int> &getIndices() const { return mIndices; }

protected:
	void importPoints(const PxVec3 *points, int numPoints, const int *indices, PxVec3 *planeNormal, bool &isConvex);
	void clipEars();

	inline float cross(const PxVec3 &p0, const PxVec3 &p1);
	bool inTriangle(const PxVec3 &p, const PxVec3 &p0, const PxVec3 &p1, const PxVec3 &p2);

	PolygonTriangulator(SimScene* scene);
	virtual ~PolygonTriangulator();

	SimScene* mScene;

	shdfnd::Array<PxVec3> mPoints;
	shdfnd::Array<int> mIndices;

	struct Corner {
		int prev;
		int next;
		bool isEar;
		float angle;
	};
	shdfnd::Array<Corner> mCorners;
};

}
}
}

#endif
#endif

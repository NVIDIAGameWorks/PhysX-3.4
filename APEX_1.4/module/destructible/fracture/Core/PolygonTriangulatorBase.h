/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef POLYGON_TRIANGULATOR_BASE_H
#define POLYGON_TRIANGULATOR_BASE_H

#include <PxVec3.h>
#include <PsArray.h>
#include <PsUserAllocated.h>
#include <PxMath.h>

namespace nvidia
{
namespace fracture
{
namespace base
{
	class SimScene;

// ------------------------------------------------------------------------------

class PolygonTriangulator : public UserAllocated {
	friend class SimScene;
public: 
	// singleton pattern
	//static PolygonTriangulator* getInstance();
	//static void destroyInstance();

	void triangulate(const PxVec3 *points, int numPoints, const int *indices = NULL, PxVec3 *planeNormal = NULL);
	const nvidia::Array<int> &getIndices() const { return mIndices; }

protected:
	void importPoints(const PxVec3 *points, int numPoints, const int *indices, PxVec3 *planeNormal, bool &isConvex);
	void clipEars();

	inline float cross(const PxVec3 &p0, const PxVec3 &p1);
	bool inTriangle(const PxVec3 &p, const PxVec3 &p0, const PxVec3 &p1, const PxVec3 &p2);

	PolygonTriangulator(SimScene* scene);
	virtual ~PolygonTriangulator();

	SimScene* mScene;

	nvidia::Array<PxVec3> mPoints;
	nvidia::Array<int> mIndices;

	struct Corner {
		int prev;
		int next;
		bool isEar;
		float angle;
	};
	nvidia::Array<Corner> mCorners;
};

}
}
}

#endif
#endif

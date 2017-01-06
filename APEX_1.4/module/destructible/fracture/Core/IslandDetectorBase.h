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
#ifndef ISLAND_DETECTOR_BASE_H
#define ISLAND_DETECTOR_BASE_H

// Matthias Muller-Fischer

#include <PxVec3.h>
#include <PsArray.h>
#include "IceBoxPruningBase.h"
#include <PsUserAllocated.h>

namespace nvidia
{
namespace fracture
{
namespace base
{

class SimScene;
class Convex;

// ---------------------------------------------------------------------------------------
class IslandDetector : public UserAllocated {
	friend class SimScene;
public:
	// singleton pattern
	//static IslandDetector* getInstance();
	//static void destroyInstance();

	void detect(const nvidia::Array<Convex*> &convexes, bool computeFaceCoverage);

	struct Island {
		int size;
		int firstNr;
	};
	const nvidia::Array<Island> &getIslands() { return mIslands; }
	const nvidia::Array<int> &getIslandConvexes() { return mIslandConvexes; }

	const nvidia::Array<float> &getFaceCoverage() { return mFaceCoverage; }

	class Edge{
	public:
		int n0, n1;
		float area;
	};
	const nvidia::Array<Edge> &getNeighborEdges();

protected:
	IslandDetector(SimScene* scene);
	virtual ~IslandDetector(){}

	void createConnectivity(const nvidia::Array<Convex*> &convexes, bool computeFaceCoverage);
	static bool touching(const Convex *c0, int faceNr, const Convex *c1, float eps);
	void createIslands();

	float faceArea(const Convex *c, int faceNr);
	float faceIntersectionArea(const Convex *c0, int faceNr0, const Convex *c1, int faceNr1);

	bool axisOverlap(const Convex* c0, const Convex* c1, PxVec3 &dir);
	bool overlap(const Convex* c0, const Convex* c1);

	void addOverlapConnectivity(const nvidia::Array<Convex*> &convexes);

	SimScene* mScene;

	// auxiliary
	nvidia::Array<int> mFirstNeighbor;
	struct Neighbor {
		int convexNr;
		int next;
		float area;
	};
	nvidia::Array<Neighbor> mNeighbors;
	struct Face {
		int convexNr;
		int faceNr;
		float orderVal;
		int globalNr;
		bool operator < (const Face &f) const { return orderVal < f.orderVal; }
	};
	bool mNeigborPairsDirty;
	nvidia::Array<Edge> mNeighborPairs;
	nvidia::Array<float> mNeighborAreaPairs;
	nvidia::Array<Face> mFaces;
	nvidia::Array<int> mColors;
	nvidia::Array<int> mQueue;

	nvidia::Array<PxVec3> mCutPolys[2];

	// overlaps
	BoxPruning mBoxPruning;
	nvidia::Array<PxBounds3> mBounds;
	nvidia::Array<uint32_t> mPairs;

	// output
	nvidia::Array<Island> mIslands;
	nvidia::Array<int> mIslandConvexes;
	nvidia::Array<float> mFaceCoverage;
};

}
}
}

#endif
#endif
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
#ifndef DELAUNAY_2D_BASE_H
#define DELAUNAY_2D_BASE_H

#include <PxVec3.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

namespace nvidia
{
namespace fracture
{
namespace base
{

// ------------------------------------------------------------------------------

struct Point;
struct Edge;
struct Triangle;
class SimScene;

// ------------------------------------------------------------------------------

class Delaunay2d : public UserAllocated {
	friend class SimScene;
public: 
	// singleton pattern
	//static Delaunay2d* getInstance();
	//static void destroyInstance();

	void triangulate(const PxVec3 *vertices, int numVerts, int byteStride, bool removeFarVertices = true);
	const nvidia::Array<int> getIndices() const { return mIndices; }

	// voronoi mesh, needs triangle mesh
	void computeVoronoiMesh();
	
	struct Convex {
		int firstVert;
		int numVerts;
		int firstNeighbor;
		int numNeighbors;
	};

	const nvidia::Array<Convex> getConvexes() const { return mConvexes; }
	const nvidia::Array<PxVec3> getConvexVerts() const { return mConvexVerts; }
	const nvidia::Array<int> getConvexNeighbors() const { return mConvexNeighbors; }

protected:
	Delaunay2d(SimScene* scene);
	virtual ~Delaunay2d();

	void clear();
	void delaunayTriangulation();

	struct Edge {
		Edge() {}
		Edge(int np0, int np1) { p0 = np0; p1 = np1; }
		bool operator == (const Edge &e) {
			return (p0 == e.p0 && p1 == e.p1) || (p0 == e.p1 && p1 == e.p0);
		}
		int p0, p1;
	};

	struct Triangle {
		int p0, p1, p2;
		PxVec3 center;
		float circumRadiusSquared;
	};

	void addTriangle(int p0, int p1, int p2);
	void getCircumSphere(const PxVec3 &p0, const PxVec3 &p1, const PxVec3 &p2,
		PxVec3 &center, float &radiusSquared);

	SimScene* mScene;

	nvidia::Array<PxVec3> mVertices;
	nvidia::Array<int> mIndices;

	nvidia::Array<Triangle> mTriangles;
	nvidia::Array<Edge> mEdges;

	nvidia::Array<Convex> mConvexes;
	nvidia::Array<PxVec3> mConvexVerts;
	nvidia::Array<int> mConvexNeighbors;

	int mFirstFarVertex;
};

}
}
}

// ------------------------------------------------------------------------------


#endif
#endif

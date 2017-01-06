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
#ifndef MESH_CLIPPER_BASE_H
#define MESH_CLIPPER_BASE_H

// Matthias Muller-Fischer

#include <PxVec3.h>
#include <PsArray.h>
#include <PxTransform.h>
#include <PxBounds3.h>
#include <PsUserAllocated.h>

//#include "Vec3.h"
//#include "Bounds3.h"
//#include "Transform.h"

namespace nvidia
{
namespace fracture
{
namespace base
{
//using namespace ::M;
class Convex;
class SimScene;

// ---------------------------------------------------------------------------------------
class MeshClipper : public UserAllocated {
	friend class SimScene;
public:
	// singleton pattern
	//static MeshClipper* getInstance();
	//static void destroyInstance();

	void init(const Convex *meshConvex);
	bool clip(const Convex *clipConvex, const PxTransform &pxTrans, float texScale, bool splitIslands,
		bool &empty, bool &completelyInside);

	struct Mesh {
		void clear();
		nvidia::Array<PxVec3> vertices;
		nvidia::Array<PxVec3> normals;
		nvidia::Array<PxVec3> tangents;
		nvidia::Array<float> texCoords;
		nvidia::Array<int> polyStarts;
		nvidia::Array<int> polyIndices;
		nvidia::Array<int> polyNeighbors;
	};

	int getNumMeshes();
	const Mesh &getMesh(int meshNr);

	bool loadDump();

protected:
	MeshClipper(SimScene* scene);
	virtual ~MeshClipper() {}

	//static PxVec3 conv(const Vec3 &v);
	//static Vec3 conv(const PxVec3 &v);
	//static Transform conv(const PxTransform &t);
	//static Plane conv(const PxPlane &p);

	void createGrid();
	bool getGridCandidates(const PxBounds3 &bounds);

	void generateCandidates(const PxTransform &pxTrans);
	bool createInternalPolygons(const PxTransform &pxTrans);
	bool createFacePolys();
	bool testNeighbors(int meshNr);

	bool splitIslands();

	bool insideVisualMesh(const PxVec3 &pos) const;

	void computeClipFaceNeighbors();

	SimScene* mScene;

	// input
	const Convex *mMeshConvex;
	const Convex *mClipConvex;
	nvidia::Array<int> mClipFaceNeighbors;
	float mTexScale;

	// output
	nvidia::Array<Mesh> mOutMeshes;

	// auxiliaries
	struct PolyVert {
		void init() {
			pos = normal = tangent = PxVec3(0.0, 0.0, 0.0); u = 0.0f; v = 0.0f; 
			id = -1; edgeNeighbor = -1; edgeOnFace = false; nextVert = -1; 
		}
		PxVec3 pos;
		PxVec3 normal;
		PxVec3 tangent;
		double u, v;
		int id;
		int edgeNeighbor;
		bool edgeOnFace;
		int nextVert;
	};

	struct FaceVert {
		void init() {
			id = -1; prevFaceNr = -1; nextFaceNr = -1; neighborPolyNr = -1; neighborPos = -1; 
			nextVert = -1; visited = false;
		}
		int id;
		int prevFaceNr, nextFaceNr;
		int neighborPolyNr;
		int neighborPos;
		int nextVert;
		bool visited;
	};

	nvidia::Array<PxVec3> mMeshVertices;
	nvidia::Array<bool> mPolyInside;
	nvidia::Array<int>  mPolyCandidates;

	nvidia::Array<PolyVert> mClipPolyVerts[2];
	nvidia::Array<int> mClipPolyStarts[2];
	nvidia::Array<PolyVert> mCuts;
	nvidia::Array<int> mFaceVertNrs;
	nvidia::Array<FaceVert> mFaceVerts;
	nvidia::Array<PxBounds3> mPolyBounds;

	struct Edge {
		void init(int i0, int i1, int edgeNr, int faceNr) {
			if (i0 < i1) { this->i0 = i0; this->i1 = i1; }
			else         { this->i0 = i1; this->i1 = i0; }
			this->edgeNr = edgeNr; this->faceNr = faceNr;
		}
		bool operator == (const Edge &e) const { return i0 == e.i0 && i1 == e.i1; }
		bool operator < (const Edge &e) const { return i0 < e.i0 || (i0 == e.i0 && i1 < e.i1); }
		int i0, i1;
		int edgeNr, faceNr;
	};
	nvidia::Array<Edge> mEdges;

	// map from int to out mesh
	nvidia::Array<int> mFirstOutPoly;
	nvidia::Array<int> mNumOutPolys;
	nvidia::Array<int> mOutVertNr;
	nvidia::Array<int> mFirstOutFacePoly;
	nvidia::Array<int> mNumOutFacePolys;

	nvidia::Array<int> mFaceTypes;
	nvidia::Array<int> mFaceOfPoly;
	nvidia::Array<int> mFaceQueue;

	// split islands
	nvidia::Array<int> mColors;
	nvidia::Array<int> mQueue;
	nvidia::Array<int> mNewVertNr;
	nvidia::Array<int> mNewPolyNr;

	// grid
	nvidia::Array<int> mGridFirstPoly;
	nvidia::Array<int> mGridPolys;
	int mGridNumX, mGridNumY, mGridNumZ;
	float mGridSpacing;
	PxBounds3 mGridBounds;
	nvidia::Array<int> mGridPolyMarks;
	int mGridCurrentMark;

	// error handling
	bool check(bool test, const char *message);
	bool mDumpOnError;
};

}
}
}

#endif
#endif

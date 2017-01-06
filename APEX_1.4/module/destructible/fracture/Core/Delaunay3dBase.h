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
#ifndef DELAUNAY_3D_BASE_H
#define DELAUNAY_3D_BASE_H

// Matthias Muller-Fischer

#include <PxVec3.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

using namespace nvidia;

#include "CompoundGeometryBase.h"

namespace nvidia
{
namespace fracture
{
namespace base
{
	class SimScene;

// ---------------------------------------------------------------------------------------
class Delaunay3d : public UserAllocated {
	friend class SimScene;
public:
	// singleton pattern
	//static Delaunay3d* getInstance();
	//static void destroyInstance();

	// tetra mesh
	void tetrahedralize(const PxVec3 *vertices, int numVerts, int byteStride, bool removeExtraVerts = true);
	const nvidia::Array<int> getTetras() const { return mIndices; }

	// voronoi mesh, needs tetra mesh
	void computeVoronoiMesh();
	
	const CompoundGeometry &getGeometry() { return mGeom; }

protected:
	Delaunay3d(SimScene* scene): mScene(scene) {}
	virtual ~Delaunay3d() {}

	// ------------------------------------------------------
	struct Edge 
	{
		void init(int i0, int i1, int tetraNr, int neighborNr = -1) {
			this->tetraNr = tetraNr;
			this->neighborNr = neighborNr;
			if (i0 > i1) { this->i0 = i0; this->i1 = i1; }
			else { this->i0 = i1; this->i1 = i0; }
		}
		bool operator <(const Edge &e) const {
			if (i0 < e.i0) return true;
			if (i0 > e.i0) return false;
			if (i1 < e.i1) return true;
			if (i1 > e.i1) return false;
			return (neighborNr < e.neighborNr);
		}
		bool operator ==(Edge &e) const {
			return i0 == e.i0 && i1 == e.i1;
		}
		int i0, i1;
		int tetraNr;
		int neighborNr;
	};

	// ------------------------------------------------------
	struct Tetra 
	{
		void init(int i0, int i1, int i2, int i3) {
			ids[0] = i0; ids[1] = i1; ids[2] = i2; ids[3] = i3;
			neighborNrs[0] = neighborNrs[1] = neighborNrs[2] = neighborNrs[3] = -1;
			circumsphereDirty = true;
			center = PxVec3(0.0f, 0.0f, 0.0f);
			radiusSquared = 0.0f;
			deleted = false;
		}
		inline int& neighborOf(int i0, int i1, int i2) {
			if (ids[0] != i0 && ids[0] != i1 && ids[0] != i2) return neighborNrs[0]; 
			if (ids[1] != i0 && ids[1] != i1 && ids[1] != i2) return neighborNrs[1]; 
			if (ids[2] != i0 && ids[2] != i1 && ids[2] != i2) return neighborNrs[2]; 
			if (ids[3] != i0 && ids[3] != i1 && ids[3] != i2) return neighborNrs[3]; 
			return neighborNrs[0];
		}
		
		int ids[4];
		int neighborNrs[4];
		bool circumsphereDirty;
		PxVec3 center;
		float radiusSquared;
		bool deleted;

		static const int sideIndices[4][3];
	};

	// ------------------------------------------------------
	void clear();
	void delaunayTetrahedralization();

	int findSurroundingTetra(int startTetra, const PxVec3 &p) const;
	void updateCircumSphere(Tetra &tetra);
	bool pointInCircumSphere(Tetra &tetra, const PxVec3 &p);
	void retriangulate(int tetraNr, int vertNr);
	void compressTetrahedra(bool removeExtraVerts);

	SimScene* mScene;

	int mFirstFarVertex;
	int mLastFarVertex;
	nvidia::Array<PxVec3> mVertices;
	nvidia::Array<Tetra> mTetras;
	nvidia::Array<int> mIndices;

	CompoundGeometry mGeom;

	mutable nvidia::Array<int> mTetMarked;
	mutable int mTetMark;

};

}
}
}

#endif
#endif

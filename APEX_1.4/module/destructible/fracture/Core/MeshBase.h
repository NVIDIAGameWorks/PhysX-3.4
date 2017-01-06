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
#ifndef MESH_BASE
#define MESH_BASE

#include <PxVec3.h>
#include <PxVec2.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

namespace nvidia
{
namespace fracture
{
namespace base
{

// -----------------------------------------------------------------------------------
class Mesh : public UserAllocated
{
public:
	Mesh() {};
	virtual ~Mesh() {};

	const nvidia::Array<PxVec3> &getVertices() const { return mVertices; }
	const nvidia::Array<PxVec3> &getNormals() const { return mNormals; }
	const nvidia::Array<PxVec2> &getTexCoords() const { return mTexCoords; }
	const nvidia::Array<uint32_t> &getIndices() const { return mIndices; }
	const nvidia::Array<int> &getNeighbors() const { return mNeighbors; }

	struct SubMesh {
		void init() { /*name = "";*/ firstIndex = -1; numIndices = 0; }
		//std::string name;
		int firstIndex;
		int numIndices;
	};

	const nvidia::Array<SubMesh> &getSubMeshes() const { return mSubMeshes; }

	void normalize(const PxVec3 &center, float diagonal);
	void scale(float diagonal);
	void getBounds(PxBounds3 &bounds, int subMeshNr = -1) const;

	void flipV(); // flips v values to hand coordinate system change (Assumes normalized coordinates)

	bool computeNeighbors();
	bool computeWeldedNeighbors();

protected:
	void clear();
	void updateNormals();

	nvidia::Array<PxVec3> mVertices;
	nvidia::Array<PxVec3> mNormals;
	nvidia::Array<PxVec2> mTexCoords;

	nvidia::Array<SubMesh> mSubMeshes;
	nvidia::Array<uint32_t> mIndices;
	nvidia::Array<int> mNeighbors;
};

}
}
}

#endif
#endif
#include "RTdef.h"
#if RT_COMPILE
#ifndef MESH_BASE
#define MESH_BASE

#include <foundation/PxVec3.h>
#include <foundation/PxVec2.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

namespace physx
{
namespace fracture
{
namespace base
{

// -----------------------------------------------------------------------------------
class Mesh : public ::physx::shdfnd::UserAllocated
{
public:
	Mesh() {};
	virtual ~Mesh() {};

	const shdfnd::Array<PxVec3> &getVertices() const { return mVertices; }
	const shdfnd::Array<PxVec3> &getNormals() const { return mNormals; }
	const shdfnd::Array<PxVec2> &getTexCoords() const { return mTexCoords; }
	const shdfnd::Array<PxU32> &getIndices() const { return mIndices; }
	const shdfnd::Array<int> &getNeighbors() const { return mNeighbors; }

	struct SubMesh {
		void init() { /*name = "";*/ firstIndex = -1; numIndices = 0; }
		//std::string name;
		int firstIndex;
		int numIndices;
	};

	const shdfnd::Array<SubMesh> &getSubMeshes() const { return mSubMeshes; }

	void normalize(const PxVec3 &center, float diagonal);
	void scale(float diagonal);
	void getBounds(PxBounds3 &bounds, int subMeshNr = -1) const;

	void flipV(); // flips v values to hand coordinate system change (Assumes normalized coordinates)

	bool computeNeighbors();
	bool computeWeldedNeighbors();

protected:
	void clear();
	void updateNormals();

	shdfnd::Array<PxVec3> mVertices;
	shdfnd::Array<PxVec3> mNormals;
	shdfnd::Array<PxVec2> mTexCoords;

	shdfnd::Array<SubMesh> mSubMeshes;
	shdfnd::Array<PxU32> mIndices;
	shdfnd::Array<int> mNeighbors;
};

}
}
}

#endif
#endif
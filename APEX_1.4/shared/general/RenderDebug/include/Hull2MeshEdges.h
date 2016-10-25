#ifndef Hull2MESHEDGES_H

#define Hull2MESHEDGES_H

// This is a small code snippet which will take a convex hull described as an array of plane equations and, from that,
// produce either an edge list (for wireframe debug visualization) or a triangle mesh.

#include "PxVec3.h"
#include "PxPlane.h"

struct HullEdge
{
	physx::PxVec3	e0,e1;
};

struct HullMesh
{
	uint32_t		mVertexCount;
	uint32_t		mTriCount;
	const physx::PxVec3 *mVertices;
	const uint16_t	*mIndices;
};

class Hull2MeshEdges
{
public:
	// Convert convex hull into a list of edges.
	virtual const HullEdge *getHullEdges(uint32_t planeCount,		// Number of source planes
										const physx::PxPlane *planes,	// The array of plane equations
										uint32_t &edgeCount) = 0;	// Contains the number of edges built

	virtual bool getHullMesh(uint32_t planeCount,				// Number of source planes
							 const physx::PxPlane *planes,			// The array of plane equations
							 HullMesh &m) = 0;						// The triangle mesh produced

	virtual void release(void) = 0;
protected:
	virtual ~Hull2MeshEdges(void)
	{

	}
};

Hull2MeshEdges *createHull2MeshEdges(void);

#endif

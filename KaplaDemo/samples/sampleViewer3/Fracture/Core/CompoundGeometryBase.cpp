#include "RTdef.h"
#if RT_COMPILE
#include "CompoundGeometryBase.h"

namespace physx
{
namespace fracture
{
namespace base
{

// -------------------------------------------------------------------------------
void CompoundGeometry::clear()
{
	convexes.clear();
	vertices.clear();;
	normals.clear();
	indices.clear();
	neighbors.clear();
	planes.clear();
}

// -------------------------------------------------------------------------------
bool CompoundGeometry::loadFromFile(const char * /*filename*/)
{
	return true;
}

// -------------------------------------------------------------------------------
bool CompoundGeometry::saveFromFile(const char * /*filename*/)
{
	return true;
}

// -------------------------------------------------------------------------------
void CompoundGeometry::initConvex(Convex &c)
{
	c.firstVert = vertices.size();
	c.numVerts = 0;
	c.firstNormal = normals.size();
	c.firstIndex = indices.size();
	c.numFaces = 0;
	c.firstPlane = 0;
	c.firstNeighbor = neighbors.size();
	c.numNeighbors = 0;
	c.radius = 0.f;
	c.isSphere = false;
}

// -------------------------------------------------------------------------------
void CompoundGeometry::derivePlanes()
{
	planes.clear();
	PxPlane p;

	for (int i = 0; i < (int)convexes.size(); i++) {
		Convex &c = convexes[i];
		c.firstPlane = planes.size();
		int *ids = &indices[c.firstIndex];
		PxVec3 *verts = &vertices[c.firstVert];
		for (int j = 0; j < c.numFaces; j++) {
			int num = *ids++;
			*ids++; //int flags = *ids++;
			if (num < 3) 
				p = PxPlane(1.0f, 0.0f, 0.0f, 0.0f);
			else {
				p.n = PxVec3(0.0f, 0.0f, 0.0f);
				for (int k = 1; k < num-1; k++) {
					const PxVec3 &p0 = verts[ids[0]];
					const PxVec3 &p1 = verts[ids[k]];
					const PxVec3 &p2 = verts[ids[k+1]];
					p.n += (p1-p0).cross(p2-p1);
				}
				p.n.normalize();
				p.d = p.n.dot(verts[ids[0]]);
			}
			planes.pushBack(p);
			ids += num;
		}
	}
}

}
}
}
#endif
#include "CompoundGeometry.h"

#define DEBUG_DRAW 1

#if DEBUG_DRAW
#include <windows.h>
#include <GL/gl.h>
#endif

// -------------------------------------------------------------------------------
void CompoundGeometry::debugDraw(int maxConvexes) const
{
#if DEBUG_DRAW
	const bool drawConvexes = true;
	const bool drawWireframe = true;
	const bool drawClipped = false;
	const float clipMaxX = 0.0f;

	if (drawConvexes) {
		float s = drawClipped ? 1.0f : 0.95f;

		if (drawWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		PxVec3 n;
		int num = convexes.size();
		if (maxConvexes > 0 && maxConvexes < num)
			num = maxConvexes;

		for (int i = 0; i < num; i++) {

			switch (i%7) {
				case 0 : glColor3f(1.0f, 0.0f, 0.0f); break;
				case 1 : glColor3f(0.0f, 1.0f, 0.0f); break;
				case 2 : glColor3f(1.0f, 0.0f, 1.0f); break;
				case 3 : glColor3f(0.0f, 1.0f, 1.0f); break;
				case 4 : glColor3f(1.0f, 0.0f, 1.0f); break;
				case 5 : glColor3f(1.0f, 1.0f, 0.0f); break;
				case 6 : glColor3f(1.0f, 1.0f, 1.0f); break;
			};

			const Convex &c = convexes[i];
			const PxVec3 *verts = &vertices[c.firstVert];
			PxVec3 center;
			center = PxVec3(0.0f, 0.0f, 0.0f);
			for (int j = 0; j < c.numVerts; j++)
				center += verts[j];
			center /= (float)c.numVerts;
			if (drawClipped && center.x > clipMaxX)
				continue;

			const int *ids = &indices[c.firstIndex];
			for (int j = 0; j < c.numFaces; j++) {
				int numVerts = *ids++;
				int flags = *ids++;

				n = (verts[ids[1]] - verts[ids[0]]).cross(verts[ids[2]] - verts[ids[0]]);
				n.normalize();
				glNormal3f(n.x, n.y, n.z);

				glBegin(GL_POLYGON);
				for (int k = 0; k < numVerts; k++) {
					PxVec3 p = verts[ids[k]];
					p = center + (p - center) * s;
					glVertex3f(p.x, p.y, p.z);
				}
				glEnd();

				ids += numVerts;
			}
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
}
#include "CompoundCreator.h"
#include <algorithm>
#include <assert.h>
#include "CompoundGeometry.h"

#define DEBUG_DRAW 1

#if DEBUG_DRAW
#include <windows.h>
#include <GL/gl.h>
#endif

// -----------------------------------------------------------------------------
void CompoundCreator::debugDraw()
{
#if DEBUG_DRAW

	const bool drawEdges = false;

	if (drawEdges) {
		glBegin(GL_LINES);
		for (int i = 0; i < (int)mTetEdges.size(); i++) {
			TetEdge &e = mTetEdges[i];
			if (e.onSurface)
				glColor3f(1.0f, 0.0f, 0.0f);
			else
				glColor3f(1.0f, 1.0f, 0.0f);

			PxVec3 &p0 = mTetVertices[e.i0];
			PxVec3 &p1 = mTetVertices[e.i1];
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p1.x, p1.y, p1.z);
		}
		glEnd();
	}
	
#endif
}
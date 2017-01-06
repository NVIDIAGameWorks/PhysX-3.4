/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef APEX_CSG_HULL_H
#define APEX_CSG_HULL_H

#include "ApexUsingNamespace.h"
#include "authoring/ApexCSGMath.h"
#include "PsArray.h"
#include "PxFileBuf.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace ApexCSG
{

/* Convex hull that handles unbounded sets. */

class Hull
{
public:
	struct Edge
	{
		uint32_t	m_indexV0;
		uint32_t	m_indexV1;
		uint32_t	m_indexF1;
		uint32_t	m_indexF2;
	};

	struct EdgeType
	{
		enum Enum
		{
			LineSegment,
			Ray,
			Line
		};
	};

	PX_INLINE					Hull()
	{
		setToAllSpace();
	}
	PX_INLINE					Hull(const Hull& geom)
	{
		*this = geom;
	}

	PX_INLINE	void			setToAllSpace()
	{
		clear();
		allSpace = true;
	}
	PX_INLINE	void			setToEmptySet()
	{
		clear();
		emptySet = true;
	}

	void			intersect(const Plane& plane, Real distanceTol);

	PX_INLINE	void			transform(const Mat4Real& tm, const Mat4Real& cofTM);

	PX_INLINE	uint32_t	getFaceCount() const
	{
		return faces.size();
	}
	PX_INLINE	const Plane&	getFace(uint32_t faceIndex) const
	{
		return faces[faceIndex];
	}

	PX_INLINE	uint32_t	getEdgeCount() const
	{
		return edges.size();
	}
	PX_INLINE	const Edge&		getEdge(uint32_t edgeIndex) const
	{
		return edges[edgeIndex];
	}

	PX_INLINE	uint32_t	getVertexCount() const
	{
		return vertexCount;
	}
	PX_INLINE	const Pos&		getVertex(uint32_t vertexIndex) const
	{
		return *(const Pos*)(vectors.begin() + vertexIndex);
	}

	PX_INLINE	bool			isEmptySet() const
	{
		return emptySet;
	}
	PX_INLINE	bool			isAllSpace() const
	{
		return allSpace;
	}

	Real			calculateVolume() const;

	// Edge accessors
	PX_INLINE	EdgeType::Enum	getType(const Edge& edge) const
	{
		return (EdgeType::Enum)((uint32_t)(edge.m_indexV0 >= vertexCount) + (uint32_t)(edge.m_indexV1 >= vertexCount));
	}
	PX_INLINE	const Pos&		getV0(const Edge& edge)	const
	{
		return *(Pos*)(vectors.begin() + edge.m_indexV0);
	}
	PX_INLINE	const Pos&		getV1(const Edge& edge)	const
	{
		return *(Pos*)(vectors.begin() + edge.m_indexV1);
	}
	PX_INLINE	const Dir&		getDir(const Edge& edge)	const
	{
		PX_ASSERT(edge.m_indexV1 >= vertexCount);
		return *(Dir*)(vectors.begin() + edge.m_indexV1);
	}
	PX_INLINE	uint32_t	getF1(const Edge& edge)	const
	{
		return edge.m_indexF1;
	}
	PX_INLINE	uint32_t	getF2(const Edge& edge)	const
	{
		return edge.m_indexF2;
	}

	// Serialization
	void			serialize(physx::PxFileBuf& stream) const;
	void			deserialize(physx::PxFileBuf& stream, uint32_t version);

protected:
	PX_INLINE	void			clear();

	bool			testConsistency(Real distanceTol, Real angleTol) const;

	// Faces
	physx::Array<Plane>		faces;
	physx::Array<Edge>		edges;
	physx::Array<Vec4Real>	vectors;
	uint32_t						vertexCount;	// vectors[i], i >= vertexCount, are used to store vectors for ray and line edges
	bool						allSpace;
	bool						emptySet;
};

PX_INLINE void
Hull::transform(const Mat4Real& tm, const Mat4Real& cofTM)
{
	for (uint32_t i = 0; i < faces.size(); ++i)
	{
		Plane& face = faces[i];
		face = cofTM * face;
		face.normalize();
	}

	for (uint32_t i = 0; i < vectors.size(); ++i)
	{
		Vec4Real& vector = vectors[i];
		vector = tm * vector;
	}
}

PX_INLINE void
Hull::clear()
{
	vectors.reset();
	edges.reset();
	faces.reset();
	vertexCount = 0;
	allSpace = false;
	emptySet = false;
}


};	// namespace ApexCSG

#endif

#endif // #define APEX_CSG_HULL_H

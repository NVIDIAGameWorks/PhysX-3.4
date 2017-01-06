/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef APEX_CSG_SERIALIZATION_H
#define APEX_CSG_SERIALIZATION_H

#include "ApexUsingNamespace.h"
#include "ApexSharedUtils.h"
#include "ApexStream.h"
#include "authoring/ApexCSGDefs.h"
#include "authoring/ApexCSGHull.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace nvidia
{
namespace apex
{

/* Version for serialization */
struct Version
{
	enum Enum
	{
		Initial = 0,
		RevisedMeshTolerances,
		UsingOnlyPositionDataInVertex,
		SerializingTriangleFrames,
		UsingGSA,
		SerializingMeshBounds,
		AddedInternalTransform,
		IncidentalMeshDistinction,

		Count,
		Current = Count - 1
	};
};


// Vec<T,D>
template<typename T, int D>
PX_INLINE physx::PxFileBuf&
operator << (physx::PxFileBuf& stream, const ApexCSG::Vec<T, D>& v)
{
	for (uint32_t i = 0; i < D; ++i)
	{
		stream << v[(int32_t)i];
	}
	return stream;
}

template<typename T, int D>
PX_INLINE physx::PxFileBuf&
operator >> (physx::PxFileBuf& stream, ApexCSG::Vec<T, D>& v)
{
	for (uint32_t i = 0; i < D; ++i)
	{
		stream >> v[(int32_t)i];
	}

	return stream;
}


// Edge
PX_INLINE void
serialize(physx::PxFileBuf& stream, const ApexCSG::Hull::Edge& e)
{
	stream << e.m_indexV0 << e.m_indexV1 << e.m_indexF1 << e.m_indexF2;
}

PX_INLINE void
deserialize(physx::PxFileBuf& stream, uint32_t version, ApexCSG::Hull::Edge& e)
{
	PX_UNUSED(version);	// Initial

	stream >> e.m_indexV0 >> e.m_indexV1 >> e.m_indexF1 >> e.m_indexF2;
}


// Region
PX_INLINE void
serialize(physx::PxFileBuf& stream, const ApexCSG::Region& r)
{
	stream << r.side;
}

PX_INLINE void
deserialize(physx::PxFileBuf& stream, uint32_t version, ApexCSG::Region& r)
{
	if (version < Version::UsingGSA)
	{
		ApexCSG::Hull hull;
		hull.deserialize(stream, version);
	}

	stream >> r.side;
}


// Surface
PX_INLINE void
serialize(physx::PxFileBuf& stream, const ApexCSG::Surface& s)
{
	stream << s.planeIndex;
	stream << s.triangleIndexStart;
	stream << s.triangleIndexStop;
	stream << s.totalTriangleArea;
}

PX_INLINE void
deserialize(physx::PxFileBuf& stream, uint32_t version, ApexCSG::Surface& s)
{
	PX_UNUSED(version);	// Initial

	stream >> s.planeIndex;
	stream >> s.triangleIndexStart;
	stream >> s.triangleIndexStop;
	stream >> s.totalTriangleArea;
}


// Triangle
PX_INLINE void
serialize(physx::PxFileBuf& stream, const ApexCSG::Triangle& t)
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		stream << t.vertices[i];
	}
	stream << t.submeshIndex;
	stream << t.smoothingMask;
	stream << t.extraDataIndex;
	stream << t.normal;
	stream << t.area;
}

PX_INLINE void
deserialize(physx::PxFileBuf& stream, uint32_t version, ApexCSG::Triangle& t)
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		stream >> t.vertices[i];
		if (version < Version::UsingOnlyPositionDataInVertex)
		{
			ApexCSG::Dir v;
			stream >> v;	// normal
			stream >> v;	// tangent
			stream >> v;	// binormal
			ApexCSG::UV uv;
			for (uint32_t uvN = 0; uvN < VertexFormat::MAX_UV_COUNT; ++uvN)
			{
				stream >> uv;	// UVs
			}
			ApexCSG::Color c;
			stream >> c;	// color
		}
	}
	stream >> t.submeshIndex;
	stream >> t.smoothingMask;
	stream >> t.extraDataIndex;
	stream >> t.normal;
	stream >> t.area;
}

// Interpolator
PX_INLINE void
serialize(physx::PxFileBuf& stream, const ApexCSG::Interpolator& t)
{
	t.serialize(stream);
}

PX_INLINE void
deserialize(physx::PxFileBuf& stream, uint32_t version, ApexCSG::Interpolator& t)
{
	t.deserialize(stream, version);
}

}
};	// namespace nvidia::apex

#endif

#endif // #define APEX_CSG_SERIALIZATION_H

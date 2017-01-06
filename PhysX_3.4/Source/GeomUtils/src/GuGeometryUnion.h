// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_GEOMETRY_UNION_H
#define GU_GEOMETRY_UNION_H

#include "foundation/PxBounds3.h"
#include "GuSIMDHelpers.h"
#include <stddef.h>
#include "PxBoxGeometry.h"
#include "PxSphereGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxPlaneGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxTriangleMeshGeometry.h"
#include "PxHeightFieldGeometry.h"
#include "PsAllocator.h"
#include "GuBox.h"
#include "GuCenterExtents.h"
#include "GuSphere.h"
#include "GuCapsule.h"

namespace physx
{

namespace Gu
{
	struct ConvexHullData;
	class TriangleMesh;
	struct HeightFieldData;
	class GeometryUnion;
}


//
// Summary of our material approach:
//
// On the API level, materials are accessed via pointer. Internally we store indices into the material table.
// The material table is stored in the SDK and the materials are shared among scenes. To make this threadsafe,
// we have the following approach:
//
// - Every scene has a copy of the SDK master material table
// - At the beginning of a simulation step, the scene material table gets synced to the master material table.
// - While the simulation is running, the scene table does not get touched.
// - Each shape stores the indices of its material(s). When the simulation is not running and a user requests the
//   materials of the shape, the indices are used to fetch the material from the master material table. When the
//   the simulation is running then the same indices are used internally to fetch the materials from the scene
//   material table. If a user changes the materials of a shape while the simulation is running, the index list
//   will not get touched, instead the new materials get buffered and synced at the end of the simulation.
// - This whole scheme only works as long as the position of a material in the material table does not change
//   when other materials get deleted/inserted. The data structure of the material table makes sure that is the case.
//

struct PX_PHYSX_COMMON_API MaterialIndicesStruct
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
// PX_SERIALIZATION
	MaterialIndicesStruct(const PxEMPTY)	{}
	static void getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	MaterialIndicesStruct() : indices(NULL), numIndices(0)
	{
	}

	~MaterialIndicesStruct()
	{
	}

	void allocate(PxU16 size)
	{
		indices = reinterpret_cast<PxU16*>(PX_ALLOC(sizeof(PxU16) * size, "MaterialIndicesStruct::allocate"));
		numIndices = size;
	}

	void deallocate()
	{
		PX_FREE(indices);
		numIndices = 0;
	}
	PxU16*	indices; // the remap table for material index
	PxU16	numIndices; // the size of the remap table
	PxU16	pad; // pad for serialization
#if PX_P64_FAMILY
	PxU32	pad64; // pad for serialization
#endif
};

struct PxConvexMeshGeometryLL: public PxConvexMeshGeometry
{
	const Gu::ConvexHullData*			hullData;
	bool								gpuCompatible;
};

struct PxTriangleMeshGeometryLL: public PxTriangleMeshGeometry
{
	const Gu::TriangleMesh*				meshData;
	const PxU16*						materialIndices;
	MaterialIndicesStruct				materials;	
};

struct PxHeightFieldGeometryLL : public PxHeightFieldGeometry
{
	const Gu::HeightFieldData*			heightFieldData;			
	MaterialIndicesStruct				materials;
};

// We sometimes overload capsule code for spheres, so every sphere should have 
// valid capsule data (height = 0). This is preferable to a typedef so that we
// can maintain traits separately for a sphere, but some care is required to deal
// with the fact that when a reference to a capsule is extracted, it may have its
// type field set to eSPHERE

template <typename T>
struct PxcGeometryTraits
{
	enum {TypeID = PxGeometryType::eINVALID };
};
template <typename T> struct PxcGeometryTraits<const T> { enum { TypeID = PxcGeometryTraits<T>::TypeID }; };

template <> struct PxcGeometryTraits<PxBoxGeometry>				{ enum { TypeID = PxGeometryType::eBOX }; };
template <> struct PxcGeometryTraits<PxSphereGeometry>			{ enum { TypeID = PxGeometryType::eSPHERE }; };
template <> struct PxcGeometryTraits<PxCapsuleGeometry>			{ enum { TypeID = PxGeometryType::eCAPSULE }; };
template <> struct PxcGeometryTraits<PxPlaneGeometry>			{ enum { TypeID = PxGeometryType::ePLANE }; };
template <> struct PxcGeometryTraits<PxConvexMeshGeometryLL>	{ enum { TypeID = PxGeometryType::eCONVEXMESH }; };
template <> struct PxcGeometryTraits<PxTriangleMeshGeometryLL>	{ enum { TypeID = PxGeometryType::eTRIANGLEMESH }; };
template <> struct PxcGeometryTraits<PxHeightFieldGeometryLL> 	{ enum { TypeID = PxGeometryType::eHEIGHTFIELD }; };
template<class T> PX_CUDA_CALLABLE PX_FORCE_INLINE void checkType(const Gu::GeometryUnion& geometry);
template<> PX_CUDA_CALLABLE PX_INLINE void checkType<PxCapsuleGeometry>(const Gu::GeometryUnion& geometry);
template<> PX_CUDA_CALLABLE PX_INLINE void checkType<const PxCapsuleGeometry>(const Gu::GeometryUnion& geometry);


namespace Gu
{

class InvalidGeometry : public PxGeometry
{
public:
	PX_CUDA_CALLABLE PX_FORCE_INLINE InvalidGeometry() : PxGeometry(PxGeometryType::eINVALID) {}
};

class PX_PHYSX_COMMON_API GeometryUnion
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
	GeometryUnion(const PxEMPTY)	{}
	static	void	getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	PX_CUDA_CALLABLE PX_FORCE_INLINE						GeometryUnion()						{ reinterpret_cast<InvalidGeometry&>(mGeometry) = InvalidGeometry(); }
	PX_CUDA_CALLABLE PX_FORCE_INLINE						GeometryUnion(const PxGeometry& g)	{ set(g);	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxGeometry&		getGeometry()				const	{ return reinterpret_cast<const PxGeometry&>(mGeometry);			}
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxGeometryType::Enum	getType()					const	{ return reinterpret_cast<const PxGeometry&>(mGeometry).getType();	}

	PX_CUDA_CALLABLE void	set(const PxGeometry& g);

	template<class Geom> PX_CUDA_CALLABLE PX_FORCE_INLINE Geom& get()
	{
		checkType<Geom>(*this);
		return reinterpret_cast<Geom&>(mGeometry);
	}

	template<class Geom> PX_CUDA_CALLABLE PX_FORCE_INLINE const Geom& get() const
	{
		checkType<Geom>(*this);
		return reinterpret_cast<const Geom&>(mGeometry);
	}

private:

	union {
		void*	alignment;	// PT: Makes sure the class is at least aligned to pointer size. See DE6803. 
		PxU8	box[sizeof(PxBoxGeometry)];
		PxU8	sphere[sizeof(PxSphereGeometry)];
		PxU8	capsule[sizeof(PxCapsuleGeometry)];
		PxU8	plane[sizeof(PxPlaneGeometry)];
		PxU8	convex[sizeof(PxConvexMeshGeometryLL)];
		PxU8	mesh[sizeof(PxTriangleMeshGeometryLL)];
		PxU8	heightfield[sizeof(PxHeightFieldGeometryLL)];
		PxU8	invalid[sizeof(InvalidGeometry)];
	} mGeometry;
};
}


template<class T> PX_CUDA_CALLABLE PX_FORCE_INLINE void checkType(const Gu::GeometryUnion& geometry)
{
	PX_ASSERT(PxU32(geometry.getType()) == PxU32(PxcGeometryTraits<T>::TypeID));
	PX_UNUSED(geometry);
}

template<> PX_CUDA_CALLABLE PX_FORCE_INLINE void checkType<PxCapsuleGeometry>(const Gu::GeometryUnion& geometry)
{
	PX_ASSERT(geometry.getType() == PxGeometryType::eCAPSULE || geometry.getType() == PxGeometryType::eSPHERE);
	PX_UNUSED(geometry);
}

template<> PX_CUDA_CALLABLE PX_FORCE_INLINE void checkType<const PxCapsuleGeometry>(const Gu::GeometryUnion& geometry)
{
	PX_ASSERT(geometry.getType()== PxGeometryType::eCAPSULE || geometry.getType() == PxGeometryType::eSPHERE);
	PX_UNUSED(geometry);
}

// the shape structure relies on punning capsules and spheres 
PX_COMPILE_TIME_ASSERT(PX_OFFSET_OF(PxCapsuleGeometry, radius) == PX_OFFSET_OF(PxSphereGeometry, radius));
}

#endif

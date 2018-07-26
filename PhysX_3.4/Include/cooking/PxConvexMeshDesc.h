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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_COLLISION_NXCONVEXMESHDESC
#define PX_COLLISION_NXCONVEXMESHDESC
/** \addtogroup cooking
@{
*/

#include "foundation/PxVec3.h"
#include "foundation/PxFlags.h"
#include "common/PxCoreUtilityTypes.h"
#include "geometry/PxConvexMesh.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief Flags which describe the format and behavior of a convex mesh.
*/
struct PxConvexFlag
{
	enum Enum
	{
		/**
		Denotes the use of 16-bit vertex indices in PxConvexMeshDesc::triangles or PxConvexMeshDesc::polygons.
		(otherwise, 32-bit indices are assumed)
		@see #PxConvexMeshDesc.indices
		*/
		e16_BIT_INDICES		=	(1<<0),

		/**
		Automatically recomputes the hull from the vertices. If this flag is not set, you must provide the entire geometry manually.

		\note There are two different algorithms for hull computation, please see PxConvexMeshCookingType. 

		@see PxConvexMeshCookingType
		*/
		eCOMPUTE_CONVEX		=	(1<<1),	

		/**
		\brief Checks and removes almost zero-area triangles during convex hull computation. 
		The rejected area size is specified in PxCookingParams::areaTestEpsilon

		\note This flag is only used in combination with eCOMPUTE_CONVEX.

		\note If this flag is used in combination with eINFLATE_CONVEX, the newly added triangles 
		by the inflation algorithm are not checked (size of the triangles depends on PxCooking::skinWidth).  

		@see PxCookingParams PxCookingParams::areaTestEpsilon
		*/		
		eCHECK_ZERO_AREA_TRIANGLES		=	(1<<2),

		/**
		\brief Inflates the convex points according to skin width.

		\note eINFLATE_CONVEX flag has been deprecated. The flag is automatically used when
		PxConvexMeshCookingType::eINFLATION_INCREMENTAL_HULL is set. The default algorithm 
		PxConvexMeshCookingType::eQUICK_HULL ignores this flag, inflation is not used. 

		@see PxCookingParams PxConvexMeshCookingType
		*/
		PX_DEPRECATED eINFLATE_CONVEX		=	(1<<3),

		/**
		\brief Quantizes the input vertices using the k-means clustering

		\note The input vertices are quantized to PxConvexMeshDesc::quantizedCount
		see http://en.wikipedia.org/wiki/K-means_clustering

		*/
		eQUANTIZE_INPUT = (1 << 4),

		/**
		\brief Disables the convex mesh validation to speed-up hull creation. Please use separate validation
		function in checked/debug builds. Creating a convex mesh with invalid input data without prior validation
		may result in undefined behavior. 

		@see PxCooking::validateConvexMesh
		*/
		eDISABLE_MESH_VALIDATION = (1 << 5),

		/**
		\brief Enables plane shifting vertex limit algorithm.

		Plane shifting is an alternative algorithm for the case when the computed hull has more vertices 
		than the specified vertex limit.

		The default algorithm computes the full hull, and an OBB around the input vertices. This OBB is then sliced
		with the hull planes until the vertex limit is reached.The default algorithm requires the vertex limit 
		to be set to at least 8, and typically produces results that are much better quality than are produced 
		by plane shifting.

		When plane shifting is enabled, the hull computation stops when vertex limit is reached. The hull planes
		are then shifted to contain all input vertices, and the new plane intersection points are then used to 
		generate the final hull with the given vertex limit.Plane shifting may produce sharp edges to vertices 
		very far away from the input cloud, and does not guarantee that all input vertices are inside the resulting
		hull.However, it can be used with a vertex limit as low as 4.
		*/
		ePLANE_SHIFTING = (1 << 6),

		/**
		\brief Inertia tensor computation is faster using SIMD code, but the precision is lower, which may result 
		in incorrect inertia for very thin hulls.
		*/
		eFAST_INERTIA_COMPUTATION = (1 << 7),

		/**
		\brief Convex hulls are created with respect to GPU simulation limitations. Vertex limit is set to 64 and
		vertex limit per face is internally set to 32.
		\note Can be used only with eCOMPUTE_CONVEX flag.
		*/
		eGPU_COMPATIBLE = (1 << 8),

		/**
		\brief Convex hull input vertices are shifted to be around origin to provide better computation stability.
		It is recommended to provide input vertices around the origin, otherwise use this flag to improve 
		numerical stability.
		\note Is used only with eCOMPUTE_CONVEX flag.
		*/
		eSHIFT_VERTICES = (1 << 9)
	};
};

/**
\brief collection of set bits defined in PxConvexFlag.

@see PxConvexFlag
*/
typedef PxFlags<PxConvexFlag::Enum,PxU16> PxConvexFlags;
PX_FLAGS_OPERATORS(PxConvexFlag::Enum,PxU16)

/**
\brief Descriptor class for #PxConvexMesh.
\note The number of vertices and the number of convex polygons in a cooked convex mesh is limited to 256.

@see PxConvexMesh PxConvexMeshGeometry PxShape PxPhysics.createConvexMesh()

*/
class PxConvexMeshDesc
{
public:

	/**
	\brief Vertex positions data in PxBoundedData format.

	<b>Default:</b> NULL
	*/
	PxBoundedData points;

	/**
	\brief Polygons data in PxBoundedData format.
	<p>Pointer to first polygon. </p>

	<b>Default:</b> NULL	

	@see PxHullPolygon
	*/
	PxBoundedData polygons;

	/**
	\brief Polygon indices data in PxBoundedData format.
	<p>Pointer to first index.</p>

	<b>Default:</b> NULL	

	<p>This is declared as a void pointer because it is actually either an PxU16 or a PxU32 pointer.</p>

	@see PxHullPolygon PxConvexFlag::e16_BIT_INDICES
	*/
	PxBoundedData indices;

	/**
	\brief Flags bits, combined from values of the enum ::PxConvexFlag

	<b>Default:</b> 0
	*/
	PxConvexFlags flags;

	/**
	\brief Limits the number of vertices of the result convex mesh. Hard maximum limit is 256
	and minimum limit is 4 if PxConvexFlag::ePLANE_SHIFTING is used, otherwise the minimum
	limit is 8.

	\note Vertex limit is only used when PxConvexFlag::eCOMPUTE_CONVEX is specified.
	\note The please see PxConvexFlag::ePLANE_SHIFTING for algorithm explanation

	@see PxConvexFlag::ePLANE_SHIFTING

	<b>Range:</b> [4, 255]<br>
	<b>Default:</b> 255
	*/
	PxU16 vertexLimit;	

	/**
	\brief Maximum number of vertices after quantization. The quantization is done during the vertex cleaning phase. 
	The quantization is applied when PxConvexFlag::eQUANTIZE_INPUT is specified.

	@see PxConvexFlag::eQUANTIZE_INPUT

	<b>Range:</b> [4, 65535]<br>
	<b>Default:</b> 255
	*/
	PxU16 quantizedCount;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE PxConvexMeshDesc();
	/**
	\brief (re)sets the structure to the default.	
	*/
	PX_INLINE void setToDefault();
	/**
	\brief Returns true if the descriptor is valid.

	\return True if the current settings are valid
	*/
	PX_INLINE bool isValid() const;
};

PX_INLINE PxConvexMeshDesc::PxConvexMeshDesc()	//constructor sets to default
: vertexLimit(255), quantizedCount(255)
{
}

PX_INLINE void PxConvexMeshDesc::setToDefault()
{
	*this = PxConvexMeshDesc();
}

PX_INLINE bool PxConvexMeshDesc::isValid() const
{
	// Check geometry
	if(points.count < 3 ||	//at least 1 trig's worth of points
		(points.count > 0xffff && flags & PxConvexFlag::e16_BIT_INDICES))
		return false;
	if(!points.data)
		return false;
	if(points.stride < sizeof(PxVec3))	//should be at least one point's worth of data
		return false;
	if (quantizedCount < 4)
		return false;

	// Check topology
	if(polygons.data)
	{
		if(polygons.count < 4) // we require 2 neighbors for each vertex - 4 polygons at least
			return false;

		if(!indices.data) // indices must be provided together with polygons
			return false;

		PxU32 limit = (flags & PxConvexFlag::e16_BIT_INDICES) ? sizeof(PxU16) : sizeof(PxU32);
		if(indices.stride < limit) 
			return false;

		limit = sizeof(PxHullPolygon);
		if(polygons.stride < limit) 
			return false;
	}
	else
	{
		// We can compute the hull from the vertices
		if(!(flags & PxConvexFlag::eCOMPUTE_CONVEX))
			return false;	// If the mesh is convex and we're not allowed to compute the hull,
							// you have to provide it completely (geometry & topology).
	}

	if((flags & PxConvexFlag::ePLANE_SHIFTING) &&  vertexLimit < 4)
	{
		return false;
	}

	if (!(flags & PxConvexFlag::ePLANE_SHIFTING) && vertexLimit < 8)
	{
		return false;
	}

	if(vertexLimit > 256)
	{
		return false;
	}
	return true;
}

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif

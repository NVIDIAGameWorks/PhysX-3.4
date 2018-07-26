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


#ifndef PX_PHYSICS_GEOMUTILS_NX_TRIANGLEMESH
#define PX_PHYSICS_GEOMUTILS_NX_TRIANGLEMESH
/** \addtogroup geomutils 
@{ */

#include "foundation/PxVec3.h"
#include "foundation/PxBounds3.h"
#include "common/PxPhysXCommonConfig.h"
#include "common/PxBase.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief Enables the dynamic rtree mesh feature. It is recommended to use this feature for scene queries only.
@see PxTriangleMesh::getVerticesForModification
@see PxTriangleMesh::refitBVH
*/
#define PX_ENABLE_DYNAMIC_MESH_RTREE 1

/**
\brief Mesh midphase structure. This enum is used to select the desired acceleration structure for midphase queries
 (i.e. raycasts, overlaps, sweeps vs triangle meshes).

 The PxMeshMidPhase::eBVH33 structure is the one used in recent PhysX versions (up to PhysX 3.3). It has great performance and is
 supported on all platforms.

 The PxMeshMidPhase::eBVH34 structure is a revisited implementation introduced in PhysX 3.4. It can be significantly faster both
 in terms of cooking performance and runtime performance, but it is currently only available on platforms supporting the
 SSE2 instuction set.
*/
struct PxMeshMidPhase
{
	enum Enum
	{
		eBVH33 = 0,		//!< Default midphase mesh structure, as used up to PhysX 3.3
		eBVH34 = 1,		//!< New midphase mesh structure, introduced in PhysX 3.4

		eINVALID = 2,	//!< Invalid mesh midphase
		eLAST = eINVALID
	};
};

/**
\brief Flags for the mesh geometry properties.

Used in ::PxTriangleMeshFlags.
*/
struct PxTriangleMeshFlag
{
	enum Enum
	{
		e16_BIT_INDICES	= (1<<1),	//!< The triangle mesh has 16bits vertex indices.
		eADJACENCY_INFO	= (1<<2)	//!< The triangle mesh has adjacency information build.
	};
};

/**
\brief collection of set bits defined in PxTriangleMeshFlag.

@see PxTriangleMeshFlag
*/
typedef PxFlags<PxTriangleMeshFlag::Enum,PxU8> PxTriangleMeshFlags;
PX_FLAGS_OPERATORS(PxTriangleMeshFlag::Enum,PxU8)

/**

\brief A triangle mesh, also called a 'polygon soup'.

It is represented as an indexed triangle list. There are no restrictions on the
triangle data. 

To avoid duplicating data when you have several instances of a particular 
mesh positioned differently, you do not use this class to represent a 
mesh object directly. Instead, you create an instance of this mesh via
the PxTriangleMeshGeometry and PxShape classes.

<h3>Creation</h3>

To create an instance of this class call PxPhysics::createTriangleMesh(),
and release() to delete it. This is only possible
once you have released all of its PxShape instances.


<h3>Visualizations:</h3>
\li #PxVisualizationParameter::eCOLLISION_AABBS
\li #PxVisualizationParameter::eCOLLISION_SHAPES
\li #PxVisualizationParameter::eCOLLISION_AXES
\li #PxVisualizationParameter::eCOLLISION_FNORMALS
\li #PxVisualizationParameter::eCOLLISION_EDGES

@see PxTriangleMeshDesc PxTriangleMeshGeometry PxShape PxPhysics.createTriangleMesh()
*/

class PxTriangleMesh : public PxBase
{
	public:
	/**
	\brief Returns the number of vertices.
	\return	number of vertices
	@see getVertices()
	*/
	 virtual	PxU32				getNbVertices()									const	= 0;

	/**
	\brief Returns the vertices.
	\return	array of vertices
	@see getNbVertices()
	*/
	virtual	const PxVec3*			getVertices()									const	= 0;

#if PX_ENABLE_DYNAMIC_MESH_RTREE
	/**
	\brief Returns all mesh vertices for modification.

	This function will return the vertices of the mesh so that their positions can be changed in place.
	After modifying the vertices you must call refitBVH for the refitting to actually take place.
	This function maintains the old mesh topology (triangle indices).	

	\return  inplace vertex coordinates for each existing mesh vertex.

	\note works only for PxMeshMidPhase::eBVH33
	\note Size of array returned is equal to the number returned by getNbVertices().
	\note This function operates on cooked vertex indices.
	\note This means the index mapping and vertex count can be different from what was provided as an input to the cooking routine.
	\note To achieve unchanged 1-to-1 index mapping with orignal mesh data (before cooking) please use the following cooking flags:
	\note eWELD_VERTICES = 0, eDISABLE_CLEAN_MESH = 1.
	\note It is also recommended to make sure that a call to validateTriangleMesh returns true if mesh cleaning is disabled.
	@see getNbVertices()
	@see refitBVH()	
	*/
	virtual PxVec3*					getVerticesForModification() = 0;

	/**
	\brief Refits BVH for mesh vertices.

	This function will refit the mesh BVH to correctly enclose the new positions updated by getVerticesForModification.
	Mesh BVH will not be reoptimized by this function so significantly different new positions will cause significantly reduced performance.	

	\return New bounds for the entire mesh.

	\note works only for PxMeshMidPhase::eBVH33
	\note PhysX does not keep a mapping from the mesh to mesh shapes that reference it.
	\note Call PxShape::setGeometry on each shape which references the mesh, to ensure that internal data structures are updated to reflect the new geometry.
	\note PxShape::setGeometry does not guarantee correct/continuous behavior when objects are resting on top of old or new geometry.
	\note It is also recommended to make sure that a call to validateTriangleMesh returns true if mesh cleaning is disabled.
	\note Active edges information will be lost during refit, the rigid body mesh contact generation might not perform as expected.
	@see getNbVertices()
	@see getVerticesForModification()	
	*/
	virtual PxBounds3				refitBVH() = 0;
#endif // PX_ENABLE_DYNAMIC_MESH_RTREE

	/**
	\brief Returns the number of triangles.
	\return	number of triangles
	@see getTriangles() getTrianglesRemap()
	*/
	virtual	PxU32					getNbTriangles()								const	= 0;

	/**
	\brief Returns the triangle indices.

	The indices can be 16 or 32bit depending on the number of triangles in the mesh.
	Call getTriangleMeshFlags() to know if the indices are 16 or 32 bits.

	The number of indices is the number of triangles * 3.

	\return	array of triangles
	@see getNbTriangles() getTriangleMeshFlags() getTrianglesRemap()
	*/
	virtual	const void*				getTriangles()									const	= 0;

	/**
	\brief Reads the PxTriangleMesh flags.
	
	See the list of flags #PxTriangleMeshFlag

	\return The values of the PxTriangleMesh flags.

	@see PxTriangleMesh
	*/
	virtual	PxTriangleMeshFlags		getTriangleMeshFlags()							const = 0;

	/**
	\brief Returns the triangle remapping table.

	The triangles are internally sorted according to various criteria. Hence the internal triangle order
	does not always match the original (user-defined) order. The remapping table helps finding the old
	indices knowing the new ones:

		remapTable[ internalTriangleIndex ] = originalTriangleIndex

	\return	the remapping table (or NULL if 'PxCookingParams::suppressTriangleMeshRemapTable' has been used)
	@see getNbTriangles() getTriangles() PxCookingParams::suppressTriangleMeshRemapTable
	*/
	virtual	const PxU32*			getTrianglesRemap()							const	= 0;


	/**	
	\brief Decrements the reference count of a triangle mesh and releases it if the new reference count is zero.	
	
	@see PxPhysics.createTriangleMesh()
	*/
	virtual void					release()											= 0;

	/**
	\brief Returns material table index of given triangle

	This function takes a post cooking triangle index.

	\param[in] triangleIndex (internal) index of desired triangle
	\return Material table index, or 0xffff if no per-triangle materials are used
	*/
	virtual	PxMaterialTableIndex	getTriangleMaterialIndex(PxTriangleID triangleIndex)	const	= 0;

	/**
	\brief Returns the local-space (vertex space) AABB from the triangle mesh.

	\return	local-space bounds
	*/
	virtual	PxBounds3				getLocalBounds()							const	= 0;

	/**
	\brief Returns the reference count for shared meshes.

	At creation, the reference count of the mesh is 1. Every shape referencing this mesh increments the
	count by 1.	When the reference count reaches 0, and only then, the mesh gets destroyed automatically.

	\return the current reference count.
	*/
	virtual PxU32					getReferenceCount()							const	= 0;

	/**
	\brief Acquires a counted reference to a triangle mesh.

	This method increases the reference count of the triangle mesh by 1. Decrement the reference count by calling release()
	*/
	virtual void					acquireReference()									= 0;

protected:
	PX_INLINE						PxTriangleMesh(PxType concreteType, PxBaseFlags baseFlags) : PxBase(concreteType, baseFlags)	{}
	PX_INLINE						PxTriangleMesh(PxBaseFlags baseFlags) : PxBase(baseFlags)										{}
	virtual							~PxTriangleMesh() {}

	virtual	bool					isKindOf(const char* name) const { return !::strcmp("PxTriangleMesh", name) || PxBase::isKindOf(name); }
};

/**

\brief A triangle mesh containing the PxMeshMidPhase::eBVH33 structure.

@see PxMeshMidPhase
*/
class PxBVH33TriangleMesh : public PxTriangleMesh
{
	public:
protected:
	PX_INLINE						PxBVH33TriangleMesh(PxType concreteType, PxBaseFlags baseFlags) : PxTriangleMesh(concreteType, baseFlags) {}
	PX_INLINE						PxBVH33TriangleMesh(PxBaseFlags baseFlags) : PxTriangleMesh(baseFlags) {}
	virtual							~PxBVH33TriangleMesh() {}
	virtual	bool					isKindOf(const char* name) const { return !::strcmp("PxBVH33TriangleMesh", name) || PxTriangleMesh::isKindOf(name); }
};

/**

\brief A triangle mesh containing the PxMeshMidPhase::eBVH34 structure.

@see PxMeshMidPhase
*/
class PxBVH34TriangleMesh : public PxTriangleMesh
{
	public:
protected:
	PX_INLINE						PxBVH34TriangleMesh(PxType concreteType, PxBaseFlags baseFlags) : PxTriangleMesh(concreteType, baseFlags) {}
	PX_INLINE						PxBVH34TriangleMesh(PxBaseFlags baseFlags) : PxTriangleMesh(baseFlags) {}
	virtual							~PxBVH34TriangleMesh() {}
	virtual	bool					isKindOf(const char* name) const { return !::strcmp("PxBVH34TriangleMesh", name) || PxTriangleMesh::isKindOf(name); }
};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif

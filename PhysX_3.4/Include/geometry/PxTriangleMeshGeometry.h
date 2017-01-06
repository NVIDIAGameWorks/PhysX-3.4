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


#ifndef PX_PHYSICS_NX_TRIANGLEMESH_GEOMETRY
#define PX_PHYSICS_NX_TRIANGLEMESH_GEOMETRY
/** \addtogroup geomutils
@{
*/
#include "geometry/PxGeometry.h"
#include "geometry/PxMeshScale.h"
#include "common/PxCoreUtilityTypes.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxTriangleMesh;


/**
\brief Flags controlling the simulated behavior of the triangle mesh geometry.

Used in ::PxMeshGeometryFlags.
*/
struct PxMeshGeometryFlag
{
	enum Enum
	{
		eDOUBLE_SIDED = (1<<1)	//!< Meshes with this flag set are treated as double-sided.
								//!< This flag is currently only used for raycasts and sweeps (it is ignored for overlap queries).
								//!< For detailed specifications of this flag for meshes and heightfields please refer to the Geometry Query section of the user guide.
	};
};

/**
\brief collection of set bits defined in PxMeshGeometryFlag.

@see PxMeshGeometryFlag
*/
typedef PxFlags<PxMeshGeometryFlag::Enum,PxU8> PxMeshGeometryFlags;
PX_FLAGS_OPERATORS(PxMeshGeometryFlag::Enum,PxU8)

/**
\brief Triangle mesh geometry class.

This class unifies a mesh object with a scaling transform, and 
lets the combined object be used anywhere a PxGeometry is needed.

The scaling is a transform along arbitrary axes contained in the scale object.
The vertices of the mesh in geometry (or shape) space is the 
PxMeshScale::toMat33() transform, multiplied by the vertex space vertices 
in the PxConvexMesh object.
*/
class PxTriangleMeshGeometry : public PxGeometry 
{
public:
	/**
	\brief Default constructor.

	Creates an empty object with a NULL mesh and identity scale.
	*/
	PX_INLINE PxTriangleMeshGeometry() : 
		PxGeometry	(PxGeometryType::eTRIANGLEMESH), 
		triangleMesh(NULL)
	{}

	/**
	\brief Constructor.
	\param[in] mesh		Mesh pointer. May be NULL, though this will not make the object valid for shape construction.
	\param[in] scaling	Scale factor.
	\param[in] flags	Mesh flags.
	\
	*/
	PX_INLINE PxTriangleMeshGeometry(	PxTriangleMesh* mesh, 
										const PxMeshScale& scaling = PxMeshScale(), 
										PxMeshGeometryFlags flags = PxMeshGeometryFlags()) :
		PxGeometry	(PxGeometryType::eTRIANGLEMESH), 
		scale		(scaling), 
		meshFlags	(flags), 
		triangleMesh(mesh) 
	{}

	/**
	\brief Returns true if the geometry is valid.

	\return  True if the current settings are valid for shape creation.

	\note A valid triangle mesh has a positive scale value in each direction (scale.scale.x > 0, scale.scale.y > 0, scale.scale.z > 0).
	It is illegal to call PxRigidActor::createShape and PxPhysics::createShape with a triangle mesh that has zero extents in any direction.

	@see PxRigidActor::createShape, PxPhysics::createShape
	*/
	PX_INLINE bool isValid() const;

public:
	PxMeshScale			scale;				//!< The scaling transformation.
	PxMeshGeometryFlags	meshFlags;			//!< Mesh flags.
	PxPadding<3>		paddingFromFlags;	//!< padding for mesh flags
	PxTriangleMesh*		triangleMesh;		//!< A reference to the mesh object.
};


PX_INLINE bool PxTriangleMeshGeometry::isValid() const
{
	if(mType != PxGeometryType::eTRIANGLEMESH)
		return false;
	if(!scale.scale.isFinite() || !scale.rotation.isUnit())
		return false;
	if(!scale.isValidForTriangleMesh())
		return false;
	if(!triangleMesh)
		return false;

	return true;
}

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif

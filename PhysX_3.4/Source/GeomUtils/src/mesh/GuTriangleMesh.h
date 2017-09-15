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

#ifndef GU_TRIANGLEMESH_H
#define GU_TRIANGLEMESH_H

#include "foundation/PxIO.h"
#include "PxSimpleTriangleMesh.h"
#include "PxTriangleMeshGeometry.h"
#include "CmScaling.h"
#include "GuTriangleMesh.h"
#include "GuTriangle32.h"
#include "CmRefCountable.h"
#include "PxTriangle.h"
#include "PxTriangleMesh.h"
#include "CmRenderOutput.h"
#include "GuMeshData.h"
#include "GuCenterExtents.h"

namespace physx
{

class GuMeshFactory;
class PxMeshScale;

namespace Gu
{

#if PX_VC
#pragma warning(push)
#pragma warning(disable: 4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif

// Possible optimization: align the whole struct to cache line
class TriangleMesh : public PxTriangleMesh, public Ps::UserAllocated, public Cm::RefCountable
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:

// PX_SERIALIZATION
														TriangleMesh(PxBaseFlags baseFlags)	: PxTriangleMesh(baseFlags), Cm::RefCountable(PxEmpty) {}
						virtual void					exportExtraData(PxSerializationContext& ctx);
								void					importExtraData(PxDeserializationContext&);
	PX_PHYSX_COMMON_API	static	void					getBinaryMetaData(PxOutputStream& stream);
						virtual	void					release();

								void					resolveReferences(PxDeserializationContext& ) {}
						virtual	void					requires(PxProcessPxBaseCallback&){}
//~PX_SERIALIZATION

// Cm::RefCountable
						virtual	void					onRefCountZero();
//~Cm::RefCountable

														TriangleMesh(GuMeshFactory& factory, TriangleMeshData& data);
						 virtual						~TriangleMesh();
	
// PxTriangleMesh
						virtual	PxU32					getNbVertices()						const	{ return mNbVertices; }
						virtual	const PxVec3*			getVertices()						const	{ return mVertices; }
						virtual	const PxU32*			getTrianglesRemap()					const	{ return mFaceRemap; }
						virtual	PxU32					getNbTriangles()					const	{ return mNbTriangles; }
						virtual	const void*				getTriangles()						const	{ return mTriangles; }
						virtual	PxTriangleMeshFlags		getTriangleMeshFlags()				const	{ return PxTriangleMeshFlags(mFlags); }
						virtual	PxMaterialTableIndex	getTriangleMaterialIndex(PxTriangleID triangleIndex) const {
																				return hasPerTriangleMaterials() ? getMaterials()[triangleIndex] : PxMaterialTableIndex(0xffff);	}
	
#if PX_ENABLE_DYNAMIC_MESH_RTREE
						virtual PxVec3*					getVerticesForModification();
						virtual PxBounds3				refitBVH();
#endif 

						virtual	PxBounds3				getLocalBounds()					const
						{
							PX_ASSERT(mAABB.isValid());
							return PxBounds3::centerExtents(mAABB.mCenter, mAABB.mExtents);
						}

						virtual	void					acquireReference()							{ incRefCount();  }
						virtual	PxU32					getReferenceCount()					const	{ return getRefCount(); }
//~PxTriangleMesh
						// PT: this one is just to prevent instancing Gu::TriangleMesh.
						// But you should use PxBase::getConcreteType() instead to avoid the virtual call.
						virtual	PxMeshMidPhase::Enum	getMidphaseID()						const	= 0;

	PX_FORCE_INLINE				const PxU32*			getFaceRemap()						const	{ return mFaceRemap;											}
	PX_FORCE_INLINE				bool					has16BitIndices()					const	{ return (mFlags & PxMeshFlag::e16_BIT_INDICES) ? true : false;	}
	PX_FORCE_INLINE				bool					hasPerTriangleMaterials()			const	{ return mMaterialIndices != NULL;								}
	PX_FORCE_INLINE				PxU32					getNbVerticesFast()					const	{ return mNbVertices;		}
	PX_FORCE_INLINE				PxU32					getNbTrianglesFast()				const	{ return mNbTriangles;		}
	PX_FORCE_INLINE				const void*				getTrianglesFast()					const	{ return mTriangles;		}
	PX_FORCE_INLINE				const PxVec3*			getVerticesFast()					const	{ return mVertices;			}
	PX_FORCE_INLINE				const PxU32*			getAdjacencies()					const	{ return mAdjacencies;		}
	PX_FORCE_INLINE				PxReal					getGeomEpsilon()					const	{ return mGeomEpsilon;		}
	PX_FORCE_INLINE				const CenterExtents&	getLocalBoundsFast()				const	{ return mAABB;				}
	PX_FORCE_INLINE				const PxU16*			getMaterials()						const	{ return mMaterialIndices;	}
	PX_FORCE_INLINE				const PxU8*				getExtraTrigData()					const	{ return mExtraTrigData;	}

	PX_FORCE_INLINE				const CenterExtentsPadded&	getPaddedBounds()				const
														{
															// PT: see compile-time assert in cpp
															return static_cast<const CenterExtentsPadded&>(mAABB);
														}

	PX_FORCE_INLINE				void					computeWorldTriangle(
															PxTriangle& worldTri, PxTriangleID triangleIndex, const Cm::Matrix34& worldMatrix, bool flipNormal = false,
													PxU32* PX_RESTRICT vertexIndices=NULL, PxU32* PX_RESTRICT adjacencyIndices=NULL) const;
	PX_FORCE_INLINE				void							getLocalTriangle(PxTriangle& localTri, PxTriangleID triangleIndex, bool flipNormal = false) const;

								void					setMeshFactory(GuMeshFactory* factory) { mMeshFactory = factory; }

protected:
								PxU32					mNbVertices;
								PxU32					mNbTriangles;
								PxVec3*					mVertices;
								void*					mTriangles;				//!< 16 (<= 0xffff #vertices) or 32 bit trig indices (mNbTriangles * 3)
								// 16 bytes block

		// PT: WARNING: bounds must be followed by at least 32bits of data for safe SIMD loading
								CenterExtents			mAABB;
								PxU8*					mExtraTrigData;			//one per trig
								PxReal					mGeomEpsilon;			//!< see comments in cooking code referencing this variable
								// 16 bytes block
		/*
		low 3 bits (mask: 7) are the edge flags:
		b001 = 1 = ignore edge 0 = edge v0-->v1
		b010 = 2 = ignore edge 1 = edge v0-->v2
		b100 = 4 = ignore edge 2 = edge v1-->v2
		*/
								PxU8					mFlags;					//!< Flag whether indices are 16 or 32 bits wide
																					//!< Flag whether triangle adajacencies are build
								PxU16*					mMaterialIndices;		//!< the size of the array is numTriangles.
								PxU32*					mFaceRemap;				//!< new faces to old faces mapping (after cleaning, etc). Usage: old = faceRemap[new]
								PxU32*					mAdjacencies;			//!< Adjacency information for each face - 3 adjacent faces
																					//!< Set to 0xFFFFffff if no adjacent face
	
								GuMeshFactory*			mMeshFactory;					// PT: changed to pointer for serialization
public:
								
								// GRB data -------------------------
								void *					mGRB_triIndices;				//!< GRB: GPU-friendly tri indices [uint4]

								// TODO avoroshilov: cooking - adjacency info - duplicated, remove it and use 'mAdjacencies' and 'mExtraTrigData' see GuTriangleMesh.cpp:325
								void *					mGRB_triAdjacencies;			//!< GRB: adjacency data, with BOUNDARY and NONCONVEX flags (flags replace adj indices where applicable)

								PxU32*					mGRB_faceRemap;					//!< GRB : gpu to cpu triangle indice remap
								void*					mGRB_BV32Tree;					//!< GRB: BV32 tree
								// End of GRB data ------------------

};

#if PX_VC
#pragma warning(pop)
#endif

} // namespace Gu

PX_FORCE_INLINE void Gu::TriangleMesh::computeWorldTriangle(PxTriangle& worldTri, PxTriangleID triangleIndex, const Cm::Matrix34& worldMatrix, bool flipNormal,
	PxU32* PX_RESTRICT vertexIndices, PxU32* PX_RESTRICT adjacencyIndices) const
{
	PxU32 vref0, vref1, vref2;
	if(has16BitIndices())
	{
		const Gu::TriangleT<PxU16>& T = (reinterpret_cast<const Gu::TriangleT<PxU16>*>(getTrianglesFast()))[triangleIndex];
		vref0 = T.v[0];
		vref1 = T.v[1];
		vref2 = T.v[2];
	}
	else
	{
		const Gu::TriangleT<PxU32>& T = (reinterpret_cast<const Gu::TriangleT<PxU32>*>(getTrianglesFast()))[triangleIndex];
		vref0 = T.v[0];
		vref1 = T.v[1];
		vref2 = T.v[2];
	}
	if (flipNormal)
		Ps::swap<PxU32>(vref1, vref2);
	const PxVec3* PX_RESTRICT vertices = getVerticesFast();
	worldTri.verts[0] = worldMatrix.transform(vertices[vref0]);
	worldTri.verts[1] = worldMatrix.transform(vertices[vref1]);
	worldTri.verts[2] = worldMatrix.transform(vertices[vref2]);

	if(vertexIndices)
	{
		vertexIndices[0] = vref0;
		vertexIndices[1] = vref1;
		vertexIndices[2] = vref2;
	}

	if(adjacencyIndices)
	{
		if(getAdjacencies())
		{
			adjacencyIndices[0] = flipNormal ? getAdjacencies()[triangleIndex*3 + 2] : getAdjacencies()[triangleIndex*3 + 0];
			adjacencyIndices[1] = getAdjacencies()[triangleIndex*3 + 1];
			adjacencyIndices[2] = flipNormal ? getAdjacencies()[triangleIndex*3 + 0] : getAdjacencies()[triangleIndex*3 + 2];
		}
		else
		{
			adjacencyIndices[0] = 0xffffffff;
			adjacencyIndices[1] = 0xffffffff;
			adjacencyIndices[2] = 0xffffffff;
		}
	}
}

PX_FORCE_INLINE void Gu::TriangleMesh::getLocalTriangle(PxTriangle& localTri, PxTriangleID triangleIndex, bool flipNormal) const
{
	PxU32 vref0, vref1, vref2;
	if(has16BitIndices())
	{
		const Gu::TriangleT<PxU16>& T = (reinterpret_cast<const Gu::TriangleT<PxU16>*>(getTrianglesFast()))[triangleIndex];
		vref0 = T.v[0];
		vref1 = T.v[1];
		vref2 = T.v[2];
	}
	else
	{
		const Gu::TriangleT<PxU32>& T = (reinterpret_cast<const Gu::TriangleT<PxU32>*>(getTrianglesFast()))[triangleIndex];
		vref0 = T.v[0];
		vref1 = T.v[1];
		vref2 = T.v[2];
	}
	if (flipNormal)
		Ps::swap<PxU32>(vref1, vref2);
	const PxVec3* PX_RESTRICT vertices = getVerticesFast();
	localTri.verts[0] = vertices[vref0];
	localTri.verts[1] = vertices[vref1];
	localTri.verts[2] = vertices[vref2];
}

PX_INLINE float computeSweepData(const PxTriangleMeshGeometry& triMeshGeom, /*const Cm::FastVertex2ShapeScaling& scaling,*/ PxVec3& sweepOrigin, PxVec3& sweepExtents, PxVec3& sweepDir, float distance)
{
	PX_ASSERT(!Cm::isEmpty(sweepOrigin, sweepExtents));

	const PxVec3 endPt = sweepOrigin + sweepDir*distance;
	PX_ASSERT(!Cm::isEmpty(endPt, sweepExtents));

	const Cm::FastVertex2ShapeScaling meshScaling(triMeshGeom.scale.getInverse()); // shape to vertex transform

	const PxMat33& vertex2ShapeSkew = meshScaling.getVertex2ShapeSkew();

	const PxVec3 originBoundsCenter = vertex2ShapeSkew * sweepOrigin;
	const PxVec3 originBoundsExtents = Cm::basisExtent(vertex2ShapeSkew.column0, vertex2ShapeSkew.column1, vertex2ShapeSkew.column2, sweepExtents);

	sweepOrigin = originBoundsCenter;
	sweepExtents = originBoundsExtents;
	sweepDir = (vertex2ShapeSkew * endPt) - originBoundsCenter;
	return sweepDir.normalizeSafe();
}

}

#endif

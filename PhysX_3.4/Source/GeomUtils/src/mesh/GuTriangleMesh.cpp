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
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PsIntrinsics.h"
#include "GuMidphaseInterface.h"
#include "GuSerialize.h"
#include "GuMeshFactory.h"
#include "CmRenderOutput.h"
#include "PxVisualizationParameter.h"
#include "GuConvexEdgeFlags.h"
#include "GuBox.h"
#include "PxMeshScale.h"
#include "CmUtils.h"

using namespace physx;

namespace physx
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PxConcreteType::Enum gTable[] = {	PxConcreteType::eTRIANGLE_MESH_BVH33,
											PxConcreteType::eTRIANGLE_MESH_BVH34
										};

Gu::TriangleMesh::TriangleMesh(GuMeshFactory& factory, TriangleMeshData& d)
:	PxTriangleMesh(PxType(gTable[d.mType]), PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
,	mNbVertices				(d.mNbVertices)
,	mNbTriangles			(d.mNbTriangles)
,	mVertices				(d.mVertices)
,	mTriangles				(d.mTriangles)
,	mAABB					(d.mAABB)
,	mExtraTrigData			(d.mExtraTrigData)
,	mGeomEpsilon			(d.mGeomEpsilon)
,	mFlags					(d.mFlags)
,	mMaterialIndices		(d.mMaterialIndices)
,	mFaceRemap				(d.mFaceRemap)
,	mAdjacencies			(d.mAdjacencies)

,	mMeshFactory			(&factory)

,	mGRB_triIndices					(d.mGRB_triIndices)

,	mGRB_triAdjacencies				(d.mGRB_triAdjacencies)
,	mGRB_vertValency				(d.mGRB_vertValency)
,	mGRB_adjVertStart				(d.mGRB_adjVertStart)
,	mGRB_adjVertices				(d.mGRB_adjVertices)

,	mGRB_meshAdjVerticiesTotal		(d.mGRB_meshAdjVerticiesTotal)
,	mGRB_faceRemap					(d.mGRB_faceRemap)
,	mGRB_BV32Tree					(d.mGRB_BV32Tree)
{
	// this constructor takes ownership of memory from the data object
	d.mVertices = 0;
	d.mTriangles = 0;
	d.mExtraTrigData = 0;
	d.mFaceRemap = 0;
	d.mAdjacencies = 0;
	d.mMaterialIndices = 0;

	d.mGRB_triIndices = 0;

	d.mGRB_triAdjacencies = 0;
	d.mGRB_vertValency = 0;
	d.mGRB_adjVertStart = 0;
	d.mGRB_adjVertices = 0;
	d.mGRB_faceRemap = 0;
	d.mGRB_BV32Tree = 0;

	// PT: 'getPaddedBounds()' is only safe if we make sure the bounds member is followed by at least 32bits of data
	PX_COMPILE_TIME_ASSERT(PX_OFFSET_OF(Gu::TriangleMesh, mExtraTrigData)>=PX_OFFSET_OF(Gu::TriangleMesh, mAABB)+4);
	
}

Gu::TriangleMesh::~TriangleMesh() 
{ 	
	if(getBaseFlags() & PxBaseFlag::eOWNS_MEMORY)
	{
		PX_FREE_AND_RESET(mExtraTrigData);
		PX_FREE_AND_RESET(mFaceRemap);
		PX_FREE_AND_RESET(mAdjacencies);
		PX_FREE_AND_RESET(mMaterialIndices);
		PX_FREE_AND_RESET(mTriangles);
		PX_FREE_AND_RESET(mVertices);

		PX_FREE_AND_RESET(mGRB_triIndices); 

		PX_FREE_AND_RESET(mGRB_triAdjacencies);
		PX_FREE_AND_RESET(mGRB_vertValency);
		PX_FREE_AND_RESET(mGRB_adjVertStart);
		PX_FREE_AND_RESET(mGRB_adjVertices);
		PX_FREE_AND_RESET(mGRB_faceRemap);

		BV32Tree* bv32Tree = reinterpret_cast<BV32Tree*>(mGRB_BV32Tree);
		PX_DELETE_AND_RESET(bv32Tree);

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PT: used to be automatic but making it manual saves bytes in the internal mesh

void Gu::TriangleMesh::exportExtraData(PxSerializationContext& stream)
{
	//PX_DEFINE_DYNAMIC_ARRAY(TriangleMesh, mVertices, PxField::eVEC3, mNbVertices, Ps::PxFieldFlag::eSERIALIZE),
	if(mVertices)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mVertices, mNbVertices * sizeof(PxVec3));
	}

	if(mTriangles)
	{
		const PxU32 triangleSize = mFlags & PxTriangleMeshFlag::e16_BIT_INDICES ? sizeof(PxU16) : sizeof(PxU32);
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mTriangles, mNbTriangles * 3 * triangleSize);
	}

	//PX_DEFINE_DYNAMIC_ARRAY(TriangleMesh, mExtraTrigData, PxField::eBYTE, mNbTriangles, Ps::PxFieldFlag::eSERIALIZE),
	if(mExtraTrigData)
	{
		// PT: it might not be needed to 16-byte align this array of PxU8....
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mExtraTrigData, mNbTriangles * sizeof(PxU8));
	}

	if(mMaterialIndices)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mMaterialIndices, mNbTriangles * sizeof(PxU16));
	}

	if(mFaceRemap)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mFaceRemap, mNbTriangles * sizeof(PxU32));
	}

	if(mAdjacencies)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mAdjacencies, mNbTriangles * sizeof(PxU32) * 3);
	}
}

void Gu::TriangleMesh::importExtraData(PxDeserializationContext& context)
{
	// PT: vertices are followed by indices, so it will be safe to V4Load vertices from a deserialized binary file
	if(mVertices)
		mVertices = context.readExtraData<PxVec3, PX_SERIAL_ALIGN>(mNbVertices);

	if(mTriangles)
	{
		if(mFlags & PxTriangleMeshFlag::e16_BIT_INDICES)
			mTriangles = context.readExtraData<PxU16, PX_SERIAL_ALIGN>(3*mNbTriangles);
		else
			mTriangles = context.readExtraData<PxU32, PX_SERIAL_ALIGN>(3*mNbTriangles);
	}

	if(mExtraTrigData)
		mExtraTrigData = context.readExtraData<PxU8, PX_SERIAL_ALIGN>(mNbTriangles);

	if(mMaterialIndices)
		mMaterialIndices = context.readExtraData<PxU16, PX_SERIAL_ALIGN>(mNbTriangles);

	if(mFaceRemap)
		mFaceRemap = context.readExtraData<PxU32, PX_SERIAL_ALIGN>(mNbTriangles);

	if(mAdjacencies)
		mAdjacencies = context.readExtraData<PxU32, PX_SERIAL_ALIGN>(3*mNbTriangles);
}

void Gu::TriangleMesh::onRefCountZero()
{
	if(mMeshFactory->removeTriangleMesh(*this))
	{
		const PxType type = getConcreteType();
		GuMeshFactory* mf = mMeshFactory;
		Cm::deletePxBase(this);
		mf->notifyFactoryListener(this, type);
		return;
	}

	// PT: if we reach this point, we didn't find the mesh in the Physics object => don't delete!
	// This prevents deleting the object twice.
	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Gu::TriangleMesh::release: double deletion detected!");
}
//~PX_SERIALIZATION

void Gu::TriangleMesh::release()
{
	decRefCount();
}

#if PX_ENABLE_DYNAMIC_MESH_RTREE
PxVec3 * Gu::TriangleMesh::getVerticesForModification()
{
	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxTriangleMesh::getVerticesForModification() is only supported for meshes with PxMeshMidPhase::eBVHDynamic.");

	return NULL;
}

PxBounds3 Gu::TriangleMesh::refitBVH()
{
	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxTriangleMesh::refitBVH() is only supported for meshes with PxMeshMidPhase::eBVHDynamic.");

	return PxBounds3(mAABB.getMin(), mAABB.getMax());
}
#endif

#if PX_ENABLE_DEBUG_VISUALIZATION

static void getTriangle(const Gu::TriangleMesh&, PxU32 i, PxVec3* wp, const PxVec3* vertices, const void* indices, bool has16BitIndices)
{
	PxU32 ref0, ref1, ref2;

	if(!has16BitIndices)
	{
		const PxU32* dtriangles = reinterpret_cast<const PxU32*>(indices);
		ref0 = dtriangles[i*3+0];
		ref1 = dtriangles[i*3+1];
		ref2 = dtriangles[i*3+2];
	}
	else
	{
		const PxU16* wtriangles = reinterpret_cast<const PxU16*>(indices);
		ref0 = wtriangles[i*3+0];
		ref1 = wtriangles[i*3+1];
		ref2 = wtriangles[i*3+2];
	}

	wp[0] = vertices[ref0];
	wp[1] = vertices[ref1];
	wp[2] = vertices[ref2];
}

static void getTriangle(const Gu::TriangleMesh& mesh, PxU32 i, PxVec3* wp, const PxVec3* vertices, const void* indices, const Cm::Matrix34& absPose, bool has16BitIndices)
{
	PxVec3 localVerts[3];
	getTriangle(mesh, i, localVerts, vertices, indices, has16BitIndices);

	wp[0] = absPose.transform(localVerts[0]);
	wp[1] = absPose.transform(localVerts[1]);
	wp[2] = absPose.transform(localVerts[2]);
}

static void visualizeActiveEdges(Cm::RenderOutput& out, const Gu::TriangleMesh& mesh, PxU32 nbTriangles, const PxU32* results, const Cm::Matrix34& absPose, const PxMat44& midt)
{
	const PxU8* extraTrigData = mesh.getExtraTrigData();
	PX_ASSERT(extraTrigData);

	const PxVec3* vertices = mesh.getVerticesFast();
	const void* indices = mesh.getTrianglesFast();

	const PxU32 ecolor = PxU32(PxDebugColor::eARGB_YELLOW);
	const bool has16Bit = mesh.has16BitIndices();
	for(PxU32 i=0; i<nbTriangles; i++)
	{
		const PxU32 index = results ? results[i] : i;

		PxVec3 wp[3];
		getTriangle(mesh, index, wp, vertices, indices, absPose, has16Bit);

		const PxU32 flags = extraTrigData[index];

		if(flags & Gu::ETD_CONVEX_EDGE_01)
		{
			out << midt << ecolor << Cm::RenderOutput::LINES << wp[0] << wp[1];
		}
		if(flags & Gu::ETD_CONVEX_EDGE_12)
		{
			out << midt << ecolor << Cm::RenderOutput::LINES << wp[1] << wp[2];
		}
		if(flags & Gu::ETD_CONVEX_EDGE_20)
		{
			out << midt << ecolor << Cm::RenderOutput::LINES << wp[0] << wp[2];
		}
	}
}

void Gu::TriangleMesh::debugVisualize(
	Cm::RenderOutput& out, const PxTransform& pose, const PxMeshScale& scaling, const PxBounds3& cullbox,
	const PxU64 mask, const PxReal fscale, const PxU32 numMaterials) const 
{
	PX_UNUSED(numMaterials);

	//bool cscale = !!(mask & ((PxU64)1 << PxVisualizationParameter::eCULL_BOX));
	const PxU64 cullBoxMask = PxU64(1) << PxVisualizationParameter::eCULL_BOX;
	bool cscale = ((mask & cullBoxMask) == cullBoxMask);

	const PxMat44 midt(PxIdentity);
	const Cm::Matrix34 absPose(PxMat33(pose.q) * scaling.toMat33(), pose.p);

	PxU32 nbTriangles = getNbTrianglesFast();
	const PxU32 nbVertices = getNbVerticesFast();
	const PxVec3* vertices = getVerticesFast();
	const void* indices = getTrianglesFast();

	const PxDebugColor::Enum colors[] = 
	{
		PxDebugColor::eARGB_BLACK,		
		PxDebugColor::eARGB_RED,		
		PxDebugColor::eARGB_GREEN,		
		PxDebugColor::eARGB_BLUE,		
		PxDebugColor::eARGB_YELLOW,	
		PxDebugColor::eARGB_MAGENTA,	
		PxDebugColor::eARGB_CYAN,		
		PxDebugColor::eARGB_WHITE,		
		PxDebugColor::eARGB_GREY,		
		PxDebugColor::eARGB_DARKRED,	
		PxDebugColor::eARGB_DARKGREEN,	
		PxDebugColor::eARGB_DARKBLUE,	
	};

	const PxU32 colorCount = sizeof(colors)/sizeof(PxDebugColor::Enum);

	if(cscale)
	{
		const Gu::Box worldBox(
			(cullbox.maximum + cullbox.minimum)*0.5f,
			(cullbox.maximum - cullbox.minimum)*0.5f,
			PxMat33(PxIdentity));
		
		// PT: TODO: use the callback version here to avoid allocating this huge array
		PxU32* results = reinterpret_cast<PxU32*>(PX_ALLOC_TEMP(sizeof(PxU32)*nbTriangles, "tmp triangle indices"));
		LimitedResults limitedResults(results, nbTriangles, 0);
		Midphase::intersectBoxVsMesh(worldBox, *this, pose, scaling, &limitedResults);
		nbTriangles = limitedResults.mNbResults;

		if (fscale)
		{
			const PxU32 fcolor = PxU32(PxDebugColor::eARGB_DARKRED);

			for (PxU32 i=0; i<nbTriangles; i++)
			{
				const PxU32 index = results[i];
				PxVec3 wp[3];
				getTriangle(*this, index, wp, vertices, indices, absPose, has16BitIndices());

				const PxVec3 center = (wp[0] + wp[1] + wp[2]) / 3.0f;
				PxVec3 normal = (wp[0] - wp[1]).cross(wp[0] - wp[2]);
				PX_ASSERT(!normal.isZero());
				normal = normal.getNormalized();

				out << midt << fcolor <<
						Cm::DebugArrow(center, normal * fscale);
			}
		}

		if (mask & (PxU64(1) << PxVisualizationParameter::eCOLLISION_SHAPES))
		{
			const PxU32 scolor = PxU32(PxDebugColor::eARGB_MAGENTA);

			out << midt << scolor;	// PT: no need to output this for each segment!

			PxDebugLine* segments = out.reserveSegments(nbTriangles*3);
			for(PxU32 i=0; i<nbTriangles; i++)
			{
				const PxU32 index = results[i];
				PxVec3 wp[3];
				getTriangle(*this, index, wp, vertices, indices, absPose, has16BitIndices());
				segments[0] = PxDebugLine(wp[0], wp[1], scolor);
				segments[1] = PxDebugLine(wp[1], wp[2], scolor);
				segments[2] = PxDebugLine(wp[2], wp[0], scolor);
				segments+=3;
			}
		}

		if ((mask & (PxU64(1) << PxVisualizationParameter::eCOLLISION_EDGES)) && mExtraTrigData)
			visualizeActiveEdges(out, *this, nbTriangles, results, absPose, midt);

		PX_FREE(results);
	}
	else
	{
		if (fscale)
		{
			const PxU32 fcolor = PxU32(PxDebugColor::eARGB_DARKRED);

			for (PxU32 i=0; i<nbTriangles; i++)
			{
				PxVec3 wp[3];
				getTriangle(*this, i, wp, vertices, indices, absPose, has16BitIndices());

				const PxVec3 center = (wp[0] + wp[1] + wp[2]) / 3.0f;
				PxVec3 normal = (wp[0] - wp[1]).cross(wp[0] - wp[2]);
				PX_ASSERT(!normal.isZero());
				normal = normal.getNormalized();

				out << midt << fcolor <<
						Cm::DebugArrow(center, normal * fscale);
			}
		}

		if (mask & (PxU64(1) << PxVisualizationParameter::eCOLLISION_SHAPES))
		{
			PxU32 scolor = PxU32(PxDebugColor::eARGB_MAGENTA);

			out << midt << scolor;	// PT: no need to output this for each segment!

			PxVec3* transformed = reinterpret_cast<PxVec3*>(PX_ALLOC(sizeof(PxVec3)*nbVertices, "PxVec3"));
			for(PxU32 i=0;i<nbVertices;i++)
				transformed[i] = absPose.transform(vertices[i]);

			PxDebugLine* segments = out.reserveSegments(nbTriangles*3);
			for (PxU32 i=0; i<nbTriangles; i++)
			{
				PxVec3 wp[3];
				getTriangle(*this, i, wp, transformed, indices, has16BitIndices());
				const PxU32 localMaterialIndex = getTriangleMaterialIndex(i);
				scolor = colors[localMaterialIndex % colorCount];
				
				segments[0] = PxDebugLine(wp[0], wp[1], scolor);
				segments[1] = PxDebugLine(wp[1], wp[2], scolor);
				segments[2] = PxDebugLine(wp[2], wp[0], scolor);
				segments+=3;
			}

			PX_FREE(transformed);
		}

		if ((mask & (PxU64(1) << PxVisualizationParameter::eCOLLISION_EDGES)) && mExtraTrigData)
			visualizeActiveEdges(out, *this, nbTriangles, NULL, absPose, midt);
	}
}

#endif // #if PX_ENABLE_DEBUG_VISUALIZATION

} // namespace physx

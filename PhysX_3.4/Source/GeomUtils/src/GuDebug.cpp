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

#include "GuDebug.h"
#include "GuHeightFieldUtil.h"
#include "GuTriangleMesh.h"
#include "GuConvexMesh.h"
#include "PxVisualizationParameter.h"
#include "PxBoxGeometry.h"
#include "PxSphereGeometry.h"
#include "PxPlaneGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxGeometryQuery.h"
#include "PxMeshQuery.h"

using namespace physx;
using namespace Gu;

#if PX_ENABLE_DEBUG_VISUALIZATION

static void visualizeSphere(const PxSphereGeometry& geometry,	Cm::RenderOutput& out, const PxTransform& absPose)
{
	const PxU32 scolor = PxU32(PxDebugColor::eARGB_MAGENTA);

	out << scolor << absPose << Cm::DebugCircle(100, geometry.radius);

	PxMat44 rotPose(absPose);
	Ps::swap(rotPose.column1, rotPose.column2);
	rotPose.column1 = -rotPose.column1;
	out << scolor << rotPose << Cm::DebugCircle(100, geometry.radius);

	Ps::swap(rotPose.column0, rotPose.column2);
	rotPose.column0 = -rotPose.column0;
	out << scolor << rotPose << Cm::DebugCircle(100, geometry.radius);
}

static void visualizePlane(const PxPlaneGeometry& /*geometry*/, Cm::RenderOutput& out, const PxTransform& absPose)
{
	const PxU32 scolor = PxU32(PxDebugColor::eARGB_MAGENTA);

	PxMat44 rotPose(absPose);
	Ps::swap(rotPose.column1, rotPose.column2);
	rotPose.column1 = -rotPose.column1;

	Ps::swap(rotPose.column0, rotPose.column2);
	rotPose.column0 = -rotPose.column0;
	for(PxReal radius = 2.0f; radius < 20.0f ; radius += 2.0f)
		out << scolor << rotPose << Cm::DebugCircle(100, radius*radius);
}

static void visualizeCapsule(const PxCapsuleGeometry& geometry, Cm::RenderOutput& out, const PxTransform& absPose)
{
	out << PxU32(PxDebugColor::eARGB_MAGENTA);
	out.outputCapsule(geometry.radius, geometry.halfHeight, absPose);
}

static void visualizeBox(const PxBoxGeometry& geometry, Cm::RenderOutput& out, const PxTransform& absPose)
{
	out << PxU32(PxDebugColor::eARGB_MAGENTA);
	out << absPose << Cm::DebugBox(geometry.halfExtents);
}

static void visualizeConvexMesh(const PxConvexMeshGeometry& geometry, Cm::RenderOutput& out, const PxTransform& absPose)
{
	(static_cast<const ConvexMesh*>(geometry.convexMesh))->debugVisualize(out, absPose, geometry.scale);
}

static void visualizeTriangleMesh(const PxTriangleMeshGeometry& geometry, Cm::RenderOutput& out, const PxTransform& absPose, 
						   const PxBounds3& cullbox, const PxU64 mask, const PxReal fscale,  const PxU32 numMaterials)
{
	(static_cast<const TriangleMesh*>(geometry.triangleMesh))->debugVisualize(out, absPose, geometry.scale, cullbox, mask, fscale, numMaterials);
}

static void visualizeHeightField(const PxHeightFieldGeometry& hfGeometry,
	Cm::RenderOutput& out, const PxTransform& absPose, const PxBounds3& cullbox,
	const PxU64 mask)
{
	const HeightField* heightfield = static_cast<const HeightField*>(hfGeometry.heightField);
	const bool cscale = !!(mask & (PxU64(1) << PxVisualizationParameter::eCULL_BOX));

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

	if (mask & (PxU64(1) << PxVisualizationParameter::eCOLLISION_SHAPES))
	{

		// PT: TODO: the debug viz for HFs is minimal at the moment...
		PxU32 scolor = PxU32(PxDebugColor::eARGB_YELLOW);
		const PxMat44 midt = PxMat44(PxIdentity);

		HeightFieldUtil hfUtil(hfGeometry);

		const PxU32 nbRows = heightfield->getNbRowsFast();
		const PxU32 nbColumns = heightfield->getNbColumnsFast();
		const PxU32 nbVerts = nbRows * nbColumns;
		const PxU32 nbTriangles = 2 * nbVerts;

		out << midt << scolor;	// PT: no need to output the same matrix/color for each triangle

		if(cscale)
		{
			const PxTransform pose0((cullbox.maximum + cullbox.minimum)*0.5f);
			const PxBoxGeometry boxGeometry((cullbox.maximum - cullbox.minimum)*0.5f);

			const PxTransform pose1(absPose);

			PxU32* results = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nbTriangles, "tmp triangle indices"));

			bool overflow = false;
			PxU32 nbTouchedTris = PxMeshQuery::findOverlapHeightField(boxGeometry, pose0, hfGeometry, pose1, results, nbTriangles, 0, overflow);
		
			PxDebugLine* segments = out.reserveSegments(nbTouchedTris*3);

			for (PxU32 i=0; i<nbTouchedTris; i++)
			{
				const PxU32 index= results[i];
				PxTriangle currentTriangle;
				PxMeshQuery::getTriangle(hfGeometry, pose1, index, currentTriangle);

				//The check has been done in the findOverlapHeightField
				//if(heightfield->isValidTriangle(index) && heightfield->getTriangleMaterial(index) != PxHeightFieldMaterial::eHOLE)
				{
					const PxU16 localMaterialIndex = heightfield->getTriangleMaterialIndex(index);
					scolor = colors[localMaterialIndex % colorCount];

					segments[0] = PxDebugLine(currentTriangle.verts[0], currentTriangle.verts[1], scolor);
					segments[1] = PxDebugLine(currentTriangle.verts[1], currentTriangle.verts[2], scolor);
					segments[2] = PxDebugLine(currentTriangle.verts[2], currentTriangle.verts[0], scolor);
					segments+=3;
				}
			}
			PX_FREE(results);
		}
		else
		{

			// PT: transform vertices only once
			PxVec3* tmpVerts = reinterpret_cast<PxVec3*>(PX_ALLOC(sizeof(PxVec3)*nbVerts, "PxVec3"));
			// PT: TODO: optimize the following line
			for(PxU32 i=0;i<nbVerts;i++)
				tmpVerts[i] = absPose.transform(hfUtil.hf2shapep(heightfield->getVertex(i)));

			for(PxU32 i=0; i<nbTriangles; i++)
			{
				// PT: TODO: optimize away the useless divisions/modulos in the lines below
				if(heightfield->isValidTriangle(i) && heightfield->getTriangleMaterial(i) != PxHeightFieldMaterial::eHOLE)
				{
					PxU32 vi0, vi1, vi2;
					heightfield->getTriangleVertexIndices(i, vi0, vi1, vi2);
					const PxU16 localMaterialIndex = heightfield->getTriangleMaterialIndex(i);
					out << colors[localMaterialIndex % colorCount];

					const PxVec3& vw0 = tmpVerts[vi0];
					const PxVec3& vw1 = tmpVerts[vi1];
					const PxVec3& vw2 = tmpVerts[vi2];
				
					out.outputSegment(vw0, vw1);
					out.outputSegment(vw1, vw2);
					out.outputSegment(vw2, vw0);
				}
			}
			PX_FREE(tmpVerts);
		}
	}	
}

namespace physx
{
namespace Gu
{

void Debug::visualize(const PxGeometry& geometry,
					  Cm::RenderOutput& out, 
					  const PxTransform& absPose, 
					  const PxBounds3& cullbox,
					  const PxU64 mask, 
					  const PxReal fscale, 
					  const PxU32 numMaterials)
{
	
	const bool cull((mask & (PxU64(1) << PxVisualizationParameter::eCULL_BOX)) != 0);
	const bool collisionShapes((mask & (PxU64(1) << PxVisualizationParameter::eCOLLISION_SHAPES)) != 0);

	if(cull && !cullbox.intersects(PxGeometryQuery::getWorldBounds(geometry, absPose, 0.0f)))
		return;

	// triangle meshes can render active edges, but for other types we can just early out if there are no collision shapes
	if(!collisionShapes && geometry.getType() != PxGeometryType::eTRIANGLEMESH)
		return;

	switch(geometry.getType())
	{
	case PxGeometryType::eSPHERE:
		visualizeSphere(static_cast<const PxSphereGeometry&>(geometry), out, absPose);
		break;
	case PxGeometryType::eBOX:
		visualizeBox(static_cast<const PxBoxGeometry&>(geometry), out, absPose);
		break;
	case PxGeometryType::ePLANE:
		visualizePlane(static_cast<const PxPlaneGeometry&>(geometry), out, absPose);
		break;
	case PxGeometryType::eCAPSULE:
		visualizeCapsule(static_cast<const PxCapsuleGeometry&>(geometry), out, absPose);
		break;
	case PxGeometryType::eCONVEXMESH:
		visualizeConvexMesh(static_cast<const PxConvexMeshGeometry&>(geometry), out, absPose);
		break;
	case PxGeometryType::eTRIANGLEMESH:
		visualizeTriangleMesh(static_cast<const PxTriangleMeshGeometry&>(geometry), out, absPose, cullbox, mask, fscale, numMaterials);
		break;
	case PxGeometryType::eHEIGHTFIELD:
		visualizeHeightField(static_cast<const PxHeightFieldGeometry&>(geometry), out, absPose, cullbox, mask);
		break;
	case PxGeometryType::eINVALID:
		break;
	case PxGeometryType::eGEOMETRY_COUNT:
		break;
	}
}
}
}

#endif

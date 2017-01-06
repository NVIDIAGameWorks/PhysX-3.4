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


#ifndef PX_EDGELIST
#define PX_EDGELIST

// PT: this file should be moved to cooking lib

#include "foundation/Px.h"
#include "PsUserAllocated.h"

// Data/code shared with LL
#include "GuEdgeListData.h"

namespace physx
{
namespace Gu
{
	//! The edge-list creation structure.
	struct EDGELISTCREATE
	{
								EDGELISTCREATE() :
								NbFaces			(0),
								DFaces			(NULL),
								WFaces			(NULL),
								FacesToEdges	(false),
								EdgesToFaces	(false),
								Verts			(NULL),
								Epsilon			(0.1f)
								{}
				
				PxU32			NbFaces;		//!< Number of faces in source topo
				const PxU32*	DFaces;			//!< List of faces (dwords) or NULL
				const PxU16*	WFaces;			//!< List of faces (words) or NULL

				bool			FacesToEdges;
				bool			EdgesToFaces;
				const PxVec3*	Verts;
				float			Epsilon;
	};

	class EdgeList : public Ps::UserAllocated
	{
		public:
												EdgeList();
												~EdgeList();

				bool							load(PxInputStream& stream);
		// Data access
		PX_INLINE	PxU32						getNbEdges()							const	{ return mData.mNbEdges;						}
		PX_INLINE	const Gu::EdgeData*			getEdges()								const	{ return mData.mEdges;							}
		PX_INLINE	const Gu::EdgeData&			getEdge(PxU32 edge_index)				const	{ return mData.mEdges[edge_index];				}
		//
		PX_INLINE	PxU32						getNbFaces()							const	{ return mData.mNbFaces;						}
		PX_INLINE	const Gu::EdgeTriangleData* getEdgeTriangles()						const	{ return mData.mEdgeFaces;						}
		PX_INLINE	const Gu::EdgeTriangleData& getEdgeTriangle(PxU32 face_index)		const	{ return mData.mEdgeFaces[face_index];			}
		//
		PX_INLINE	const Gu::EdgeDescData*		getEdgeToTriangles()					const	{ return mData.mEdgeToTriangles;				}
		PX_INLINE	const Gu::EdgeDescData&		getEdgeToTriangles(PxU32 edge_index)	const	{ return mData.mEdgeToTriangles[edge_index];	}
		PX_INLINE	const PxU32*				getFacesByEdges()						const	{ return mData.mFacesByEdges;					}
		PX_INLINE	PxU32						getFacesByEdges(PxU32 face_index)		const	{ return mData.mFacesByEdges[face_index];		}

		protected:
				Gu::EdgeListData				mData;					//!< Pointer to edgelist data
	};

	class EdgeListBuilder : public EdgeList
	{
		public:
												EdgeListBuilder();
												~EdgeListBuilder();

					bool						init(const EDGELISTCREATE& create);
		private:
					bool						createFacesToEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces);
					bool						createEdgesToFaces(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces);
					bool						computeActiveEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces, const PxVec3* verts, float epsilon);
	};
}

}

#endif

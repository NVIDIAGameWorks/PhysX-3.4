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


#ifndef PX_COLLISION_TriangleMeshBUILDER
#define PX_COLLISION_TriangleMeshBUILDER

#include "GuMeshData.h"
#include "cooking/PxCooking.h"

namespace physx
{
	namespace Gu
	{
		class EdgeListBuilder;
	}

	class TriangleMeshBuilder
	{
		public:
											TriangleMeshBuilder(Gu::TriangleMeshData& mesh, const PxCookingParams& params);
		virtual								~TriangleMeshBuilder();

		virtual	PxMeshMidPhase::Enum		getMidphaseID()									const	= 0;
		// Called by base code when midphase structure should be built
		virtual	void						createMidPhaseStructure()								= 0;
		// Called by base code when midphase structure should be saved
		virtual	void						saveMidPhaseStructure(PxOutputStream& stream)	const	= 0;
		// Called by base code when mesh index format has changed and the change should be reflected in midphase structure
		virtual	void						onMeshIndexFormatChange()								{}

				bool						cleanMesh(bool validate, PxTriangleMeshCookingResult::Enum* condition);
				void						remapTopology(const PxU32* order);
		
				void						createSharedEdgeData(bool buildAdjacencies, bool buildActiveEdges);

				void						recordTriangleIndices();
				void						createGRBMidPhaseAndData(const PxU32 originalTriangleCount);
				void						createGRBData();

				bool						loadFromDesc(const PxTriangleMeshDesc&, PxTriangleMeshCookingResult::Enum* condition ,bool validate = false);
				bool						save(PxOutputStream& stream, bool platformMismatch, const PxCookingParams& params) const;
				void						checkMeshIndicesSize();
	PX_FORCE_INLINE	Gu::TriangleMeshData&	getMeshData()	{ return mMeshData;	}
	protected:
				void						computeLocalBounds();
				bool						importMesh(const PxTriangleMeshDesc& desc, const PxCookingParams& params, PxTriangleMeshCookingResult::Enum* condition ,bool validate = false);

				TriangleMeshBuilder& operator=(const TriangleMeshBuilder&);
				Gu::EdgeListBuilder*		edgeList;
				const PxCookingParams&		mParams;
				Gu::TriangleMeshData&		mMeshData;

				void						releaseEdgeList();
				void						createEdgeList();
	};

	class RTreeTriangleMeshBuilder : public TriangleMeshBuilder
	{
		public:
											RTreeTriangleMeshBuilder(const PxCookingParams& params);
		virtual								~RTreeTriangleMeshBuilder();

		virtual	PxMeshMidPhase::Enum		getMidphaseID()									const	{ return PxMeshMidPhase::eBVH33;	}
		virtual	void						createMidPhaseStructure();
		virtual	void						saveMidPhaseStructure(PxOutputStream& stream)	const;

				Gu::RTreeTriangleData		mData;
	};

	class BV4TriangleMeshBuilder : public TriangleMeshBuilder
	{
		public:
											BV4TriangleMeshBuilder(const PxCookingParams& params);
		virtual								~BV4TriangleMeshBuilder();

		virtual	PxMeshMidPhase::Enum		getMidphaseID()									const	{ return PxMeshMidPhase::eBVH34;	}
		virtual	void						createMidPhaseStructure();
		virtual	void						saveMidPhaseStructure(PxOutputStream& stream)	const;
		virtual	void						onMeshIndexFormatChange();

		Gu::BV4TriangleData			mData;
	};

	class BV32TriangleMeshBuilder
	{
	public:
		static	void createMidPhaseStructure(const PxCookingParams& params, Gu::TriangleMeshData& meshData, Gu::BV32Tree& bv32Tree);
		static	void saveMidPhaseStructure(Gu::BV32Tree* tree, PxOutputStream& stream);
	};

}

#endif

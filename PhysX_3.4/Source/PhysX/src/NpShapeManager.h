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


#ifndef PX_PHYSICS_NP_SHAPE_MANAGER
#define PX_PHYSICS_NP_SHAPE_MANAGER

#include "NpShape.h"
#include "CmPtrTable.h"
#include "SqSceneQueryManager.h"

#if PX_ENABLE_DEBUG_VISUALIZATION
#include "CmRenderOutput.h"
#endif

namespace physx
{

namespace Sq
{
	class SceneQueryManager;
	class PruningStructure;
}

class NpScene;

class NpShapeManager : public Ps::UserAllocated
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
	static			void					getBinaryMetaData(PxOutputStream& stream);
											NpShapeManager(const PxEMPTY);
					void					exportExtraData(PxSerializationContext& stream);
					void					importExtraData(PxDeserializationContext& context);
//~PX_SERIALIZATION
											NpShapeManager();
											~NpShapeManager();

	PX_FORCE_INLINE	PxU32					getNbShapes()		const	{ return mShapes.getCount();									}
	PX_FORCE_INLINE	NpShape* const*			getShapes()			const	{ return reinterpret_cast<NpShape*const*>(mShapes.getPtrs());	}
					PxU32					getShapes(PxShape** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;

					void					attachShape(NpShape& shape, PxRigidActor& actor);
					void					detachShape(NpShape& s, PxRigidActor &actor, bool wakeOnLostTouch);
					bool					shapeIsAttached(NpShape& s) const;
					void					detachAll(NpScene *scene);

					void					teardownSceneQuery(Sq::SceneQueryManager& sqManager, const NpShape& shape);
					void					setupSceneQuery(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, const NpShape& shape);

	PX_FORCE_INLINE void					setPrunerData(PxU32 index, Sq::PrunerData data)
											{
												PX_ASSERT(index<getNbShapes());
												mSceneQueryData.getPtrs()[index] = reinterpret_cast<void*>(data);
											}

	PX_FORCE_INLINE Sq::PrunerData			getPrunerData(PxU32 index)	const
											{
												PX_ASSERT(index<getNbShapes());
												return Sq::PrunerData(mSceneQueryData.getPtrs()[index]);
											}

					void					setupAllSceneQuery(NpScene* scene, const PxRigidActor& actor, bool hasPrunerStructure, const PxBounds3* bounds=NULL);
					void					teardownAllSceneQuery(Sq::SceneQueryManager& sqManager);
					void					markAllSceneQueryForUpdate(Sq::SceneQueryManager& shapeManager);
					
					Sq::PrunerData			findSceneQueryData(const NpShape& shape) const;

					PxBounds3				getWorldBounds(const PxRigidActor&) const;

	PX_FORCE_INLINE	void					setPruningStructure(Sq::PruningStructure* ps) { mPruningStructure = ps;		}
	PX_FORCE_INLINE	Sq::PruningStructure*	getPruningStructure()					const { return mPruningStructure;	}

					void					clearShapesOnRelease(Scb::Scene& s, PxRigidActor&);
					void					releaseExclusiveUserReferences();

#if PX_ENABLE_DEBUG_VISUALIZATION
					void					visualize(Cm::RenderOutput& out, NpScene* scene, const PxRigidActor& actor);
#endif
					// for batching
	PX_FORCE_INLINE	const Cm::PtrTable&		getShapeTable() const 		{	return mShapes; }
protected:
					void					setupSceneQuery(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, PxU32 index);
					void					teardownSceneQuery(Sq::SceneQueryManager& sqManager, PxU32 index);

					// PT: TODO: revisit this. We don't need two arrays.
					Cm::PtrTable			mShapes;
					Cm::PtrTable			mSceneQueryData;	// 1-1 correspondence with shapes - TODO: allocate on scene insertion or combine with the shape array for better caching
					Sq::PruningStructure*	mPruningStructure;  // Shape scene query data are pre-build in pruning structure
};

}

#endif

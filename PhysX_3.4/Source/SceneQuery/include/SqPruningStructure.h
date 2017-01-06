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


#ifndef SQ_PRUNING_STRUCTURE
#define SQ_PRUNING_STRUCTURE
/** \addtogroup physics
@{ */

#include "CmPhysXCommon.h"

#include "PxPruningStructure.h"

#include "PsUserAllocated.h"

namespace physx
{
	namespace Sq
	{				
		class AABBTreeRuntimeNode;

		struct PruningIndex
		{
			enum Enum
			{
				eSTATIC		= 0,
				eDYNAMIC	= 1,

				eCOUNT		= 2
			};
		};

		class PruningStructure : public PxPruningStructure, public Ps::UserAllocated
		{
			PX_NOCOPY(PruningStructure)
		public:
			// PX_SERIALIZATION            
													PruningStructure(PxBaseFlags baseFlags);			
			virtual			void					resolveReferences(PxDeserializationContext& );
			static			PruningStructure*		createObject(PxU8*& address, PxDeserializationContext& context);
			static			void					getBinaryMetaData(PxOutputStream& stream);
			void									exportExtraData(PxSerializationContext&);
			void									importExtraData(PxDeserializationContext&);
			virtual			void					requires(PxProcessPxBaseCallback&);
			//~PX_SERIALIZATION

			// PX_PRUNING_STRUCTURE
			virtual			PxU32					getRigidActors(PxRigidActor** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;
			virtual			PxU32					getNbRigidActors()								const	{ return mNbActors;						}
							void					release();
			// ~PX_PRUNING_STRUCTURE

													PruningStructure();
													~PruningStructure();

							bool					build(PxRigidActor*const* actors, PxU32 nbActors);			

			PX_FORCE_INLINE	PxU32					getNbActors()									const	{ return mNbActors;						}
			PX_FORCE_INLINE	PxActor*const*			getActors()										const	{ return mActors;						}

			PX_FORCE_INLINE	AABBTreeRuntimeNode*	getTreeNodes(PruningIndex::Enum currentTree)	const	{ return mAABBTreeNodes[currentTree];	}
			PX_FORCE_INLINE	PxU32					getTreeNbNodes(PruningIndex::Enum currentTree)	const	{ return mNbNodes[currentTree];			}

			PX_FORCE_INLINE	PxU32*					getTreeIndices(PruningIndex::Enum currentTree)	const	{ return mAABBTreeIndices[currentTree];	}
			PX_FORCE_INLINE	PxU32					getNbObjects(PruningIndex::Enum currentTree)	const	{ return mNbObjects[currentTree];		}

			PX_FORCE_INLINE	bool					isValid()										const	{ return mValid;	}
							void					invalidate(PxActor* actor);

		private:						
							PxU32					mNbNodes[2];			// Nb nodes in AABB tree
							AABBTreeRuntimeNode*	mAABBTreeNodes[2];		// AABB tree runtime nodes
							PxU32					mNbObjects[2];			// Nb objects in AABB tree
							PxU32*					mAABBTreeIndices[2];	// AABB tree indices
							PxU32					mNbActors;				// Nb actors from which the pruner structure was build
							PxActor**				mActors;				// actors used for pruner structure build, used later for serialization
							bool					mValid;					// pruning structure validity
		};
	} // namespace Sq

}

/** @} */
#endif // SQ_PRUNING_STRUCTURE

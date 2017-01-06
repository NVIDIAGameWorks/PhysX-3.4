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


#ifndef PX_PHYSICS_NP_SHAPE
#define PX_PHYSICS_NP_SHAPE

#include "PxShape.h"
#include "buffering/ScbShape.h"
#include "PxMetaData.h"

namespace physx
{

struct NpInternalShapeFlag
{
	enum Enum
	{
		eEXCLUSIVE				= (1<<0)
	};
};

/**
\brief collection of set bits defined in PxShapeFlag.

@see PxShapeFlag
*/
typedef PxFlags<NpInternalShapeFlag::Enum,PxU8> NpInternalShapeFlags;
PX_FLAGS_OPERATORS(NpInternalShapeFlag::Enum,PxU8)


class NpScene;
class NpShapeManager;

namespace Scb
{
	class Scene;
	class RigidObject;
}

namespace Sc
{
	class MaterialCore;
}

class NpShape : public PxShape, public Ps::UserAllocated, public Cm::RefCountable
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
												NpShape(PxBaseFlags baseFlags);
	virtual			void						exportExtraData(PxSerializationContext& stream);
					void						importExtraData(PxDeserializationContext& context);
	virtual			void						requires(PxProcessPxBaseCallback& c);
					void						resolveReferences(PxDeserializationContext& context);
	static			NpShape*					createObject(PxU8*& address, PxDeserializationContext& context);
	static			void						getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
												NpShape(const PxGeometry& geometry,
													PxShapeFlags shapeFlags,
													const PxU16* materialIndices,
													PxU16 materialCount, 
													bool isExclusive);

	virtual										~NpShape();
	
	//---------------------------------------------------------------------------------
	// PxShape implementation
	//---------------------------------------------------------------------------------
	virtual			void						release(); //!< call to release from actor
	virtual			void						acquireReference();

	virtual			PxGeometryType::Enum		getGeometryType() const;

	virtual			void						setGeometry(const PxGeometry&);
	virtual			PxGeometryHolder			getGeometry() const;
	virtual			bool						getBoxGeometry(PxBoxGeometry&) const;
	virtual			bool						getSphereGeometry(PxSphereGeometry&) const;
	virtual			bool						getCapsuleGeometry(PxCapsuleGeometry&) const;
	virtual			bool						getPlaneGeometry(PxPlaneGeometry&) const;
	virtual			bool						getConvexMeshGeometry(PxConvexMeshGeometry& g) const;
	virtual			bool						getTriangleMeshGeometry(PxTriangleMeshGeometry& g) const;
	virtual			bool						getHeightFieldGeometry(PxHeightFieldGeometry& g) const;

	virtual			PxRigidActor*				getActor() const;

	virtual			void						setLocalPose(const PxTransform& pose);
	virtual			PxTransform					getLocalPose() const;

	virtual			void						setSimulationFilterData(const PxFilterData& data);
	virtual			PxFilterData				getSimulationFilterData() const;
	virtual			void						setQueryFilterData(const PxFilterData& data);
	virtual			PxFilterData				getQueryFilterData() const;

	virtual			void						setMaterials(PxMaterial*const* materials, PxU16 materialCount);
	virtual			PxU16						getNbMaterials()															const;
	virtual			PxU32						getMaterials(PxMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;
	virtual			PxMaterial*					getMaterialFromInternalFaceIndex(PxU32 faceIndex)							const;

	virtual			void						setContactOffset(PxReal);
	virtual			PxReal						getContactOffset() const;

	virtual			void						setRestOffset(PxReal);
	virtual			PxReal						getRestOffset() const;

	virtual			void						setFlag(PxShapeFlag::Enum flag, bool value);
	virtual			void						setFlags( PxShapeFlags inFlags );
	virtual			PxShapeFlags				getFlags() const;

	virtual			bool						isExclusive() const;

	virtual			void					    setName(const char* debugName);
	virtual			const char*					getName() const;

	//---------------------------------------------------------------------------------
	// RefCountable implementation
	//---------------------------------------------------------------------------------

	// Ref counting for shapes works like this: 
	// * for exclusive shapes the actor has a counted reference
	// * for shared shapes, each actor has a counted reference, and the user has a counted reference
	// * for either kind, each instance of the shape in a scene (i.e. each shapeSim) causes the reference count to be incremented by 1.
	// Because these semantics aren't clear to users, this reference count should not be exposed in the API

	virtual			void						onRefCountZero();

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------

					void						setFlagsInternal( PxShapeFlags inFlags );

	PX_FORCE_INLINE	PxShapeFlags				getFlagsFast()			const	{ return mShape.getFlags();									}
	PX_FORCE_INLINE	PxShapeFlags				getFlagsUnbuffered()	const	{ return mShape.getScShape().getFlags();					}
	PX_FORCE_INLINE	PxGeometryType::Enum		getGeometryTypeFast()	const	{ return mShape.getGeometryType();							}
	PX_FORCE_INLINE	const Gu::GeometryUnion&	getGeometryFast()		const	{ return mShape.getGeometryUnion();							}
	PX_FORCE_INLINE const PxTransform&			getLocalPoseFast()		const	{ return mShape.getShape2Actor();							}
	PX_FORCE_INLINE PxU32						getActorCount()			const	{ return PxU32(mExclusiveAndActorCount & ACTOR_COUNT_MASK);	}
	PX_FORCE_INLINE PxI32						isExclusiveFast()		const	{ return mExclusiveAndActorCount & EXCLUSIVE_MASK;			}

	PX_FORCE_INLINE	const PxFilterData&			getQueryFilterDataFast() const
												{
													return mShape.getScShape().getQueryFilterData();	// PT: this one doesn't need double-buffering
												}

	PX_FORCE_INLINE	const Scb::Shape&			getScbShape()			const	{ return mShape;	}
	PX_FORCE_INLINE	Scb::Shape&					getScbShape()					{ return mShape;	}

	PX_INLINE		PxMaterial*					getMaterial(PxU32 index) const { return mShape.getMaterial(index); }
	static			bool						checkMaterialSetup(const PxGeometry& geom, const char* errorMsgPrefix, PxMaterial*const* materials, PxU16 materialCount);

					void						onActorAttach(PxRigidActor& actor);
					void						onActorDetach();

					// These methods are used only for sync'ing, and may only be called on exclusive shapes since only exclusive shapes have buffering
					Sc::RigidCore&				getScRigidObjectExclusive() const;
					void						releaseInternal();

					NpScene*					getOwnerScene()				const;	// same distinctions as for NpActor
private:
					NpScene*					getAPIScene()				const;

					void						incMeshRefCount();
					void						decMeshRefCount();
					Cm::RefCountable*			getMeshRefCountable();
					bool						isWritable();
					void						updateSQ(const char* errorMessage);

					PxRigidActor*				mActor;							// Auto-resolving refs breaks DLL loading for some reason
					Scb::Shape					mShape;
					const char*					mName;

					static const PxI32 EXCLUSIVE_MASK = 0x80000000;
					static const PxI32 ACTOR_COUNT_MASK = 0x7fffffff;
					
					volatile PxI32				mExclusiveAndActorCount;
};

}

#endif

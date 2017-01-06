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


#ifndef PX_PHYSICS_NP_RIGIDSTATIC
#define PX_PHYSICS_NP_RIGIDSTATIC

#include "NpRigidActorTemplate.h"
#include "PxRigidStatic.h"
#include "ScbRigidStatic.h"

#include "PxMetaData.h"

namespace physx
{

namespace Scb
{
	class RigidObject;
}

class NpRigidStatic;
typedef NpRigidActorTemplate<PxRigidStatic> NpRigidStaticT;

class NpRigidStatic : public NpRigidStaticT
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
											NpRigidStatic(PxBaseFlags baseFlags) : NpRigidStaticT(baseFlags), mRigidStatic(PxEmpty) {}
	virtual			void					requires(PxProcessPxBaseCallback& c);
	static			NpRigidStatic*			createObject(PxU8*& address, PxDeserializationContext& context);
	static			void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	virtual									~NpRigidStatic();

	//---------------------------------------------------------------------------------
	// PxActor implementation
	//---------------------------------------------------------------------------------
	virtual			void					release();

	virtual			PxActorType::Enum		getType() const { return PxActorType::eRIGID_STATIC; }

	//---------------------------------------------------------------------------------
	// PxRigidActor implementation
	//---------------------------------------------------------------------------------

	virtual			PxShape*				createShape(const PxGeometry& geometry, PxMaterial*const* material,
														PxU16 materialCount, PxShapeFlags shapeFlags);

	// Pose
	virtual			void 					setGlobalPose(const PxTransform& pose, bool wake);
	virtual			PxTransform				getGlobalPose() const;

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
											NpRigidStatic(const PxTransform& pose);

	virtual			void					switchToNoSim();
	virtual			void					switchFromNoSim();

#if PX_CHECKED
	bool									checkConstraintValidity() const;
#endif

	PX_FORCE_INLINE	const Scb::Actor&		getScbActorFast()		const	{ return mRigidStatic;	}
	PX_FORCE_INLINE	Scb::Actor&				getScbActorFast()				{ return mRigidStatic;	}

	PX_FORCE_INLINE	const Scb::RigidStatic&	getScbRigidStaticFast()	const	{ return mRigidStatic;	}
	PX_FORCE_INLINE	Scb::RigidStatic&		getScbRigidStaticFast()			{ return mRigidStatic;	}

	PX_FORCE_INLINE	const PxTransform&		getGlobalPoseFast()		const	{ return mRigidStatic.getActor2World();	}

#if PX_ENABLE_DEBUG_VISUALIZATION
public:
					void					visualize(Cm::RenderOutput& out, NpScene* scene);
#endif

private:
					Scb::RigidStatic 		mRigidStatic;
};

}

#endif

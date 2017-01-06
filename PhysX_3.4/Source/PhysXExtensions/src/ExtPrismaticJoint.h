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


#ifndef NP_PRISMATICJOINTCONSTRAINT_H
#define NP_PRISMATICJOINTCONSTRAINT_H

#include "ExtJoint.h"
#include "PxPrismaticJoint.h"
#include "PxTolerancesScale.h"
#include "CmUtils.h"

namespace physx
{
struct PxPrismaticJointGeneratedValues;
namespace Ext
{
	struct PrismaticJointData : public JointData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		PxJointLinearLimitPair	limit;
		PxReal					projectionLinearTolerance;
		PxReal					projectionAngularTolerance;

		PxPrismaticJointFlags	jointFlags;
		// forestall compiler complaints about not being able to generate a constructor
	private:
		PrismaticJointData(const PxJointLinearLimitPair &pair):
			limit(pair) {}
	};

    typedef Joint<PxPrismaticJoint, PxPrismaticJointGeneratedValues> PrismaticJointT;
   
	class PrismaticJoint : public PrismaticJointT
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
									PrismaticJoint(PxBaseFlags baseFlags) : PrismaticJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context);
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		PrismaticJoint*	createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		virtual						~PrismaticJoint()
		{
			if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
				PX_FREE(mData);
		}

		PrismaticJoint(const PxTolerancesScale& scale,
					   PxRigidActor* actor0, const PxTransform& localFrame0, 
					   PxRigidActor* actor1, const PxTransform& localFrame1)
		: PrismaticJointT(PxJointConcreteType::ePRISMATIC, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
		{
			PrismaticJointData* data = reinterpret_cast<PrismaticJointData*>(PX_ALLOC(sizeof(PrismaticJointData), "PrismaticJointData"));
			Cm::markSerializedMem(data, sizeof(PrismaticJointData));
			mData = data;

			data->limit = PxJointLinearLimitPair(scale, -PX_MAX_F32/3, PX_MAX_F32/3);
			data->projectionLinearTolerance = 1e10f;
			data->projectionAngularTolerance = PxPi;
			data->jointFlags = PxPrismaticJointFlags();

			initCommonData(*data, actor0, localFrame0, actor1, localFrame1);
		}

		PxReal getPosition()				const	{	return getRelativeTransform().p.x;		}
		PxReal getVelocity()				const 	{	return getRelativeLinearVelocity().x;	}


		void					setProjectionAngularTolerance(PxReal tolerance);
		PxReal					getProjectionAngularTolerance()					const;

		void					setProjectionLinearTolerance(PxReal tolerance);
		PxReal					getProjectionLinearTolerance()					const;

		PxJointLinearLimitPair	getLimit()										const;
		void					setLimit(const PxJointLinearLimitPair& limit);

		PxPrismaticJointFlags	getPrismaticJointFlags(void)					const;
		void					setPrismaticJointFlags(PxPrismaticJointFlags flags);
		void					setPrismaticJointFlag(PxPrismaticJointFlag::Enum flag, bool value);
		
		bool					attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);
		
		static const PxConstraintShaderTable& getConstraintShaderTable() { return sShaders; }

		virtual PxConstraintSolverPrep getPrep() const { return sShaders.solverPrep; }

	private:
		PX_FORCE_INLINE PrismaticJointData& data() const				
		{	
			return *static_cast<PrismaticJointData*>(mData);
		}

		static PxConstraintShaderTable sShaders;
	};
} // namespace Ext

namespace Ext
{
	extern "C"  PxU32 PrismaticJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale& invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w);
	
	// global function to share the joint shaders with API capture	
	extern "C" const PxConstraintShaderTable* GetPrismaticJointShaderTable();
}

}

#endif

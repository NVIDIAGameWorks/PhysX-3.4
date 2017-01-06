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


#ifndef EXT_PVD_H
#define EXT_PVD_H

#if PX_SUPPORT_PVD

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PxJoint.h"
#include "PxPvdDataStream.h"
#include "PxExtensionMetaDataObjects.h"
#include "PvdTypeNames.h"
#include "PxPvdObjectModelBaseTypes.h"

namespace physx
{

class PxJoint;
class PxD6Joint;
class PxDistanceJoint;
class PxFixedJoint;
class PxPrismaticJoint;
class PxRevoluteJoint;
class PxSphericalJoint;
}

#define JOINT_GROUP 3
namespace physx
{
namespace pvdsdk {
	#define DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP( type ) DEFINE_PVD_TYPE_NAME_MAP( physx::type, "physx3", #type )

	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxFixedJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxFixedJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxDistanceJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxDistanceJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPrismaticJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPrismaticJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRevoluteJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRevoluteJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSphericalJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSphericalJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxD6Joint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxD6JointGeneratedValues)
#undef DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP	
} //pvdsdk
} // physx

namespace physx
{
namespace Ext
{
	using namespace physx::pvdsdk;	
	
	class Pvd: public physx::shdfnd::UserAllocated
	{
		Pvd& operator=(const Pvd&);
	public:
		class PvdNameSpace
		{
		
		public:
			PvdNameSpace(PvdDataStream& conn, const char* name);
			~PvdNameSpace();
		private:
			PvdNameSpace& operator=(const PvdNameSpace&);
			PvdDataStream& mConnection;
		};

		static void setActors( PvdDataStream& PvdDataStream, 
			const PxJoint& inJoint, const PxConstraint& c, const PxActor* newActor0, const PxActor* newActor1 );
		
		template<typename TObjType>
		static void createInstance( PvdDataStream& inStream, const PxConstraint& c, const TObjType& inSource )
		{				
			inStream.createInstance( &inSource );
			inStream.pushBackObjectRef( c.getScene(), "Joints", &inSource );

			class ConstraintUpdateCmd : public PvdDataStream::PvdCommand
			{
				ConstraintUpdateCmd &operator=(const ConstraintUpdateCmd&) { PX_ASSERT(0); return *this; } //PX_NOCOPY doesn't work for local classes
			public:

				const PxConstraint& mConstraint;
				const PxJoint& mJoint;

				PxRigidActor* actor0, *actor1;
				ConstraintUpdateCmd(const PxConstraint& constraint, const PxJoint& joint):PvdDataStream::PvdCommand(), mConstraint(constraint), mJoint(joint)
				{
					mConstraint.getActors( actor0, actor1 ); 
				}

							//Assigned is needed for copying
				ConstraintUpdateCmd(const ConstraintUpdateCmd& cmd)
					:PvdDataStream::PvdCommand(), mConstraint(cmd.mConstraint), mJoint(cmd.mJoint)
				{					
				}

				virtual bool canRun(PvdInstanceDataStream &inStream_ )
				{
					PX_ASSERT(inStream_.isInstanceValid(&mJoint));
					//When run this command, the constraint maybe buffer removed
					return ((actor0 == NULL) || inStream_.isInstanceValid(actor0))
						&&  ((actor1 == NULL) || inStream_.isInstanceValid(actor1));
				}
				virtual void run( PvdInstanceDataStream &inStream_ )
				{
					//When run this command, the constraint maybe buffer removed
					if(!inStream_.isInstanceValid(&mJoint))
						return;

					PxRigidActor* actor0_, *actor1_;
					mConstraint.getActors( actor0_, actor1_ );

					if ( actor0_ && (inStream_.isInstanceValid(actor0_)) )
						inStream_.pushBackObjectRef( actor0_, "Joints", &mJoint );
					if ( actor1_ && (inStream_.isInstanceValid(actor1_)) )
						inStream_.pushBackObjectRef( actor1_, "Joints", &mJoint );
					const void* parent = actor0_ ? actor0_ : actor1_;
					inStream_.setPropertyValue( &mJoint, "Parent", parent );
				}
			};

			ConstraintUpdateCmd* cmd = PX_PLACEMENT_NEW(inStream.allocateMemForCmd(sizeof(ConstraintUpdateCmd)),
				ConstraintUpdateCmd)(c, inSource);
			
			if(cmd->canRun( inStream ))
				cmd->run( inStream );
			else
				inStream.pushPvdCommand( *cmd );
		}

		template<typename jointtype, typename structValue>
		static void updatePvdProperties(PvdDataStream& pvdConnection, const jointtype& joint)
		{
			structValue theValueStruct( &joint );
			pvdConnection.setPropertyMessage( &joint, theValueStruct );
		}
		
		template<typename jointtype>
		static void simUpdate(PvdDataStream& /*pvdConnection*/, const jointtype& /*joint*/) {}		
		
		template<typename jointtype>
		static void createPvdInstance(PvdDataStream& pvdConnection, const PxConstraint& c, const jointtype& joint)
		{
			createInstance<jointtype>( pvdConnection, c, joint );		
		}

		static void releasePvdInstance(PvdDataStream& pvdConnection, const PxConstraint& c, const PxJoint& joint);
		static void sendClassDescriptions(PvdDataStream& pvdConnection);
	};
} // ext

} // physx

#endif // PX_SUPPORT_PVD
#endif // EXT_PVD_H

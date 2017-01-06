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

// suppress LNK4221
#include "foundation/PxPreprocessor.h"
PX_DUMMY_SYMBOL

#if PX_SUPPORT_PVD
#include "ExtPvd.h"
#include "PxExtensionMetaDataObjects.h"

#include "ExtD6Joint.h"
#include "ExtFixedJoint.h"
#include "ExtSphericalJoint.h"
#include "ExtDistanceJoint.h"
#include "ExtSphericalJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtPrismaticJoint.h"
#include "ExtJointMetaDataExtensions.h"
#include "PvdMetaDataPropertyVisitor.h"
#include "PvdMetaDataDefineProperties.h"

namespace physx
{
namespace Ext
{
	using namespace physx::Vd;

	template<typename TObjType, typename TOperator>
	inline void visitPvdInstanceProperties( TOperator inOperator )
	{
		PxClassInfoTraits<TObjType>().Info.visitInstanceProperties( makePvdPropertyFilter( inOperator ), 0 );	
	}

	template<typename TObjType, typename TOperator>
	inline void visitPvdProperties( TOperator inOperator )
	{
		PvdPropertyFilter<TOperator> theFilter( makePvdPropertyFilter( inOperator ) );
		PxU32 thePropCount = PxClassInfoTraits<TObjType>().Info.visitBaseProperties( theFilter );
		PxClassInfoTraits<TObjType>().Info.visitInstanceProperties( theFilter, thePropCount );
	}

	Pvd::PvdNameSpace::PvdNameSpace(physx::pvdsdk::PvdDataStream& conn, const char* /*name*/)
		: mConnection(conn)
	{
	}

	Pvd::PvdNameSpace::~PvdNameSpace()
	{
	}

	void Pvd::releasePvdInstance(physx::pvdsdk::PvdDataStream& pvdConnection, const PxConstraint& c, const PxJoint& joint)
	{
		if(!pvdConnection.isConnected())
			return;
		//remove from scene and from any attached actors.
		PxRigidActor* actor0, *actor1;
		c.getActors( actor0, actor1 );
		PxScene* scene = c.getScene();
		if(scene)	pvdConnection.removeObjectRef( scene, "Joints", &joint );
		if ( actor0 && actor0->getScene() ) pvdConnection.removeObjectRef( actor0, "Joints", &joint );
		if ( actor1 && actor1->getScene()) pvdConnection.removeObjectRef( actor1, "Joints", &joint );
		pvdConnection.destroyInstance(&joint);
	}

	template<typename TObjType>
	void registerProperties( PvdDataStream& inStream )
	{
		inStream.createClass<TObjType>();
		PvdPropertyDefinitionHelper& theHelper( inStream.getPropertyDefinitionHelper() );
		PvdClassInfoDefine theDefinitionObj( theHelper, getPvdNamespacedNameForType<TObjType>() );
		visitPvdInstanceProperties<TObjType>( theDefinitionObj );
	}

	template<typename TObjType, typename TValueStructType>
	void registerPropertiesAndValueStruct( PvdDataStream& inStream )
	{
		inStream.createClass<TObjType>();
		inStream.deriveClass<PxJoint,TObjType>();
		PvdPropertyDefinitionHelper& theHelper( inStream.getPropertyDefinitionHelper() );
		{
			PvdClassInfoDefine theDefinitionObj( theHelper, getPvdNamespacedNameForType<TObjType>() );
			visitPvdInstanceProperties<TObjType>( theDefinitionObj );
		}
		{
			PvdClassInfoValueStructDefine theDefinitionObj( theHelper );
			visitPvdProperties<TObjType>( theDefinitionObj );
			theHelper.addPropertyMessage<TObjType,TValueStructType>();
		}
	}

	void Pvd::sendClassDescriptions(physx::pvdsdk::PvdDataStream& inStream)
	{
		if (inStream.isClassExist<PxJoint>())
		      return;

		{ //PxJoint
			registerProperties<PxJoint>( inStream );
			inStream.createProperty<PxJoint,ObjectRef>( "Parent", "parents" );
			registerPropertiesAndValueStruct<PxDistanceJoint,PxDistanceJointGeneratedValues>( inStream);
			registerPropertiesAndValueStruct<PxFixedJoint,PxFixedJointGeneratedValues>( inStream);
			registerPropertiesAndValueStruct<PxPrismaticJoint,PxPrismaticJointGeneratedValues>( inStream);
			registerPropertiesAndValueStruct<PxSphericalJoint,PxSphericalJointGeneratedValues>( inStream);
			registerPropertiesAndValueStruct<PxRevoluteJoint,PxRevoluteJointGeneratedValues>( inStream);
			registerPropertiesAndValueStruct<PxD6Joint,PxD6JointGeneratedValues>( inStream);
		}
	}
	
	void Pvd::setActors( physx::pvdsdk::PvdDataStream& inStream, const PxJoint& inJoint, const PxConstraint& c, const PxActor* newActor0, const PxActor* newActor1 )
	{
		PxRigidActor* actor0, *actor1;
		c.getActors( actor0, actor1 );
		if ( actor0 )
			inStream.removeObjectRef( actor0, "Joints", &inJoint );
		if ( actor1 )
			inStream.removeObjectRef( actor1, "Joints", &inJoint );
		
		if ( newActor0 && newActor0->getScene())
			inStream.pushBackObjectRef( newActor0, "Joints", &inJoint );
		if ( newActor1 && newActor1->getScene())
			inStream.pushBackObjectRef( newActor1, "Joints", &inJoint );

		inStream.setPropertyValue( &inJoint, "Actors.actor0",  reinterpret_cast<const void*>(newActor0) );
		inStream.setPropertyValue( &inJoint, "Actors.actor1", reinterpret_cast<const void*>(newActor1) );
		const void* parent = newActor0 ? newActor0 : newActor1;
		inStream.setPropertyValue( &inJoint, "Parent", parent );

		if((newActor0 && !newActor0->getScene()) || (newActor1 && !newActor1->getScene()))
		{
			inStream.removeObjectRef( c.getScene(), "Joints", &inJoint );
		}

	}
}

}

#endif // PX_SUPPORT_PVD

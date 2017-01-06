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

#ifndef PX_REPX_META_DATA_PROPERTY_VISITOR_H
#define PX_REPX_META_DATA_PROPERTY_VISITOR_H
#include "PvdMetaDataPropertyVisitor.h"

namespace physx {

	template<PxU32 TKey, typename TObjectType,typename TSetPropType, typename TPropertyType>
	struct PxRepXPropertyAccessor : public Vd::ValueStructOffsetRecord
	{
		typedef PxPropertyInfo<TKey,TObjectType,TSetPropType,TPropertyType> TPropertyInfoType;
		typedef TPropertyType prop_type;

		const TPropertyInfoType	mProperty;
		PxRepXPropertyAccessor( const TPropertyInfoType& inProp )
			: mProperty( inProp )
		{
		}
		prop_type get( const TObjectType* inObj ) const { return mProperty.get( inObj ); }
		void set( TObjectType* inObj, prop_type val ) const { return mProperty.set( inObj, val ); }

	private:
		PxRepXPropertyAccessor& operator=(const PxRepXPropertyAccessor&);
	};

	template<typename TSetPropType, typename TPropertyType>
	struct PxRepXPropertyAccessor<PxPropertyInfoName::PxRigidDynamic_WakeCounter, PxRigidDynamic, TSetPropType, TPropertyType> : public Vd::ValueStructOffsetRecord
	{
		typedef PxPropertyInfo<PxPropertyInfoName::PxRigidDynamic_WakeCounter,PxRigidDynamic,TSetPropType,TPropertyType> TPropertyInfoType;
		typedef TPropertyType prop_type;

		const TPropertyInfoType	mProperty;
		PxRepXPropertyAccessor( const TPropertyInfoType& inProp )
			: mProperty( inProp )
		{
		}
		prop_type get( const PxRigidDynamic* inObj ) const { return mProperty.get( inObj ); }
		void set( PxRigidDynamic* inObj, prop_type val ) const
		{ 
			PX_UNUSED(val);
			PxRigidBodyFlags flags = inObj->getRigidBodyFlags();
			if( !(flags & PxRigidBodyFlag::eKINEMATIC) )
				return mProperty.set( inObj, val ); 
		}
	private:
		PxRepXPropertyAccessor& operator=(const PxRepXPropertyAccessor&);
	};

	typedef PxReadOnlyPropertyInfo<PxPropertyInfoName::PxArticulationLink_InboundJoint, PxArticulationLink, PxArticulationJoint *> TIncomingJointPropType;


	//RepX cares about fewer property types than PVD does,
	//but I want to reuse the accessor architecture as it
	//greatly simplifies clients dealing with complex datatypes
	template<typename TOperatorType>
	struct RepXPropertyFilter
	{
		RepXPropertyFilter<TOperatorType> &operator=(const RepXPropertyFilter<TOperatorType> &);

		Vd::PvdPropertyFilter<TOperatorType> mFilter;
		RepXPropertyFilter( TOperatorType op ) : mFilter( op ) {}
		RepXPropertyFilter( const RepXPropertyFilter<TOperatorType>& other ) : mFilter( other.mFilter ) {}
		
		template<PxU32 TKey, typename TObjType, typename TPropertyType>
		void operator()( const PxReadOnlyPropertyInfo<TKey,TObjType,TPropertyType>&, PxU32 ) {} //repx ignores read only and write only properties
		template<PxU32 TKey, typename TObjType, typename TPropertyType>
		void operator()( const PxWriteOnlyPropertyInfo<TKey,TObjType,TPropertyType>&, PxU32 ) {}
		template<PxU32 TKey, typename TObjType, typename TCollectionType>
		void operator()( const PxReadOnlyCollectionPropertyInfo<TKey, TObjType, TCollectionType>&, PxU32 ) {}

		template<PxU32 TKey, typename TObjType, typename TCollectionType, typename TFilterType>
		void operator()( const PxReadOnlyFilteredCollectionPropertyInfo<TKey, TObjType, TCollectionType, TFilterType >&, PxU32 ) {}
		//forward these properties verbatim.
		template<PxU32 TKey, typename TObjType, typename TIndexType, typename TPropertyType>
		void operator()( const PxIndexedPropertyInfo<TKey, TObjType, TIndexType, TPropertyType >& inProp, PxU32 idx ) 
		{
			mFilter( inProp, idx );
		}
        
        template<PxU32 TKey, typename TObjType, typename TIndexType, typename TPropertyType>
		void operator()( const PxFixedSizeLookupTablePropertyInfo<TKey, TObjType, TIndexType,TPropertyType >& inProp, PxU32 idx ) 
		{
			mFilter( inProp, idx );
		}
			
		template<PxU32 TKey, typename TObjType, typename TIndexType, typename TPropertyType>
		void operator()( const PxExtendedIndexedPropertyInfo<TKey, TObjType, TIndexType, TPropertyType >& inProp, PxU32 idx) 
		{
            mFilter( inProp, idx);
		}

        template<PxU32 TKey, typename TObjType, typename TIndexType, typename TIndex2Type, typename TPropertyType>
		void operator()( const PxDualIndexedPropertyInfo<TKey, TObjType, TIndexType, TIndex2Type, TPropertyType >& inProp, PxU32 idx ) 
		{
			mFilter( inProp, idx );
		}

		template<PxU32 TKey, typename TObjType, typename TIndexType, typename TIndex2Type, typename TPropertyType>
		void operator()( const PxExtendedDualIndexedPropertyInfo<TKey, TObjType, TIndexType, TIndex2Type, TPropertyType >& inProp, PxU32 idx ) 
		{
			mFilter( inProp, idx );
		}

		template<PxU32 TKey, typename TObjType, typename TPropertyType>
		void operator()( const PxRangePropertyInfo<TKey, TObjType, TPropertyType>& inProp, PxU32 idx )
		{
			mFilter( inProp, idx );
		}
	
		template<PxU32 TKey, typename TObjType, typename TPropertyType>
		void operator()( const PxBufferCollectionPropertyInfo<TKey, TObjType, TPropertyType>& inProp, PxU32 count )
		{
			mFilter( inProp, count );
		}

		template<PxU32 TKey, typename TObjType, typename TSetPropType, typename TPropertyType>
		void operator()( const PxPropertyInfo<TKey,TObjType,TSetPropType,TPropertyType>& prop, PxU32 ) 
		{
			PxRepXPropertyAccessor< TKey, TObjType, TSetPropType, TPropertyType > theAccessor( prop );
			mFilter.mOperator.pushName( prop.mName );
			mFilter.template handleAccessor<TKey>( theAccessor );
			mFilter.mOperator.popName();
		}

		void operator()( const PxRigidActorShapeCollection& inProp, PxU32 )
		{
			mFilter.mOperator.pushName( "Shapes" );
			mFilter.mOperator.handleShapes( inProp );
			mFilter.mOperator.popName();
		}

		void operator()( const PxArticulationLinkCollectionProp& inProp, PxU32 )
		{
			mFilter.mOperator.pushName( "Links" );
			mFilter.mOperator.handleArticulationLinks( inProp );
			mFilter.mOperator.popName();
		}

		void operator()( const PxShapeMaterialsProperty& inProp, PxU32 )
		{
			mFilter.mOperator.pushName( "Materials" );
			mFilter.mOperator.handleShapeMaterials( inProp );
			mFilter.mOperator.popName();
		}

		void operator()( const TIncomingJointPropType& inProp, PxU32 )
		{
			mFilter.mOperator.handleIncomingJoint( inProp );
		}
        
        void operator()( const PxShapeGeometryProperty& inProp, PxU32 )
		{
			mFilter.mOperator.handleGeometryProperty( inProp );
		}
		
#define DEFINE_REPX_PROPERTY_NOP(datatype)																\
		template<PxU32 TKey, typename TObjType, typename TSetPropType>										\
		void operator()( const PxPropertyInfo<TKey,TObjType,TSetPropType,datatype>&, PxU32 ){}
		
		DEFINE_REPX_PROPERTY_NOP( const void* )
		DEFINE_REPX_PROPERTY_NOP( void* )
		DEFINE_REPX_PROPERTY_NOP( PxSimulationFilterCallback * )
		DEFINE_REPX_PROPERTY_NOP( physx::PxTaskManager * )
		DEFINE_REPX_PROPERTY_NOP( PxSimulationFilterShader * )
		DEFINE_REPX_PROPERTY_NOP( PxSimulationFilterShader)
		DEFINE_REPX_PROPERTY_NOP( PxContactModifyCallback * )
		DEFINE_REPX_PROPERTY_NOP( PxCCDContactModifyCallback * )
		DEFINE_REPX_PROPERTY_NOP( PxSimulationEventCallback * )
		DEFINE_REPX_PROPERTY_NOP( physx::PxGpuDispatcher* )
		DEFINE_REPX_PROPERTY_NOP( physx::PxCpuDispatcher * )
		DEFINE_REPX_PROPERTY_NOP( PxRigidActor )
		DEFINE_REPX_PROPERTY_NOP( const PxRigidActor )
		DEFINE_REPX_PROPERTY_NOP( PxRigidActor& )
		DEFINE_REPX_PROPERTY_NOP( const PxRigidActor& )
		DEFINE_REPX_PROPERTY_NOP( PxScene* )
		DEFINE_REPX_PROPERTY_NOP( PxAggregate * )
		DEFINE_REPX_PROPERTY_NOP( PxArticulation& )
		DEFINE_REPX_PROPERTY_NOP( const PxArticulationLink * )
		DEFINE_REPX_PROPERTY_NOP( const PxParticleFluid * )
		DEFINE_REPX_PROPERTY_NOP( const PxParticleSystem * )
		DEFINE_REPX_PROPERTY_NOP( const PxRigidDynamic * )
		DEFINE_REPX_PROPERTY_NOP( const PxRigidStatic * )
		DEFINE_REPX_PROPERTY_NOP( const PxParticleBase * )
		DEFINE_REPX_PROPERTY_NOP( PxStridedData ) //These are handled in a custom fasion.
	};
}

#endif

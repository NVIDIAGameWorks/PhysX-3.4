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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PX_META_DATA_EXTENSIONS_H
#define PX_META_DATA_EXTENSIONS_H
#include "PxMetaDataObjects.h"

#if PX_SUPPORT_PVD
#include "PxPvdObjectModelBaseTypes.h"

namespace physx { namespace pvdsdk {

	template<> PX_INLINE NamespacedName getPvdNamespacedNameForType<physx::PxMetaDataPlane>() { return getPvdNamespacedNameForType<PxVec4>(); }
	template<> PX_INLINE NamespacedName getPvdNamespacedNameForType<physx::PxRigidActor*>() { return getPvdNamespacedNameForType<VoidPtr>(); }
	
}}
#endif

namespace physx
{
namespace Vd
{
//Additional properties that exist only in pvd land.
struct PxPvdOnlyProperties
{
	enum Enum
	{
		FirstProp = PxPropertyInfoName::LastPxPropertyInfoName,
		PxScene_Frame,
		PxScene_Contacts,
		PxScene_SimulateElapsedTime,
#define DEFINE_ENUM_RANGE( stem, count ) \
	stem##Begin, \
	stem##End = stem##Begin + count

		//I can't easily add up the number of required property entries, but it is large due to the below
		//geometry count squared properties.  Thus I punt and allocate way more than I need right now.
		DEFINE_ENUM_RANGE( PxScene_SimulationStatistics, 1000 ),
		DEFINE_ENUM_RANGE( PxSceneDesc_Limits, PxPropertyInfoName::PxSceneLimits_PropertiesStop - PxPropertyInfoName::PxSceneLimits_PropertiesStart ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumBroadPhaseAdds, PxSimulationStatistics::eVOLUME_COUNT ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumBroadPhaseRemoves, PxSimulationStatistics::eVOLUME_COUNT ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumShapes, PxGeometryType::eGEOMETRY_COUNT ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumDiscreteContactPairs, PxGeometryType::eGEOMETRY_COUNT * PxGeometryType::eGEOMETRY_COUNT ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumModifiedContactPairs, PxGeometryType::eGEOMETRY_COUNT * PxGeometryType::eGEOMETRY_COUNT ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumSweptIntegrationPairs, PxGeometryType::eGEOMETRY_COUNT * PxGeometryType::eGEOMETRY_COUNT ),
		DEFINE_ENUM_RANGE( PxSimulationStatistics_NumTriggerPairs, PxGeometryType::eGEOMETRY_COUNT * PxGeometryType::eGEOMETRY_COUNT ),
		DEFINE_ENUM_RANGE( PxRigidDynamic_SolverIterationCounts, 2 ),
 		DEFINE_ENUM_RANGE( PxArticulation_SolverIterationCounts, 2 ),
		DEFINE_ENUM_RANGE( PxArticulationJoint_SwingLimit, 2 ),
		DEFINE_ENUM_RANGE( PxArticulationJoint_TwistLimit, 2 ),
		DEFINE_ENUM_RANGE( PxConvexMeshGeometry_Scale, PxPropertyInfoName::PxMeshScale_PropertiesStop - PxPropertyInfoName::PxMeshScale_PropertiesStart ),
		DEFINE_ENUM_RANGE( PxTriangleMeshGeometry_Scale, PxPropertyInfoName::PxMeshScale_PropertiesStop - PxPropertyInfoName::PxMeshScale_PropertiesStart ),
#if PX_USE_CLOTH_API
		DEFINE_ENUM_RANGE( PxCloth_MotionConstraintScaleBias, 2 ),
#endif
		

#if PX_USE_PARTICLE_SYSTEM_API
		PxParticleSystem_Positions,
		PxParticleSystem_Velocities,
		PxParticleSystem_RestOffsets,
		PxParticleSystem_CollisionNormals,
		PxParticleSystem_Flags,
		PxParticleFluid_Densities,
#endif
#if PX_USE_CLOTH_API
		PxCloth_ParticleBuffer,
		PxCloth_ParticleAccelerations,
		PxCloth_MotionConstraints,
		PxCloth_CollisionSpheres,
		PxCloth_CollisionSpherePairs,
		PxCloth_CollisionPlanes,
		PxCloth_CollisionConvexMasks,
		PxCloth_CollisionTriangles,
		PxCloth_VirtualParticles,
		PxCloth_VirtualParticleWeights,
		PxCloth_SeparationConstraints,
		PxCloth_SelfCollisionIndices,
		PxCloth_RestPositions,
#endif
		LastPxPvdOnlyProperty
	};
};

template<PxU32 TKey, typename TObjectType, typename TPropertyType, PxU32 TEnableFlag>
struct PxBufferPropertyInfo : PxReadOnlyPropertyInfo< TKey, TObjectType, TPropertyType >
{
	typedef PxReadOnlyPropertyInfo< TKey, TObjectType, TPropertyType > TBaseType;
	typedef typename TBaseType::TGetterType TGetterType;
	PxBufferPropertyInfo( const char* inName, TGetterType inGetter )
		: TBaseType( inName, inGetter )
	{
	}
	bool isEnabled( PxU32 inFlags ) const { return (inFlags & TEnableFlag) > 0; }
};


#define DECLARE_BUFFER_PROPERTY( objectType, baseType, propType, propName, fieldName, flagName )												\
typedef PxBufferPropertyInfo< PxPvdOnlyProperties::baseType##_##propName, objectType, propType, flagName > T##objectType##propName##Base;		\
inline propType get##propName( const objectType* inData ) { return inData->fieldName; }															\
struct baseType##propName##Property : T##objectType##propName##Base																				\
{																																				\
	baseType##propName##Property()  : T##objectType##propName##Base( #propName, get##propName ){}												\
};

#if PX_USE_PARTICLE_SYSTEM_API
DECLARE_BUFFER_PROPERTY( PxParticleReadData,		PxParticleSystem, PxStrideIterator<const PxVec3>,			Positions, positionBuffer, PxParticleReadDataFlag::ePOSITION_BUFFER )
DECLARE_BUFFER_PROPERTY( PxParticleReadData,		PxParticleSystem, PxStrideIterator<const PxVec3>,			Velocities, velocityBuffer, PxParticleReadDataFlag::eVELOCITY_BUFFER )
DECLARE_BUFFER_PROPERTY( PxParticleReadData,		PxParticleSystem, PxStrideIterator<const PxF32>,			RestOffsets, restOffsetBuffer, PxParticleReadDataFlag::eREST_OFFSET_BUFFER )
DECLARE_BUFFER_PROPERTY( PxParticleReadData,		PxParticleSystem, PxStrideIterator<const PxVec3>,			CollisionNormals, collisionNormalBuffer, PxParticleReadDataFlag::eCOLLISION_NORMAL_BUFFER )
DECLARE_BUFFER_PROPERTY( PxParticleReadData,		PxParticleSystem, PxStrideIterator<const PxParticleFlags>,	Flags, flagsBuffer, PxParticleReadDataFlag::eFLAGS_BUFFER )
DECLARE_BUFFER_PROPERTY( PxParticleFluidReadData,	PxParticleFluid,  PxStrideIterator<const PxF32>,			Densities, densityBuffer, PxParticleReadDataFlag::eDENSITY_BUFFER )

template<typename TOperator>
inline void visitParticleSystemBufferProperties( TOperator inOperator )
{
	inOperator( PxParticleSystemPositionsProperty(), 0 );
	inOperator( PxParticleSystemVelocitiesProperty(), 1 );
	inOperator( PxParticleSystemRestOffsetsProperty(), 2 );
	inOperator( PxParticleSystemCollisionNormalsProperty(), 3 );
	inOperator( PxParticleSystemFlagsProperty(), 4 );
}

template<typename TOperator>
inline void visitParticleFluidBufferProperties( TOperator inOperator )
{
	inOperator( PxParticleFluidDensitiesProperty(), 0 );
}
#endif // PX_USE_PARTICLE_SYSTEM_API


template<PxU32 PropertyKey, typename TEnumType >
struct IndexerToNameMap
{
	PxEnumTraits<TEnumType> Converter;
};

struct ValueStructOffsetRecord
{
	mutable bool	mHasValidOffset;
	mutable PxU32	mOffset;
	ValueStructOffsetRecord() : mHasValidOffset( false ), mOffset( 0 ) {}
	void setupValueStructOffset( PxU32 inValue ) const
	{
		mHasValidOffset = true;
		mOffset = inValue;
	}
};

template<PxU32 TKey, typename TObjectType, typename TPropertyType>
struct PxPvdReadOnlyPropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxReadOnlyPropertyInfo<TKey,TObjectType,TPropertyType> TPropertyInfoType;
	typedef TPropertyType prop_type;

	const TPropertyInfoType	mProperty;
	PxPvdReadOnlyPropertyAccessor( const TPropertyInfoType& inProp )
		: mProperty( inProp )
	{
	}
	prop_type get( const TObjectType* inObj ) const { return mProperty.get( inObj ); }

private:
	PxPvdReadOnlyPropertyAccessor& operator=(const PxPvdReadOnlyPropertyAccessor&);
};

template<PxU32 TKey, typename TObjectType, typename TPropertyType>
struct PxBufferCollectionPropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxBufferCollectionPropertyInfo< TKey, TObjectType, TPropertyType > TPropertyInfoType;
	typedef TPropertyType prop_type;
	const TPropertyInfoType& mProperty;
	const char* mName;
	
	PxBufferCollectionPropertyAccessor( const TPropertyInfoType& inProp, const char* inName )
		: mProperty( inProp )
		, mName( inName )
	{
	}
	
	const char* name() const { return mName; }
	PxU32 size( const TObjectType* inObj ) const { return mProperty.size( inObj ); }
	PxU32 get( const TObjectType* inObj, prop_type* buffer, PxU32 inNumItems) const { return mProperty.get( inObj, buffer, inNumItems); }
	void set( TObjectType* inObj, prop_type* inBuffer, PxU32 inNumItems ) const { mProperty.set( inObj, inBuffer, inNumItems ); }
};

template<PxU32 TKey, typename TObjectType, typename TIndexType, typename TPropertyType>
struct PxPvdIndexedPropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxIndexedPropertyInfo< TKey, TObjectType, TIndexType, TPropertyType > TPropertyInfoType;
	typedef TPropertyType prop_type;
	TIndexType mIndex;
	const TPropertyInfoType& mProperty;
	PxPvdIndexedPropertyAccessor( const TPropertyInfoType& inProp, PxU32 inIndex )
		: mIndex( static_cast<TIndexType>( inIndex ) )
		, mProperty( inProp )
	{
	}
	prop_type get( const TObjectType* inObj ) const { return mProperty.get( inObj, mIndex ); }
	void set( TObjectType* inObj, prop_type val ) const { mProperty.set( inObj, mIndex, val ); }

	void operator = (PxPvdIndexedPropertyAccessor&) {}
};

template<PxU32 TKey, typename TObjectType, typename TIndexType, typename TPropertyType>
struct PxPvdExtendedIndexedPropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxExtendedIndexedPropertyInfo< TKey, TObjectType, TIndexType, TPropertyType > TPropertyInfoType;
	typedef TPropertyType prop_type;
	TIndexType mIndex;
	const TPropertyInfoType& mProperty;
	PxPvdExtendedIndexedPropertyAccessor( const TPropertyInfoType& inProp, PxU32 inIndex )
		: mIndex( static_cast<TIndexType>( inIndex ) )
		, mProperty( inProp )
	{
	}

	PxU32 size( const TObjectType* inObj ) const { return mProperty.size( inObj ); }
	prop_type get( const TObjectType* inObj, TIndexType index ) const { return mProperty.get( inObj, index ); }
	void set( TObjectType* inObj, TIndexType index, prop_type val ) const { mProperty.set( inObj, index, val ); }

	void operator = (PxPvdExtendedIndexedPropertyAccessor&) {}
};

template<PxU32 TKey, typename TObjectType, typename TIndexType, typename TPropertyType>
struct PxPvdFixedSizeLookupTablePropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxFixedSizeLookupTablePropertyInfo< TKey, TObjectType, TIndexType, TPropertyType > TPropertyInfoType;
	typedef TPropertyType prop_type;
	TIndexType	mIndex;
	
	const TPropertyInfoType& mProperty;
	PxPvdFixedSizeLookupTablePropertyAccessor( const TPropertyInfoType& inProp, const PxU32 inIndex3 )
		: mIndex( static_cast<TIndexType>( inIndex3 ) )
		, mProperty( inProp )
	{
	}

	PxU32 size( const TObjectType* inObj ) const { return mProperty.size( inObj ); }
	prop_type getX( const TObjectType* inObj, const TIndexType index ) const { return mProperty.getX( inObj, index ); }
	prop_type getY( const TObjectType* inObj, const TIndexType index ) const { return mProperty.getY( inObj, index ); }
	void addPair(  TObjectType* inObj, const PxReal x, const PxReal y ) { const_cast<TPropertyInfoType&>(mProperty).addPair( inObj, x, y );  }
	void clear( TObjectType* inObj ) { const_cast<TPropertyInfoType&>(mProperty).clear( inObj );  }
	void operator = (PxPvdFixedSizeLookupTablePropertyAccessor&) {}
};

template<PxU32 TKey, typename TObjectType, typename TIdx0Type, typename TIdx1Type, typename TPropertyType>
struct PxPvdDualIndexedPropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxDualIndexedPropertyInfo< TKey, TObjectType, TIdx0Type, TIdx1Type, TPropertyType > TPropertyInfoType;
	typedef TPropertyType prop_type;
	TIdx0Type mIdx0;
	TIdx1Type mIdx1;
	const TPropertyInfoType& mProperty;

	PxPvdDualIndexedPropertyAccessor( const TPropertyInfoType& inProp, PxU32 idx0, PxU32 idx1 )
		: mIdx0( static_cast<TIdx0Type>( idx0 ) )
		, mIdx1( static_cast<TIdx1Type>( idx1 ) )
		, mProperty( inProp )
	{
	}
	prop_type get( const TObjectType* inObj ) const { return mProperty.get( inObj, mIdx0, mIdx1 ); }
	void set( TObjectType* inObj, prop_type val ) const { mProperty.set( inObj, mIdx0, mIdx1, val ); }

private:
	PxPvdDualIndexedPropertyAccessor& operator = (const PxPvdDualIndexedPropertyAccessor&);
};

template<PxU32 TKey, typename TObjectType, typename TIdx0Type, typename TIdx1Type, typename TPropertyType>
struct PxPvdExtendedDualIndexedPropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxExtendedDualIndexedPropertyInfo< TKey, TObjectType, TIdx0Type, TIdx1Type, TPropertyType > TPropertyInfoType;
	typedef TPropertyType prop_type;
	TIdx0Type mIdx0;
	TIdx1Type mIdx1;
	const TPropertyInfoType& mProperty;
	
	PxPvdExtendedDualIndexedPropertyAccessor( const TPropertyInfoType& inProp, PxU32 idx0, PxU32 idx1 )
		: mIdx0( static_cast<TIdx0Type>( idx0 ) )
		, mIdx1( static_cast<TIdx1Type>( idx1 ) )
		, mProperty( inProp )
	{
	}
	prop_type get( const TObjectType* inObj ) const { return mProperty.get( inObj, mIdx0, mIdx1 ); }
	void set( TObjectType* inObj, prop_type val ) const { mProperty.set( inObj, mIdx0, mIdx1, val ); }

private:
	PxPvdExtendedDualIndexedPropertyAccessor& operator = (const PxPvdExtendedDualIndexedPropertyAccessor&);
};

template<PxU32 TKey, typename TObjType, typename TPropertyType>
struct PxPvdRangePropertyAccessor : public ValueStructOffsetRecord
{
	typedef PxRangePropertyInfo<TKey, TObjType, TPropertyType> TPropertyInfoType;
	typedef TPropertyType prop_type;
	bool mFirstValue;
	const TPropertyInfoType& mProperty;

	PxPvdRangePropertyAccessor( const TPropertyInfoType& inProp, bool inFirstValue )
		: mFirstValue( inFirstValue )
		, mProperty( inProp )
	{
	}

	prop_type get( const TObjType* inObj ) const {
		prop_type first,second;
		mProperty.get( inObj, first, second );
		return mFirstValue ? first : second;
	}
	void set( TObjType* inObj, prop_type val ) const 
	{ 
		prop_type first,second;
		mProperty.get( inObj, first, second );
		if ( mFirstValue ) mProperty.set( inObj, val, second ); 
		else mProperty.set( inObj, first, val );
	}

	void operator = (PxPvdRangePropertyAccessor&) {}
};


template<typename TDataType>
struct IsFlagsType
{
	bool FlagData;
};

template<typename TEnumType, typename TStorageType>
struct IsFlagsType<PxFlags<TEnumType, TStorageType> > 
{
	const PxU32ToName* FlagData;
	IsFlagsType<PxFlags<TEnumType, TStorageType> > () : FlagData( PxEnumTraits<TEnumType>().NameConversion ) {}
};



template<typename TDataType>
struct PvdClassForType
{
	bool Unknown;
};

}

}

#endif

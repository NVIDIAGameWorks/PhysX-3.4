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

#ifndef PX_METADATACOMPARE_H
#define PX_METADATACOMPARE_H
#include "PxMetaDataObjects.h"
#include "PsInlineArray.h"

//Implement a basic equality comparison system based on the meta data system.
//if you implement a particular areequal specialized to exactly your type
//before including this file it will be called in preference to the completely 
//generic one shown here.


//If you don't care about the failure prop name you are welcome to pass in 'null',
template<typename TBaseObjType>
bool areEqual( const TBaseObjType& lhs, const TBaseObjType& rhs, const char** outFailurePropName );
//We don't have the ability right now to handle these types.
inline bool areEqual( const PxParticleSystem&, const PxParticleSystem& ) { return true; }
inline bool areEqual( const PxParticleFluid&, const PxParticleFluid& ) { return true; }
inline bool areEqual( const PxAggregate&, const PxAggregate& ) { return true; }
inline bool areEqual( const PxSimulationFilterShader&, const PxSimulationFilterShader&  ) { return true; }
inline bool areEqual( const PxSimulationFilterCallback&, const PxSimulationFilterCallback& ) { return true; }
inline bool areEqual( const PxConvexMesh&, const PxConvexMesh& ) { return true; }
inline bool areEqual( const PxTriangleMesh&, const PxTriangleMesh& ) { return true; }
inline bool areEqual( const PxBVH33TriangleMesh&, const PxBVH33TriangleMesh& ) { return true; }
inline bool areEqual( const PxBVH34TriangleMesh&, const PxBVH34TriangleMesh& ) { return true; }
inline bool areEqual( const PxHeightField&, const PxHeightField& ) { return true; }
inline bool areEqual( const void* inLhs, const void* inRhs ) { return inLhs == inRhs; }
inline bool areEqual( void* inLhs, void* inRhs ) { return inLhs == inRhs; }

//Operators are copied, so this object needs to point
//to the important data rather than reference or own it.
template<typename TBaseObjType>
struct EqualityOp
{
	bool* mVal;
	const TBaseObjType* mLhs;
	const TBaseObjType* mRhs;
	const char** mFailurePropName;

	EqualityOp( bool& inVal, const TBaseObjType& inLhs, const TBaseObjType& inRhs, const char*& inFailurePropName ) 
		: mVal( &inVal ) 
		, mLhs( &inLhs ) 
		, mRhs( &inRhs )
		, mFailurePropName( &inFailurePropName )
	{
	}

	bool hasFailed() { return *mVal == false; }
	//Ensure failure propagates the result a ways.
	void update( bool inResult, const char* inName ) 
	{ 
		*mVal = *mVal && inResult; 
		if ( hasFailed() )
			*mFailurePropName = inName;
	}
	
	//ignore any properties pointering back to the scene.
	template<PxU32 TKey, typename TObjType>
	void operator()( const PxReadOnlyPropertyInfo<TKey, TObjType, PxScene*> & inProp, PxU32 ) {}

	template<PxU32 TKey, typename TObjType>
	void operator()( const PxReadOnlyPropertyInfo<TKey, TObjType, const PxScene*> & inProp, PxU32 ) {}

	//ignore all of these properties because they just point back to the 'this' object and cause
	//a stack overflow.

	//Children is unnecessary and articulation points back to the source.
	void operator()( const PxReadOnlyCollectionPropertyInfo<PxPropertyInfoName::PxArticulationLink_Children, PxArticulationLink, PxArticulationLink* >& inProp, PxU32 ) {}
	void operator()( const PxReadOnlyCollectionPropertyInfo<PxPropertyInfoName::PxRigidActor_Constraints, PxRigidActor, PxConstraint* >& inProp, PxU32 ){}
	void operator()( const PxReadOnlyCollectionPropertyInfo<PxPropertyInfoName::PxAggregate_Actors, PxAggregate, PxActor* >& inProp, PxU32 ) {}
#if PX_USE_CLOTH_API
    void operator()( const PxWriteOnlyPropertyInfo<PxPropertyInfoName::PxCloth_TargetPose, PxCloth, const PxTransform &  >& inProp, PxU32 ) {}
	void operator()( const PxWriteOnlyPropertyInfo<PxPropertyInfoName::PxCloth_InertiaScale, PxCloth, PxReal >& inProp, PxU32 ) {}
	void operator()( const PxWriteOnlyPropertyInfo<PxPropertyInfoName::PxCloth_DragCoefficient, PxCloth, PxReal >& inProp, PxU32 ) {}
#endif
		
    template<PxU32 TKey, typename TObjType, typename TGetPropType>
	void operator()( const PxBufferCollectionPropertyInfo<TKey, TObjType, TGetPropType> & inProp, PxU32 )
	{
		
	}
	

    template<PxU32 TKey, typename TObjType, typename TGetPropType>
	void operator()( const PxReadOnlyPropertyInfo<TKey, TObjType, TGetPropType> & inProp, PxU32 )
	{
		if ( hasFailed() ) return;
		TGetPropType lhs( inProp.get( mLhs ) );
		TGetPropType rhs( inProp.get( mRhs ) );
		update( areEqual( lhs, rhs, NULL ), inProp.mName );
	}
	
	template<PxU32 TKey, typename TObjType, typename TPropType>
	void operator()( const PxRangePropertyInfo<TKey, TObjType, TPropType> & inProp, PxU32 )
	{
		if ( hasFailed() ) return;
		TPropType lhsl, lhsr, rhsl, rhsr;
		inProp.get( mLhs, lhsl, lhsr );
		inProp.get( mRhs, rhsl, rhsr );
		update( areEqual( lhsl, rhsl, NULL ), inProp.mName );
		update( areEqual( lhsr, rhsr, NULL ), inProp.mName );
	}

	//Indexed properties where we don't know the range of index types are ignored
	template<PxU32 TKey, typename TObjType, typename TIndexType, typename TPropType>
	void compareIndex( const PxIndexedPropertyInfo<TKey, TObjType, TIndexType, TPropType> &, bool ) {}
	
	template<PxU32 TKey, typename TObjType, typename TIndexType, typename TPropType>
	void compareIndex( const PxIndexedPropertyInfo<TKey, TObjType, TIndexType, TPropType> &inProp, const PxU32ToName* inNames ) 
	{
		for ( const PxU32ToName* theName = inNames;
			theName->mName != NULL && !hasFailed();
			++theName )
		{
			TIndexType theIndex( static_cast<TIndexType>( theName->mValue ) );
			update( areEqual( inProp.get( mLhs, theIndex ), inProp.get( mRhs, theIndex ), NULL ), inProp.mName );
		}
	}
	
	template<PxU32 TKey, typename TObjType, typename TIndexType, typename TPropType>
	void operator()( const PxIndexedPropertyInfo<TKey, TObjType, TIndexType, TPropType> & inProp, PxU32 )
	{
		if ( hasFailed() ) return;
		compareIndex( inProp, PxEnumTraits<TIndexType>().NameConversion );
	}
	
	template<PxU32 TKey, typename TObjType, typename TCollectionType>
	void operator()( const PxReadOnlyCollectionPropertyInfo<TKey, TObjType, TCollectionType> & inProp, PxU32 )
	{
		if ( hasFailed() ) return;
		physx::shdfnd::InlineArray<TCollectionType, 20> lhsArray;
		physx::shdfnd::InlineArray<TCollectionType, 20> rhsArray;
		PxU32 size = inProp.size( mLhs );
		if ( size != inProp.size( mRhs ) )
			update( false, inProp.mName );
		else
		{
			lhsArray.resize( size );
			rhsArray.resize( size );
			inProp.get( mLhs, lhsArray.begin(), size );
			inProp.get( mRhs, rhsArray.begin(), size );
			for ( PxU32 idx =0; idx < size && !hasFailed(); ++idx )
				update( areEqual( lhsArray[idx], rhsArray[idx], NULL ), inProp.mName );
		}
	}

	//Filtered collections where we can't know the range of filter values are ignored.
	template<PxU32 TKey, typename TObjType, typename TFilterType, typename TCollectionType>
	void compare( const PxReadOnlyFilteredCollectionPropertyInfo< TKey, TObjType, TFilterType, TCollectionType >&, bool ) {}

	template<PxU32 TKey, typename TObjType, typename TFilterType, typename TCollectionType>
	void compare( const PxReadOnlyFilteredCollectionPropertyInfo< TKey, TObjType, TFilterType, TCollectionType >& inProp, const PxU32ToName* inNames )
	{
		//Exaustively compare all items.
		physx::shdfnd::InlineArray<TCollectionType*, 20> lhsArray;
		physx::shdfnd::InlineArray<TCollectionType*, 20> rhsArray;
		for ( const PxU32ToName* theName = inNames;
			theName->mName != NULL && !hasFailed();
			++theName )
		{
			TFilterType theFilter( static_cast<TFilterType>( theName->mValue ) );
			PxU32 size = inProp.size( mLhs, theFilter );
			if ( size != inProp.size( mRhs, theFilter ) )
				update( false, inProp.mName );
			else
			{
				lhsArray.resize( size );
				rhsArray.resize( size );
				inProp.get( mLhs, theFilter, lhsArray.begin(), size );
				inProp.get( mRhs, theFilter, rhsArray.begin(), size );
				for ( PxU32 idx =0; idx < size && !hasFailed(); ++idx )
					update( areEqual( lhsArray[idx], rhsArray[idx], NULL ), inProp.mName );
			}
		}
	}

	template<PxU32 TKey, typename TObjType, typename TFilterType, typename TCollectionType>
	void operator()( const PxReadOnlyFilteredCollectionPropertyInfo< TKey, TObjType, TFilterType, TCollectionType >& inProp, PxU32 )
	{
		if ( hasFailed() ) return;
		compare( inProp, PxEnumTraits<TFilterType>().NameConversion );
	}

	template<typename TGeometryType, typename TPropertyType>
	void compareGeometry( const TPropertyType& inProp )
	{
		TGeometryType lhs;
		TGeometryType rhs;
		bool lsuc = inProp.getGeometry( mLhs, lhs );
		bool rsuc = inProp.getGeometry( mRhs, rhs );
		if ( !( lsuc && rsuc ) )
			update( false, inProp.mName );
		else
			update( areEqual( lhs, rhs, NULL ), inProp.mName );
	}

	void operator()( const PxShapeGeometryProperty& inProp, PxU32 )
	{
		if ( hasFailed() ) return;
		PxGeometryType::Enum lhsType( inProp.getGeometryType( mLhs ) );
		PxGeometryType::Enum rhsType( inProp.getGeometryType( mRhs ) );
		if ( lhsType != rhsType )
			update( false, inProp.mName );
		else
		{
			switch( lhsType )
			{
			case PxGeometryType::eSPHERE: compareGeometry<PxSphereGeometry>(inProp); break;
			case PxGeometryType::ePLANE: compareGeometry<PxPlaneGeometry>(inProp); break;
			case PxGeometryType::eCAPSULE: compareGeometry<PxCapsuleGeometry>(inProp); break;
			case PxGeometryType::eBOX: compareGeometry<PxBoxGeometry>(inProp); break;
			case PxGeometryType::eCONVEXMESH: compareGeometry<PxConvexMeshGeometry>(inProp); break;
			case PxGeometryType::eTRIANGLEMESH: compareGeometry<PxTriangleMeshGeometry>(inProp); break;
			case PxGeometryType::eHEIGHTFIELD: compareGeometry<PxHeightFieldGeometry>(inProp); break;
			default: PX_ASSERT( false ); break;
			}
		}
	}
};

inline bool areEqual( const char* lhs, const char* rhs, const char**, const PxUnknownClassInfo& )
{
	if ( lhs && rhs ) return strcmp( lhs, rhs ) == 0;
	if ( lhs || rhs ) return false;
	return true;
}

inline bool areEqual( PxReal inLhs, PxReal inRhs )
{
	return PxAbs( inLhs - inRhs ) < 1e-5f;
}

inline bool areEqual( PxVec3& lhs, PxVec3& rhs ) 
{ 
	return areEqual( lhs.x, rhs.x )
		&& areEqual( lhs.y, rhs.y )
		&& areEqual( lhs.z, rhs.z );
}

inline bool areEqual( const PxVec3& lhs, const PxVec3& rhs ) 
{ 
	return areEqual( lhs.x, rhs.x )
		&& areEqual( lhs.y, rhs.y )
		&& areEqual( lhs.z, rhs.z );
}

inline bool areEqual( const PxVec4& lhs, const PxVec4& rhs ) 
{ 
	return areEqual( lhs.x, rhs.x )
		&& areEqual( lhs.y, rhs.y )
		&& areEqual( lhs.z, rhs.z )
		&& areEqual( lhs.w, rhs.w );
}

inline bool areEqual( const PxQuat& lhs, const PxQuat& rhs )
{
	return areEqual( lhs.x, rhs.x )
		&& areEqual( lhs.y, rhs.y )
		&& areEqual( lhs.z, rhs.z )
		&& areEqual( lhs.w, rhs.w );
}


inline bool areEqual( const PxTransform& lhs, const PxTransform& rhs )
{
	return areEqual(lhs.p, rhs.p) && areEqual(lhs.q, rhs.q);
}


inline bool areEqual( const PxBounds3& inLhs, const PxBounds3& inRhs )
{
	return areEqual(inLhs.minimum,inRhs.minimum)
		&& areEqual(inLhs.maximum,inRhs.maximum);
}

inline bool areEqual( const PxMetaDataPlane& lhs, const PxMetaDataPlane& rhs ) 
{ 
	return areEqual( lhs.normal.x, rhs.normal.x )
		&& areEqual( lhs.normal.y, rhs.normal.y )
		&& areEqual( lhs.normal.z, rhs.normal.z )
		&& areEqual( lhs.distance, rhs.distance );
}

template<typename TBaseObjType>
inline bool areEqual( const TBaseObjType& lhs, const TBaseObjType& rhs ) 
{ 
	return lhs == rhs; 
}

//If we don't know the class type, we must result in == operator
template<typename TBaseObjType>
inline bool areEqual( const TBaseObjType& lhs, const TBaseObjType& rhs, const char**, const PxUnknownClassInfo& )
{
	return areEqual( lhs, rhs );
}

//If we don't know the class type, we must result in == operator
template<typename TBaseObjType, typename TTraitsType>
inline bool areEqual( const TBaseObjType& lhs, const TBaseObjType& rhs, const char** outFailurePropName, const TTraitsType& )
{
	const char* theFailureName = NULL;
	bool result = true;
	static int i = 0;
	++i;
	visitAllProperties<TBaseObjType>( EqualityOp<TBaseObjType>( result, lhs, rhs, theFailureName ) );
	if ( outFailurePropName != NULL && theFailureName )
		*outFailurePropName = theFailureName;
	return result;
}


template<typename TBaseObjType>
inline bool areEqualPointerCheck( const TBaseObjType& lhs, const TBaseObjType& rhs, const char** outFailurePropName, int ) 
{
	return areEqual( lhs, rhs, outFailurePropName, PxClassInfoTraits<TBaseObjType>().Info );
}

inline bool areEqualPointerCheck( const void* lhs, const void* rhs, const char**, bool )
{
	return lhs == rhs;
}

inline bool areEqualPointerCheck( const char* lhs, const char* rhs, const char** outFailurePropName, bool )
{
	bool bRet = true;
	if ( lhs && rhs ) bRet = strcmp( lhs, rhs ) == 0;
	else if ( lhs || rhs ) bRet = false;
	
	return bRet;
}

inline bool areEqualPointerCheck( void* lhs, void* rhs, const char**, bool )
{
	return lhs == rhs;
}

template<typename TBaseObjType>
inline bool areEqualPointerCheck( const TBaseObjType& lhs, const TBaseObjType& rhs, const char** outFailurePropName, bool ) 
{
	if ( lhs && rhs )
		return areEqual( *lhs, *rhs, outFailurePropName );
	if ( lhs || rhs )
		return false;
	return true;
}

template < typename Tp >
struct is_pointer { static const int val = 0; };

template < typename Tp >
struct is_pointer<Tp*> { static const bool val = true; };


template<typename TBaseObjType>
inline bool areEqual( const TBaseObjType& lhs, const TBaseObjType& rhs, const char** outFailurePropName )
{
	return areEqualPointerCheck( lhs, rhs, outFailurePropName, is_pointer<TBaseObjType>::val );
}

#endif
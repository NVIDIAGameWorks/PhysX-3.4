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

#ifndef PVD_META_DATA_DEFINE_PROPERTIES_H
#define PVD_META_DATA_DEFINE_PROPERTIES_H

#if PX_SUPPORT_PVD

#include "PvdMetaDataPropertyVisitor.h"
#include "PxPvdDataStreamHelpers.h"
#include "PxPvdDataStream.h"
#include "PxCoreUtilityTypes.h"


namespace physx
{
namespace Vd
{	
	using namespace physx::shdfnd;
	using namespace physx::pvdsdk;

	template<typename TPropType>
	struct PropertyDefinitionOp
	{
		void defineProperty( PvdPropertyDefinitionHelper& mHelper, NamespacedName mClassKey )
		{
			mHelper.createProperty( mClassKey, "", getPvdNamespacedNameForType<TPropType>(), PropertyType::Scalar );
		}
	};
	template<>
	struct PropertyDefinitionOp<const char*>
	{
		void defineProperty( PvdPropertyDefinitionHelper& mHelper, NamespacedName mClassKey )
		{
			mHelper.createProperty( mClassKey, "", getPvdNamespacedNameForType<StringHandle>(), PropertyType::Scalar );
		}
	};
#define DEFINE_PROPERTY_DEFINITION_OP_NOP( type ) \
	template<> struct PropertyDefinitionOp<type> { void defineProperty( PvdPropertyDefinitionHelper&, NamespacedName ){} };

	//NOP out these two types.
	DEFINE_PROPERTY_DEFINITION_OP_NOP( PxStridedData )
	DEFINE_PROPERTY_DEFINITION_OP_NOP( PxBoundedData )

#define DEFINE_PROPERTY_DEFINITION_OBJECT_REF( type )										\
	template<> struct PropertyDefinitionOp<type> {											\
	void defineProperty( PvdPropertyDefinitionHelper& mHelper, NamespacedName mClassKey)	\
	{																						\
		mHelper.createProperty( mClassKey, "", getPvdNamespacedNameForType<ObjectRef>(), PropertyType::Scalar ); \
	}																						\
	};

	DEFINE_PROPERTY_DEFINITION_OBJECT_REF( PxTriangleMesh* )
	DEFINE_PROPERTY_DEFINITION_OBJECT_REF( PxBVH33TriangleMesh* )
	DEFINE_PROPERTY_DEFINITION_OBJECT_REF( PxBVH34TriangleMesh* )
	DEFINE_PROPERTY_DEFINITION_OBJECT_REF( PxConvexMesh* )
	DEFINE_PROPERTY_DEFINITION_OBJECT_REF( PxHeightField* )


struct PvdClassInfoDefine
{
	PvdPropertyDefinitionHelper& mHelper;
	NamespacedName mClassKey;

	PvdClassInfoDefine( PvdPropertyDefinitionHelper& info, NamespacedName inClassName )
		: mHelper( info )
		, mClassKey( inClassName ) { }

	PvdClassInfoDefine( const PvdClassInfoDefine& other )
		: mHelper( other.mHelper )
		, mClassKey( other.mClassKey )
	{
	}

	void defineProperty( NamespacedName inDtype, const char* semantic = "", PropertyType::Enum inPType = PropertyType::Scalar )
	{
		mHelper.createProperty( mClassKey, semantic, inDtype, inPType ); 
	}

	void pushName( const char* inName )
	{
		mHelper.pushName( inName );
	}
	
	void pushBracketedName( const char* inName) 
	{
		mHelper.pushBracketedName( inName );
	}

	void popName()
	{
		mHelper.popName();
	}

	inline void defineNameValueDefs( const PxU32ToName* theConversions )
	{
		while( theConversions->mName != NULL )
		{
			mHelper.addNamedValue( theConversions->mName, theConversions->mValue );
			++theConversions;
		}
	}

	template<typename TAccessorType>
	void simpleProperty( PxU32, TAccessorType& /*inProp */)
	{
		typedef typename TAccessorType::prop_type TPropertyType;
		PropertyDefinitionOp<TPropertyType>().defineProperty( mHelper, mClassKey );
	}

	template<typename TAccessorType, typename TInfoType>
	void extendedIndexedProperty( PxU32* key, const TAccessorType& inProp, TInfoType&  )
	{
		simpleProperty(*key, inProp);
	}

	template<typename TDataType>
	static NamespacedName getNameForEnumType()
	{
		size_t s = sizeof( TDataType );
		switch(s)
		{
		case 1: return getPvdNamespacedNameForType<PxU8>();
		case 2: return getPvdNamespacedNameForType<PxU16>();
		case 4: return getPvdNamespacedNameForType<PxU32>();
		default: return getPvdNamespacedNameForType<PxU64>();
		}
	}
	
	template<typename TAccessorType>
	void enumProperty( PxU32 /*key*/, TAccessorType& /*inProp*/, const PxU32ToName* inConversions )
	{
		typedef typename TAccessorType::prop_type TPropType;
		defineNameValueDefs( inConversions );
		defineProperty( getNameForEnumType<TPropType>(), "Enumeration Value" );
	}

	template<typename TAccessorType>
	void flagsProperty( PxU32 /*key*/, const TAccessorType& /*inAccessor*/, const PxU32ToName* inConversions )
	{
		typedef typename TAccessorType::prop_type TPropType;
		defineNameValueDefs( inConversions );
		defineProperty( getNameForEnumType<TPropType>(), "Bitflag" );
	}

	template<typename TAccessorType, typename TInfoType>
	void complexProperty( PxU32* key, const TAccessorType& inAccessor, TInfoType& inInfo )
	{
		PxU32 theOffset = inAccessor.mOffset;
		inInfo.visitBaseProperties( makePvdPropertyFilter( *this, key, &theOffset ) );
		inInfo.visitInstanceProperties( makePvdPropertyFilter( *this, key, &theOffset ) );
	}
	
	template<typename TAccessorType, typename TInfoType>
	void bufferCollectionProperty( PxU32* key, const TAccessorType& inAccessor, TInfoType& inInfo )
	{
		complexProperty(key, inAccessor, inInfo);
	}

	template<PxU32 TKey, typename TObjectType, typename TPropertyType, PxU32 TEnableFlag>
	void handleBuffer( const PxBufferPropertyInfo<TKey, TObjectType, PxStrideIterator< const TPropertyType >, TEnableFlag>& inProp )
	{
		mHelper.pushName( inProp.mName );
		defineProperty( getPvdNamespacedNameForType<TPropertyType>(), "", PropertyType::Array );
		mHelper.popName();
	}
	
	template<PxU32 TKey, typename TObjectType, typename TEnumType, typename TStorageType, PxU32 TEnableFlag>
	void handleFlagsBuffer( const PxBufferPropertyInfo<TKey, TObjectType, PxStrideIterator<const PxFlags<TEnumType, TStorageType> >, TEnableFlag>& inProp, const PxU32ToName* inConversion )
	{
		mHelper.pushName( inProp.mName );
		defineNameValueDefs( inConversion );
		defineProperty( getPvdNamespacedNameForType<TStorageType>(), "Bitflag", PropertyType::Array );
		mHelper.popName();
	}

	template<PxU32 TKey, typename TObjectType, typename TPropertyType, PxU32 TEnableFlag>
	void handleBuffer( const PxBufferPropertyInfo<TKey, TObjectType, const Array< TPropertyType >&, TEnableFlag>& inProp )
	{
		mHelper.pushName( inProp.mName );
		defineProperty( getPvdNamespacedNameForType<TPropertyType>(), "", PropertyType::Array );
		mHelper.popName();
	}

	template<PxU32 TKey, typename TObjectType, typename TCollectionType>
	void handleCollection( const PxReadOnlyCollectionPropertyInfo<TKey, TObjectType, TCollectionType>& inProp )
	{
		mHelper.pushName( inProp.mName );
		defineProperty( getPvdNamespacedNameForType<TCollectionType>(), "", PropertyType::Array );
		mHelper.popName();
	}
	
	template<PxU32 TKey, typename TObjectType, typename TEnumType>
	void handleCollection( const PxReadOnlyCollectionPropertyInfo<TKey, TObjectType, TEnumType>& inProp, const PxU32ToName* inConversions )
	{
		mHelper.pushName( inProp.mName );
		defineNameValueDefs( inConversions );
		defineProperty( getNameForEnumType<TEnumType>(), "Enumeration Value", PropertyType::Array );
		mHelper.popName();
	}

private:
	PvdClassInfoDefine& operator=(const PvdClassInfoDefine&);
};

template<typename TPropType>
struct SimplePropertyValueStructOp
{
	void addPropertyMessageArg( PvdPropertyDefinitionHelper& mHelper, PxU32 inOffset )
	{
		mHelper.addPropertyMessageArg<TPropType>( inOffset );
	}
};

#define DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_OP_NOP( type ) \
template<> struct SimplePropertyValueStructOp<type> { void addPropertyMessageArg( PvdPropertyDefinitionHelper&, PxU32 ){}};

DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_OP_NOP( PxStridedData )
DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_OP_NOP( PxBoundedData )

#define DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_VOIDPTR_OP( type )						\
template<> struct SimplePropertyValueStructOp<type> {								\
void addPropertyMessageArg( PvdPropertyDefinitionHelper& mHelper, PxU32 inOffset )	\
{																					\
	mHelper.addPropertyMessageArg<VoidPtr>( inOffset );								\
}																					\
};

DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_VOIDPTR_OP( PxTriangleMesh* )
DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_VOIDPTR_OP( PxBVH33TriangleMesh* )
DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_VOIDPTR_OP( PxBVH34TriangleMesh* )
DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_VOIDPTR_OP( PxConvexMesh* )
DEFINE_SIMPLE_PROPERTY_VALUE_STRUCT_VOIDPTR_OP( PxHeightField* )


struct PvdClassInfoValueStructDefine
{
private:
	PvdClassInfoValueStructDefine& operator=(const PvdClassInfoValueStructDefine&);
public:

	PvdPropertyDefinitionHelper& mHelper;

	PvdClassInfoValueStructDefine( PvdPropertyDefinitionHelper& info )
		: mHelper( info )
	{ }

	PvdClassInfoValueStructDefine( const PvdClassInfoValueStructDefine& other )
		: mHelper( other.mHelper )
	{
	}

	void defineValueStructOffset( const ValueStructOffsetRecord& inProp, PxU32 inPropSize )
	{
		if( inProp.mHasValidOffset )
		{
			switch( inPropSize )
			{
			case 8: mHelper.addPropertyMessageArg<PxU64>( inProp.mOffset ); break;
			case 4: mHelper.addPropertyMessageArg<PxU32>( inProp.mOffset ); break;
			case 2: mHelper.addPropertyMessageArg<PxU16>( inProp.mOffset ); break;
			default: 
				PX_ASSERT(1 == inPropSize);
				mHelper.addPropertyMessageArg<PxU8>( inProp.mOffset ); break;
			}
		}
	}

	void pushName( const char* inName )
	{
		mHelper.pushName( inName );
	}
	
	void pushBracketedName( const char* inName) 
	{
		mHelper.pushBracketedName( inName );
	}

	void popName()
	{
		mHelper.popName();
	}

    template<typename TAccessorType, typename TInfoType>
	void bufferCollectionProperty( PxU32* /*key*/, const TAccessorType& /*inAccessor*/, TInfoType& /*inInfo*/ )
	{
		//complexProperty(key, inAccessor, inInfo);
	}

	template<typename TAccessorType>
	void simpleProperty( PxU32 /*key*/, TAccessorType& inProp )
	{
		typedef typename TAccessorType::prop_type TPropertyType;
		if ( inProp.mHasValidOffset )
		{
			SimplePropertyValueStructOp<TPropertyType>().addPropertyMessageArg( mHelper, inProp.mOffset );
		}
	}
	
	template<typename TAccessorType>
	void enumProperty( PxU32 /*key*/, TAccessorType& inAccessor, const PxU32ToName* /*inConversions */)
	{
		typedef typename TAccessorType::prop_type TPropType;
		defineValueStructOffset( inAccessor, sizeof( TPropType ) );
	}

	template<typename TAccessorType>
	void flagsProperty( PxU32 /*key*/, const TAccessorType& inAccessor, const PxU32ToName* /*inConversions */)
	{
		typedef typename TAccessorType::prop_type TPropType;
		defineValueStructOffset( inAccessor, sizeof( TPropType ) );
	}

	template<typename TAccessorType, typename TInfoType>
	void complexProperty( PxU32* key, const TAccessorType& inAccessor, TInfoType& inInfo )
	{
		PxU32 theOffset = inAccessor.mOffset;
		inInfo.visitBaseProperties( makePvdPropertyFilter( *this, key, &theOffset ) );
		inInfo.visitInstanceProperties( makePvdPropertyFilter( *this, key, &theOffset ) );
	}

	template<PxU32 TKey, typename TObjectType, typename TCollectionType>
	void handleCollection( const PxReadOnlyCollectionPropertyInfo<TKey, TObjectType, TCollectionType>& /*prop*/ )
	{
	}
	
	template<PxU32 TKey, typename TObjectType, typename TEnumType>
	void handleCollection( const PxReadOnlyCollectionPropertyInfo<TKey, TObjectType, TEnumType>& /*prop*/, const PxU32ToName* /*inConversions */)
	{
	}

    template<PxU32 TKey, typename TObjectType, typename TInfoType>
	void handleCollection( const PxBufferCollectionPropertyInfo<TKey, TObjectType, TInfoType>& /*prop*/,  const TInfoType& /*inInfo */)
	{
	}
};

}

}

#endif
#endif

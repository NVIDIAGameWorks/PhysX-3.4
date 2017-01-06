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
#ifndef PX_REPX_SERIALIZER_IMPL_H
#define PX_REPX_SERIALIZER_IMPL_H

#include "PsUserAllocated.h"
#include "SnXmlVisitorWriter.h"
#include "SnXmlVisitorReader.h"

namespace physx { 
	using namespace Sn;

	/**
	 *	The repx serializer impl takes the raw, untyped repx extension interface
	 *	and implements the simpler functions plus does the reinterpret-casts required 
	 *	for any object to implement the serializer safely.
	 */
	template<typename TLiveType>
	struct RepXSerializerImpl : public PxRepXSerializer, shdfnd::UserAllocated
	{
	protected:
		RepXSerializerImpl( const RepXSerializerImpl& inOther );
		RepXSerializerImpl& operator=( const RepXSerializerImpl& inOther );

	public:
		PxAllocatorCallback& mAllocator;

		RepXSerializerImpl( PxAllocatorCallback& inAllocator )
			: mAllocator( inAllocator )
		{
		}
				
		virtual const char* getTypeName() { return PxTypeInfo<TLiveType>::name(); }
		
		virtual void objectToFile( const PxRepXObject& inLiveObject, PxCollection* inCollection, XmlWriter& inWriter, MemoryBuffer& inTempBuffer, PxRepXInstantiationArgs& inArgs )
		{
			const TLiveType* theObj = reinterpret_cast<const TLiveType*>( inLiveObject.serializable );
			objectToFileImpl( theObj, inCollection, inWriter, inTempBuffer, inArgs );
		}

		virtual PxRepXObject fileToObject( XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection )
		{
			TLiveType* theObj( allocateObject( inArgs ) );
			if ( theObj )
				if(fileToObjectImpl( theObj, inReader, inAllocator, inArgs, inCollection ))
					return PxCreateRepXObject(theObj);
			return PxRepXObject();
		}
		
		virtual void objectToFileImpl( const TLiveType* inObj, PxCollection* inCollection, XmlWriter& inWriter, MemoryBuffer& inTempBuffer, PxRepXInstantiationArgs& /*inArgs*/)
		{
			writeAllProperties( inObj, inWriter, inTempBuffer, *inCollection );
		}

		virtual bool fileToObjectImpl( TLiveType* inObj, XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection )
		{
			return readAllProperties( inArgs, inReader, inObj, inAllocator, *inCollection );
		}

		virtual TLiveType* allocateObject( PxRepXInstantiationArgs& inArgs ) = 0;
	};
}

#endif

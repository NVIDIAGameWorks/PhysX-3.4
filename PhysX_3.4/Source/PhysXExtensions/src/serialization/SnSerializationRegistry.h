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

#ifndef PX_PHYSICS_SN_SERIALIZATION_REGISTRY
#define PX_PHYSICS_SN_SERIALIZATION_REGISTRY

#include "PxSerialization.h"
#include "PxRepXSerializer.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PsArray.h"
#include "PsHashMap.h"

namespace physx
{

namespace Cm { class Collection; }

namespace Sn {
	
	class SerializationRegistry : public PxSerializationRegistry, public Ps::UserAllocated
	{
	public:
		SerializationRegistry(PxPhysics& physics);					
		virtual						~SerializationRegistry();

		virtual void				release(){ PX_DELETE(this);  }
		
		PxPhysics&			        getPhysics() const			{ return mPhysics; }
		
		//binary
		void						registerSerializer(PxType type, PxSerializer& serializer);
		PxSerializer*               unregisterSerializer(PxType type);
		void						registerBinaryMetaDataCallback(PxBinaryMetaDataCallback callback);	
		void						getBinaryMetaData(PxOutputStream& stream) const;
		const PxSerializer*			getSerializer(PxType type) const;
		const char*			        getSerializerName(PxU32 index) const;
		PxType                      getSerializerType(PxU32 index) const;
		PxU32                       getNbSerializers() const { return mSerializers.size(); } 
		//repx
		void						registerRepXSerializer(PxType type, PxRepXSerializer& serializer);
		PxRepXSerializer*			getRepXSerializer(const char* typeName) const;
		PxRepXSerializer*           unregisterRepXSerializer(PxType type);
	
	protected:
		SerializationRegistry &operator=(const SerializationRegistry &);
	private:
		typedef Ps::CoalescedHashMap<PxType, PxSerializer*>		SerializerMap;
		typedef Ps::HashMap<PxType, PxRepXSerializer*>	        RepXSerializerMap;

		PxPhysics&										mPhysics;
		SerializerMap									mSerializers;
		RepXSerializerMap								mRepXSerializers;
		Ps::Array<PxBinaryMetaDataCallback>				mMetaDataCallbacks;	
	};

	void  sortCollection(Cm::Collection& collection,  SerializationRegistry& sr, bool isRepx);
} // Sn

} // physx



#endif


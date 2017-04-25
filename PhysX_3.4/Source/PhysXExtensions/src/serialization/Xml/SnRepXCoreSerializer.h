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
#ifndef SN_REPX_CORE_SERIALIZER_H
#define SN_REPX_CORE_SERIALIZER_H
/** \addtogroup RepXSerializers
  @{
*/
#include "foundation/PxSimpleTypes.h"
#include "SnRepXSerializerImpl.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class XmlReader;
	class XmlMemoryAllocator;
	class XmlWriter;
	class MemoryBuffer;
				
	struct PxMaterialRepXSerializer : RepXSerializerImpl<PxMaterial>
	{
		PxMaterialRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxMaterial>( inCallback ) {}
		virtual PxMaterial* allocateObject( PxRepXInstantiationArgs& );
	};

	struct PxShapeRepXSerializer : public RepXSerializerImpl<PxShape>
	{
		PxShapeRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxShape>( inCallback ) {}
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxShape* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};
	
	struct PxBVH33TriangleMeshRepXSerializer  : public RepXSerializerImpl<PxBVH33TriangleMesh>
	{
		PxBVH33TriangleMeshRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxBVH33TriangleMesh>( inCallback ) {}
		virtual void objectToFileImpl( const PxBVH33TriangleMesh*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxBVH33TriangleMesh* allocateObject( PxRepXInstantiationArgs&  ) { return NULL; }
	};
	struct PxBVH34TriangleMeshRepXSerializer  : public RepXSerializerImpl<PxBVH34TriangleMesh>
	{
		PxBVH34TriangleMeshRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxBVH34TriangleMesh>( inCallback ) {}
		virtual void objectToFileImpl( const PxBVH34TriangleMesh*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxBVH34TriangleMesh* allocateObject( PxRepXInstantiationArgs&  ) { return NULL; }
	};

	struct PxHeightFieldRepXSerializer : public RepXSerializerImpl<PxHeightField>
	{
		PxHeightFieldRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxHeightField>( inCallback ) {}
		virtual void objectToFileImpl( const PxHeightField*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxHeightField* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};
	
	struct PxConvexMeshRepXSerializer  : public RepXSerializerImpl<PxConvexMesh>
	{
		PxConvexMeshRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxConvexMesh>( inCallback ) {}
		virtual void objectToFileImpl( const PxConvexMesh*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxConvexMesh* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};

	struct PxRigidStaticRepXSerializer : public RepXSerializerImpl<PxRigidStatic>
	{
		PxRigidStaticRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxRigidStatic>( inCallback ) {}
		virtual PxRigidStatic* allocateObject( PxRepXInstantiationArgs& );
	};

	struct PxRigidDynamicRepXSerializer : public RepXSerializerImpl<PxRigidDynamic>
	{
		PxRigidDynamicRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxRigidDynamic>( inCallback ) {}
		virtual PxRigidDynamic* allocateObject( PxRepXInstantiationArgs& );
	};
	
	struct PxArticulationRepXSerializer  : public RepXSerializerImpl<PxArticulation>
	{
		PxArticulationRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxArticulation>( inCallback ) {}
		virtual void objectToFileImpl( const PxArticulation*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxArticulation* allocateObject( PxRepXInstantiationArgs& );
	};
	
	struct PxAggregateRepXSerializer :  public RepXSerializerImpl<PxAggregate>
	{
		PxAggregateRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxAggregate>( inCallback ) {}
		virtual void objectToFileImpl( const PxAggregate*, PxCollection*, XmlWriter& , MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxAggregate* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};

#if PX_USE_CLOTH_API
	struct PxClothFabricRepXSerializer : public RepXSerializerImpl<PxClothFabric>
	{
		PxClothFabricRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxClothFabric>( inCallback ) {}
		virtual void objectToFileImpl( const PxClothFabric*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxClothFabric* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};

	struct PxClothRepXSerializer : public RepXSerializerImpl<PxCloth>
	{
		PxClothRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxCloth>( inCallback ) {}
		virtual void objectToFileImpl( const PxCloth*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxCloth* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};
#endif
	
#if PX_USE_PARTICLE_SYSTEM_API
	template<typename TParticleType>
	struct PxParticleRepXSerializer : RepXSerializerImpl<TParticleType>
	{
		PxParticleRepXSerializer(PxAllocatorCallback& inCallback) : RepXSerializerImpl<TParticleType>(inCallback) {}
		virtual void objectToFileImpl(const TParticleType*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs&);
		virtual PxRepXObject fileToObject(XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection*);
		virtual TParticleType* allocateObject(PxRepXInstantiationArgs&) { return NULL; }
	};

#if PX_SUPPORT_EXTERN_TEMPLATE
	// explicit template instantiations declarations
	extern template struct PxParticleRepXSerializer<PxParticleSystem>;
	extern template struct PxParticleRepXSerializer<PxParticleFluid>;
#endif
#endif

#if !PX_DOXYGEN
} // namespace physx
#endif

#endif
/** @} */


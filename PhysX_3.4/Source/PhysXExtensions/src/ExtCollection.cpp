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

#include "PxBase.h"
#include "PsArray.h"
#include "PxShape.h"
#include "PxConvexMesh.h"
#include "PxTriangleMesh.h"
#include "PxHeightField.h"
#include "PxMaterial.h"
#include "PxJoint.h"
#include "PxConstraintExt.h"
#include "PxArticulation.h"
#include "PxAggregate.h"
#include "PxPhysics.h"
#include "PxScene.h"
#include "PxPruningStructure.h"
#include "PxCollectionExt.h"


using namespace physx;

void PxCollectionExt::releaseObjects(PxCollection& collection, bool releaseExclusiveShapes)
{
	shdfnd::Array<PxBase*> releasableObjects;

	for (PxU32 i = 0; i < collection.getNbObjects(); ++i)
	{	
		PxBase* s = &collection.getObject(i);
		// pruning structure must be released before its actors
		if(s->is<PxPruningStructure>())
		{
			if(!releasableObjects.empty())
			{
				PxBase* first = releasableObjects[0];
				releasableObjects.pushBack(first);
				releasableObjects[0] = s;
			}
		}
		else
		{
			if (s->isReleasable() && (releaseExclusiveShapes || !s->is<PxShape>() || !s->is<PxShape>()->isExclusive()))
				releasableObjects.pushBack(s);
		}
	}

	for (PxU32 i = 0; i < releasableObjects.size(); ++i)
		releasableObjects[i]->release();		

	while (collection.getNbObjects() > 0)
		collection.remove(collection.getObject(0));
}


void PxCollectionExt::remove(PxCollection& collection, PxType concreteType, PxCollection* to)
{	
	shdfnd::Array<PxBase*> removeObjects;
	
	for (PxU32 i = 0; i < collection.getNbObjects(); i++)
	{
		PxBase& object = collection.getObject(i);
		if(concreteType == object.getConcreteType())
		{
			if(to)
			   to->add(object);	

			removeObjects.pushBack(&object);
		}
	}

	for (PxU32 i = 0; i < removeObjects.size(); ++i)
		collection.remove(*removeObjects[i]);
}

PxCollection* PxCollectionExt::createCollection(PxPhysics& physics)
{
	PxCollection* collection = PxCreateCollection();
	if (!collection)
		return NULL;

	// Collect convexes
	{
		shdfnd::Array<PxConvexMesh*> objects(physics.getNbConvexMeshes());
		const PxU32 nb = physics.getConvexMeshes(objects.begin(), objects.size());
		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect triangle meshes
	{
		shdfnd::Array<PxTriangleMesh*> objects(physics.getNbTriangleMeshes());
		const PxU32 nb = physics.getTriangleMeshes(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect heightfields
	{
		shdfnd::Array<PxHeightField*> objects(physics.getNbHeightFields());
		const PxU32 nb = physics.getHeightFields(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect materials
	{
		shdfnd::Array<PxMaterial*> objects(physics.getNbMaterials());
		const PxU32 nb = physics.getMaterials(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

#if PX_USE_CLOTH_API
	// Collect cloth fabrics
	{
		shdfnd::Array<PxClothFabric*> objects(physics.getNbClothFabrics());
		const PxU32 nb = physics.getClothFabrics(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}
#endif

	// Collect shapes
	{
		shdfnd::Array<PxShape*> objects(physics.getNbShapes());
		const PxU32 nb = physics.getShapes(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}
	return collection;
}

PxCollection* PxCollectionExt::createCollection(PxScene& scene)
{
	PxCollection* collection = PxCreateCollection();
	if (!collection)
		return NULL;

	// Collect actors
	{
		PxActorTypeFlags selectionFlags = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;

#if PX_USE_PARTICLE_SYSTEM_API
		selectionFlags |= PxActorTypeFlag::ePARTICLE_SYSTEM | PxActorTypeFlag::ePARTICLE_FLUID;
#endif
#if PX_USE_CLOTH_API
		selectionFlags |= PxActorTypeFlag::eCLOTH;
#endif

		shdfnd::Array<PxActor*> objects(scene.getNbActors(selectionFlags));
		const PxU32 nb = scene.getActors(selectionFlags, objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}


	// Collect constraints
	{
		shdfnd::Array<PxConstraint*> objects(scene.getNbConstraints());
		const PxU32 nb = scene.getConstraints(objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
		{
			PxU32 typeId;
			PxJoint* joint = reinterpret_cast<PxJoint*>(objects[i]->getExternalReference(typeId));
			if(typeId == PxConstraintExtIDs::eJOINT)
				collection->add(*joint);
		}
	}

	// Collect articulations
	{
		shdfnd::Array<PxArticulation*> objects(scene.getNbArticulations());
		const PxU32 nb = scene.getArticulations(objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect aggregates
	{
		shdfnd::Array<PxAggregate*> objects(scene.getNbAggregates());
		const PxU32 nb = scene.getAggregates(objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	return collection;
}

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


// ****************************************************************************
// This snippet illustrates simple use of the PxSpatialIndex data structure
//
// Note that spatial index has been marked as deprecated and will be removed 
// in future releases
//
// We create a number of spheres, and raycast against them in a random direction 
// from the origin. When a raycast hits a sphere, we teleport it to a random 
// location
// ****************************************************************************

#include <ctype.h>

#include "PxPhysicsAPI.h"

#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetUtils/SnippetUtils.h"

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxFoundation*			gFoundation = NULL;

static const PxU32 SPHERE_COUNT = 500;

float rand01()
{
	return float(rand())/RAND_MAX;
}

struct Sphere : public PxSpatialIndexItem
{
	Sphere() 
	{
		radius = 1;
		resetPosition();
	}

	Sphere(PxReal r)
	: radius(r)
	{
		resetPosition();
	}

	void resetPosition()
	{
		do
		{
			position = PxVec3(rand01()-0.5f, rand01()-0.5f, rand01()-0.5f)*(5+rand01()*5);
		}
		while (position.normalize()==0.0f);
	}

	PxBounds3 getBounds()
	{
		// Geometry queries err on the side of reporting positive results in the face of floating point inaccuracies. 
		// To ensure that a geometry query only reports true when the bounding boxes in the BVH overlap, use 
		// getWorldBounds, which has a third parameter that scales the bounds slightly (default is scaling by 1.01f)

		return PxGeometryQuery::getWorldBounds(PxSphereGeometry(radius), PxTransform(position));
	}

	PxVec3 position;
	PxReal radius;
	PxSpatialIndexItemId id;
};

Sphere					gSpheres[SPHERE_COUNT];
PxSpatialIndex*			gBvh;

void init()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gBvh = PxCreateSpatialIndex();

	// insert the spheres into the BVH, recording the ID so we can later update them
	for(PxU32 i=0;i<SPHERE_COUNT;i++)
		gSpheres[i].id = gBvh->insert(gSpheres[i], gSpheres[i].getBounds());

	// force a full rebuild of the BVH
	gBvh->rebuildFull();

	// hint that should rebuild over the course of approximately 20 rebuildStep() calls
	gBvh->setIncrementalRebuildRate(20);
}


struct HitCallback : public PxSpatialLocationCallback
{
	HitCallback(const PxVec3 p, const PxVec3& d): position(p), direction(d), closest(FLT_MAX), hitSphere(NULL) {}
	PxAgain onHit(PxSpatialIndexItem& item, PxReal distance, PxReal& shrunkDistance)
	{
		PX_UNUSED(distance);

		Sphere& s = static_cast<Sphere&>(item);
		PxRaycastHit hitData;

		// the ray hit the sphere's AABB, now we do a ray-sphere intersection test to find out if the ray hit the sphere

		PxU32 hit = PxGeometryQuery::raycast(position, direction, 
											 PxSphereGeometry(s.radius), PxTransform(s.position),
											 1e6, PxHitFlag::eDEFAULT,
											 1, &hitData);

		// if the raycast hit and it's closer than what we had before, shrink the maximum length of the raycast

		if(hit && hitData.distance < closest)
		{
			closest = hitData.distance;
			hitSphere = &s;
			shrunkDistance = hitData.distance;
		}

		// and continue the query

		return true;
	}

	PxVec3 position, direction;
	PxReal closest;
	Sphere* hitSphere;
};

void step()
{
	for(PxU32 hits=0; hits<10;)
	{
		// raycast in a random direction from the origin, and teleport the closest sphere we find

		PxVec3 dir = PxVec3(rand01()-0.5f, rand01()-0.5f, rand01()-0.5f).getNormalized();
		HitCallback callback(PxVec3(0), dir);
		gBvh->raycast(PxVec3(0), dir, FLT_MAX, callback);
		Sphere* hit = callback.hitSphere;
		if(hit)
		{
			hit->resetPosition();
			gBvh->update(hit->id, hit->getBounds());
			hits++;
		}
	}

	// run an incremental rebuild step in the background
	gBvh->rebuildStep();

}

	
void cleanup()
{
	gBvh->release();
	gFoundation->release();
	
	printf("SnippetSpatialIndex done.\n");
}

int snippetMain(int, const char*const*)
{
	static const PxU32 frameCount = 100;
	init();
	for(PxU32 i=0; i<frameCount; i++)
		step();
	cleanup();

	return 0;
}

/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_COLLISION_H
#define APEX_COLLISION_H

#include "ApexDefs.h"

#include "ApexUsingNamespace.h"
#include "PxVec3.h"
#include "PxMat33.h"

namespace nvidia
{
namespace apex
{



struct Segment
{
	PxVec3 p0;
	PxVec3 p1;
};

struct Capsule : Segment
{
	float radius;
};

struct Triangle
{
	Triangle(const PxVec3& a, const PxVec3& b, const PxVec3& c) : v0(a), v1(b), v2(c) {}
	PxVec3 v0;
	PxVec3 v1;
	PxVec3 v2;
};

struct Box
{
	PxVec3	center;
	PxVec3	extents;
	PxMat33	rot;
};


bool capsuleCapsuleIntersection(const Capsule& worldCaps0, const Capsule& worldCaps1, float tolerance = 1.2);
bool boxBoxIntersection(const Box& worldBox0, const Box& worldBox1);

float APEX_pointTriangleSqrDst(const Triangle& triangle, const PxVec3& position);
float APEX_segmentSegmentSqrDist(const Segment& seg0, const Segment& seg1, float* s, float* t);
float APEX_pointSegmentSqrDist(const Segment& seg, const PxVec3& point, float* param = 0);
uint32_t APEX_RayCapsuleIntersect(const PxVec3& origin, const PxVec3& dir, const Capsule& capsule, float s[2]);

bool APEX_RayTriangleIntersect(const PxVec3& orig, const PxVec3& dir, const PxVec3& a, const PxVec3& b, const PxVec3& c, float& t, float& u, float& v);

} // namespace apex
} // namespace nvidia


#endif // APEX_COLLISION_H

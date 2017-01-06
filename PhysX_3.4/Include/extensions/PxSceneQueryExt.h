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


#ifndef PX_PHYSICS_EXTENSIONS_SCENE_QUERY_H
#define PX_PHYSICS_EXTENSIONS_SCENE_QUERY_H
/** \addtogroup extensions
  @{
*/

#include "PxPhysXConfig.h"

#include "PxScene.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

// These types have been deprecated (removed) in PhysX 3.4. We typedef them to the new types here for easy migration from 3.3 to 3.4.
typedef PxQueryHit				PxSceneQueryHit;
typedef PxQueryFilterData		PxSceneQueryFilterData;
typedef PxQueryFilterCallback	PxSceneQueryFilterCallback;
typedef PxQueryCache			PxSceneQueryCache;
typedef PxHitFlag				PxSceneQueryFlag;
typedef PxHitFlags				PxSceneQueryFlags;

/**
\brief utility functions for use with PxScene, related to scene queries.

Some of these functions have been deprecated (removed) in PhysX 3.4. We re-implement them here for easy migration from 3.3 to 3.4.

@see PxShape
*/

class PxSceneQueryExt
{
public:

	/**
	\brief Raycast returning any blocking hit, not necessarily the closest.
	
	Returns whether any rigid actor is hit along the ray.

	\note Shooting a ray from within an object leads to different results depending on the shape type. Please check the details in article SceneQuery. User can ignore such objects by using one of the provided filter mechanisms.

	\param[in] scene		The scene
	\param[in] origin		Origin of the ray.
	\param[in] unitDir		Normalized direction of the ray.
	\param[in] distance		Length of the ray. Needs to be larger than 0.
	\param[out] hit			Raycast hit information.
	\param[in] filterData	Filtering data and simple logic.
	\param[in] filterCall	Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to be blocking.
	\param[in] cache		Cached hit shape (optional). Ray is tested against cached shape first. If no hit is found the ray gets queried against the scene.
							Note: Filtering is not executed for a cached shape if supplied; instead, if a hit is found, it is assumed to be a blocking hit.
							Note: Using past touching hits as cache will produce incorrect behavior since the cached hit will always be treated as blocking.
	\param[in] queryClient	ID of the client doing the query
	\return True if a blocking hit was found.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFilterData PxSceneQueryFilterCallback PxSceneQueryCache PxSceneQueryHit
	*/
	static bool raycastAny(	const PxScene& scene,
							const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
							PxSceneQueryHit& hit, const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
							PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
							PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT);

	/**
	\brief Raycast returning a single result.
	
	Returns the first rigid actor that is hit along the ray. Data for a blocking hit will be returned as specified by the outputFlags field. Touching hits will be ignored.

	\note Shooting a ray from within an object leads to different results depending on the shape type. Please check the details in article SceneQuery. User can ignore such objects by using one of the provided filter mechanisms.

	\param[in] scene		The scene
	\param[in] origin		Origin of the ray.
	\param[in] unitDir		Normalized direction of the ray.
	\param[in] distance		Length of the ray. Needs to be larger than 0.
	\param[in] outputFlags	Specifies which properties should be written to the hit information
	\param[out] hit			Raycast hit information.
	\param[in] filterData	Filtering data and simple logic.
	\param[in] filterCall	Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to be blocking.
	\param[in] cache		Cached hit shape (optional). Ray is tested against cached shape first then against the scene.
							Note: Filtering is not executed for a cached shape if supplied; instead, if a hit is found, it is assumed to be a blocking hit.
							Note: Using past touching hits as cache will produce incorrect behavior since the cached hit will always be treated as blocking.
	\param[in] queryClient	ID of the client doing the query
	\return True if a blocking hit was found.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFlags PxRaycastHit PxSceneQueryFilterData PxSceneQueryFilterCallback PxSceneQueryCache
	*/
	static bool raycastSingle(	const PxScene& scene,
								const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								PxSceneQueryFlags outputFlags, PxRaycastHit& hit,
								const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
								PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
								PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT);

	/**
	\brief Raycast returning multiple results.
	
	Find all rigid actors that get hit along the ray. Each result contains data as specified by the outputFlags field.

	\note Touching hits are not ordered.

	\note Shooting a ray from within an object leads to different results depending on the shape type. Please check the details in article SceneQuery. User can ignore such objects by using one of the provided filter mechanisms.

	\param[in] scene			The scene
	\param[in] origin			Origin of the ray.
	\param[in] unitDir			Normalized direction of the ray.
	\param[in] distance			Length of the ray. Needs to be larger than 0.
	\param[in] outputFlags		Specifies which properties should be written to the hit information
	\param[out] hitBuffer		Raycast hit information buffer. If the buffer overflows, the blocking hit is returned as the last entry together with an arbitrary subset
								of the nearer touching hits (typically the query should be restarted with a larger buffer).
	\param[in] hitBufferSize	Size of the hit buffer.
	\param[out] blockingHit		True if a blocking hit was found. If found, it is the last in the buffer, preceded by any touching hits which are closer. Otherwise the touching hits are listed.
	\param[in] filterData		Filtering data and simple logic.
	\param[in] filterCall		Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to be touching.
	\param[in] cache			Cached hit shape (optional). Ray is tested against cached shape first then against the scene.
								Note: Filtering is not executed for a cached shape if supplied; instead, if a hit is found, it is assumed to be a blocking hit.
								Note: Using past touching hits as cache will produce incorrect behavior since the cached hit will always be treated as blocking.
	\param[in] queryClient		ID of the client doing the query
	\return Number of hits in the buffer, or -1 if the buffer overflowed.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFlags PxRaycastHit PxSceneQueryFilterData PxSceneQueryFilterCallback PxSceneQueryCache
	*/
	static PxI32 raycastMultiple(	const PxScene& scene,
									const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
									PxSceneQueryFlags outputFlags,
									PxRaycastHit* hitBuffer, PxU32 hitBufferSize, bool& blockingHit,
									const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
									PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
									PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT);

	/**
	\brief Sweep returning any blocking hit, not necessarily the closest.
	
	Returns whether any rigid actor is hit along the sweep path.

	\note If a shape from the scene is already overlapping with the query shape in its starting position, behavior is controlled by the PxSceneQueryFlag::eINITIAL_OVERLAP flag.

	\param[in] scene		The scene
	\param[in] geometry		Geometry of object to sweep (supported types are: box, sphere, capsule, convex).
	\param[in] pose			Pose of the sweep object.
	\param[in] unitDir		Normalized direction of the sweep.
	\param[in] distance		Sweep distance. Needs to be larger than 0. Will be clamped to PX_MAX_SWEEP_DISTANCE.
	\param[in] queryFlags	Combination of PxSceneQueryFlag defining the query behavior
	\param[out] hit			Sweep hit information.
	\param[in] filterData	Filtering data and simple logic.
	\param[in] filterCall	Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to be blocking.
	\param[in] cache		Cached hit shape (optional). Sweep is performed against cached shape first. If no hit is found the sweep gets queried against the scene.
							Note: Filtering is not executed for a cached shape if supplied; instead, if a hit is found, it is assumed to be a blocking hit.
							Note: Using past touching hits as cache will produce incorrect behavior since the cached hit will always be treated as blocking.
	\param[in] queryClient	ID of the client doing the query
	\param[in] inflation	This parameter creates a skin around the swept geometry which increases its extents for sweeping. The sweep will register a hit as soon as the skin touches a shape, and will return the corresponding distance and normal.
	\return True if a blocking hit was found.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFilterData PxSceneQueryFilterCallback PxSceneQueryHit PxSceneQueryCache
	*/
	static bool sweepAny(	const PxScene& scene,
							const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
							PxSceneQueryFlags queryFlags,
							PxSceneQueryHit& hit,
							const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
							PxSceneQueryFilterCallback* filterCall = NULL,
							const PxSceneQueryCache* cache = NULL,
							PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT,
							PxReal inflation = 0.0f);

	/**
	\brief Sweep returning a single result.
	
	Returns the first rigid actor that is hit along the ray. Data for a blocking hit will be returned as specified by the outputFlags field. Touching hits will be ignored.

	\note If a shape from the scene is already overlapping with the query shape in its starting position, behavior is controlled by the PxSceneQueryFlag::eINITIAL_OVERLAP flag.

	\param[in] scene		The scene
	\param[in] geometry		Geometry of object to sweep (supported types are: box, sphere, capsule, convex).
	\param[in] pose			Pose of the sweep object.
	\param[in] unitDir		Normalized direction of the sweep.
	\param[in] distance		Sweep distance. Needs to be larger than 0. Will be clamped to PX_MAX_SWEEP_DISTANCE.
	\param[in] outputFlags	Specifies which properties should be written to the hit information.
	\param[out] hit			Sweep hit information.
	\param[in] filterData	Filtering data and simple logic.
	\param[in] filterCall	Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to be blocking.
	\param[in] cache		Cached hit shape (optional). Sweep is performed against cached shape first then against the scene.
							Note: Filtering is not executed for a cached shape if supplied; instead, if a hit is found, it is assumed to be a blocking hit.
							Note: Using past touching hits as cache will produce incorrect behavior since the cached hit will always be treated as blocking.
	\param[in] queryClient	ID of the client doing the query
	\param[in] inflation	This parameter creates a skin around the swept geometry which increases its extents for sweeping. The sweep will register a hit as soon as the skin touches a shape, and will return the corresponding distance and normal.
	\return True if a blocking hit was found.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFlags PxSweepHit PxSceneQueryFilterData PxSceneQueryFilterCallback PxSceneQueryCache
	*/
	static bool sweepSingle(const PxScene& scene,
							const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
							PxSceneQueryFlags outputFlags,
							PxSweepHit& hit,
							const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
							PxSceneQueryFilterCallback* filterCall = NULL,
							const PxSceneQueryCache* cache = NULL,
							PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT, PxReal inflation=0.0f);

	/**
	\brief Sweep returning multiple results.
	
	Find all rigid actors that get hit along the sweep. Each result contains data as specified by the outputFlags field.

	\note Touching hits are not ordered.

	\note If a shape from the scene is already overlapping with the query shape in its starting position, behavior is controlled by the PxSceneQueryFlag::eINITIAL_OVERLAP flag.

	\param[in] scene			The scene
	\param[in] geometry			Geometry of object to sweep (supported types are: box, sphere, capsule, convex).
	\param[in] pose				Pose of the sweep object.
	\param[in] unitDir			Normalized direction of the sweep.
	\param[in] distance			Sweep distance. Needs to be larger than 0. Will be clamped to PX_MAX_SWEEP_DISTANCE.
	\param[in] outputFlags		Specifies which properties should be written to the hit information.
	\param[out] hitBuffer		Sweep hit information buffer. If the buffer overflows, the blocking hit is returned as the last entry together with an arbitrary subset
								of the nearer touching hits (typically the query should be restarted with a larger buffer).
	\param[in] hitBufferSize	Size of the hit buffer.
	\param[out] blockingHit		True if a blocking hit was found. If found, it is the last in the buffer, preceded by any touching hits which are closer. Otherwise the touching hits are listed.
	\param[in] filterData		Filtering data and simple logic.
	\param[in] filterCall		Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to be touching.
	\param[in] cache			Cached hit shape (optional). Sweep is performed against cached shape first then against the scene.
								Note: Filtering is not executed for a cached shape if supplied; instead, if a hit is found, it is assumed to be a blocking hit.
								Note: Using past touching hits as cache will produce incorrect behavior since the cached hit will always be treated as blocking.
	\param[in] queryClient		ID of the client doing the query
	\param[in] inflation		This parameter creates a skin around the swept geometry which increases its extents for sweeping. The sweep will register a hit as soon as the skin touches a shape, and will return the corresponding distance and normal.
	\return Number of hits in the buffer, or -1 if the buffer overflowed.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFlags PxSweepHit PxSceneQueryFilterData PxSceneQueryFilterCallback PxSceneQueryCache
	*/
	static PxI32 sweepMultiple(	const PxScene& scene,
								const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								PxSceneQueryFlags outputFlags, PxSweepHit* hitBuffer, PxU32 hitBufferSize, bool& blockingHit,
								const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
								PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
								PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT, PxReal inflation = 0.0f);

	/**
	\brief Test overlap between a geometry and objects in the scene.
	
	\note Filtering: Overlap tests do not distinguish between touching and blocking hit types. Both get written to the hit buffer.

	\note PxHitFlag::eMESH_MULTIPLE and PxHitFlag::eMESH_BOTH_SIDES have no effect in this case

	\param[in] scene			The scene
	\param[in] geometry			Geometry of object to check for overlap (supported types are: box, sphere, capsule, convex).
	\param[in] pose				Pose of the object.
	\param[out] hitBuffer		Buffer to store the overlapping objects to. If the buffer overflows, an arbitrary subset of overlapping objects is stored (typically the query should be restarted with a larger buffer).
	\param[in] hitBufferSize	Size of the hit buffer. 
	\param[in] filterData		Filtering data and simple logic.
	\param[in] filterCall		Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to overlap.
	\param[in] queryClient		ID of the client doing the query
	\return Number of hits in the buffer, or -1 if the buffer overflowed.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFlags PxSceneQueryFilterData PxSceneQueryFilterCallback
	*/
	static PxI32 overlapMultiple(	const PxScene& scene,
									const PxGeometry& geometry, const PxTransform& pose,
									PxOverlapHit* hitBuffer, PxU32 hitBufferSize,
									const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
									PxSceneQueryFilterCallback* filterCall = NULL,
									PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT);

	/**
	\brief Test returning, for a given geometry, any overlapping object in the scene.
	
	\note Filtering: Overlap tests do not distinguish between touching and blocking hit types. Both trigger a hit.

	\note PxHitFlag::eMESH_MULTIPLE and PxHitFlag::eMESH_BOTH_SIDES have no effect in this case
	
	\param[in] scene			The scene
	\param[in] geometry			Geometry of object to check for overlap (supported types are: box, sphere, capsule, convex).
	\param[in] pose				Pose of the object.
	\param[out] hit				Pointer to store the overlapping object to.
	\param[in] filterData		Filtering data and simple logic.
	\param[in] filterCall		Custom filtering logic (optional). Only used if the corresponding #PxHitFlag flags are set. If NULL, all hits are assumed to overlap.
	\param[in] queryClient		ID of the client doing the query
	\return True if an overlap was found.

	\deprecated PxActorClientBehaviorFlag feature has been deprecated in PhysX version 3.4

	@see PxSceneQueryFlags PxSceneQueryFilterData PxSceneQueryFilterCallback
	*/
	static bool	overlapAny(	const PxScene& scene,
							const PxGeometry& geometry, const PxTransform& pose,
							PxOverlapHit& hit,
							const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
							PxSceneQueryFilterCallback* filterCall = NULL,
							PX_DEPRECATED PxClientID queryClient = PX_DEFAULT_CLIENT);
};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif

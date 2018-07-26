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


#ifndef PX_PHYSICS_NX_SCENEQUERYDESC
#define PX_PHYSICS_NX_SCENEQUERYDESC
/** \addtogroup physics 
@{ */

#include "PxPhysXConfig.h"
#include "PxClient.h"
#include "PxFiltering.h"
#include "PxQueryFiltering.h"
#include "foundation/PxAssert.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

struct	PxSweepHit;
struct	PxRaycastHit;

/**
\brief Batched query status.

\deprecated The batched query feature has been deprecated in PhysX version 3.4
*/
struct PX_DEPRECATED PxBatchQueryStatus
{
	enum Enum
	{
		/**
		\brief This is the initial state before a query starts.
		*/
		ePENDING = 0,

		/**
		\brief The query is finished; results have been written into the result and hit buffers.
		*/
		eSUCCESS,

		/**
		\brief The query results were incomplete due to touch hit buffer overflow. Blocking hit is still correct.
		*/
		eOVERFLOW
	};
};

/**
\brief Generic struct for receiving results of single query in a batch. Gets templated on hit type PxRaycastHit, PxSweepHit or PxOverlapHit.

\deprecated The batched query feature has been deprecated in PhysX version 3.4
*/
template<typename HitType>
struct PX_DEPRECATED PxBatchQueryResult
{
	HitType			block;			//!< Holds the closest blocking hit for a single query in a batch. Only valid if hasBlock is true.
	HitType*		touches;		//!< This pointer will either be set to NULL for 0 nbTouches or will point
									//!< into the user provided batch query results buffer specified in PxBatchQueryDesc.
	PxU32			nbTouches;		//!< Number of touching hits returned by this query, works in tandem with touches pointer.
	void*			userData;		//!< Copy of the userData pointer specified in the corresponding query.
	PxU8			queryStatus;	//!< Takes on values from PxBatchQueryStatus::Enum.
	bool			hasBlock;		//!< True if there was a blocking hit.
	PxU16			pad;			//!< pads the struct to 16 bytes.

	/** \brief Computes the number of any hits in this result, blocking or touching. */
	PX_INLINE PxU32				getNbAnyHits() const				{ return nbTouches + (hasBlock ? 1 : 0); }

	/** \brief Convenience iterator used to access any hits in this result, blocking or touching. */
	PX_INLINE const HitType&	getAnyHit(const PxU32 index) const	{ PX_ASSERT(index < nbTouches + (hasBlock ? 1 : 0));
																		return index < nbTouches ? touches[index] : block; }
};

/** \brief Convenience typedef for the result of a batched raycast query. */
typedef PX_DEPRECATED PxBatchQueryResult<PxRaycastHit>	PxRaycastQueryResult;

/** \brief Convenience typedef for the result of a batched sweep query. */
typedef PX_DEPRECATED PxBatchQueryResult<PxSweepHit>		PxSweepQueryResult;

/** \brief Convenience typedef for the result of a batched overlap query. */
typedef PX_DEPRECATED PxBatchQueryResult<PxOverlapHit>	PxOverlapQueryResult;

/**
\brief Struct for #PxBatchQuery memory pointers.

\deprecated The batched query feature has been deprecated in PhysX version 3.4
 
@see PxBatchQuery PxBatchQueryDesc
*/
struct PX_DEPRECATED PxBatchQueryMemory
 {
 	/**
	\brief The pointer to the user-allocated buffer for results of raycast queries in corresponding order of issue
 
 	\note The size should be large enough to fit the number of expected raycast queries.
 
 	@see PxRaycastQueryResult 
 	*/
 	PxRaycastQueryResult*			userRaycastResultBuffer;
 
 	/**
 	\brief The pointer to the user-allocated buffer for raycast touch hits.
 	\note The size of this buffer should be large enough to store PxRaycastHit. 
 	If the buffer is too small to store hits, the related PxRaycastQueryResult.queryStatus will be set to eOVERFLOW
 
 	*/
 	PxRaycastHit*					userRaycastTouchBuffer;
 
 	/**
 	\brief The pointer to the user-allocated buffer for results of sweep queries in corresponding order of issue
 
 	\note The size should be large enough to fit the number of expected sweep queries.
 
 	@see PxRaycastQueryResult 
 	*/
 	PxSweepQueryResult*				userSweepResultBuffer;
 
 	/**
 	\brief The pointer to the user-allocated buffer for sweep hits.
 	\note The size of this buffer should be large enough to store PxSweepHit. 
 	If the buffer is too small to store hits, the related PxSweepQueryResult.queryStatus will be set to eOVERFLOW
 
 	*/
 	PxSweepHit*						userSweepTouchBuffer;
 
 	/**
 	\brief The pointer to the user-allocated buffer for results of overlap queries in corresponding order of issue
 
 	\note The size should be large enough to fit the number of expected overlap queries.
 
 	@see PxRaycastQueryResult 
 	*/
 	PxOverlapQueryResult*			userOverlapResultBuffer;
 
 	/**
 	\brief The pointer to the user-allocated buffer for overlap hits.
 	\note The size of this buffer should be large enough to store the hits returned. 
 	If the buffer is too small to store hits, the related PxOverlapQueryResult.queryStatus will be set to eABORTED

 	*/
 	PxOverlapHit*					userOverlapTouchBuffer;
 
 	/** \brief Capacity of the user-allocated userRaycastTouchBuffer in elements */
 	PxU32							raycastTouchBufferSize;
 
 	/** \brief Capacity of the user-allocated userSweepTouchBuffer in elements */
 	PxU32							sweepTouchBufferSize;
 
 	/** \brief Capacity of the user-allocated userOverlapTouchBuffer in elements */
 	PxU32							overlapTouchBufferSize;

 	/** \return Capacity of the user-allocated userRaycastResultBuffer in elements (max number of raycast() calls before execute() call) */
	PX_FORCE_INLINE	PxU32			getMaxRaycastsPerExecute() const	{ return raycastResultBufferSize; }

 	/** \return Capacity of the user-allocated userSweepResultBuffer in elements (max number of sweep() calls before execute() call) */
	PX_FORCE_INLINE	PxU32			getMaxSweepsPerExecute() const		{ return sweepResultBufferSize; }

 	/** \return Capacity of the user-allocated userOverlapResultBuffer in elements (max number of overlap() calls before execute() call) */
	PX_FORCE_INLINE	PxU32			getMaxOverlapsPerExecute() const	{ return overlapResultBufferSize; }

	PxBatchQueryMemory(PxU32 raycastResultBufferSize_, PxU32 sweepResultBufferSize_, PxU32 overlapResultBufferSize_) :
		userRaycastResultBuffer	(NULL),
		userRaycastTouchBuffer	(NULL),
		userSweepResultBuffer	(NULL),
		userSweepTouchBuffer	(NULL),
		userOverlapResultBuffer	(NULL),
		userOverlapTouchBuffer	(NULL),
		raycastTouchBufferSize	(0),
		sweepTouchBufferSize	(0),
		overlapTouchBufferSize	(0),
		raycastResultBufferSize	(raycastResultBufferSize_),
		sweepResultBufferSize	(sweepResultBufferSize_),
		overlapResultBufferSize	(overlapResultBufferSize_)
	{
	}

protected:
 	PxU32							raycastResultBufferSize;
 	PxU32							sweepResultBufferSize;
 	PxU32							overlapResultBufferSize;
};

/**
\brief Descriptor class for #PxBatchQuery.

\deprecated The batched query feature has been deprecated in PhysX version 3.4

@see PxBatchQuery PxSceneQueryExecuteMode
*/
class PX_DEPRECATED PxBatchQueryDesc
{
public:

	/**
	\brief Shared global filter data which will get passed into the filter shader.

	\note The provided data will get copied to internal buffers and this copy will be used for filtering calls.

	<b>Default:</b> NULL

	@see PxSimulationFilterShader
	*/
	void*							filterShaderData;

	/**
	\brief Size (in bytes) of the shared global filter data #filterShaderData.

	<b>Default:</b> 0

	@see PxSimulationFilterShader filterShaderData
	*/
	PxU32							filterShaderDataSize;

	/**
	\brief The custom preFilter shader to use for filtering.

	@see PxBatchQueryPreFilterShader PxDefaultPreFilterShader
	*/
	PxBatchQueryPreFilterShader		preFilterShader;	

		/**
	\brief The custom postFilter shader to use for filtering.

	@see PxBatchQueryPostFilterShader PxDefaultPostFilterShader
	*/
	PxBatchQueryPostFilterShader	postFilterShader;	

	/**
	\brief client that creates and owns this scene query.

	This value will be used as an override when PX_DEFAULT_CLIENT value is passed to the query in PxQueryFilterData.clientId.

	@see PxScene::createClient()
	*/
	PX_DEPRECATED PxClientID		ownerClient;

	/**
	\brief User memory buffers for the query.

	@see PxBatchQueryMemory
	*/
	PxBatchQueryMemory				queryMemory;	

	/**
	\brief Construct a batch query with specified maximum number of queries per batch.

	If the number of raycasts/sweeps/overlaps per execute exceeds the limit, the query will be discarded with a warning.

	\param maxRaycastsPerExecute	Maximum number of raycast() calls allowed before execute() call.
									This has to match the amount of memory allocated for PxBatchQueryMemory::userRaycastResultBuffer.
	\param maxSweepsPerExecute	Maximum number of sweep() calls allowed before execute() call.
									This has to match the amount of memory allocated for PxBatchQueryMemory::userSweepResultBuffer.
	\param maxOverlapsPerExecute	Maximum number of overlap() calls allowed before execute() call.
									This has to match the amount of memory allocated for PxBatchQueryMemory::userOverlapResultBuffer.
	*/
	PX_INLINE						PxBatchQueryDesc(PxU32 maxRaycastsPerExecute, PxU32 maxSweepsPerExecute, PxU32 maxOverlapsPerExecute);
	PX_INLINE bool					isValid() const;
};


PX_INLINE PxBatchQueryDesc::PxBatchQueryDesc(PxU32 maxRaycastsPerExecute, PxU32 maxSweepsPerExecute, PxU32 maxOverlapsPerExecute) :
	filterShaderData		(NULL),
	filterShaderDataSize	(0),
	preFilterShader			(NULL),
	postFilterShader		(NULL),
	ownerClient				(PX_DEFAULT_CLIENT),
	queryMemory				(maxRaycastsPerExecute, maxSweepsPerExecute, maxOverlapsPerExecute)
{
}


PX_INLINE bool PxBatchQueryDesc::isValid() const
{ 
	if ( ((filterShaderDataSize == 0) && (filterShaderData != NULL)) ||
		 ((filterShaderDataSize > 0) && (filterShaderData == NULL)) )
		 return false;

	return true;
}

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif

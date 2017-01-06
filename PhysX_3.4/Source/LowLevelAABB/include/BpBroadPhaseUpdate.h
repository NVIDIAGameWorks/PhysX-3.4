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


#ifndef BP_BROADPHASE_UPDATE_H
#define BP_BROADPHASE_UPDATE_H

#include "foundation/PxAssert.h"
#include "foundation/PxUnionCast.h"
#include "CmPhysXCommon.h"
#include "PxBroadPhase.h"
#include "Ps.h"

namespace physx
{

namespace Bp
{

#if PX_USE_16_BIT_HANDLES
	typedef PxU16 ShapeHandle;
	typedef PxU16 BpHandle;
#define BP_INVALID_BP_HANDLE	0xffff
#else
	typedef PxU32 ShapeHandle;
	typedef PxU32 BpHandle;
#define BP_INVALID_BP_HANDLE	0x3fffffff
#endif

#define ALIGN_SIZE_16(size) ((unsigned(size)+15)&(unsigned(~15)))


	/*
	\brief Encode a single float value with lossless encoding to integer
	*/
	PX_FORCE_INLINE PxU32 encodeFloat(PxU32 ir)
	{
		//we may need to check on -0 and 0
		//But it should make no practical difference.
		if(ir & PX_SIGN_BITMASK) //negative?
			return ~ir;//reverse sequence of negative numbers
		else
			return ir | PX_SIGN_BITMASK; // flip sign
	}

	/*
	\brief Encode a single float value with lossless encoding to integer
	*/
	PX_FORCE_INLINE PxU32 decodeFloat(PxU32 ir)
	{
		if(ir & PX_SIGN_BITMASK) //positive?
			return ir & ~PX_SIGN_BITMASK; //flip sign
		else
			return ~ir; //undo reversal
	}


/**
\brief Integer representation of PxBounds3 used by BroadPhase
@see BroadPhaseUpdateData
*/


typedef PxU32 ValType;

class IntegerAABB
{
public:

	enum
	{
		MIN_X = 0,
		MIN_Y,
		MIN_Z,
		MAX_X,
		MAX_Y,
		MAX_Z
	};

	IntegerAABB();

	IntegerAABB(const PxBounds3& b, PxReal contactDistance)
	{
		encode(PxBounds3(b.minimum-PxVec3(contactDistance), b.maximum+PxVec3(contactDistance)));
	}

	/*
	\brief Return the minimum along a specified axis
	\param[in] i is the axis
	*/
	PX_FORCE_INLINE ValType	getMin(PxU32 i)	const	{	return (mMinMax)[MIN_X+i];	}

	/*
	\brief Return the maximum along a specified axis
	\param[in] i is the axis
	*/
	PX_FORCE_INLINE ValType	getMax(PxU32 i)	const	{	return (mMinMax)[MAX_X+i];	}

	/*
	\brief Return one of the six min/max values of the bound 
	\param[in] isMax determines whether a min or max value is returned
	\param[in] index is the axis 
	*/
	PX_FORCE_INLINE ValType	getExtent(PxU32 isMax, PxU32 index) const
	{
		PX_ASSERT(isMax<=1);
		return (mMinMax)[3*isMax+index];
	}

	/*
	\brief Return the minimum on the x axis
	*/
	PX_FORCE_INLINE ValType getMinX() const { return mMinMax[MIN_X]; }

	/*
	\brief Return the minimum on the y axis
	*/
	PX_FORCE_INLINE ValType getMinY() const { return mMinMax[MIN_Y]; }

	/*
	\brief Return the minimum on the z axis
	*/
	PX_FORCE_INLINE ValType getMinZ() const { return mMinMax[MIN_Z]; }

	/*
	\brief Return the maximum on the x axis
	*/
	PX_FORCE_INLINE ValType getMaxX() const { return mMinMax[MAX_X]; }

	/*
	\brief Return the maximum on the y axis
	*/
	PX_FORCE_INLINE ValType getMaxY() const { return mMinMax[MAX_Y]; }

	/*
	\brief Return the maximum on the z axis
	*/
	PX_FORCE_INLINE ValType getMaxZ() const { return mMinMax[MAX_Z]; }

	/*
	\brief Encode float bounds so they are stored as integer bounds
	\param[in] bounds is the bounds to be encoded 
	\note The integer values of minima are always even, while the integer values of maxima are always odd
	\note The encoding process masks off the last four bits for minima and masks on the last four bits for maxima.
	This keeps the bounds constant when its shape is subjected to small global pose perturbations.  In turn, this helps 
	reduce computational effort in the broadphase update by reducing the amount of sorting required on near-stationary 
	bodies that are aligned along one or more axis.
	@see decode
	*/
	PX_FORCE_INLINE	void encode(const PxBounds3& bounds)
	{
		const PxU32* PX_RESTRICT min = PxUnionCast<const PxU32*, const PxF32*>(&bounds.minimum.x);
		const PxU32* PX_RESTRICT max = PxUnionCast<const PxU32*, const PxF32*>(&bounds.maximum.x);
		//Avoid min=max by enforcing the rule that mins are even and maxs are odd.
		mMinMax[MIN_X] = encodeFloatMin(min[0]);
		mMinMax[MIN_Y] = encodeFloatMin(min[1]);
		mMinMax[MIN_Z] = encodeFloatMin(min[2]);
		mMinMax[MAX_X] = encodeFloatMax(max[0]) | (1<<2);
		mMinMax[MAX_Y] = encodeFloatMax(max[1]) | (1<<2);
		mMinMax[MAX_Z] = encodeFloatMax(max[2]) | (1<<2);
	}

	/*
	\brief Decode from integer bounds to float bounds	
	\param[out] bounds is the decoded float bounds
	\note Encode followed by decode will produce a float bound larger than the original
	due to the masking in encode.
	@see encode
	*/
	PX_FORCE_INLINE	void decode(PxBounds3& bounds)	const
	{
		PxU32* PX_RESTRICT min = PxUnionCast<PxU32*, PxF32*>(&bounds.minimum.x);
		PxU32* PX_RESTRICT max = PxUnionCast<PxU32*, PxF32*>(&bounds.maximum.x);
		min[0] = decodeFloat(mMinMax[MIN_X]);
		min[1] = decodeFloat(mMinMax[MIN_Y]);
		min[2] = decodeFloat(mMinMax[MIN_Z]);
		max[0] = decodeFloat(mMinMax[MAX_X]);
		max[1] = decodeFloat(mMinMax[MAX_Y]);
		max[2] = decodeFloat(mMinMax[MAX_Z]);
	}

	/*
	\brief Encode a single minimum value from integer bounds to float bounds	
	\note The encoding process masks off the last four bits for minima
	@see encode
	*/
	static PX_FORCE_INLINE ValType encodeFloatMin(PxU32 source)
	{
		return ((encodeFloat(source) >> eGRID_SNAP_VAL) - 1) << eGRID_SNAP_VAL;
	}

	/*
	\brief Encode a single maximum value from integer bounds to float bounds	
	\note The encoding process masks on the last four bits for maxima
	@see encode
	*/
	static PX_FORCE_INLINE ValType encodeFloatMax(PxU32 source)
	{
		return ((encodeFloat(source) >> eGRID_SNAP_VAL) + 1) << eGRID_SNAP_VAL;
	}

	/*
	\brief Shift the encoded bounds by a specified vector
	\param[in] shift is the vector used to shift the bounds
	*/
	PX_FORCE_INLINE void shift(const PxVec3& shift)
	{
		::physx::PxBounds3 elemBounds;
		decode(elemBounds);
		elemBounds.minimum -= shift;
		elemBounds.maximum -= shift;
		encode(elemBounds);
	}

	/*
	\brief Test if this aabb lies entirely inside another aabb
	\param[in] box is the other box
	\return True if this aabb lies entirely inside box
	*/
	PX_INLINE bool isInside(const IntegerAABB& box) const
	{
		if(box.mMinMax[MIN_X]>mMinMax[MIN_X])	return false;
		if(box.mMinMax[MIN_Y]>mMinMax[MIN_Y])	return false;
		if(box.mMinMax[MIN_Z]>mMinMax[MIN_Z])	return false;
		if(box.mMinMax[MAX_X]<mMinMax[MAX_X])	return false;
		if(box.mMinMax[MAX_Y]<mMinMax[MAX_Y])	return false;
		if(box.mMinMax[MAX_Z]<mMinMax[MAX_Z])	return false;
		return true;
	}

	/*
	\brief Test if this aabb and another intersect
	\param[in] b is the other box
	\return True if this aabb and b intersect
	*/
	PX_FORCE_INLINE bool intersects(const IntegerAABB& b) const
	{
		return !(b.mMinMax[MIN_X] > mMinMax[MAX_X] || mMinMax[MIN_X] > b.mMinMax[MAX_X] ||
			b.mMinMax[MIN_Y] > mMinMax[MAX_Y] || mMinMax[MIN_Y] > b.mMinMax[MAX_Y] ||
			b.mMinMax[MIN_Z] > mMinMax[MAX_Z] || mMinMax[MIN_Z] > b.mMinMax[MAX_Z]);
	}

	PX_FORCE_INLINE bool intersects1D(const IntegerAABB& b, const PxU32 axis) const
	{
		const PxU32 maxAxis = axis + 3;
		return !(b.mMinMax[axis] > mMinMax[maxAxis] || mMinMax[axis] > b.mMinMax[maxAxis]);
	}


	/*
	\brief Expand bounds to include another
	\note This is used to compute the aggregate bounds of multiple shape bounds
	\param[in] b is the bounds to be included
	*/
	PX_FORCE_INLINE void include(const IntegerAABB& b)
	{
		mMinMax[MIN_X] = PxMin(mMinMax[MIN_X], b.mMinMax[MIN_X]);
		mMinMax[MIN_Y] = PxMin(mMinMax[MIN_Y], b.mMinMax[MIN_Y]);
		mMinMax[MIN_Z] = PxMin(mMinMax[MIN_Z], b.mMinMax[MIN_Z]);
		mMinMax[MAX_X] = PxMax(mMinMax[MAX_X], b.mMinMax[MAX_X]);
		mMinMax[MAX_Y] = PxMax(mMinMax[MAX_Y], b.mMinMax[MAX_Y]);
		mMinMax[MAX_Z] = PxMax(mMinMax[MAX_Z], b.mMinMax[MAX_Z]);
	}

	/*
	\brief Set the bounds to (max, max, max), (min, min, min)
	*/
	PX_INLINE void setEmpty()
	{
		mMinMax[MIN_X] = mMinMax[MIN_Y] = mMinMax[MIN_Z] = 0xff7fffff;  //PX_IR(PX_MAX_F32);
		mMinMax[MAX_X] = mMinMax[MAX_Y] = mMinMax[MAX_Z] = 0x00800000;	///PX_IR(0.0f);
	}


	ValType mMinMax[6];

private:

	enum
	{
		eGRID_SNAP_VAL = 4
	};
};

PX_FORCE_INLINE ValType encodeMin(const PxBounds3& bounds, PxU32 axis, PxReal contactDistance)
{
	const PxReal val = bounds.minimum[axis] - contactDistance;
	const PxU32 min = PxUnionCast<PxU32, PxF32>(val);
	const PxU32 m = IntegerAABB::encodeFloatMin(min);
	return m;

}

PX_FORCE_INLINE ValType encodeMax(const PxBounds3& bounds, PxU32 axis, PxReal contactDistance)
{
	const PxReal val = bounds.maximum[axis] + contactDistance;
	const PxU32 max = PxUnionCast<PxU32, PxF32>(val);
	const PxU32 m = IntegerAABB::encodeFloatMax(max) | (1<<2);
	return m;
}

class BroadPhase;

class BroadPhaseUpdateData
{
public:

     /**
	 \brief A structure detailing the changes to the collection of aabbs, whose overlaps are computed in the broadphase.
	 The structure consists of per-object arrays of object bounds and object groups, and three arrays that index
	 into the per-object arrays, denoting the bounds which are to be created, updated and removed in the broad phase.

	 * each entry in the object arrays represents the same shape or aggregate from frame to frame.
	 * each entry in an index array must be less than the capacity of the per-object arrays.
	 * no index value may appear in more than one index array, and may not occur more than once in that array.

	 An index value is said to be "in use" if it has appeared in a created list in a previous update, and has not
	 since occurred in a removed list.

	 \param[in] created an array of indices describing the bounds that must be inserted into the broadphase.
	 Each index in the array must not be in use.

	 \param[in] updated an array of indices (referencing the boxBounds and boxGroups arrays) describing the bounds
	 that have moved since the last broadphase update. Each index in the array must be in use, and each object
	 whose index is in use and whose AABB has changed must appear in the update list.

	 \param[in] removed an array of indices describing the bounds that must be removed from the broad phase. Each index in
	 the array must be in use.

	 \param[in] boxBounds an array of bounds coordinates for the AABBs to be processed by the broadphase.

	 An entry is valid if its values are integer bitwise representations of floating point numbers that satisfy max>min in each dimension,
	 along with a further rule that minima(maxima) must have even(odd) values. 

	 Each entry whose index is either in use or appears in the created array must be valid. An entry whose index is either not in use or
	 appears in the removed array need not be valid.

	 \param[in]  boxGroups an array of group ids, one for each bound, used for pair filtering.  Bounds with the same group id will not be
	 reported as overlap pairs by the broad phase.  Zero is reserved for static bounds.

	 Entries in this array are immutable: the only way to change the group of an object is to remove it from the broad phase and reinsert
	 it at a different index (recall that each index must appear at most once in the created/updated/removed lists).

	 \param[in]  boxesCapacity the length of the boxBounds and boxGroups arrays.

	 @see BroadPhase::update
	 */
	BroadPhaseUpdateData(
		const ShapeHandle* created, const PxU32 createdSize, 
		const ShapeHandle* updated, const PxU32 updatedSize, 
		const ShapeHandle* removed, const PxU32 removedSize, 
		const PxBounds3* boxBounds, const ShapeHandle* boxGroups, const PxReal* boxContactDistances, const PxU32 boxesCapacity,
		const bool stateChanged) :
		mCreated		(created),
		mCreatedSize	(createdSize),
		mUpdated		(updated),
		mUpdatedSize	(updatedSize),
		mRemoved		(removed),
		mRemovedSize	(removedSize),
		mBoxBounds		(boxBounds),
		mBoxGroups		(boxGroups),
		mContactDistance(boxContactDistances),
		mBoxesCapacity	(boxesCapacity),
		mStateChanged	(stateChanged)
	{
	}

	PX_FORCE_INLINE	const ShapeHandle*	getCreatedHandles()		const { return mCreated;			}
	PX_FORCE_INLINE	PxU32				getNumCreatedHandles()	const { return mCreatedSize;		}

	PX_FORCE_INLINE	const ShapeHandle*	getUpdatedHandles()		const { return mUpdated;			}
	PX_FORCE_INLINE	PxU32				getNumUpdatedHandles()	const { return mUpdatedSize;		}

	PX_FORCE_INLINE	const ShapeHandle*	getRemovedHandles()		const { return mRemoved;			}
	PX_FORCE_INLINE	PxU32				getNumRemovedHandles()	const { return mRemovedSize;		}

	PX_FORCE_INLINE	const PxBounds3*	getAABBs()				const { return mBoxBounds;			}
	PX_FORCE_INLINE	const ShapeHandle*	getGroups()				const { return mBoxGroups;			}
	PX_FORCE_INLINE	PxU32				getCapacity()			const { return mBoxesCapacity;		}

	PX_FORCE_INLINE	const PxReal*		getContactDistance()	const { return mContactDistance;	}

	PX_FORCE_INLINE	bool				getStateChanged()		const { return mStateChanged;		}

#if PX_CHECKED
	static bool isValid(const BroadPhaseUpdateData& updateData, const BroadPhase& bp);
	bool isValid() const;
#endif

private:

	const ShapeHandle*	mCreated;
	PxU32				mCreatedSize;

	const ShapeHandle*	mUpdated;
	PxU32				mUpdatedSize;

	const ShapeHandle*	mRemoved;
	PxU32				mRemovedSize;

	const PxBounds3*	mBoxBounds;
	const ShapeHandle*	mBoxGroups;
	const PxReal*		mContactDistance;
	PxU32				mBoxesCapacity;
	bool				mStateChanged;
};

} //namespace Bp

} //namespace physx

#endif //BP_BROADPHASE_UPDATE_H

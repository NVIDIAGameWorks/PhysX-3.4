/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CONSTRAINED_DISTRIBUTOR_H__
#define __APEX_CONSTRAINED_DISTRIBUTOR_H__

#include "Apex.h"
#include "PsUserAllocated.h"

namespace nvidia
{
namespace apex
{

template <typename T = uint32_t>
class ApexConstrainedDistributor
{
public:
	ApexConstrainedDistributor()
	{
	}

	PX_INLINE void resize(uint32_t size)
	{
		mConstraintDataArray.resize(size);
	}
	PX_INLINE void setBenefit(uint32_t index, float benefit)
	{
		PX_ASSERT(index < mConstraintDataArray.size());
		mConstraintDataArray[index].benefit = benefit;
	}
	PX_INLINE void setTargetValue(uint32_t index, T targetValue)
	{
		PX_ASSERT(index < mConstraintDataArray.size());
		mConstraintDataArray[index].targetValue = targetValue;
	}
	PX_INLINE T getResultValue(uint32_t index) const
	{
		PX_ASSERT(index < mConstraintDataArray.size());
		return mConstraintDataArray[index].resultValue;
	}

	void solve(T totalValueLimit)
	{
		uint32_t size = mConstraintDataArray.size();
		if (size == 0)
		{
			return;
		}
		if (size == 1)
		{
			ConstraintData& data = mConstraintDataArray.front();
			data.resultValue = PxMin(data.targetValue, totalValueLimit);
			return;
		}

		float totalBenefit = 0;
		T totalValue = 0;
		for (uint32_t i = 0; i < size; i++)
		{
			ConstraintData& data = mConstraintDataArray[i];

			totalBenefit += data.benefit;
			totalValue += data.targetValue;

			data.resultValue = data.targetValue;
		}
		if (totalValue <= totalValueLimit)
		{
			//resultValue was setted in prev. for-scope
			return;
		}

		mConstraintSortPairArray.resize(size);
		for (uint32_t i = 0; i < size; i++)
		{
			ConstraintData& data = mConstraintDataArray[i];

			data.weight = (totalValueLimit * data.benefit / totalBenefit);
			if (data.weight > 0)
			{
				mConstraintSortPairArray[i].key = (data.targetValue / data.weight);
			}
			else
			{
				mConstraintSortPairArray[i].key = FLT_MAX;
				data.resultValue = 0; //reset resultValue
			}
			mConstraintSortPairArray[i].index = i;
		}

		nvidia::sort(mConstraintSortPairArray.begin(), size, ConstraintSortPredicate());

		for (uint32_t k = 0; k < size; k++)
		{
			float firstKey = mConstraintSortPairArray[k].key;
			if (firstKey == FLT_MAX)
			{
				break;
			}
			ConstraintData& firstData = mConstraintDataArray[mConstraintSortPairArray[k].index];

			//special case when k == i
			float sumWeight = firstData.weight;
			T sum = firstData.targetValue;
			for (uint32_t i = k + 1; i < size; i++)
			{
				const ConstraintData& data = mConstraintDataArray[mConstraintSortPairArray[i].index];

				sumWeight += data.weight;
				const T value = static_cast<T>(firstKey * data.weight);
				PX_ASSERT(value <= data.targetValue);
				sum += value;
			}

			if (sum > totalValueLimit)
			{
				for (uint32_t i = k; i < size; i++)
				{
					ConstraintData& data = mConstraintDataArray[mConstraintSortPairArray[i].index];

					const T value = static_cast<T>(totalValueLimit * data.weight / sumWeight);
					PX_ASSERT(value <= data.targetValue);
					data.resultValue = value;
				}
				break;
			}
			//allready here: firstData.resultData = firstData.targetValue
			totalValueLimit -= firstData.targetValue;
		}
	}

private:
	struct ConstraintData
	{
		float	benefit;     //input benefit
		T		targetValue; //input constraint on value
		float	weight;      //temp
		T		resultValue; //output
	};
	struct ConstraintSortPair
	{
		float key;
		uint32_t index;
	};
	class ConstraintSortPredicate
	{
	public:
		PX_INLINE bool operator()(const ConstraintSortPair& a, const ConstraintSortPair& b) const
		{
			return a.key < b.key;
		}
	};

	physx::Array<ConstraintData>		mConstraintDataArray;
	physx::Array<ConstraintSortPair>	mConstraintSortPairArray;
};

}
} // end namespace nvidia::apex

#endif

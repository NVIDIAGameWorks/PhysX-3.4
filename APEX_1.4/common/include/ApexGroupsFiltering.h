/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_GROUPS_FILTERING_H__
#define __APEX_GROUPS_FILTERING_H__

#include "ApexDefs.h"

namespace nvidia
{
namespace apex
{

template <typename G>
class ApexGroupsFiltering
{
	typedef void (*FilterOp)(const G& mask0, const G& mask1, G& result);

	static void filterOp_AND(const G& mask0, const G& mask1, G& result)
	{
		result = (mask0 & mask1);
	}
	static void filterOp_OR(const G& mask0, const G& mask1, G& result)
	{
		result = (mask0 | mask1);
	}
	static void filterOp_XOR(const G& mask0, const G& mask1, G& result)
	{
		result = (mask0 ^ mask1);
	}
	static void filterOp_NAND(const G& mask0, const G& mask1, G& result)
	{
		result = ~(mask0 & mask1);
	}
	static void filterOp_NOR(const G& mask0, const G& mask1, G& result)
	{
		result = ~(mask0 | mask1);
	}
	static void filterOp_NXOR(const G& mask0, const G& mask1, G& result)
	{
		result = ~(mask0 ^ mask1);
	}
	static void filterOp_SWAP_AND(const G& mask0, const G& mask1, G& result)
	{
		result = SWAP_AND(mask0, mask1);
	}

	GroupsFilterOp::Enum	mFilterOp0, mFilterOp1, mFilterOp2;
	bool					mFilterBool;
	G						mFilterConstant0;
	G						mFilterConstant1;

public:
	ApexGroupsFiltering()
	{
		mFilterOp0 = mFilterOp1 = mFilterOp2 = GroupsFilterOp::AND;
		mFilterBool = false;
		setZero(mFilterConstant0);
		setZero(mFilterConstant1);
	}

	bool	setFilterOps(GroupsFilterOp::Enum op0, GroupsFilterOp::Enum op1, GroupsFilterOp::Enum op2)
	{
		if (mFilterOp0 != op0 || mFilterOp1 != op1 || mFilterOp2 != op2)
		{
			mFilterOp0 = op0;
			mFilterOp1 = op1;
			mFilterOp2 = op2;
			return true;
		}
		return false;
	}
	void	getFilterOps(GroupsFilterOp::Enum& op0, GroupsFilterOp::Enum& op1, GroupsFilterOp::Enum& op2) const
	{
		op0 = mFilterOp0;
		op1 = mFilterOp1;
		op2 = mFilterOp2;
	}

	bool	setFilterBool(bool flag)
	{
		if (mFilterBool != flag)
		{
			mFilterBool = flag;
			return true;
		}
		return false;
	}
	bool	getFilterBool() const
	{
		return mFilterBool;
	}

	bool	setFilterConstant0(const G& mask)
	{
		if (mFilterConstant0 != mask)
		{
			mFilterConstant0 = mask;
			return true;
		}
		return false;
	}
	G		getFilterConstant0() const
	{
		return mFilterConstant0;
	}
	bool	setFilterConstant1(const G& mask)
	{
		if (mFilterConstant1 != mask)
		{
			mFilterConstant1 = mask;
			return true;
		}
		return false;
	}
	G		getFilterConstant1() const
	{
		return mFilterConstant1;
	}

	bool	operator()(const G& mask0, const G& mask1) const
	{
		static const FilterOp sFilterOpList[] =
		{
			&filterOp_AND,
			&filterOp_OR,
			&filterOp_XOR,
			&filterOp_NAND,
			&filterOp_NOR,
			&filterOp_NXOR,
			&filterOp_SWAP_AND,
		};

		if (hasBits(mask0) & hasBits(mask1))
		{
			G result0, result1, result;
			sFilterOpList[mFilterOp0](mask0, mFilterConstant0, result0);
			sFilterOpList[mFilterOp1](mask1, mFilterConstant1, result1);
			sFilterOpList[mFilterOp2](result0, result1, result);
			return (hasBits(result) == mFilterBool);
		}
		return true;
	}
};


}
} // end namespace nvidia::apex

#endif

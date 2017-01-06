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

#ifndef PT_GRID_CELL_VECTOR_H
#define PT_GRID_CELL_VECTOR_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "foundation/PxVec3.h"
#include <string.h>
#include "PxvConfig.h"
#include "PsMathUtils.h"

namespace physx
{

namespace Pt
{

/*!
Simple integer vector in R3 with basic operations.

Used to define coordinates of a uniform grid cell.
*/
class GridCellVector
{
  public:
	PX_CUDA_CALLABLE PX_FORCE_INLINE GridCellVector()
	{
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE GridCellVector(const GridCellVector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE GridCellVector(PxI16 _x, PxI16 _y, PxI16 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE GridCellVector(const PxVec3& realVec, PxReal scale)
	{
		set(realVec, scale);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE bool operator==(const GridCellVector& v) const
	{
		return ((x == v.x) && (y == v.y) && (z == v.z));
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE bool operator!=(const GridCellVector& v) const
	{
		return ((x != v.x) || (y != v.y) || (z != v.z));
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector operator+(const GridCellVector& v)
	{
		return GridCellVector(x + v.x, y + v.y, z + v.z);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector& operator+=(const GridCellVector& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector operator-(const GridCellVector& v)
	{
		return GridCellVector(x - v.x, y - v.y, z - v.z);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector& operator-=(const GridCellVector& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector& operator=(const GridCellVector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector operator<<(const PxU32 shift) const
	{
		return GridCellVector(x << shift, y << shift, z << shift);
	}

	//! Shift grid cell coordinates (can be used to retrieve coordinates of a coarser grid cell that contains the
	//! defined cell)
	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector operator>>(const PxU32 shift) const
	{
		return GridCellVector(x >> shift, y >> shift, z >> shift);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector& operator<<=(const PxU32 shift)
	{
		x <<= shift;
		y <<= shift;
		z <<= shift;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const GridCellVector& operator>>=(const PxU32 shift)
	{
		x >>= shift;
		y >>= shift;
		z >>= shift;
		return *this;
	}

	//! Set grid cell coordinates based on a point in space and a scaling factor
	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(const PxVec3& realVec, PxReal scale)
	{
		set(realVec * scale);
	}

#ifdef __CUDACC__
	//! Set grid cell coordinates based on a point in space
	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(const PxVec3& realVec)
	{
		x = static_cast<PxI16>(floorf(realVec.x));
		y = static_cast<PxI16>(floorf(realVec.y));
		z = static_cast<PxI16>(floorf(realVec.z));
	}
#else
	//! Set grid cell coordinates based on a point in space
	PX_FORCE_INLINE void set(const PxVec3& realVec)
	{
		x = static_cast<PxI16>(Ps::floor(realVec.x));
		y = static_cast<PxI16>(Ps::floor(realVec.y));
		z = static_cast<PxI16>(Ps::floor(realVec.z));
	}
#endif

	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(PxI16 _x, PxI16 _y, PxI16 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE void setZero()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE bool isZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}

  public:
	PxI16 x;
	PxI16 y;
	PxI16 z;
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_GRID_CELL_VECTOR_H

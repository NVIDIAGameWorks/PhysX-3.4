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

#ifndef GU_AXES_H
#define GU_AXES_H

#include "foundation/PxSimpleTypes.h"

namespace physx
{
namespace Gu
{
	enum PointComponent
	{
		X_					= 0,
		Y_					= 1,
		Z_					= 2,
		W_					= 3,

		PC_FORCE_DWORD		= 0x7fffffff
	};

	enum AxisOrder
	{
		AXES_XYZ			= (X_)|(Y_<<2)|(Z_<<4),
		AXES_XZY			= (X_)|(Z_<<2)|(Y_<<4),
		AXES_YXZ			= (Y_)|(X_<<2)|(Z_<<4),
		AXES_YZX			= (Y_)|(Z_<<2)|(X_<<4),
		AXES_ZXY			= (Z_)|(X_<<2)|(Y_<<4),
		AXES_ZYX			= (Z_)|(Y_<<2)|(X_<<4),

		AXES_FORCE_DWORD	= 0x7fffffff
	};

	class Axes
	{
		public:

		PX_FORCE_INLINE			Axes(AxisOrder order)
								{
									mAxis0 = PxU32((order   ) & 3);
									mAxis1 = PxU32((order>>2) & 3);
									mAxis2 = PxU32((order>>4) & 3);
								}
		PX_FORCE_INLINE			~Axes()		{}

						PxU32	mAxis0;
						PxU32	mAxis1;
						PxU32	mAxis2;
	};
}

}

#endif

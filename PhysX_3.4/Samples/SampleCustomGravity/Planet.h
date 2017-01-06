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

#ifndef PLANET_H
#define PLANET_H

#include "SampleAllocator.h"
#include "foundation/PxVec3.h"

	class PlanetMesh
	{
		public:
											PlanetMesh();
											~PlanetMesh();

							void			generate(const PxVec3& center, PxF32 radius, PxU32 nbX, PxU32 nbY);

			PX_FORCE_INLINE	PxU32			getNbVerts()	const	{ return mNbVerts;	}
			PX_FORCE_INLINE	PxU32			getNbTris()		const	{ return mNbTris;	}
			PX_FORCE_INLINE	const PxVec3*	getVerts()		const	{ return mVerts;	}
			PX_FORCE_INLINE	const PxU32*	getIndices()	const	{ return mIndices;	}

		protected:
							PxU32			mNbVerts;
							PxU32			mNbTris;
							PxVec3*			mVerts;
							PxU32*			mIndices;

							PxU8			checkCulling(const PxVec3& center, PxU32 index0, PxU32 index1, PxU32 index2)	const;
	};

#endif

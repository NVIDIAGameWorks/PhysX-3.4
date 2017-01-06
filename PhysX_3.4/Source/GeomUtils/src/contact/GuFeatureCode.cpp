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

#include "GuConvexEdgeFlags.h"
#include "GuFeatureCode.h"

using namespace physx;
using namespace Gu;

static FeatureCode computeFeatureCode(PxReal u, PxReal v)
{
	// Analysis
	if(u==0.0f)
	{
		if(v==0.0f)
		{
			// Vertex 0
			return FC_VERTEX0;
		}
		else if(v==1.0f)
		{
			// Vertex 2
			return FC_VERTEX2;
		}
		else
		{
			// Edge 0-2
			return FC_EDGE20;
		}
	}
	else if(u==1.0f)
	{
		if(v==0.0f)
		{
			// Vertex 1
			return FC_VERTEX1;
		}
	}
	else
	{
		if(v==0.0f)
		{
			// Edge 0-1
			return FC_EDGE01;
		}
		else
		{
			if((u+v)>=0.9999f)
			{
				// Edge 1-2
				return FC_EDGE12;
			}
			else
			{
				// Face
				return FC_FACE;
			}
		}
	}
	return FC_UNDEFINED;
}


bool Gu::selectNormal(PxU8 data, PxReal u, PxReal v)
{
	bool useFaceNormal = false;
	const FeatureCode FC = computeFeatureCode(u, v);
	switch(FC)
	{
		case FC_VERTEX0:
			if(!(data & (Gu::ETD_CONVEX_EDGE_01|Gu::ETD_CONVEX_EDGE_20)))
				useFaceNormal = true;
			break;
		case FC_VERTEX1:
			if(!(data & (Gu::ETD_CONVEX_EDGE_01|Gu::ETD_CONVEX_EDGE_12)))
				useFaceNormal = true;
			break;
		case FC_VERTEX2:
			if(!(data & (Gu::ETD_CONVEX_EDGE_12|Gu::ETD_CONVEX_EDGE_20)))
				useFaceNormal = true;
			break;
		case FC_EDGE01:
			if(!(data & Gu::ETD_CONVEX_EDGE_01))
				useFaceNormal = true;
			break;
		case FC_EDGE12:
			if(!(data & Gu::ETD_CONVEX_EDGE_12))
				useFaceNormal = true;
			break;
		case FC_EDGE20:
			if(!(data & Gu::ETD_CONVEX_EDGE_20))
				useFaceNormal = true;
			break;
		case FC_FACE:
			useFaceNormal = true;
			break;
		case FC_UNDEFINED:
			break;
	};
	return useFaceNormal;
}


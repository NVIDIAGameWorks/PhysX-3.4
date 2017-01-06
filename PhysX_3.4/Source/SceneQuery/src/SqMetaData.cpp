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

#include "PxMetaData.h"

#include "SqPruningStructure.h"

using namespace physx;
using namespace Sq;

///////////////////////////////////////////////////////////////////////////////

void PruningStructure::getBinaryMetaData(PxOutputStream& stream)
{	
	PX_DEF_BIN_METADATA_VCLASS(stream, PruningStructure)
		PX_DEF_BIN_METADATA_BASE_CLASS(stream, PruningStructure, PxBase)		

		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mNbNodes[0], 0)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mNbNodes[1], 0)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, AABBTreeRuntimeNode, mAABBTreeNodes[0], PxMetaDataFlag::ePTR)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, AABBTreeRuntimeNode, mAABBTreeNodes[1], PxMetaDataFlag::ePTR)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mNbObjects[0], 0)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mNbObjects[1], 0)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mAABBTreeIndices[0], PxMetaDataFlag::ePTR)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mAABBTreeIndices[1], PxMetaDataFlag::ePTR)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxU32, mNbActors, 0)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, PxActor*, mActors, PxMetaDataFlag::ePTR)
		PX_DEF_BIN_METADATA_ITEM(stream, PruningStructure, bool, mValid, 0)
}



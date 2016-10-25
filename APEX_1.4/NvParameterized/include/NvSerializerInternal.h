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
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

#ifndef PX_SERIALIZER_INTERNAL_H
#define PX_SERIALIZER_INTERNAL_H

/*!
\file
\brief NvParameterized serializer factory
*/


#include "nvparameterized/NvSerializer.h"
#include "nvparameterized/NvParameterizedTraits.h"

namespace NvParameterized
{

PX_PUSH_PACK_DEFAULT

/**
\brief A factory function to create an instance of the Serializer class
\param [in] type serializer type (binary, xml, etc.)
\param [in] traits traits-object to do memory allocation, legacy version conversions, callback calls, etc.
*/
Serializer *internalCreateSerializer(Serializer::SerializeType type, Traits *traits);


// Query the current platform
const SerializePlatform &GetCurrentPlatform();
bool GetPlatform(const char *name, SerializePlatform &platform);
const char *GetPlatformName(const SerializePlatform &platform);


PX_POP_PACK

} // end of namespace

#endif

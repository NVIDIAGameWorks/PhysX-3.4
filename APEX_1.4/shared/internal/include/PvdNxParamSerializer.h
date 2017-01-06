/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#if TODO_PVD_NXPARAM_SERIALIZER

#ifndef PVD_NXPARAM_SERIALIZER
#define PVD_NXPARAM_SERIALIZER
#include "PxSimpleTypes.h"
#include "nvparameterized/NvParameterized.h"

namespace PVD
{
class PvdDataStream;
}

namespace NvParameterized
{
class Interface;
}

namespace PvdNxParamSerializer
{

NvParameterized::ErrorType
traverseParamDefTree(NvParameterized::Interface& obj,
                     PVD::PvdDataStream* remoteDebugger,
                     void* curPvdObj,
                     NvParameterized::Handle& handle);

}; // namespacePvdNxParamSerializer

#endif // #ifndef PVD_NXPARAM_SERIALIZER

#endif
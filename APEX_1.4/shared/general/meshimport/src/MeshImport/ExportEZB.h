/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef EXPORT_EZB_H

#define EXPORT_EZB_H

#include "MiFileInterface.h"
#include "MeshImport.h"

#define MESH_BINARY_VERSION 101

namespace mimp
{

void * serializeEZB(const MeshSystem *ms,MiU32 &dlen);

};// end of namespace

#endif

/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef EXPORT_APX_H

#define EXPORT_APX_H

#include "MiFileInterface.h"
#include "MeshImport.h"

namespace mimp
{

void * serializeAPX(const MeshSystem *ms,MiU32 &dlen);

};// end of namespace

#endif

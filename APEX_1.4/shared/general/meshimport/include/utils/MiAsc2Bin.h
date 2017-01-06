/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef ASC2BIN_H
#define ASC2BIN_H

// types:
//
//          f   : 4 byte MiF32
//          d   : 4 byte integer
//          c   : 1 byte character
//          b   : 1 byte integer
//          h   : 2 byte integer
//          p   : 4 byte const char *
//          x1  : 1 byte hex
//          x2  : 2 byte hex
//          x4  : 4 byte hex (etc)
// example usage:
//
//    Asc2Bin("1 2 3 4 5 6",1,"fffff",0);

#include "MiPlatformConfig.h"

namespace mimp
{

void * Asc2Bin(const char *source,const MiI32 count,const char *ctype,void *dest=0);

};

#endif // ASC2BIN_H

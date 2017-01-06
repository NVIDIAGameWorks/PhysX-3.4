/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DECLARE_ARRAY_H

#define DECLARE_ARRAY_H

#error "Do not include DeclareArray.h anywhere!"
// PH: Also, don't use DeclareArray anymore, use physx::Array< >  or Array< > directly

#include "ApexUsingNamespace.h"
#include "PsArray.h"

#define DeclareArray(x) physx::Array< x >


#endif

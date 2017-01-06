/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef ICERADIXSORT_H
#define ICERADIXSORT_H

#include "IceRevisitedRadixBase.h"

namespace nvidia
{
namespace fracture
{

class RadixSort : public nvidia::fracture::base::RadixSort
{
};

}
}

#endif // __ICERADIXSORT_H__
#endif
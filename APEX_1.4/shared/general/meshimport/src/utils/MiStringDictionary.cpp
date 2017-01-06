/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "MiStringDictionary.h"
#pragma  warning(disable:4555)

namespace mimp
{

StringDict *gStringDict=0;

const char *nullstring  = "null";
const char *emptystring = "";

const StringRef StringRef::Null		= SGET( nullstring );
const StringRef StringRef::Empty	= SGET( emptystring );

const StringRef StringRef::EmptyInitializer()
{
	return SGET( emptystring );
}

};

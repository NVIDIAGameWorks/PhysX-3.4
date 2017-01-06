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
#ifndef PX_EXTENSIONS_COMMON_H
#define PX_EXTENSIONS_COMMON_H

#include <stdint.h>
#include "foundation/PxSimpleTypes.h"
#include "PxPhysXConfig.h"

struct PropertyOverride
{
	const char* TypeName;
	const char* PropName;
	const char* OverridePropName;
	PropertyOverride( const char* tn, const char* pn, const char* opn )
		: TypeName( tn )
		, PropName( pn )
		, OverridePropName( opn )
	{
	}
};

//Property overrides will output this exact property name instead of the general
//property name that would be used.  The properties need to have no template arguments
//and exactly the same initialization as the classes they are overriding.
struct DisabledPropertyEntry
{
	const char* mTypeName;
	const char* mPropertyName;
	DisabledPropertyEntry( const char* inTypeName, const char* inValueName )
		: mTypeName( inTypeName )
		, mPropertyName( inValueName )
	{
	}
};


struct CustomProperty
{
	const char* mPropertyType;
	const char* mTypeName;
	const char* mValueStructDefinition;
	const char* mValueStructInit;

	CustomProperty( const char* inTypeName, const char* inName, const char* inPropertyType, const char* valueStructDefinition, const char* valueStructInit )
		: mPropertyType( inPropertyType )
		, mTypeName( inTypeName )
		, mValueStructDefinition( valueStructDefinition )
		, mValueStructInit( valueStructInit )
	{
	}
};

#endif // PX_EXTENSIONS_COMMON_H

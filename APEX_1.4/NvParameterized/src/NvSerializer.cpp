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

/*!
\file
\brief NvParameterized serializer implementation
*/

#include <string.h>
#include <ctype.h>
#include <new> // for placement new

#include "PxSimpleTypes.h"

#include "nvparameterized/NvSerializer.h"
#include "XmlSerializer.h"
#include "BinSerializer.h"
#include "nvparameterized/NvParameterizedTraits.h"

namespace
{
	static bool isInitialized = false;
	static NvParameterized::SerializePlatform platform;
}

namespace NvParameterized
{

const SerializePlatform &GetCurrentPlatform()
{
	if( isInitialized )
		return platform;

	platform.osVer = SerializePlatform::ANY_VERSION; //Do we need this at all???

	//Determine compiler
#	if PX_PS4 == 1
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.compilerVer = SerializePlatform::ANY_VERSION;
#	elif PX_VC != 0
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.compilerVer = _MSC_VER;
#	elif PX_XBOXONE == 1
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.compilerVer = _MSC_VER;
#	elif PX_GCC_FAMILY == 1
		platform.compilerType =SerializePlatform:: COMP_GCC;
		platform.compilerVer = (__GNUC__ << 16) + __GNUC_MINOR__;
#	elif PX_CW == 1
		platform.compilerType = SerializePlatform::COMP_MW;
#		error "TODO: define version of Metrowerks compiler"
#	else
#		error "Unknown compiler"
#	endif

	//Determine OS
#	if PX_WINDOWS_FAMILY == 1
		platform.osType = SerializePlatform::OS_WINDOWS;
#	elif PX_APPLE_FAMILY == 1
		platform.osType = SerializePlatform::OS_MACOSX;
#	elif PX_PS3 == 1
		platform.osType = SerializePlatform::OS_LV2;
#	elif PX_X360 == 1
		platform.osType = SerializePlatform::OS_XBOX;
		platform.osVer = _XBOX_VER;
#	elif PX_XBOXONE == 1
		platform.osType = SerializePlatform::OS_XBOXONE;
#	elif PX_PS4 == 1
		platform.osType = SerializePlatform::OS_PS4;
#	elif PX_ANDROID == 1
		platform.osType = SerializePlatform::OS_ANDROID;
#	elif PX_LINUX == 1
		platform.osType = SerializePlatform::OS_LINUX;
#	elif PX_NX == 1
		platform.osType = SerializePlatform::OS_HOS;		
#	else
#		error "Undefined OS"
#	endif

	//Determine arch
#	if PX_X86 == 1
		platform.archType = SerializePlatform::ARCH_X86;
#	elif PX_APPLE_FAMILY == 1
		platform.archType = SerializePlatform::ARCH_X86;
#	elif PX_X64 == 1
		platform.archType = SerializePlatform::ARCH_X86_64;
#	elif PX_PPC == 1
		platform.archType = SerializePlatform::ARCH_PPC;
#	elif PX_PS3 == 1
		platform.archType = SerializePlatform::ARCH_CELL;
#	elif PX_ARM == 1
		platform.archType = SerializePlatform::ARCH_ARM;
#	elif PX_A64 == 1
		platform.archType = SerializePlatform::ARCH_ARM_64;		
#	else
#		error "Unknown architecture"
#	endif

	isInitialized = true;
	return platform;
}

bool GetPlatform(const char *name, SerializePlatform &platform_)
{
	platform_.osVer = platform_.compilerVer = SerializePlatform::ANY_VERSION;

	if( 0 == strcmp("VcXbox", name) || 0 == strcmp("VcXbox360", name))
	{
		platform_.archType = SerializePlatform::ARCH_PPC;
		platform_.compilerType = SerializePlatform::COMP_VC;
		platform_.osType = SerializePlatform::OS_XBOX;
	}
	else if( 0 == strcmp("VcXboxOne", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86_64;
		platform_.compilerType = SerializePlatform::COMP_VC;
		platform_.osType = SerializePlatform::OS_XBOXONE;
	}
	else if( 0 == strcmp("GccPs4", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86_64;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_PS4;
	}
	else if( 0 == strcmp("VcWin32", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86;
		platform_.compilerType = SerializePlatform::COMP_VC;
		platform_.osType = SerializePlatform::OS_WINDOWS;
	}
	else if( 0 == strcmp("VcWin64", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86_64;
		platform_.compilerType = SerializePlatform::COMP_VC;
		platform_.osType = SerializePlatform::OS_WINDOWS;
	}
	else if( 0 == strcmp("GccPs3", name) )
	{
		platform_.archType = SerializePlatform::ARCH_CELL;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_LV2;
	}
	else if( 0 == strcmp("GccOsX32", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_MACOSX;
	}
	else if( 0 == strcmp("GccOsX64", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86_64;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_MACOSX;
	}
	else if( 0 == strcmp("AndroidARM", name) )
	{
		platform_.archType = SerializePlatform::ARCH_ARM;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_ANDROID;
	}
	else if (0 == strcmp("HOSARM32", name))
	{
		platform_.archType = SerializePlatform::ARCH_ARM;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_HOS;
	}
	else if (0 == strcmp("HOSARM64", name))
	{
		platform_.archType = SerializePlatform::ARCH_ARM_64;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_HOS;
	}	
	else if( 0 == strcmp("GccLinux32", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_LINUX;
	}
	else if( 0 == strcmp("GccLinux64", name) )
	{
		platform_.archType = SerializePlatform::ARCH_X86_64;
		platform_.compilerType = SerializePlatform::COMP_GCC;
		platform_.osType = SerializePlatform::OS_LINUX;
	}
	else if( 0 == strcmp("Pib", name) ) //Abstract platform for platform-independent serialization
	{
		platform_.archType = SerializePlatform::ARCH_GEN;
		platform_.compilerType = SerializePlatform::COMP_GEN;
		platform_.osType = SerializePlatform::OS_GEN;
	}
	else
		return false;

	return true;
}

const char *GetPlatformName(const SerializePlatform &platform_)
{
	static const char *unknown = "<Unknown>";

	switch(platform_.osType )
	{
	case SerializePlatform::OS_XBOX:
		return SerializePlatform::COMP_VC == platform_.compilerType
			? "VcXbox360" : unknown;

	case SerializePlatform::OS_XBOXONE:
		return SerializePlatform::COMP_VC == platform_.compilerType
			? "VcXboxOne" : unknown;

	case SerializePlatform::OS_PS4:
		return SerializePlatform::COMP_GCC == platform_.compilerType
			? "GccPs4" : unknown;

	case SerializePlatform::OS_WINDOWS:
		if( SerializePlatform::COMP_VC != platform_.compilerType )
			return unknown;

		switch(platform_.archType )
		{
		case SerializePlatform::ARCH_X86:
			return "VcWin32";

		case SerializePlatform::ARCH_X86_64:
			return "VcWin64";

		case SerializePlatform::ARCH_GEN:
		case SerializePlatform::ARCH_PPC:
		case SerializePlatform::ARCH_CELL:
		case SerializePlatform::ARCH_ARM:
		case SerializePlatform::ARCH_LAST:
		default:
			return unknown;
		}

	case SerializePlatform::OS_MACOSX:
			if( SerializePlatform::COMP_GCC != platform_.compilerType )
				return unknown;

			switch( platform_.archType )
		{
			case SerializePlatform::ARCH_X86:
				return "GccOsX32";

			case SerializePlatform::ARCH_X86_64:
				return "GccOsX64";

			default:
				return unknown;
		}

	case SerializePlatform::OS_LV2:
		return SerializePlatform::COMP_GCC == platform_.compilerType
			? "GccPs3" : unknown;

	case SerializePlatform::OS_GEN:
		return "Pib";

	case SerializePlatform::OS_ANDROID:
		return SerializePlatform::ARCH_ARM == platform_.archType
			? "AndroidARM" : 0;

	case SerializePlatform::OS_LINUX:
		if( SerializePlatform::COMP_GCC != platform_.compilerType )
			return unknown;

		switch(platform_.archType )
		{
		case SerializePlatform::ARCH_X86:
			return "GccLinux32";

		case SerializePlatform::ARCH_X86_64:
			return "GccLinux64";

		case SerializePlatform::ARCH_GEN:
		case SerializePlatform::ARCH_PPC:
		case SerializePlatform::ARCH_CELL:
		case SerializePlatform::ARCH_ARM:
		case SerializePlatform::ARCH_LAST:
		default:
			return unknown;
		}

	case SerializePlatform::OS_HOS:
		switch (platform.archType)
		{
		case SerializePlatform::ARCH_ARM:
			return "HOSARM32";
		case SerializePlatform::ARCH_ARM_64:
			return "HOSARM64";

		default:
			return unknown;
		}

	case SerializePlatform::OS_LAST:
	default:
		return unknown;
	}
}

Serializer *internalCreateSerializer(Serializer::SerializeType type, Traits *traits)
{
	switch ( type )
	{
		case Serializer::NST_XML:
			{
				void *buf = serializerMemAlloc(sizeof(XmlSerializer), traits);
				return buf ? PX_PLACEMENT_NEW(buf, XmlSerializer)(traits) : 0;
			}
		case Serializer::NST_BINARY:
			{
				void *buf = serializerMemAlloc(sizeof(BinSerializer), traits);
				return buf ? PX_PLACEMENT_NEW(buf, BinSerializer)(traits) : 0;
			}
		case Serializer::NST_LAST:
		default:
			NV_PARAM_TRAITS_WARNING(
				traits,
				"Unknown serializer type: %d",
				(int)type );
			break;
	}

	return 0;
}

Serializer::SerializeType Serializer::peekSerializeType(physx::PxFileBuf &stream)
{
	return isBinaryFormat(stream) ? Serializer::NST_BINARY
		: isXmlFormat(stream) ? Serializer::NST_XML : Serializer::NST_LAST;
}

Serializer::ErrorType Serializer::peekPlatform(physx::PxFileBuf &stream, SerializePlatform &platform_)
{
	if( isBinaryFormat(stream) )
		return peekBinaryPlatform(stream, platform_);
	
	//Xml has no native platform
	platform_ = GetCurrentPlatform();
	return Serializer::ERROR_NONE;
}

Serializer::ErrorType Serializer::deserializeMetadata(physx::PxFileBuf & /*stream*/, DeserializedMetadata & /*desData*/)
{
	//Xml currently does not implement this
	return Serializer::ERROR_NOT_IMPLEMENTED;
}

Serializer::ErrorType Serializer::deserialize(physx::PxFileBuf &stream, Serializer::DeserializedData &desData)
{
	bool tmp;
	return deserialize(stream, desData, tmp);
}

Serializer::ErrorType Serializer::deserializeInplace(void *data, uint32_t dataLen, Serializer::DeserializedData &desData)
{
	bool tmp;
	return deserializeInplace(data, dataLen, desData, tmp);
}

}; // end of namespace

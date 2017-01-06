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

#ifndef PSFASTXML_PSFASTXML_H
#define PSFASTXML_PSFASTXML_H

#include "foundation/PxSimpleTypes.h" // defines basic data types; modify for your platform as needed.
#include "foundation/PxIO.h"
#include "foundation/PxAssert.h"
#include "PsAllocator.h"

namespace physx
{
namespace shdfnd
{

class FastXml
{
	PX_NOCOPY(FastXml)

  public:
	class AttributePairs
	{
		int argc;
		const char** argv;

	  public:
		AttributePairs() : argc(0), argv(NULL)
		{
		}
		AttributePairs(int c, const char** v) : argc(c), argv(v)
		{
		}

		PX_INLINE int getNbAttr() const
		{
			return argc / 2;
		}

		const char* getKey(uint32_t index) const
		{
			PX_ASSERT((index * 2) < uint32_t(argc));
			return argv[index * 2];
		}

		const char* getValue(uint32_t index) const
		{
			PX_ASSERT((index * 2 + 1) < uint32_t(argc));
			return argv[index * 2 + 1];
		}

		const char* get(const char* attr) const
		{
			int32_t count = argc / 2;
			for(int32_t i = 0; i < count; ++i)
			{
				const char* key = argv[i * 2], *value = argv[i * 2 + 1];
				if(strcmp(key, attr) == 0)
					return value;
			}

			return NULL;
		}
	};

	/***
	* Callbacks to the user with the contents of the XML file properly digested.
	*/
	class Callback
	{
	  public:
		virtual ~Callback()
		{
		}
		virtual bool processComment(const char* comment) = 0; // encountered a comment in the XML

		// 'element' is the name of the element that is being closed.
		// depth is the recursion depth of this element.
		// Return true to continue processing the XML file.
		// Return false to stop processing the XML file; leaves the read pointer of the stream right after this close
		// tag.
		// The bool 'isError' indicates whether processing was stopped due to an error, or intentionally canceled early.
		virtual bool processClose(const char* element, uint32_t depth, bool& isError) = 0; // process the 'close'
		// indicator for a previously
		// encountered element

		// return true to continue processing the XML document, false to skip.
		virtual bool processElement(const char* elementName,    // name of the element
		                            const char* elementData,    // element data, null if none
		                            const AttributePairs& attr, // attributes
		                            int32_t lineno) = 0;        // line number in the source XML file

		// process the XML declaration header
		virtual bool processXmlDeclaration(const AttributePairs&, // attributes
		                                   const char* /*elementData*/, int32_t /*lineno*/)
		{
			return true;
		}

		virtual bool processDoctype(const char* /*rootElement*/, // Root element tag
		                            const char* /*type*/,        // SYSTEM or PUBLIC
		                            const char* /*fpi*/,         // Formal Public Identifier
		                            const char* /*uri*/)         // Path to schema file
		{
			return true;
		}

		virtual void* allocate(uint32_t size)
		{
			return getAllocator().allocate(size, "FastXml", __FILE__, __LINE__);
		}

		virtual void deallocate(void* ptr)
		{
			getAllocator().deallocate(ptr);
		}
	};

	virtual bool processXml(PxInputData& buff, bool streamFromMemory = false) = 0;

	virtual const char* getError(int32_t& lineno) = 0; // report the reason for a parsing error, and the line number
	// where it occurred.

	FastXml()
	{
	}

	virtual void release(void) = 0;

  protected:
	virtual ~FastXml()
	{
	}
};

FastXml* createFastXml(FastXml::Callback* iface);

} // shdfnd
} // physx

#endif // PSFASTXML_PSFASTXML_H

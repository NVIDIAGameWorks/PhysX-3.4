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

#ifndef PX_INPUT_DATA_FROM_PX_FILE_BUF_H

#define PX_INPUT_DATA_FROM_PX_FILE_BUF_H

#include "PxFileBuf.h"
#include "PxIO.h"

namespace physx
{
	class PxInputDataFromPxFileBuf: public physx::PxInputData
	{
	public:
		PxInputDataFromPxFileBuf(physx::PxFileBuf& fileBuf) : mFileBuf(fileBuf) {}

		// physx::PxInputData interface
		virtual uint32_t	getLength() const
		{
			return mFileBuf.getFileLength();
		}

		virtual void	seek(uint32_t offset)
		{
			mFileBuf.seekRead(offset);
		}

		virtual uint32_t	tell() const
		{
			return mFileBuf.tellRead();
		}

		// physx::PxInputStream interface
		virtual uint32_t read(void* dest, uint32_t count)
		{
			return mFileBuf.read(dest, count);
		}

		PX_NOCOPY(PxInputDataFromPxFileBuf)
	private:
		physx::PxFileBuf& mFileBuf;
	};
};

#endif

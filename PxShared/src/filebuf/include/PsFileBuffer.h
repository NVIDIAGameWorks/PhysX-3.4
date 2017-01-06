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

#ifndef PSFILEBUFFER_PSFILEBUFFER_H
#define PSFILEBUFFER_PSFILEBUFFER_H

#include "filebuf/PxFileBuf.h"

#include "Ps.h"
#include "PsUserAllocated.h"
#include <stdio.h>

namespace physx
{
namespace general_PxIOStream2
{
	using namespace shdfnd;

//Use this class if you want to use your own allocator
class PxFileBufferBase : public PxFileBuf
{
public:
	PxFileBufferBase(const char *fileName,OpenMode mode)
	{
		mOpenMode = mode;
		mFph = NULL;
		mFileLength = 0;
		mSeekRead   = 0;
		mSeekWrite  = 0;
		mSeekCurrent = 0;
		switch ( mode )
		{
			case OPEN_READ_ONLY:
				mFph = fopen(fileName,"rb");
				break;
			case OPEN_WRITE_ONLY:
				mFph = fopen(fileName,"wb");
				break;
			case OPEN_READ_WRITE_NEW:
				mFph = fopen(fileName,"wb+");
				break;
			case OPEN_READ_WRITE_EXISTING:
				mFph = fopen(fileName,"rb+");
				break;
			case OPEN_FILE_NOT_FOUND:
				break;
		}
		if ( mFph )
		{
			fseek(mFph,0L,SEEK_END);
			mFileLength = static_cast<uint32_t>(ftell(mFph));
			fseek(mFph,0L,SEEK_SET);
		}
		else
		{
			mOpenMode = OPEN_FILE_NOT_FOUND;
		}
    }

	virtual						~PxFileBufferBase()
	{
		close();
	}

	virtual void close()
	{
		if( mFph )
		{
			fclose(mFph);
			mFph = 0;
		}
	}

	virtual SeekType isSeekable(void) const
	{
		return mSeekType;
	}

	virtual		uint32_t			read(void* buffer, uint32_t size)	
	{
		uint32_t ret = 0;
		if ( mFph )
		{
			setSeekRead();
			ret = static_cast<uint32_t>(::fread(buffer,1,size,mFph));
			mSeekRead+=ret;
			mSeekCurrent+=ret;
		}
		return ret;
	}

	virtual		uint32_t			peek(void* buffer, uint32_t size)
	{
		uint32_t ret = 0;
		if ( mFph )
		{
			uint32_t loc = tellRead();
			setSeekRead();
			ret = static_cast<uint32_t>(::fread(buffer,1,size,mFph));
			mSeekCurrent+=ret;
			seekRead(loc);
		}
		return ret;
	}

	virtual		uint32_t		write(const void* buffer, uint32_t size)
	{
		uint32_t ret = 0;
		if ( mFph )
		{
			setSeekWrite();
			ret = static_cast<uint32_t>(::fwrite(buffer,1,size,mFph));
			mSeekWrite+=ret;
			mSeekCurrent+=ret;
			if ( mSeekWrite > mFileLength )
			{
				mFileLength = mSeekWrite;
			}
		}
		return ret;
	}

	virtual uint32_t tellRead(void) const
	{
		return mSeekRead;
	}

	virtual uint32_t tellWrite(void) const
	{
		return mSeekWrite;
	}

	virtual uint32_t seekRead(uint32_t loc) 
	{
		mSeekRead = loc;
		if ( mSeekRead > mFileLength )
		{
			mSeekRead = mFileLength;
		}
		return mSeekRead;
	}

	virtual uint32_t seekWrite(uint32_t loc)
	{
		mSeekWrite = loc;
		if ( mSeekWrite > mFileLength )
		{
			mSeekWrite = mFileLength;
		}
		return mSeekWrite;
	}

	virtual void flush(void)
	{
		if ( mFph )
		{
			::fflush(mFph);
		}
	}

	virtual OpenMode	getOpenMode(void) const
	{
		return mOpenMode;
	}

	virtual uint32_t getFileLength(void) const
	{
		return mFileLength;
	}

private:
	// Moves the actual file pointer to the current read location
	void setSeekRead(void) 
	{
		if ( mSeekRead != mSeekCurrent && mFph )
		{
			if ( mSeekRead >= mFileLength )
			{
				fseek(mFph,0L,SEEK_END);
			}
			else
			{
				fseek(mFph,static_cast<long>(mSeekRead),SEEK_SET);
			}
			mSeekCurrent = mSeekRead = static_cast<uint32_t>(ftell(mFph));
		}
	}
	// Moves the actual file pointer to the current write location
	void setSeekWrite(void)
	{
		if ( mSeekWrite != mSeekCurrent && mFph )
		{
			if ( mSeekWrite >= mFileLength )
			{
				fseek(mFph,0L,SEEK_END);
			}
			else
			{
				fseek(mFph,static_cast<long>(mSeekWrite),SEEK_SET);
			}
			mSeekCurrent = mSeekWrite = static_cast<uint32_t>(ftell(mFph));
		}
	}


	FILE		*mFph;
	uint32_t		mSeekRead;
	uint32_t		mSeekWrite;
	uint32_t		mSeekCurrent;
	uint32_t		mFileLength;
	SeekType	mSeekType;
	OpenMode	mOpenMode;
};

//Use this class if you want to use PhysX memory allocator
class PsFileBuffer: public PxFileBufferBase, public UserAllocated
{
public:
	PsFileBuffer(const char *fileName,OpenMode mode): PxFileBufferBase(fileName, mode) {}
};

}
using namespace general_PxIOStream2;
}

#endif // PSFILEBUFFER_PSFILEBUFFER_H

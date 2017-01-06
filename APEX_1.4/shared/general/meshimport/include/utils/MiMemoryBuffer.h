/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MI_MEMORY_BUFFER_H
#define MI_MEMORY_BUFFER_H

#include "MiFileBuf.h"

namespace mimp
{

	const MiU32 BUFFER_SIZE_DEFAULT = 4096;

//Use this class if you want to use your own allocator
class MiMemoryBufferBase : public MiFileBuf
{
	void init(const void *readMem, MiU32 readLen)
	{
		mReadBuffer = mReadLoc = (const MiU8 *)readMem;
		mReadStop   = &mReadLoc[readLen];

		mWriteBuffer = mWriteLoc = mWriteStop = NULL;
		mWriteBufferSize = 0;
		mDefaultWriteBufferSize = BUFFER_SIZE_DEFAULT;

		mOpenMode = OPEN_READ_ONLY;
		mSeekType = SEEKABLE_READ;
	}

	void init(MiU32 defaultWriteBufferSize)
	{
		mReadBuffer = mReadLoc = mReadStop = NULL;
		mWriteBuffer = mWriteLoc = mWriteStop = NULL;
		mWriteBufferSize = 0;
		mDefaultWriteBufferSize = defaultWriteBufferSize;

		mOpenMode = OPEN_READ_WRITE_NEW;
		mSeekType = SEEKABLE_READWRITE;
	}

public:
	MiMemoryBufferBase(const void *readMem,MiU32 readLen)
	{
		init(readMem, readLen);
    }

	MiMemoryBufferBase(MiU32 defaultWriteBufferSize = BUFFER_SIZE_DEFAULT)
    {
		init(defaultWriteBufferSize);
	}

	virtual ~MiMemoryBufferBase(void)
	{
		reset();
	}

	void initWriteBuffer(MiU32 size)
	{
		if ( mWriteBuffer == NULL )
		{
			if ( size < mDefaultWriteBufferSize ) size = mDefaultWriteBufferSize;
			mWriteBuffer = (MiU8 *)MI_ALLOC(size);
			MI_ASSERT( mWriteBuffer );
    		mWriteLoc    = mWriteBuffer;
    		mWriteStop	= &mWriteBuffer[size];
    		mWriteBufferSize = size;
    		mReadBuffer = mWriteBuffer;
    		mReadStop	= mWriteBuffer;
    		mReadLoc    = mWriteBuffer;
		}
    }

	void reset(void)
	{
		MI_FREE(mWriteBuffer);
		mWriteBuffer = NULL;
		mWriteBufferSize = 0;
		mWriteLoc = NULL;
		mReadBuffer = NULL;
		mReadStop = NULL;
		mReadLoc = NULL;
    }

	virtual OpenMode	getOpenMode(void) const
	{
		return mOpenMode;
	}


	SeekType isSeekable(void) const
	{
		return mSeekType;
	}

	virtual		MiU32			read(void* buffer, MiU32 size)
	{
		if ( (mReadLoc+size) > mReadStop )
		{
			size = (MiU32)(mReadStop - mReadLoc);
		}
		if ( size != 0 )
		{
			memmove(buffer,mReadLoc,size);
			mReadLoc+=size;
		}
		return size;
	}

	virtual		MiU32			peek(void* buffer, MiU32 size)
	{
		if ( (mReadLoc+size) > mReadStop )
		{
			size = (MiU32)(mReadStop - mReadLoc);
		}
		if ( size != 0 )
		{
			memmove(buffer,mReadLoc,size);
		}
		return size;
	}

	virtual		MiU32		write(const void* buffer, MiU32 size)
	{
		MI_ASSERT( mOpenMode ==	OPEN_READ_WRITE_NEW );
		if ( mOpenMode == OPEN_READ_WRITE_NEW )
		{
    		if ( (mWriteLoc+size) > mWriteStop )
    		    growWriteBuffer(size);
    		memmove(mWriteLoc,buffer,size);
    		mWriteLoc+=size;
    		mReadStop = mWriteLoc;
    	}
    	else
    	{
    		size = 0;
    	}
		return size;
	}

	MI_INLINE const MiU8 * getReadLoc(void) const { return mReadLoc; };
	MI_INLINE void advanceReadLoc(MiU32 len)
	{
		MI_ASSERT(mReadBuffer);
		if ( mReadBuffer )
		{
			mReadLoc+=len;
			if ( mReadLoc >= mReadStop )
			{
				mReadLoc = mReadStop;
			}
		}
	}

	virtual MiU32 tellRead(void) const
	{
		MiU32 ret=0;

		if ( mReadBuffer )
		{
			ret = (MiU32) (mReadLoc-mReadBuffer);
		}
		return ret;
	}

	virtual MiU32 tellWrite(void) const
	{
		return (MiU32)(mWriteLoc-mWriteBuffer);
	}

	virtual MiU32 seekRead(MiU32 loc)
	{
		MiU32 ret = 0;
		MI_ASSERT(mReadBuffer);
		if ( mReadBuffer )
		{
			mReadLoc = &mReadBuffer[loc];
			if ( mReadLoc >= mReadStop )
			{
				mReadLoc = mReadStop;
			}
			ret = (MiU32) (mReadLoc-mReadBuffer);
		}
		return ret;
	}

	virtual MiU32 seekWrite(MiU32 loc)
	{
		MiU32 ret = 0;
		MI_ASSERT( mOpenMode ==	OPEN_READ_WRITE_NEW );
		if ( mWriteBuffer )
		{
    		mWriteLoc = &mWriteBuffer[loc];
    		if ( mWriteLoc >= mWriteStop )
    		{
				mWriteLoc = mWriteStop;
			}
			ret = (MiU32)(mWriteLoc-mWriteBuffer);
		}
		return ret;
	}

	virtual void flush(void)
	{

	}

	virtual MiU32 getFileLength(void) const
	{
		MiU32 ret = 0;
		if ( mReadBuffer )
		{
			ret = (MiU32) (mReadStop-mReadBuffer);
		}
		else if ( mWriteBuffer )
		{
			ret = (MiU32)(mWriteLoc-mWriteBuffer);
		}
		return ret;
	}

	MiU32	getWriteBufferSize(void) const
	{
		return (MiU32)(mWriteLoc-mWriteBuffer);
	}

	void setWriteLoc(MiU8 *writeLoc)
	{
		MI_ASSERT(writeLoc >= mWriteBuffer && writeLoc < mWriteStop );
		mWriteLoc = writeLoc;
		mReadStop = mWriteLoc;
	}

	const MiU8 * getWriteBuffer(void) const
	{
		return mWriteBuffer;
	}

	/**
	 * Attention: if you use aligned allocator you cannot free memory with MI_FREE macros instead use deallocate method from base
	 */
	MiU8 * getWriteBufferOwnership(MiU32 &dataLen) // return the write buffer, and zero it out, the caller is taking ownership of the memory
	{
		MiU8 *ret = mWriteBuffer;
		dataLen = (MiU32)(mWriteLoc-mWriteBuffer);
		mWriteBuffer = NULL;
		mWriteLoc = NULL;
		mWriteStop = NULL;
		mWriteBufferSize = 0;
		return ret;
	}


	void alignRead(MiU32 a)
	{
		MiU32 loc = tellRead();
		MiU32 aloc = ((loc+(a-1))/a)*a;
		if ( aloc != loc )
		{
			seekRead(aloc);
		}
	}

	void alignWrite(MiU32 a)
	{
		MiU32 loc = tellWrite();
		MiU32 aloc = ((loc+(a-1))/a)*a;
		if ( aloc != loc )
		{
			seekWrite(aloc);
		}
	}

private:

	// double the size of the write buffer or at least as large as the 'size' value passed in.
	void growWriteBuffer(MiU32 size)
	{
		if ( mWriteBuffer == NULL )
		{
			if ( size < mDefaultWriteBufferSize ) size = mDefaultWriteBufferSize;
			initWriteBuffer(size);
		}
		else
		{
			MiU32 oldWriteIndex = (MiU32) (mWriteLoc - mWriteBuffer);
			MiU32 newSize =	mWriteBufferSize*2;
			MiU32 avail = newSize-oldWriteIndex;
			if ( size >= avail ) newSize = newSize+size;
			MiU8 *writeBuffer = (MiU8 *)MI_ALLOC(newSize);
			MI_ASSERT( writeBuffer );
			memmove(writeBuffer,mWriteBuffer,mWriteBufferSize);
			MI_FREE(mWriteBuffer);
			mWriteBuffer = writeBuffer;
			mWriteBufferSize = newSize;
			mWriteLoc = &mWriteBuffer[oldWriteIndex];
			mWriteStop = &mWriteBuffer[mWriteBufferSize];
			MiU32 oldReadLoc = (MiU32)(mReadLoc-mReadBuffer);
			mReadBuffer = mWriteBuffer;
			mReadStop   = mWriteLoc;
			mReadLoc = &mReadBuffer[oldReadLoc];
		}
	}

	const	MiU8	*mReadBuffer;
	const	MiU8	*mReadLoc;
	const	MiU8	*mReadStop;

			MiU8	*mWriteBuffer;
			MiU8	*mWriteLoc;
			MiU8	*mWriteStop;

			MiU32	mWriteBufferSize;
			MiU32	mDefaultWriteBufferSize;
			OpenMode	mOpenMode;
			SeekType	mSeekType;

};

//Use this class if you want to use PhysX memory allocator
class MiMemoryBuffer: public MiMemoryBufferBase, public MeshImportAllocated
{
	typedef MiMemoryBufferBase BaseClass;

public:
	MiMemoryBuffer(const void *readMem,MiU32 readLen): BaseClass(readMem, readLen) {}	
	MiMemoryBuffer(MiU32 defaultWriteBufferSize=BUFFER_SIZE_DEFAULT): BaseClass(defaultWriteBufferSize) {}
};

}

#endif // MI_MEMORY_BUFFER_H


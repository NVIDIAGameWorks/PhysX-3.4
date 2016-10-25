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
#include "FileRenderDebug.h"
#include "PsFileBuffer.h"
#include "ProcessRenderDebug.h"
#include "ClientServer.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace RENDER_DEBUG
{

static char magic[4] = { 'D', 'E', 'B', 'G' };

#define DEFAULT_FRAME_BUFFER_SIZE (1024*1024)*128 // default size is 16mb per frame.

static uint32_t isBigEndian(void)
{
	int32_t i = 1;
	bool b = *(reinterpret_cast<char*>(&i))==0;
	return b ? uint32_t(1) : uint32_t(0);
}

class DataStream
{
public:
	DataStream(void)
	{
		mWriteBufferLoc = 0;
		mWriteBufferEnd = NULL;
		mReadBuffer = NULL;
		mReadBufferLoc = 0;
	}

	void initWriteBuffer(void *_writeBuffer,uint32_t writeBufferLen)
	{
		uint8_t *writeBuffer = reinterpret_cast<uint8_t*>(_writeBuffer);
		mWriteBufferLoc = mWriteBuffer = writeBuffer;
		mWriteBufferEnd = writeBuffer+writeBufferLen;
		mReadBuffer = NULL;
	}

	void initReadBuffer(const void *readBuffer)
	{
		mReadBufferLoc = mReadBuffer = reinterpret_cast<const uint8_t *>(readBuffer);
		mWriteBufferLoc = NULL;
		mWriteBufferEnd = NULL;
		mWriteBuffer = NULL;
	}

	// store data of this type to the output stream
	template <class Type > PX_INLINE void operator<<(Type v)
	{
		PX_ASSERT(mWriteBuffer);
		PX_ASSERT(mWriteBufferLoc);
		if ( (mWriteBufferLoc+sizeof(Type)) < mWriteBufferEnd )
		{
			*reinterpret_cast<Type*>(mWriteBufferLoc) = v;
			mWriteBufferLoc+=sizeof(Type);
		}
		else
		{
			PX_ASSERT(0);
		}
	}

	template <class Type> PX_INLINE void operator>>(Type &v)
	{
		v = *reinterpret_cast<const Type*>(mReadBufferLoc);
		mReadBufferLoc+=sizeof(Type);
	}

	void readData(void *dest,uint32_t len)
	{
		memcpy(dest,mReadBufferLoc,len);
		mReadBufferLoc+=len;
	}

	PX_INLINE void writeString(const char *c)
	{
		uint32_t len = c ? uint32_t(strlen(c)) : 0;
		PX_ASSERT(mWriteBuffer);
		PX_ASSERT(mWriteBufferLoc);
		if ( (mWriteBufferLoc+len+sizeof(uint32_t) ) < mWriteBufferEnd )
		{
			*reinterpret_cast<uint32_t*>(mWriteBufferLoc) = len;
			mWriteBufferLoc+=sizeof(uint32_t);
			if ( len )
			{
				memcpy(mWriteBufferLoc,c,len);
			}
			mWriteBufferLoc+=len;
		}
		else
		{
			PX_ASSERT(0);
		}
	}

	PX_INLINE void readString(char *str,uint32_t maxLen)
	{
		uint32_t slen;
		(*this) >> slen;
		PX_UNUSED(maxLen);
		PX_ASSERT( slen < maxLen );
		memcpy(str,mReadBufferLoc,slen);
		str[slen] = 0;
		mReadBufferLoc+=slen;
	}

	PX_INLINE void writeBuffer(const void *buffer,uint32_t len)
	{
		PX_ASSERT(mWriteBufferLoc);
		if ( (mWriteBufferLoc+len ) < mWriteBufferEnd )
		{
			if ( len )
			{
				memcpy(mWriteBufferLoc,buffer,len);
			}
			mWriteBufferLoc+=len;
		}
		else
		{
			PX_ASSERT(0);
		}
	}

	PX_INLINE uint32_t getWriteBufferLength(void) const
	{
		return uint32_t(mWriteBufferLoc-mWriteBuffer);
	}

	PX_INLINE void * getWriteBufferLoc(void) const
	{
		return mWriteBufferLoc;
	}

	PX_INLINE void * getWriteBuffer(void) const
	{
		return mWriteBuffer;
	}

	PX_INLINE uint32_t getReadBufferLength(void) const
	{
		return uint32_t(mReadBufferLoc-mReadBuffer);
	}

	PX_INLINE const void *getReadBuffer(void) const
	{
		return mReadBuffer;
	}

	PX_INLINE void reset(void)
	{
		mWriteBufferLoc = mWriteBuffer;
		mReadBufferLoc = mReadBuffer;
	}

	// returns the current read buffer address and advances the read pointer past the length we are consuming.
	// avoids a memory copy
	PX_INLINE const void * getAdvanceReadLoc(uint32_t len)
	{
		const void *ret = mReadBufferLoc;
		mReadBufferLoc+=len;
		return ret;
	}

private:
	uint8_t			*mWriteBuffer;
	uint8_t			*mWriteBufferLoc;
	uint8_t			*mWriteBufferEnd;
	const uint8_t	*mReadBuffer;
	const uint8_t	*mReadBufferLoc;
};


struct FrameHeader
{
	void init(void)
	{
		mSeekLocation = 0;
		mItemCount = 0;
		mItemSize = 0;
	}
	uint32_t	mSeekLocation;
	uint32_t	mItemCount;
	uint32_t	mItemSize;
};

struct PrimitiveBatch
{
	uint32_t	mPrimitiveType;
	uint32_t	mPrimitiveCount;
	uint32_t	mPrimitiveIndex;
};

struct TypeHeader
{
	TypeHeader(void)
	{
		mSeekLocation = 0;
		mDisplayType = ProcessRenderDebug::DT_LAST;
		mPrimitiveCount = 0;
	}

	void init(ProcessRenderDebug::DisplayType type)
	{
		mSeekLocation = 0;
		mDisplayType = type;
		mPrimitiveCount = 0;
	}

	uint32_t	mSeekLocation;
	uint32_t	mDisplayType;
	uint32_t	mPrimitiveCount;
};


class FileRenderDebugImpl : public FileRenderDebug, public physx::shdfnd::UserAllocated
{
public:
	FileRenderDebugImpl(const char *fileName,bool readAccess,ProcessRenderDebug *echoLocally,ClientServer *cs) :
		mPrimitiveBatch(PX_DEBUG_EXP("FileRenderDebugImpl::mPrimitiveBatch")),
		mFrameHeaders(PX_DEBUG_EXP("FileRenderDebugImpl::mFrameHeaders"))
	{
		mEndianSwap = false;
		mClientServer = cs;
		mLastDisplayType = ProcessRenderDebug::DT_LAST;
		mData = NULL;
		mPrimitives = NULL;
		mPrimitiveCount = 0;
		mCurrentFrame = 0xFFFFFFFF;
		mFrameItemCount = 0;
		mFrameStart = 0;
		mFrameCount = 1;
		mReadAccess = readAccess;
		mEchoLocally = echoLocally;
		mMaxFrameBufferSize = DEFAULT_FRAME_BUFFER_SIZE;
		mWriteBufferData = PX_ALLOC(mMaxFrameBufferSize,"FileRenderDebugFrameBuffer");
		mWriteBuffer.initWriteBuffer(mWriteBufferData,mMaxFrameBufferSize);
		mFileBuffer = NULL;

		if ( mClientServer == NULL && fileName )
		{
			mFileBuffer = PX_NEW(physx::PsFileBuffer)(fileName, readAccess ? physx::PsFileBuffer::OPEN_READ_ONLY : physx::PsFileBuffer::OPEN_WRITE_ONLY);
			if ( mFileBuffer->isOpen() )
			{
				if ( mReadAccess )
				{
					char temp[4];
					uint32_t r = mFileBuffer->read(temp,4);
					uint32_t version = mFileBuffer->readDword();
					uint32_t bigEndian = mFileBuffer->readDword();

					if ( r == 4 && magic[0] == temp[0] && magic[1] == temp[1] && magic[2] == temp[2] && magic[3] == temp[3] && version == RENDER_DEBUG_COMM_VERSION )
					{
						if ( bigEndian != isBigEndian() )
						{
							mEndianSwap = true;
						}
						// it's valid!
						FrameHeader h;
						while ( readFrameHeader(h,*mFileBuffer ) )
						{
							mFrameHeaders.pushBack(h);
						}
						mFrameCount = mFrameHeaders.size();
					}
					else
					{
						delete mFileBuffer;
						mFileBuffer = NULL;
					}
				}
				else
				{
					mFileBuffer->write(magic, 4 );
					mFileBuffer->storeDword(RENDER_DEBUG_COMM_VERSION);
					uint32_t bigEndian = isBigEndian();
					mFileBuffer->storeDword(bigEndian);
					mFileBuffer->flush();
				}

			}
			else
			{
				delete mFileBuffer;
				mFileBuffer = NULL;
			}
		}
	}

	bool readFrameHeader(FrameHeader &h,physx::PsFileBuffer &fph)
	{
		bool ret = false;

		h.init();
		h.mItemCount = fph.readDword();
		h.mItemSize  = fph.readDword();
		h.mSeekLocation = fph.tellRead();
		if ( h.mItemCount > 0 && h.mItemSize > 0 )
		{
			fph.seekRead(h.mSeekLocation+h.mItemSize);
			ret = true;
		}
		return ret;
	}

	virtual ~FileRenderDebugImpl(void)
    {
		reset();
		PX_FREE(mWriteBufferData);
		if(mFileBuffer)
		{
			mFileBuffer->close();
    		delete mFileBuffer;
		}
    }

	virtual ProcessRenderDebug * getEchoLocal(void) const
	{
		return mEchoLocally;
	}

	bool isOk(void) const
	{
		if ( mClientServer ) return true;
		if ( mReadAccess && getFrameCount() == 0 ) return false;
		return mFileBuffer ? true : false;
	}

	virtual void processRenderDebug(const DebugPrimitive **dplist,
									uint32_t pcount,
									RENDER_DEBUG::RenderDebugInterface *iface,
									ProcessRenderDebug::DisplayType type)
	{
		if ( !mReadAccess )
		{
			PX_ASSERT(pcount);
			if ( mFrameStart == 0 )
			{
				uint32_t pv = 0;
				if ( mFileBuffer )
				{
					mFrameStart = mFileBuffer->tellWrite();
					mFileBuffer->storeDword(pv);
					mFileBuffer->storeDword(pv);
				}
			}
			if ( type != mLastDisplayType )
			{
				flushDisplayType(mLastDisplayType);
				beginDisplayType(type);
			}
			mTypeHeaders[type].mPrimitiveCount+=pcount;
			for (uint32_t i=0; i<pcount; i++)
			{
				const DebugPrimitive *dp = dplist[i];
				uint32_t plen = DebugCommand::getPrimtiveSize(*dp);
				mWriteBuffer.writeBuffer(dp,plen);
			}

			if ( mFileBuffer )
			{
				mFileBuffer->write(mWriteBuffer.getWriteBuffer(),mWriteBuffer.getWriteBufferLength());
			}
			else if ( mClientServer )
			{
				mClientServer->recordPrimitives(mFrameCount,type,pcount,mWriteBuffer.getWriteBufferLength(),mWriteBuffer.getWriteBuffer());
			}

			mWriteBuffer.initWriteBuffer(mWriteBufferData,mMaxFrameBufferSize);
			mFrameItemCount+=pcount;
		}

		if ( mEchoLocally )
		{
			mEchoLocally->processRenderDebug(dplist,pcount,iface,type);
		}
	}

	void flushDisplayType(DisplayType type)
	{
		if ( type != ProcessRenderDebug::DT_LAST && mTypeHeaders[type].mDisplayType != ProcessRenderDebug::DT_LAST )
		{
			if ( mFileBuffer )
			{
				uint32_t loc = mFileBuffer->tellWrite();
				mFileBuffer->seekWrite( mTypeHeaders[type].mSeekLocation );
				mFileBuffer->storeDword( mTypeHeaders[type].mDisplayType );
				mFileBuffer->storeDword( mTypeHeaders[type].mPrimitiveCount );
				mFileBuffer->seekWrite( loc );
			}
			mTypeHeaders[type].init(ProcessRenderDebug::DT_LAST);
			mLastDisplayType = ProcessRenderDebug::DT_LAST;
		}
	}

	void beginDisplayType(DisplayType type)
	{
		mTypeHeaders[type].init(type);
		if ( mFileBuffer )
		{
			mTypeHeaders[type].mSeekLocation = mFileBuffer->tellWrite();
			mFileBuffer->storeDword( mTypeHeaders[type].mDisplayType );
			mFileBuffer->storeDword( mTypeHeaders[type].mPrimitiveCount );
		}
		mLastDisplayType = type;
	}

	virtual void flush(RENDER_DEBUG::RenderDebugInterface *iface,ProcessRenderDebug::DisplayType type)
	{
		if ( !mReadAccess )
		{
			flushDisplayType(type);
		}
		if ( mEchoLocally )
		{
			mEchoLocally->flush(iface,type);
		}
	}

	virtual void release(void)
	{
		delete this;
	}

	virtual void processReadFrame(RENDER_DEBUG::RenderDebugInterface *iface)
	{
		if ( mReadAccess && mEchoLocally )
		{
			for (uint32_t i=0; i<mPrimitiveBatch.size(); i++)
			{
				PrimitiveBatch &b = mPrimitiveBatch[i];
				mEchoLocally->processRenderDebug(&mPrimitives[b.mPrimitiveIndex],b.mPrimitiveCount,iface,static_cast<ProcessRenderDebug::DisplayType>(b.mPrimitiveType));
				mEchoLocally->flush(iface,static_cast<ProcessRenderDebug::DisplayType>(b.mPrimitiveType));
			}
		}

	}

	virtual void flushFrame(RENDER_DEBUG::RenderDebugInterface *iface)
	{
		PX_UNUSED(iface);
	}

	virtual void finalizeFrame(void) 
	{
		if ( mFileBuffer )
		{
			if ( mFrameItemCount )
			{
				if ( mFileBuffer )
				{
					uint32_t current = mFileBuffer->tellWrite();
					uint32_t frameLength = (current-mFrameStart)-( sizeof(uint32_t)*2 );
					mFileBuffer->seekWrite(mFrameStart);
					mFileBuffer->storeDword(mFrameItemCount);
					mFileBuffer->storeDword(frameLength);
					mFileBuffer->seekWrite(current);
					mFileBuffer->flush();
				}
			}
		}
		if ( mClientServer )
		{
			mClientServer->finalizeFrame(mFrameCount);
			mClientServer->serverWait(); // wait up to 1 millisecond for the server to catch up with us...
		}
		mFrameStart = 0;
		mFrameItemCount = 0;
		mFrameCount++;
	}

	virtual uint32_t getFrameCount(void) const
	{
		return mFrameCount;
	}

	void reset(void)
	{
		PX_FREE(mPrimitives);
		PX_FREE(mData);
		mData = NULL;
		mPrimitives = NULL;
	}

	virtual void setFrame(uint32_t frameNo)
	{
		if ( !mFileBuffer ) 
			return;

		if ( mReadAccess && frameNo < mFrameCount && frameNo != mCurrentFrame )
		{
			mPrimitiveBatch.resize(0);

			FrameHeader &h = mFrameHeaders[frameNo];
			mCurrentFrame = frameNo;
			reset();
			mPrimitiveCount = h.mItemCount;
			mPrimitives = const_cast<const DebugPrimitive **>(reinterpret_cast<DebugPrimitive **>(PX_ALLOC( sizeof(DebugPrimitive *)*mPrimitiveCount, PX_DEBUG_EXP("DebugPrimitive"))));
			mData = reinterpret_cast<uint8_t*>(PX_ALLOC(h.mItemSize, PX_DEBUG_EXP("FrameItemSize")));
			mFileBuffer->seekRead(h.mSeekLocation);
			uint32_t bcount = mFileBuffer->read(mData,h.mItemSize);
			if ( bcount == h.mItemSize )
			{

				uint32_t index = 0;

				const uint8_t *scan = mData;

				while ( index < h.mItemCount )
				{
    				PrimitiveBatch b;

					uint32_t *uscan = const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(scan));
    				b.mPrimitiveType = uscan[0];
    				b.mPrimitiveCount = uscan[1];
    				b.mPrimitiveIndex = index;

					mPrimitiveBatch.pushBack(b);

    				uscan+=2;
    				scan = reinterpret_cast<uint8_t *>(uscan);

					for (uint32_t i=0; i<b.mPrimitiveCount; i++)
					{
						DebugPrimitive *prim = const_cast<DebugPrimitive*>(reinterpret_cast<const DebugPrimitive *>(scan));

						if ( mEndianSwap )
						{
							endianSwap(prim);
						}

						PX_ASSERT( prim->mCommand < DebugCommand::DEBUG_LAST );
						mPrimitives[i+index] = prim;
						uint32_t plen = DebugCommand::getPrimtiveSize(*prim);
						scan+=plen;
					}
					index+=b.mPrimitiveCount;
				}
			}
			else
			{
				reset();
			}



		}

	}

private:

	physx::PsFileBuffer	   		*mFileBuffer;

	bool					mReadAccess;
	ProcessRenderDebug	*mEchoLocally;
	uint32_t					mFrameStart;
	uint32_t					mFrameItemCount;
	uint32_t					mFrameCount;
	uint32_t					mCurrentFrame;

	uint32_t					mPrimitiveCount;
	const DebugPrimitive	**mPrimitives;
	uint8_t					*mData;
	physx::shdfnd::Array< PrimitiveBatch >  mPrimitiveBatch;
	physx::shdfnd::Array< FrameHeader >	mFrameHeaders;

	ProcessRenderDebug::DisplayType				mLastDisplayType;
	TypeHeader				mTypeHeaders[ProcessRenderDebug::DT_LAST];

	uint32_t			mMaxFrameBufferSize;
	void				*mWriteBufferData;
	DataStream			mWriteBuffer;
	ClientServer		*mClientServer;
	bool				mEndianSwap;
};

FileRenderDebug * createFileRenderDebug(const char *fileName,
										  bool readAccess,
										  ProcessRenderDebug *echoLocally,
										  ClientServer *cs)
{
	FileRenderDebugImpl *f = PX_NEW(FileRenderDebugImpl)(fileName, readAccess, echoLocally, cs);
	if (!f->isOk() )
	{
		delete f;
		f = NULL;
	}
	return static_cast< FileRenderDebug *>(f);
}



} // end of namespace

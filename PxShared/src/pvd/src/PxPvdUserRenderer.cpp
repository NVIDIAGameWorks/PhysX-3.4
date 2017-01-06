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
#include "PxPvdUserRenderer.h"
#include "PxPvdUserRenderImpl.h"
#include "PxPvdFoundation.h"
#include "PxPvdInternalByteStreams.h"
#include "PxPvdBits.h"
#include "PxPvdCommStreamTypes.h"
#include <stdarg.h>

using namespace physx;
using namespace physx::pvdsdk;

namespace
{

template <typename TStreamType>
struct RenderWriter : public RenderSerializer
{
	TStreamType& mStream;
	RenderWriter(TStreamType& stream) : mStream(stream)
	{
	}
	template <typename TDataType>
	void write(const TDataType* val, uint32_t count)
	{
		uint32_t numBytes = count * sizeof(TDataType);
		mStream.write(reinterpret_cast<const uint8_t*>(val), numBytes);
	}
	template <typename TDataType>
	void write(const TDataType& val)
	{
		write(&val, 1);
	}

	template <typename TDataType>
	void writeRef(DataRef<TDataType>& val)
	{
		uint32_t amount = val.size();
		write(amount);
		if(amount)
			write(val.begin(), amount);
	}

	virtual void streamify(uint64_t& val)
	{
		write(val);
	}
	virtual void streamify(uint32_t& val)
	{
		write(val);
	}
	virtual void streamify(float& val)
	{
		write(val);
	}
	virtual void streamify(uint8_t& val)
	{
		write(val);
	}
	virtual void streamify(DataRef<uint8_t>& val)
	{
		writeRef(val);
	}

	virtual void streamify(PvdDebugText& val)
	{
		write(val.color);
		write(val.position);
		write(val.size);

		uint32_t amount = static_cast<uint32_t>(strlen(val.string)) + 1;
		write(amount);
		if(amount)
			write(val.string, amount);
	}

	virtual void streamify(DataRef<PvdDebugPoint>& val)
	{
		writeRef(val);
	}
	virtual void streamify(DataRef<PvdDebugLine>& val)
	{
		writeRef(val);
	}
	virtual void streamify(DataRef<PvdDebugTriangle>& val)
	{
		writeRef(val);
	}

	virtual uint32_t hasData()
	{
		return false;
	}
	virtual bool isGood()
	{
		return true;
	}

  private:
	RenderWriter& operator=(const RenderWriter&);
};

struct UserRenderer : public PvdUserRenderer
{
	ForwardingMemoryBuffer mBuffer;
	uint32_t mBufferCapacity;
	RendererEventClient* mClient;

	UserRenderer(uint32_t bufferFullAmount)
	: mBuffer("UserRenderBuffer"), mBufferCapacity(bufferFullAmount), mClient(NULL)
	{
	}
	virtual ~UserRenderer()
	{
	}
	virtual void release()
	{
		PVD_DELETE(this);
	}

	template <typename TEventType>
	void handleEvent(TEventType evt)
	{
		RenderWriter<ForwardingMemoryBuffer> _writer(mBuffer);
		RenderSerializer& writer(_writer);

		PvdUserRenderTypes::Enum evtType(getPvdRenderTypeFromType<TEventType>());
		writer.streamify(evtType);
		evt.serialize(writer);
		if(mBuffer.size() >= mBufferCapacity)
			flushRenderEvents();
	}
	virtual void setInstanceId(const void* iid)
	{
		handleEvent(SetInstanceIdRenderEvent(PVD_POINTER_TO_U64(iid)));
	}
	// Draw these points associated with this instance
	virtual void drawPoints(const PvdDebugPoint* points, uint32_t count)
	{
		handleEvent(PointsRenderEvent(points, count));
	}
	// Draw these lines associated with this instance
	virtual void drawLines(const PvdDebugLine* lines, uint32_t count)
	{
		handleEvent(LinesRenderEvent(lines, count));
	}
	// Draw these triangles associated with this instance
	virtual void drawTriangles(const PvdDebugTriangle* triangles, uint32_t count)
	{
		handleEvent(TrianglesRenderEvent(triangles, count));
	}

	virtual void drawText(const PvdDebugText& text)
	{
		handleEvent(TextRenderEvent(text));
	}

	virtual void drawRenderbuffer(const PvdDebugPoint* pointData, uint32_t pointCount, const PvdDebugLine* lineData,
	                              uint32_t lineCount, const PvdDebugTriangle* triangleData, uint32_t triangleCount)
	{
		handleEvent(DebugRenderEvent(pointData, pointCount, lineData, lineCount, triangleData, triangleCount));
	}

	// Constraint visualization routines
	virtual void visualizeJointFrames(const PxTransform& parent, const PxTransform& child)
	{
		handleEvent(JointFramesRenderEvent(parent, child));
	}
	virtual void visualizeLinearLimit(const PxTransform& t0, const PxTransform& t1, float value, bool active)
	{
		handleEvent(LinearLimitRenderEvent(t0, t1, value, active));
	}
	virtual void visualizeAngularLimit(const PxTransform& t0, float lower, float upper, bool active)
	{
		handleEvent(AngularLimitRenderEvent(t0, lower, upper, active));
	}
	virtual void visualizeLimitCone(const PxTransform& t, float ySwing, float zSwing, bool active)
	{
		handleEvent(LimitConeRenderEvent(t, ySwing, zSwing, active));
	}
	virtual void visualizeDoubleCone(const PxTransform& t, float angle, bool active)
	{
		handleEvent(DoubleConeRenderEvent(t, angle, active));
	}
	// Clear the immedate buffer.
	virtual void flushRenderEvents()
	{
		if(mClient)
			mClient->handleBufferFlush(mBuffer.begin(), mBuffer.size());
		mBuffer.clear();
	}

	virtual void setClient(RendererEventClient* client)
	{
		mClient = client;
	}

  private:
	UserRenderer& operator=(const UserRenderer&);
};

template <bool swapBytes>
struct RenderReader : public RenderSerializer
{
	MemPvdInputStream mStream;
	ForwardingMemoryBuffer& mBuffer;

	RenderReader(ForwardingMemoryBuffer& buf) : mBuffer(buf)
	{
	}
	void setData(DataRef<const uint8_t> data)
	{
		mStream.setup(const_cast<uint8_t*>(data.begin()), const_cast<uint8_t*>(data.end()));
	}
	virtual void streamify(uint32_t& val)
	{
		mStream >> val;
	}
	virtual void streamify(uint64_t& val)
	{
		mStream >> val;
	}
	virtual void streamify(float& val)
	{
		mStream >> val;
	}
	virtual void streamify(uint8_t& val)
	{
		mStream >> val;
	}
	template <typename TDataType>
	void readRef(DataRef<TDataType>& val)
	{
		uint32_t count;
		mStream >> count;
		uint32_t numBytes = sizeof(TDataType) * count;

		TDataType* dataPtr = reinterpret_cast<TDataType*>(mBuffer.growBuf(numBytes));
		mStream.read(reinterpret_cast<uint8_t*>(dataPtr), numBytes);
		val = DataRef<TDataType>(dataPtr, count);
	}

	virtual void streamify(DataRef<PvdDebugPoint>& val)
	{
		readRef(val);
	}
	virtual void streamify(DataRef<PvdDebugLine>& val)
	{
		readRef(val);
	}
	virtual void streamify(DataRef<PvdDebugTriangle>& val)
	{
		readRef(val);
	}
	virtual void streamify(PvdDebugText& val)
	{
		mStream >> val.color;
		mStream >> val.position;
		mStream >> val.size;

		uint32_t len = 0;
		mStream >> len;

		uint8_t* dataPtr = mBuffer.growBuf(len);
		mStream.read(dataPtr, len);
		val.string = reinterpret_cast<const char*>(dataPtr);
	}
	virtual void streamify(DataRef<uint8_t>& val)
	{
		readRef(val);
	}
	virtual bool isGood()
	{
		return mStream.isGood();
	}
	virtual uint32_t hasData()
	{
		return uint32_t(mStream.size() > 0);
	}

  private:
	RenderReader& operator=(const RenderReader&);
};

template <>
struct RenderReader<true> : public RenderSerializer
{
	MemPvdInputStream mStream;
	ForwardingMemoryBuffer& mBuffer;
	RenderReader(ForwardingMemoryBuffer& buf) : mBuffer(buf)
	{
	}
	void setData(DataRef<const uint8_t> data)
	{
		mStream.setup(const_cast<uint8_t*>(data.begin()), const_cast<uint8_t*>(data.end()));
	}

	template <typename TDataType>
	void read(TDataType& val)
	{
		mStream >> val;
		swapBytes(val);
	}
	virtual void streamify(uint64_t& val)
	{
		read(val);
	}
	virtual void streamify(uint32_t& val)
	{
		read(val);
	}
	virtual void streamify(float& val)
	{
		read(val);
	}
	virtual void streamify(uint8_t& val)
	{
		read(val);
	}
	template <typename TDataType>
	void readRef(DataRef<TDataType>& val)
	{
		uint32_t count;
		mStream >> count;
		swapBytes(count);
		uint32_t numBytes = sizeof(TDataType) * count;

		TDataType* dataPtr = reinterpret_cast<TDataType*>(mBuffer.growBuf(numBytes));
		PVD_FOREACH(idx, count)
		RenderSerializerMap<TDataType>().serialize(*this, dataPtr[idx]);
		val = DataRef<TDataType>(dataPtr, count);
	}

	virtual void streamify(DataRef<PvdDebugPoint>& val)
	{
		readRef(val);
	}
	virtual void streamify(DataRef<PvdDebugLine>& val)
	{
		readRef(val);
	}
	virtual void streamify(DataRef<PvdDebugTriangle>& val)
	{
		readRef(val);
	}
	virtual void streamify(PvdDebugText& val)
	{
		mStream >> val.color;
		mStream >> val.position;
		mStream >> val.size;

		uint32_t len = 0;
		mStream >> len;

		uint8_t* dataPtr = mBuffer.growBuf(len);
		mStream.read(dataPtr, len);
		val.string = reinterpret_cast<const char*>(dataPtr);
	}
	virtual void streamify(DataRef<uint8_t>& val)
	{
		readRef(val);
	}
	virtual bool isGood()
	{
		return mStream.isGood();
	}
	virtual uint32_t hasData()
	{
		return uint32_t(mStream.size() > 0);
	}

  private:
	RenderReader& operator=(const RenderReader&);
};

template <bool swapBytes>
struct Parser : public PvdUserRenderParser
{
	ForwardingMemoryBuffer mBuffer;
	RenderReader<swapBytes> mReader;
	Parser() : mBuffer("PvdUserRenderParser::mBuffer"), mReader(mBuffer)
	{
	}

	void release()
	{
		PVD_DELETE(this);
	}
	void parseData(DataRef<const uint8_t> data, PvdUserRenderHandler& handler)
	{
		mReader.setData(data);
		RenderSerializer& serializer(mReader);
		while(serializer.isGood() && serializer.hasData())
		{
			mReader.mBuffer.clear();
			PvdUserRenderTypes::Enum evtType = PvdUserRenderTypes::Unknown;
			serializer.streamify(evtType);
			switch(evtType)
			{
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE(type)                                                                        \
	case PvdUserRenderTypes::type:                                                                                     \
	{                                                                                                                  \
		type##RenderEvent evt;                                                                                         \
		evt.serialize(serializer);                                                                                     \
		handler.handleRenderEvent(evt);                                                                                \
	}                                                                                                                  \
	break;
#include "PxPvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE
			case PvdUserRenderTypes::Unknown:
				PX_ASSERT(false);
				return;
			}
		}
		PX_ASSERT(serializer.isGood());
		return;
	}

	PX_NOCOPY(Parser<swapBytes>)
};
}

PvdUserRenderParser& PvdUserRenderParser::create(bool swapBytes)
{
	if(swapBytes)
		return *PVD_NEW(Parser<true>);
	else
		return *PVD_NEW(Parser<false>);
}

PvdUserRenderer* PvdUserRenderer::create(uint32_t bufferSize)
{
	return PVD_NEW(UserRenderer)(bufferSize);
}


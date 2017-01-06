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

#ifndef PXPVDSDK_PXPVDCOMMSTREAMTYPES_H
#define PXPVDSDK_PXPVDCOMMSTREAMTYPES_H

#include "foundation/PxErrorCallback.h"
#include "pvd/PxPvdTransport.h"

#include "PxPvdRenderBuffer.h"
#include "PxPvdObjectModelBaseTypes.h"
#include "PxPvdCommStreamEvents.h"
#include "PxPvdDataStream.h"
#include "PsMutex.h"

namespace physx
{
namespace profile
{
class PxProfileZone;
class PxProfileMemoryEventBuffer;
}
namespace pvdsdk
{
struct PvdErrorMessage;
class PvdObjectModelMetaData;

DEFINE_PVD_TYPE_NAME_MAP(profile::PxProfileZone, "_debugger_", "PxProfileZone")
DEFINE_PVD_TYPE_NAME_MAP(profile::PxProfileMemoryEventBuffer, "_debugger_", "PxProfileMemoryEventBuffer")
DEFINE_PVD_TYPE_NAME_MAP(PvdErrorMessage, "_debugger_", "PvdErrorMessage")
// All event streams are on the 'events' property of objects of these types
static inline NamespacedName getMemoryEventTotalsClassName()
{
	return NamespacedName("_debugger", "MemoryEventTotals");
}

class PvdOMMetaDataProvider
{
  protected:
	virtual ~PvdOMMetaDataProvider()
	{
	}

  public:
	virtual void addRef() = 0;
	virtual void release() = 0;
	virtual PvdObjectModelMetaData& lock() = 0;
	virtual void unlock() = 0;
	virtual bool createInstance(const NamespacedName& clsName, const void* instance) = 0;
	virtual bool isInstanceValid(const void* instance) = 0;
	virtual void destroyInstance(const void* instance) = 0;
	virtual int32_t getInstanceClassType(const void* instance) = 0;
};

class PvdCommStreamInternalConnection;

class PvdConnectionListener
{
  protected:
	virtual ~PvdConnectionListener()
	{
	}

  public:
	virtual void onDisconnect(PvdCommStreamInternalConnection& connection) = 0;
	virtual void onInstanceDestroyed(const void* instance) = 0;
};

class PvdCommStreamEmbeddedTypes
{
  public:
	static const char* getProfileEventStreamSemantic()
	{
		return "profile event stream";
	}
	static const char* getMemoryEventStreamSemantic()
	{
		return "memory event stream";
	}
	static const char* getRendererEventStreamSemantic()
	{
		return "render event stream";
	}
};

class PvdCommStreamEventBufferClient;

template <typename TStreamType>
struct EventStreamifier : public PvdEventSerializer
{
	TStreamType& mBuffer;
	EventStreamifier(TStreamType& buf) : mBuffer(buf)
	{
	}

	template <typename TDataType>
	void write(const TDataType& type)
	{
		mBuffer.write(reinterpret_cast<const uint8_t*>(&type), sizeof(TDataType));
	}
	template <typename TDataType>
	void write(const TDataType* type, uint32_t count)
	{
		mBuffer.write(reinterpret_cast<const uint8_t*>(type), count * sizeof(TDataType));
	}

	void writeRef(DataRef<const uint8_t> data)
	{
		uint32_t amount = static_cast<uint32_t>(data.size());
		write(amount);
		write(data.begin(), amount);
	}
	void writeRef(DataRef<StringHandle> data)
	{
		uint32_t amount = static_cast<uint32_t>(data.size());
		write(amount);
		write(data.begin(), amount);
	}
	template <typename TDataType>
	void writeRef(DataRef<TDataType> data)
	{
		uint32_t amount = static_cast<uint32_t>(data.size());
		write(amount);
		for(uint32_t idx = 0; idx < amount; ++idx)
		{
			TDataType& dtype(const_cast<TDataType&>(data[idx]));
			dtype.serialize(*this);
		}
	}

	virtual void streamify(uint16_t& val)
	{
		write(val);
	}
	virtual void streamify(uint8_t& val)
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
	virtual void streamify(uint64_t& val)
	{
		write(val);
	}
	virtual void streamify(PvdDebugText& val)
	{
		write(val.color);
		write(val.position);
		write(val.size);
		streamify(val.string);
	}

	virtual void streamify(String& val)
	{
		uint32_t len = 0;
		String temp = nonNull(val);
		if(*temp)
			len = static_cast<uint32_t>(strlen(temp) + 1);
		write(len);
		write(val, len);
	}
	virtual void streamify(DataRef<const uint8_t>& val)
	{
		writeRef(val);
	}
	virtual void streamify(DataRef<NameHandleValue>& val)
	{
		writeRef(val);
	}
	virtual void streamify(DataRef<StreamPropMessageArg>& val)
	{
		writeRef(val);
	}
	virtual void streamify(DataRef<StringHandle>& val)
	{
		writeRef(val);
	}

  private:
	EventStreamifier& operator=(const EventStreamifier&);
};

struct MeasureStream
{
	uint32_t mSize;
	MeasureStream() : mSize(0)
	{
	}
	template <typename TDataType>
	void write(const TDataType& val)
	{
		mSize += sizeof(val);
	}
	template <typename TDataType>
	void write(const TDataType*, uint32_t count)
	{
		mSize += sizeof(TDataType) * count;
	}
};

struct DataStreamState
{
	enum Enum
	{
		Open,
		SetPropertyValue,
		PropertyMessageGroup
	};
};

class ExtendedEventSerializer : public PvdEventSerializer
{
  protected:
	virtual ~ExtendedEventSerializer()
	{
	}

  public:
	virtual void setData(DataRef<const uint8_t> eventData) = 0;
	// True if this serializer performs byte swapping
	virtual bool performsSwap() = 0;

	virtual bool isGood() = 0;

	virtual void release() = 0;

	static ExtendedEventSerializer& createInputSerializer(bool swapBytes);
};

} // pvdsdk
} // physx
#endif // PXPVDSDK_PXPVDCOMMSTREAMTYPES_H

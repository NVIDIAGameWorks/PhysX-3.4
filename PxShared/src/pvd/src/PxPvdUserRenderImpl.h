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
#ifndef PXPVDSDK_PXPVDUSERRENDERIMPL_H
#define PXPVDSDK_PXPVDUSERRENDERIMPL_H

#include "PxPvdUserRenderer.h"

namespace physx
{
namespace pvdsdk
{

struct PvdUserRenderTypes
{
	enum Enum
	{
		Unknown = 0,
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE(type) type,
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA(type) type
#include "PxPvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE
	};
};

class RenderSerializer
{
  protected:
	virtual ~RenderSerializer()
	{
	}

  public:
	virtual void streamify(uint64_t& val) = 0;
	virtual void streamify(float& val) = 0;
	virtual void streamify(uint32_t& val) = 0;
	virtual void streamify(uint8_t& val) = 0;
	virtual void streamify(DataRef<uint8_t>& val) = 0;
	virtual void streamify(DataRef<PvdDebugPoint>& val) = 0;
	virtual void streamify(DataRef<PvdDebugLine>& val) = 0;
	virtual void streamify(DataRef<PvdDebugTriangle>& val) = 0;
	virtual void streamify(PvdDebugText& val) = 0;
	virtual bool isGood() = 0;
	virtual uint32_t hasData() = 0;

	void streamify(PvdUserRenderTypes::Enum& val)
	{
		uint8_t data = static_cast<uint8_t>(val);
		streamify(data);
		val = static_cast<PvdUserRenderTypes::Enum>(data);
	}
	void streamify(PxVec3& val)
	{
		streamify(val[0]);
		streamify(val[1]);
		streamify(val[2]);
	}

	void streamify(PvdColor& val)
	{
		streamify(val.r);
		streamify(val.g);
		streamify(val.b);
		streamify(val.a);
	}
	void streamify(PxTransform& val)
	{
		streamify(val.q.x);
		streamify(val.q.y);
		streamify(val.q.z);
		streamify(val.q.w);
		streamify(val.p.x);
		streamify(val.p.y);
		streamify(val.p.z);
	}
	void streamify(bool& val)
	{
		uint8_t tempVal = uint8_t(val ? 1 : 0);
		streamify(tempVal);
		val = tempVal ? true : false;
	}
};

template <typename TBulkRenderType>
struct BulkRenderEvent
{
	DataRef<TBulkRenderType> mData;
	BulkRenderEvent(const TBulkRenderType* data, uint32_t count) : mData(data, count)
	{
	}
	BulkRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(mData);
	}
};
struct SetInstanceIdRenderEvent
{
	uint64_t mInstanceId;
	SetInstanceIdRenderEvent(uint64_t iid) : mInstanceId(iid)
	{
	}
	SetInstanceIdRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(mInstanceId);
	}
};
struct PointsRenderEvent : BulkRenderEvent<PvdDebugPoint>
{
	PointsRenderEvent(const PvdDebugPoint* data, uint32_t count) : BulkRenderEvent<PvdDebugPoint>(data, count)
	{
	}
	PointsRenderEvent()
	{
	}
};
struct LinesRenderEvent : BulkRenderEvent<PvdDebugLine>
{
	LinesRenderEvent(const PvdDebugLine* data, uint32_t count) : BulkRenderEvent<PvdDebugLine>(data, count)
	{
	}
	LinesRenderEvent()
	{
	}
};
struct TrianglesRenderEvent : BulkRenderEvent<PvdDebugTriangle>
{
	TrianglesRenderEvent(const PvdDebugTriangle* data, uint32_t count) : BulkRenderEvent<PvdDebugTriangle>(data, count)
	{
	}
	TrianglesRenderEvent()
	{
	}
};
struct DebugRenderEvent
{
	DataRef<PvdDebugPoint> mPointData;
	DataRef<PvdDebugLine> mLineData;
	DataRef<PvdDebugTriangle> mTriangleData;
	DebugRenderEvent(const PvdDebugPoint* pointData, uint32_t pointCount, const PvdDebugLine* lineData,
	                 uint32_t lineCount, const PvdDebugTriangle* triangleData, uint32_t triangleCount)
	: mPointData(pointData, pointCount), mLineData(lineData, lineCount), mTriangleData(triangleData, triangleCount)
	{
	}

	DebugRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(mPointData);
		serializer.streamify(mLineData);
		serializer.streamify(mTriangleData);
	}
};

struct TextRenderEvent
{
	PvdDebugText mText;
	TextRenderEvent(const PvdDebugText& text)
	{
		mText.color = text.color;
		mText.position = text.position;
		mText.size = text.size;
		mText.string = text.string;
	}
	TextRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(mText);
	}
};

struct JointFramesRenderEvent
{
	PxTransform parent;
	PxTransform child;
	JointFramesRenderEvent(const PxTransform& p, const PxTransform& c) : parent(p), child(c)
	{
	}
	JointFramesRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(parent);
		serializer.streamify(child);
	}
};
struct LinearLimitRenderEvent
{
	PxTransform t0;
	PxTransform t1;
	float value;
	bool active;
	LinearLimitRenderEvent(const PxTransform& _t0, const PxTransform& _t1, float _value, bool _active)
	: t0(_t0), t1(_t1), value(_value), active(_active)
	{
	}
	LinearLimitRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(t0);
		serializer.streamify(t1);
		serializer.streamify(value);
		serializer.streamify(active);
	}
};
struct AngularLimitRenderEvent
{
	PxTransform t0;
	float lower;
	float upper;
	bool active;
	AngularLimitRenderEvent(const PxTransform& _t0, float _lower, float _upper, bool _active)
	: t0(_t0), lower(_lower), upper(_upper), active(_active)
	{
	}
	AngularLimitRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(t0);
		serializer.streamify(lower);
		serializer.streamify(upper);
		serializer.streamify(active);
	}
};
struct LimitConeRenderEvent
{
	PxTransform t;
	float ySwing;
	float zSwing;
	bool active;
	LimitConeRenderEvent(const PxTransform& _t, float _ySwing, float _zSwing, bool _active)
	: t(_t), ySwing(_ySwing), zSwing(_zSwing), active(_active)
	{
	}
	LimitConeRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(t);
		serializer.streamify(ySwing);
		serializer.streamify(zSwing);
		serializer.streamify(active);
	}
};
struct DoubleConeRenderEvent
{
	PxTransform t;
	float angle;
	bool active;
	DoubleConeRenderEvent(const PxTransform& _t, float _angle, bool _active) : t(_t), angle(_angle), active(_active)
	{
	}
	DoubleConeRenderEvent()
	{
	}
	void serialize(RenderSerializer& serializer)
	{
		serializer.streamify(t);
		serializer.streamify(angle);
		serializer.streamify(active);
	}
};

template <typename TDataType>
struct RenderSerializerMap
{
	void serialize(RenderSerializer& s, TDataType& d)
	{
		d.serialize(s);
	}
};
template <>
struct RenderSerializerMap<uint8_t>
{
	void serialize(RenderSerializer& s, uint8_t& d)
	{
		s.streamify(d);
	}
};

template <>
struct RenderSerializerMap<PvdDebugPoint>
{
	void serialize(RenderSerializer& s, PvdDebugPoint& d)
	{
		s.streamify(d.pos);
		s.streamify(d.color);
	}
};

template <>
struct RenderSerializerMap<PvdDebugLine>
{
	void serialize(RenderSerializer& s, PvdDebugLine& d)
	{
		s.streamify(d.pos0);
		s.streamify(d.color0);
		s.streamify(d.pos1);
		s.streamify(d.color1);
	}
};

template <>
struct RenderSerializerMap<PvdDebugTriangle>
{
	void serialize(RenderSerializer& s, PvdDebugTriangle& d)
	{
		s.streamify(d.pos0);
		s.streamify(d.color0);
		s.streamify(d.pos1);
		s.streamify(d.color1);
		s.streamify(d.pos2);
		s.streamify(d.color2);
	}
};

template <typename TDataType>
struct PvdTypeToRenderType
{
	bool compile_error;
};

#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE(type)                                                                        \
	template <>                                                                                                        \
	struct PvdTypeToRenderType<type##RenderEvent>                                                                      \
	{                                                                                                                  \
		enum Enum                                                                                                      \
		{                                                                                                              \
			EnumVal = PvdUserRenderTypes::type                                                                         \
		};                                                                                                             \
	};

#include "PxPvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE

template <typename TDataType>
PvdUserRenderTypes::Enum getPvdRenderTypeFromType()
{
	return static_cast<PvdUserRenderTypes::Enum>(PvdTypeToRenderType<TDataType>::EnumVal);
}

class PvdUserRenderHandler
{
  protected:
	virtual ~PvdUserRenderHandler()
	{
	}

  public:
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE(type) virtual void handleRenderEvent(const type##RenderEvent& evt) = 0;

#include "PxPvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE
};

class PvdUserRenderParser
{
  protected:
	virtual ~PvdUserRenderParser()
	{
	}

  public:
	virtual void release() = 0;
	virtual void parseData(DataRef<const uint8_t> data, PvdUserRenderHandler& handler) = 0;

	static PvdUserRenderParser& create(bool swapBytes);
};
}
}

#endif // PXPVDSDK_PXPVDUSERRENDERIMPL_H

/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __APEX_STREAM_H__
#define __APEX_STREAM_H__

#include "ApexDefs.h"
#include "PxPlane.h"


namespace nvidia
{
namespace apex
{

// Public, useful operators for serializing nonversioned data follow.
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, bool& b)
{
	b = (0 != stream.readByte());
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, int8_t& b)
{
	b = (int8_t)stream.readByte();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, int16_t& w)
{
	w = (int16_t)stream.readWord();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, int32_t& d)
{
	d = (int32_t)stream.readDword();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, uint8_t& b)
{
	b = stream.readByte();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, uint16_t& w)
{
	w = stream.readWord();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, uint32_t& d)
{
	d = stream.readDword();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, float& f)
{
	f = stream.readFloat();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, double& f)
{
	f = stream.readDouble();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxVec2& v)
{
	stream >> v.x >> v.y;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxVec3& v)
{
	stream >> v.x >> v.y >> v.z;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxVec4& v)
{
	stream >> v.x >> v.y >> v.z >> v.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxBounds3& b)
{
	stream >> b.minimum >> b.maximum;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxQuat& q)
{
	stream >> q.x >> q.y >> q.z >> q.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxPlane& p)
{
	stream >> p.n.x >> p.n.y >> p.n.z >> p.d;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxMat44& m)
{
	stream >> m.column0 >> m.column1 >> m.column2 >> m.column3;
	return stream;
}

// The opposite of the above operators--takes data and writes it to a stream.
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const bool b)
{
	stream.storeByte(b ? (uint8_t)1 : (uint8_t)0);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const int8_t b)
{
	stream.storeByte((uint8_t)b);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const int16_t w)
{
	stream.storeWord((uint16_t)w);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const int32_t d)
{
	stream.storeDword((uint32_t)d);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const uint8_t b)
{
	stream.storeByte(b);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const uint16_t w)
{
	stream.storeWord(w);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const uint32_t d)
{
	stream.storeDword(d);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const float f)
{
	stream.storeFloat(f);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const double f)
{
	stream.storeDouble(f);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxVec2& v)
{
	stream << v.x << v.y;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxVec3& v)
{
	stream << v.x << v.y << v.z;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxVec4& v)
{
	stream << v.x << v.y << v.z << v.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxBounds3& b)
{
	stream << b.minimum << b.maximum;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxQuat& q)
{
	stream << q.x << q.y << q.z << q.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxPlane& p)
{
	stream << p.n.x << p.n.y << p.n.z << p.d;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxMat44& m)
{
	stream << m.column0 << m.column1 << m.column2 << m.column3;
	return stream;
}


}
} // end namespace apex

#endif // __APEX_STREAM_H__
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

#ifndef PX_PHYSICS_SCB_RIGID_STATIC
#define PX_PHYSICS_SCB_RIGID_STATIC

#include "ScStaticCore.h"
#include "ScbScene.h"
#include "ScbActor.h"
#include "ScbRigidObject.h"

namespace physx
{
namespace Scb
{
#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

struct RigidStaticBuffer : public RigidObjectBuffer
{
	template <PxU32 I, PxU32 Dummy> struct Fns {};		// TODO: make the base class traits visible
	typedef Sc::StaticCore Core;
	typedef RigidStaticBuffer Buf;

	// regular attributes
	enum { BF_Base = RigidObjectBuffer::AttrCount };
	SCB_REGULAR_ATTRIBUTE_ALIGNED(BF_Base,			PxTransform,		Actor2World, 16)
};

#if PX_VC 
     #pragma warning(pop) 
#endif

class RigidStatic : public Scb::RigidObject
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	typedef RigidStaticBuffer Buf;
	typedef Sc::StaticCore Core;

public:
// PX_SERIALIZATION
										RigidStatic(const PxEMPTY) :	Scb::RigidObject(PxEmpty), mStatic(PxEmpty)	{}
	static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
	PX_INLINE							RigidStatic(const PxTransform& actor2World);
	PX_INLINE							~RigidStatic() {}

	PX_INLINE		const PxTransform&	getActor2World() const					{ return read<Buf::BF_Actor2World>(); }
	PX_INLINE		void				setActor2World(const PxTransform& m)	{ write<Buf::BF_Actor2World>(m); }

	PX_FORCE_INLINE void				onOriginShift(const PxVec3& shift)		{ mStatic.onOriginShift(shift); }

	//---------------------------------------------------------------------------------
	// Data synchronization
	//---------------------------------------------------------------------------------
	PX_INLINE void						syncState();

	static size_t getScOffset()	{ return reinterpret_cast<size_t>(&reinterpret_cast<RigidStatic*>(0)->mStatic);	}

	PX_FORCE_INLINE Sc::StaticCore&			getScStatic()				{	return mStatic; }

	PX_FORCE_INLINE void					initBufferedState()			{}

private:
			Sc::StaticCore					mStatic;

	PX_FORCE_INLINE	const Buf*		getRigidActorBuffer()	const	{ return reinterpret_cast<const Buf*>(getStream()); }
	PX_FORCE_INLINE	Buf*			getRigidActorBuffer()			{ return reinterpret_cast<Buf*>(getStream()); }

	//---------------------------------------------------------------------------------
	// Infrastructure for regular attributes
	//---------------------------------------------------------------------------------

	struct Access: public BufferedAccess<Buf, Core, RigidStatic> {};

	template<PxU32 f> PX_FORCE_INLINE typename Buf::Fns<f,0>::Arg read() const		{	return Access::read<Buf::Fns<f,0> >(*this, mStatic);	}
	template<PxU32 f> PX_FORCE_INLINE void write(typename Buf::Fns<f,0>::Arg v)		{	Access::write<Buf::Fns<f,0> >(*this, mStatic, v);		}
	template<PxU32 f> PX_FORCE_INLINE void flush(const Buf& buf)					{	Access::flush<Buf::Fns<f,0> >(*this, mStatic, buf);		}
};

RigidStatic::RigidStatic(const PxTransform& actor2World) : 
	mStatic(actor2World)
{
	setScbType(ScbType::eRIGID_STATIC);
}

//--------------------------------------------------------------
//
// Data synchronization
//
//--------------------------------------------------------------
PX_INLINE void RigidStatic::syncState()
{
	const PxU32 bufferFlags = getBufferFlags();

	if(bufferFlags & Buf::BF_ActorFlags)
		syncNoSimSwitch(*getRigidActorBuffer(), mStatic, false);

	RigidObject::syncState();

	if(bufferFlags & Buf::BF_Actor2World)
		flush<Buf::BF_Actor2World>(*getRigidActorBuffer());

	postSyncState();
}

}  // namespace Scb

}

#endif

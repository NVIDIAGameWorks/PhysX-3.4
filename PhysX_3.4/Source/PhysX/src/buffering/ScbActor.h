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

#ifndef PX_PHYSICS_SCB_FSACTOR
#define PX_PHYSICS_SCB_FSACTOR

#include "ScActorCore.h"
#include "PsUtilities.h"
#include "ScbBase.h"
#include "ScbDefs.h"

#include "PxClient.h"

namespace physx
{
namespace Scb
{
struct ActorBuffer
{
	template <PxU32 I, PxU32 Dummy> struct Fns {};
	typedef Sc::ActorCore Core;
	typedef ActorBuffer Buf;

	SCB_REGULAR_ATTRIBUTE (0, PxActorFlags,					ActorFlags)
	SCB_REGULAR_ATTRIBUTE (1, PxDominanceGroup,				DominanceGroup)
	SCB_REGULAR_ATTRIBUTE (2, PxActorClientBehaviorFlags,	ClientBehaviorFlags)

	enum { AttrCount = 3 };

protected:
	~ActorBuffer(){}
};

class Actor : public Base
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	typedef ActorBuffer Buf;
	typedef Sc::ActorCore Core;

public:
// PX_SERIALIZATION
												Actor(const PxEMPTY) : Base(PxEmpty) {}
	static			void						getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	PX_INLINE									Actor() {}

	//---------------------------------------------------------------------------------
	// Wrapper for Sc::Actor interface
	//---------------------------------------------------------------------------------
	PX_INLINE		PxActorFlags				getActorFlags() const								{ return read<Buf::BF_ActorFlags>(); }
	PX_INLINE		void						setActorFlags(PxActorFlags v);

	PX_INLINE		PxDominanceGroup			getDominanceGroup() const							{ return read<Buf::BF_DominanceGroup>(); }
	PX_INLINE		void						setDominanceGroup(PxDominanceGroup v)				{ write<Buf::BF_DominanceGroup>(v); }

	PX_INLINE		PxActorClientBehaviorFlags	getClientBehaviorFlags() const						{ return read<Buf::BF_ClientBehaviorFlags>(); }
	PX_INLINE		void						setClientBehaviorFlags(PxActorClientBehaviorFlags v){ write<Buf::BF_ClientBehaviorFlags>(v); }

	PX_INLINE		void						setOwnerClient( PxClientID inId );
	PX_INLINE		PxClientID					getOwnerClient() const								{ return getActorCore().getOwnerClient(); }	//immutable, so this should be fine. 

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
	PX_FORCE_INLINE const Core&					getActorCore() const 	{ return *reinterpret_cast<const Core*>(reinterpret_cast<size_t>(this) + sOffsets.scbToSc[getScbType()]); }
	PX_FORCE_INLINE	Core&						getActorCore()			{ return *reinterpret_cast<Core*>(reinterpret_cast<size_t>(this) + sOffsets.scbToSc[getScbType()]); }

	PX_FORCE_INLINE static const Actor&			fromSc(const Core& a)	{ return *reinterpret_cast<const Actor*>(reinterpret_cast<size_t>(&a) - sOffsets.scToScb[a.getActorCoreType()]); }
	PX_FORCE_INLINE static Actor&				fromSc(Core &a)			{ return *reinterpret_cast<Actor*>(reinterpret_cast<size_t>(&a) - sOffsets.scToScb[a.getActorCoreType()]); }

	PX_FORCE_INLINE PxActorType::Enum			getActorType()		const	{	return getActorCore().getActorCoreType();	}

protected:
	PX_INLINE		void						syncState();

	//---------------------------------------------------------------------------------
	// Infrastructure for regular attributes
	//---------------------------------------------------------------------------------

	struct Access: public BufferedAccess<Buf, Core, Actor> {};
	template<PxU32 f> PX_FORCE_INLINE typename Buf::Fns<f,0>::Arg read() const		{	return Access::read<Buf::Fns<f,0> >(*this, getActorCore());	}
	template<PxU32 f> PX_FORCE_INLINE void write(typename Buf::Fns<f,0>::Arg v)		{	Access::write<Buf::Fns<f,0> >(*this, getActorCore(), v);	}
	template<PxU32 f> PX_FORCE_INLINE void flush(Core& core, const Buf& buf)		{	Access::flush<Buf::Fns<f,0> >(*this, core, buf);			}

protected:		
	~Actor() {}

	struct Offsets
	{
		size_t scToScb[PxActorType::eACTOR_COUNT];
		size_t scbToSc[ScbType::eTYPE_COUNT];
		Offsets();
	};
	static const Offsets					sOffsets;
};

PX_INLINE void Actor::setActorFlags(PxActorFlags v)
{
#if PX_CHECKED
	PxActorFlags aFlags = getActorFlags();
	PxActorType::Enum aType = getActorType();
	if((!aFlags.isSet(PxActorFlag::eDISABLE_SIMULATION)) && v.isSet(PxActorFlag::eDISABLE_SIMULATION) &&
		(aType != PxActorType::eRIGID_DYNAMIC) && (aType != PxActorType::eRIGID_STATIC))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"PxActor::setActorFlag: PxActorFlag::eDISABLE_SIMULATION is only supported by PxRigidDynamic and PxRigidStatic objects.");
	}
#endif

	write<Buf::BF_ActorFlags>(v);
}

PX_INLINE void Actor::setOwnerClient(PxClientID inId)
{
	//This call is only valid if we aren't in a scene.
	//Thus we can't be buffering yet
	if(!isBuffering())
	{
		getActorCore().setOwnerClient( inId );
		UPDATE_PVD_PROPERTIES_OBJECT()
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, 
				"Attempt to set the client id when an actor is buffering");
	}
}

PX_INLINE void Actor::syncState()
{
	//this should be called from syncState() of derived classes

	const PxU32 flags = getBufferFlags();
	if(flags & (Buf::BF_ActorFlags|Buf::BF_DominanceGroup|Buf::BF_ClientBehaviorFlags))
	{
		Core& core = getActorCore();
		const Buf& buffer = *reinterpret_cast<const Buf*>(getStream());

		flush<Buf::BF_ActorFlags>(core, buffer);
		flush<Buf::BF_DominanceGroup>(core, buffer);
		flush<Buf::BF_ClientBehaviorFlags>(core, buffer);
	}
}

}  // namespace Scb

}

#endif

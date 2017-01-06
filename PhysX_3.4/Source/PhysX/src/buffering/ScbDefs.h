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

#ifndef PX_PHYSICS_SCB_DEFS
#define PX_PHYSICS_SCB_DEFS

#include "ScbBase.h"

// a Regular attribute of type T is one for which 
// * the SC method takes a single argument of type ArgType<T> (defined below)
// * Scb either passes that argument through, or dumps it in a buffer to flush later. 
// * PVD is notified when the variable changes
//
// For each such, we can define static methods to read and write the core and buffered variables, 
// and capture the buffering logic in the BufferedAccess class.
//
// The dummy arg is necessary here because ISO permits partial specialization of member templates
// but not full specialization.
//
// putting just accessors and mutators here allows us to change the behavior just by varying the
// BufferAccess template (e.g. to compile without buffering), and also to size-reduce that template 
// by passing function pointers if necessary

#define SCB_REGULAR_ATTRIBUTE(_val, _type, _name)											\
enum { BF_##_name = 1<<(_val) };															\
_type m##_name;																				\
template<PxU32 Dummy> struct Fns<1<<(_val),Dummy>											\
{																							\
	typedef typename ArgType<_type>::Type Arg;												\
	enum { flag = 1<<(_val) };																\
	static PX_FORCE_INLINE Arg getBuffered(const Buf& buf) { return Arg(buf.m##_name);}		\
	static PX_FORCE_INLINE void setBuffered(Buf& buf, Arg v) { buf.m##_name = v;}			\
	static PX_FORCE_INLINE Arg getCore(const Core& core) { return Arg(core.get##_name());}	\
	static PX_FORCE_INLINE void setCore(Core& core, Arg v) { core.set##_name(v);}			\
};	

#define SCB_REGULAR_ATTRIBUTE_ALIGNED(_val, _type, _name, _alignment)						\
enum { BF_##_name = 1<<(_val) };															\
PX_ALIGN(_alignment, _type) m##_name;														\
template<PxU32 Dummy> struct Fns<1<<(_val),Dummy>											\
{																							\
	typedef typename ArgType<_type>::Type Arg;												\
	enum { flag = 1<<(_val) };																\
	static PX_FORCE_INLINE Arg getBuffered(const Buf& buf) { return buf.m##_name;}			\
	static PX_FORCE_INLINE void setBuffered(Buf& buf, Arg v) { buf.m##_name = v;}			\
	static PX_FORCE_INLINE Arg getCore(const Core& core) { return core.get##_name();}		\
	static PX_FORCE_INLINE void setCore(Core& core, Arg v) { core.set##_name(v);}			\
};																											

namespace physx
{
namespace Scb
{
class Scene;

template<typename T> struct ArgType		{ typedef T Type; };
template<> struct ArgType<PxVec3>		{ typedef const PxVec3& Type; };
template<> struct ArgType<PxTransform>	{ typedef const PxTransform& Type; };
template<> struct ArgType<PxQuat>		{ typedef const PxQuat& Type; };
template<> struct ArgType<PxPlane>		{ typedef const PxPlane& Type; };
template<> struct ArgType<PxFilterData>	{ typedef const PxFilterData& Type; };

// TODO: should be able to size-reduce this if necessary by just generating one set per
// arg type instead of one per arg, by passing function pointers to the accessors/mutators/flag
// instead of instancing per type.

template<class Buf, class Core, class ScbClass, class BaseClass=Scb::Base>  // BaseClass: introduced to have Scb::Body use custom location for storing buffered property flags
struct BufferedAccess
{
	template<typename Fns>
	static PX_FORCE_INLINE typename Fns::Arg read(const BaseClass& base, const Core& core)
	{
		return base.isBuffered(Fns::flag) ? Fns::getBuffered(*reinterpret_cast<const Buf*>(base.getStream())) 
										  : Fns::getCore(core);
	}

	template<typename Fns>
	static PX_FORCE_INLINE void write(BaseClass& base, Core& core, typename Fns::Arg v)
	{
		if(!base.isBuffering())
		{
			Fns::setCore(core, v);
#if PX_SUPPORT_PVD
			if(base.getControlState() == ControlState::eIN_SCENE)
			{
				Scb::Scene* scene = base.getScbScene();
				PX_ASSERT(scene);
				scene->getScenePvdClient().updatePvdProperties(static_cast<ScbClass*>(&base));
			}
#endif
		}
		else
		{
			Fns::setBuffered(*reinterpret_cast<Buf*>(base.getStream()), v);
			base.markUpdated(Fns::flag);
		}
	}

	template<typename Fns> 
	static PX_FORCE_INLINE void flush(const BaseClass& base, Core& core, const Buf& buf)
	{
		if(base.isBuffered(Fns::flag))
			Fns::setCore(core, Fns::getBuffered(buf));
	}
};

}
}

#endif

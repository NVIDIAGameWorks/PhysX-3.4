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

#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

#include "SamplePreprocessor.h"
#include "SampleAllocator.h"
#include "SampleUserInput.h"
#include "foundation/PxAssert.h"
#include "PsIntrinsics.h"

class PhysXSampleApplication;

template<typename T, PxU32 SIZE> 
class RingBuffer
{
public:
	RingBuffer()
	: mReadCount(0)
	, mWriteCount(0)
	{
		// SIZE has to be power of two
#if PX_VC
		PX_COMPILE_TIME_ASSERT(SIZE > 0);
		PX_COMPILE_TIME_ASSERT((SIZE&(SIZE-1)) == 0);
#else
		PX_ASSERT(SIZE > 0);
		PX_ASSERT((SIZE&(SIZE-1)) == 0);
#endif
	}
	
	PX_FORCE_INLINE bool		isEmpty() const			{ return mReadCount==mWriteCount;								}
	PX_FORCE_INLINE bool		isFull() const			{ return isFull(mReadCount, mWriteCount);						}
	PX_FORCE_INLINE PxU32		size() const			{ return size(mReadCount, mWriteCount);							}
	PX_FORCE_INLINE PxU32		capacity() const		{ return SIZE;													}
	// clear is only save if called from reader thread!
	PX_FORCE_INLINE void		clear()					{ mReadCount=mWriteCount;										}
	PX_FORCE_INLINE const T&	back() const			{ PX_ASSERT(!isEmpty()); return mRing[mReadCount&moduloMask];	}
	PX_FORCE_INLINE T&			front()					{ return mRing[mWriteCount&moduloMask];							}
	PX_FORCE_INLINE void		incFront(PxU32 inc)		{ PX_ASSERT(SIZE-size() >= inc); mWriteCount+=inc;				}
	PX_FORCE_INLINE void		incBack(PxU32 inc)		{ PX_ASSERT(size() >= inc); mReadCount+=inc; 					}

	PX_FORCE_INLINE bool		pushFront(const T& e)	
	{ 
		if(!isFull())
		{
			mRing[mWriteCount&moduloMask] = e;
			Ps::memoryBarrier();
			mWriteCount++;
			return true;
		}
		else
			return false;

	}

	PX_FORCE_INLINE bool popBack(T& e)	
	{ 
		if(!isEmpty())
		{
			e = mRing[mReadCount&moduloMask];
			mReadCount++;
			return true;
		}
		else
			return false;
	}

private:
	PX_FORCE_INLINE static PxU32	moduloDistance(PxI32 r, PxI32 w) 	{ return PxU32((w-r)&moduloMask);						}
	PX_FORCE_INLINE static bool		isFull(PxI32 r, PxI32 w) 			{ return r!=w && moduloDistance(r,w)==0;				}
	PX_FORCE_INLINE static PxU32	size(PxI32 r, PxI32 w) 				{ return isFull(r, w) ? SIZE : moduloDistance(r, w);	}

private:
	static const PxU32	moduloMask = SIZE-1;
	T					mRing[SIZE];
	volatile PxI32		mReadCount;
	volatile PxI32		mWriteCount;
};


class InputEventBuffer: public SampleFramework::InputEventListener, public SampleAllocateable
{
public:

	InputEventBuffer(PhysXSampleApplication& p);
	virtual ~InputEventBuffer();

	virtual	void	onKeyDownEx(SampleFramework::SampleUserInput::KeyCode keyCode, PxU32 wParam);
	virtual	void	onAnalogInputEvent(const SampleFramework::InputEvent& , float val);
	virtual	void	onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);        
	virtual	void	onPointerInputEvent(const SampleFramework::InputEvent&, PxU32 x, PxU32 y, PxReal dx, PxReal dy, bool val);
			void 	clear();
			void 	flush();
private:
	PX_FORCE_INLINE void checkResetLastInput()
	{
		if(mResetInputCacheReq!=mResetInputCacheAck)
		{
			mLastKeyDownEx = NULL;
			mLastDigitalInput = NULL;
			mLastAnalogInput = NULL;
			mLastPointerInput = NULL;
			mResetInputCacheAck++;
			PX_ASSERT(mResetInputCacheReq==mResetInputCacheAck);
		}
	}
	struct EventType
	{
        virtual ~EventType() {}
		virtual void report(PhysXSampleApplication& app) const { }
	};
	struct KeyDownEx: public EventType
	{ 
		virtual void report(PhysXSampleApplication& app) const;

		bool isEqual(SampleFramework::SampleUserInput::KeyCode _keyCode, PxU32 _wParam)
		{
			return (_keyCode == keyCode) && (_wParam == wParam);
		}

		SampleFramework::SampleUserInput::KeyCode keyCode;
		PxU32 wParam;
	};
	struct AnalogInput: public EventType
	{ 
		virtual void report(PhysXSampleApplication& app) const;

		bool isEqual(SampleFramework::InputEvent _e, float _val)
		{
			return (_e.m_Id == e.m_Id) && (_e.m_Analog == e.m_Analog) && (_e.m_Sensitivity == e.m_Sensitivity) && (_val == val);
		}

		SampleFramework::InputEvent e;
		float val;
	};
	struct DigitalInput: public EventType
	{ 
		virtual void report(PhysXSampleApplication& app) const;

		bool isEqual(SampleFramework::InputEvent _e, bool _val)
		{
			return (_e.m_Id == e.m_Id) && (_e.m_Analog == e.m_Analog) && (_e.m_Sensitivity == e.m_Sensitivity) && (_val == val);
		}

		SampleFramework::InputEvent e;
		bool val;
	};
	struct PointerInput: public EventType
	{ 
		virtual void report(PhysXSampleApplication& app) const;

		bool isEqual(SampleFramework::InputEvent _e, PxU32 _x, PxU32 _y, PxReal _dx, PxReal _dy, bool _val)
		{
			return (_e.m_Id == e.m_Id) && (_e.m_Analog == e.m_Analog) && (_e.m_Sensitivity == e.m_Sensitivity) && 
				(_x == x) && (_y == y) && (_dx == dx) && (_dy == dy) && (_val == val);
		}

		SampleFramework::InputEvent e;
		PxU32 x;
		PxU32 y;
		PxReal dx;
		PxReal dy;
		bool val;
	};

	struct EventsUnion
	{
			template<class Event> PX_CUDA_CALLABLE PX_FORCE_INLINE Event& get()
			{
				return reinterpret_cast<Event&>(events);
			}
			template<class Event> PX_CUDA_CALLABLE PX_FORCE_INLINE const Event& get() const
			{
				return reinterpret_cast<const Event&>(events);
			}
		union
		{
			PxU8	eventType[sizeof(EventType)];
			PxU8	keyDownEx[sizeof(KeyDownEx)];
			PxU8	analogInput[sizeof(AnalogInput)];
			PxU8	digitalInput[sizeof(DigitalInput)];
			PxU8	pointerInput[sizeof(PointerInput)];
		} events;
	};

	static const PxU32					MAX_EVENTS = 64;
	static const PxU32					MAX_MOUSE_EVENTS = 48;
	static const PxU32					MAX_ANALOG_EVENTS = 48;
	RingBuffer<EventsUnion, MAX_EVENTS>	mRingBuffer; 
	volatile PxU32						mResetInputCacheReq;
	volatile PxU32						mResetInputCacheAck;
	KeyDownEx*							mLastKeyDownEx;
	DigitalInput*						mLastDigitalInput;
	AnalogInput*						mLastAnalogInput;
	PointerInput*						mLastPointerInput;
	PhysXSampleApplication&				mApp;
	bool								mClearBuffer;
};

#endif

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

#include "InputEventBuffer.h"
#include "PhysXSampleApplication.h"

using namespace physx;
using namespace SampleRenderer;
using namespace SampleFramework;
using namespace PxToolkit;

InputEventBuffer::InputEventBuffer(PhysXSampleApplication& p)
: mResetInputCacheReq(0)
, mResetInputCacheAck(0)
, mLastKeyDownEx(NULL)
, mLastDigitalInput(NULL)
, mLastAnalogInput(NULL)
, mLastPointerInput(NULL)
, mApp(p)
, mClearBuffer(false)
{
}

InputEventBuffer::~InputEventBuffer()
{
}

void InputEventBuffer::onKeyDownEx(SampleFramework::SampleUserInput::KeyCode keyCode, PxU32 wParam)
{
	checkResetLastInput();
	if(mLastKeyDownEx && mLastKeyDownEx->isEqual(keyCode, wParam))
		return;
	if(mRingBuffer.isFull())
		return;
	KeyDownEx& event = mRingBuffer.front().get<KeyDownEx>();
	PX_PLACEMENT_NEW(&event, KeyDownEx);
	event.keyCode = keyCode;
	event.wParam = wParam;
	mLastKeyDownEx = &event;
	mRingBuffer.incFront(1);
}

void InputEventBuffer::onAnalogInputEvent(const SampleFramework::InputEvent& e, float val)
{
	checkResetLastInput();
	if(mLastAnalogInput && mLastAnalogInput->isEqual(e, val))
		return;
	if(mRingBuffer.isFull() || (mRingBuffer.size() > MAX_ANALOG_EVENTS))
		return;
	AnalogInput& event = mRingBuffer.front().get<AnalogInput>();
	PX_PLACEMENT_NEW(&event, AnalogInput);
	event.e = e;
	event.val = val;
	mLastAnalogInput = &event;
	mRingBuffer.incFront(1);
}

void InputEventBuffer::onDigitalInputEvent(const SampleFramework::InputEvent& e, bool val)
{
	checkResetLastInput();
	if(mLastDigitalInput && mLastDigitalInput->isEqual(e, val))
		return;
	if(mRingBuffer.isFull())
		return;
	DigitalInput& event = mRingBuffer.front().get<DigitalInput>();
	PX_PLACEMENT_NEW(&event, DigitalInput);
	event.e = e;
	event.val = val;
	mLastDigitalInput = &event;
	mRingBuffer.incFront(1);
}

void InputEventBuffer::onPointerInputEvent(const SampleFramework::InputEvent& e, PxU32 x, PxU32 y, PxReal dx, PxReal dy, bool val)
{
	checkResetLastInput();
	if(mLastPointerInput && mLastPointerInput->isEqual(e, x, y, dx, dy, val))
		return;
	if(mRingBuffer.isFull() || (mRingBuffer.size() > MAX_MOUSE_EVENTS))
		return;
	PointerInput& event = mRingBuffer.front().get<PointerInput>();
	PX_PLACEMENT_NEW(&event, PointerInput);
	event.e = e;
	event.x = x;
	event.y = y;
	event.dx = dx;
	event.dy = dy;
	event.val = val;
	mLastPointerInput = &event;
	mRingBuffer.incFront(1);
}

void InputEventBuffer::clear()
{
	mClearBuffer = true;
}

void InputEventBuffer::flush()
{
	if(mResetInputCacheReq==mResetInputCacheAck)
		mResetInputCacheReq++;
	
	PxU32 size = mRingBuffer.size();
	Ps::memoryBarrier();
	// do not work on more than size, else input cache might become overwritten
	while(size-- && !mClearBuffer)
	{
		mRingBuffer.back().get<EventType>().report(mApp);
		mRingBuffer.incBack(1);
	}

	if(mClearBuffer)
	{
		mRingBuffer.clear();
		mClearBuffer = false;
	}
}

void InputEventBuffer::KeyDownEx::report(PhysXSampleApplication& app) const
{
	app.onKeyDownEx(keyCode, wParam);
}

void InputEventBuffer::AnalogInput::report(PhysXSampleApplication& app) const
{
	app.onAnalogInputEvent(e, val);
}

void InputEventBuffer::DigitalInput::report(PhysXSampleApplication& app) const
{
	app.onDigitalInputEvent(e, val);
}

void InputEventBuffer::PointerInput::report(PhysXSampleApplication& app) const
{
	app.onPointerInputEvent(e, x, y, dx, dy, val);
}

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

#ifndef SAMPLE_USER_INPUT_DEFINES_H
#define SAMPLE_UTILS_H

#if defined(RENDERER_WINDOWS) && !PX_XBOXONE 

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),winKey, #var); \
	if(retVal) inputEvents.push_back(retVal); } 
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),winKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined(RENDERER_WINDOWS) && PX_XBOXONE 

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),xboxonekey, #var); \
	if(retVal) inputEvents.push_back(retVal); } 
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),xboxonekey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined (RENDERER_XBOX360) 

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),xbox360key, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),xbox360key, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined (RENDERER_PS4)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),ps4Key, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),ps4Key, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined (RENDERER_PS3)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),ps3Key, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),ps3Key, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined (RENDERER_ANDROID)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),andrKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),andrKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey) {\
    const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerTouchInputEvent(SampleFramework::InputEvent(var, false),andrKey,caption, #var)); \
	if(retVal) inputEvents.push_back(retVal); }

#elif defined (RENDERER_MACOSX)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),osxKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),osxKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined (RENDERER_IOS)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)   {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),iosKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),iosKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)   {\
    const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerTouchInputEvent(SampleFramework::InputEvent(var, false),iosKey,caption, #var)); \
	if(retVal) inputEvents.push_back(retVal); }

#elif defined (RENDERER_LINUX)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false),linuxKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity),linuxKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#elif defined (RENDERER_WIIU)	

#define DIGITAL_INPUT_EVENT_DEF(var, winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, false), wiiuKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define ANALOG_INPUT_EVENT_DEF(var, sensitivity,  winKey, xbox360key, xboxonekey, ps3Key, ps4Key, andrKey, osxKey, iosKey, linuxKey, wiiuKey)  {\
	const SampleFramework::InputEvent* retVal = (SampleFramework::SamplePlatform::platform()->getSampleUserInput()->registerInputEvent(SampleFramework::InputEvent(var, true,sensitivity), wiiuKey, #var)); \
	if(retVal) inputEvents.push_back(retVal); }
#define TOUCH_INPUT_EVENT_DEF(var, caption, andrKey, iosKey)

#endif
#endif

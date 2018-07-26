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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PhysXSampleApplication.h"

#include "PxTkNamespaceMangle.h"
#include "PxTkStream.h"

#include "RendererMemoryMacros.h"
#include "SampleUtils.h"
#include "SampleStepper.h"
#include "SampleActor.h"
#include "SampleCommandLine.h"
#include "SampleConsole.h"
#include "SampleAllocator.h"
#include "SampleAllocatorSDKClasses.h"

#include "SamplePlatform.h"

#include "Renderer.h"
#include "RendererDirectionalLightDesc.h"

#include "RenderBoxActor.h"
#include "RenderSphereActor.h"
#include "RenderCapsuleActor.h"
#include "RenderMeshActor.h"
#include "RenderGridActor.h"
#include "RenderMaterial.h"
#include "RenderTexture.h"
#include "RenderPhysX3Debug.h"

#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include "PxTkFile.h"
#include "task/PxTask.h"
#include "PxToolkit.h"
#include "extensions/PxDefaultSimulationFilterShader.h"
#include "PxFiltering.h"

#include "SampleBaseInputEventIds.h"

#include <algorithm>
#include "PxTkBmpLoader.h"
#include <ctype.h>

#include "PhysXSample.h"
#include "TestGroup.h"

#include <SampleUserInputIds.h>
#include "SampleUserInputDefines.h"

#include "InputEventBuffer.h"

using namespace physx;
using namespace SampleRenderer;
using namespace SampleFramework;
using namespace PxToolkit;

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
//#define SCREEN_WIDTH	1024
//#define SCREEN_HEIGHT	768
#define	CAMERA_FOV		70.0f
#define	CAMERA_NEAR		0.1f
#define	CAMERA_FAR		10000.0f

#if PX_PS4
unsigned int sceLibcHeapExtendedAlloc = 1;  /* Switch to dynamic allocation */
size_t       sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT; /* no upper limit for heap area */
#endif

#define SAMPLE_FRAMEWORK_MEDIA_DIR	"media"

static char gSampleMediaFolder[512];

///////////////////////////////////////////////////////////////////////////////

static PhysXSampleApplication* gBase = NULL;
static void gNearPlane(Console* console, const char* text, void* userData)
{
	if(!text)
	{
		console->out("Usage: nearplane <float>");
		return;
	}

	const float val = (float)::atof(text);
	gBase->getCamera().setNearPlane(val);
}

static void gFarPlane(Console* console, const char* text, void* userData)
{
	if(!text)
	{
		console->out("Usage: farplane <float>");
		return;
	}

	const float val = (float)::atof(text);
	gBase->getCamera().setFarPlane(val);
}

static void gDrawCameraDebug(Console* console, const char* text, void* userData)
{
	gBase->getCamera().mDrawDebugData = !gBase->getCamera().mDrawDebugData;
}

static void gFreezeFrustum(Console* console, const char* text, void* userData)
{
	gBase->getCamera().mFreezeFrustum = !gBase->getCamera().mFreezeFrustum;
}

static void gVFC(Console* console, const char* text, void* userData)
{
	gBase->getCamera().mPerformVFC = !gBase->getCamera().mPerformVFC;
}

///////////////////////////////////////////////////////////////////////////////

PhysXSampleApplication::PhysXSampleApplication(const SampleCommandLine& cmdline) :
	SampleApplication					(cmdline, SAMPLE_FRAMEWORK_MEDIA_DIR),
	mTextAlphaHelp						(0.0f),
	mTextAlphaDesc						(0.0f),
	mMenuType							(MenuType::TESTS),
	mMenuVisualizationsIndexSelected	(0),
	mIsCloseRequested					(false),
	mDebugRenderer						(NULL),
	mPause								(false),
	mOneFrameUpdate						(false),
	mSwitchSample						(false),
	mShowHelp							(false),
	mShowDescription					(false),
	mShowExtendedHelp					(false),
	mHideMouseCursor					(false),
	mDrawScreenQuad						(true),
	mMenuExpand							(false),
	mRunning							(NULL),
	mSelected							(NULL),
	mSample								(NULL),
	mDefaultSamplePath					(NULL)
{
	mConsole = SAMPLE_NEW(Console)(getPlatform());
	mInputEventBuffer = SAMPLE_NEW(InputEventBuffer)(*this);
	if(mConsole)
	{
		gBase = this;
		mConsole->addCmd("nearplane", gNearPlane);
		mConsole->addCmd("farplane", gFarPlane);
		mConsole->addCmd("drawcameradebug", gDrawCameraDebug);
		mConsole->addCmd("freezefrustum", gFreezeFrustum);
		mConsole->addCmd("vfc", gVFC);
	}

	mScreenQuadTopColor		= RendererColor(0x00, 0x00, 0x80);
	mScreenQuadBottomColor	= RendererColor(0xff, 0xf0, 0xf0);

	// Projection
	mCamera.setFOV(CAMERA_FOV);
	mCamera.setNearPlane(CAMERA_NEAR);
	mCamera.setFarPlane(CAMERA_FAR);
	mCamera.setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	// View
	mCamera.setPos(PxVec3(0.0f, 10.0f, 0.0f));
	mCameraController.init(PxVec3(0.0f, 10.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.0f));
	setDefaultCameraController();

	// PT: search path for samples' media files
	strcpy(gSampleMediaFolder, getAssetPathPrefix());
	strcat(gSampleMediaFolder, SAMPLE_MEDIA_DIR);
	addSearchPath(gSampleMediaFolder);

	for (PxU32 i = 0; i < MATERIAL_COUNT; ++i)
		mManagedMaterials[i] = NULL;

	mMenuVisualizations.push_back(MenuTogglableItem(PxVisualizationParameter::eSCALE, "Debug Rendering"));

	#define MENU_PUSHV(var, title) mMenuVisualizations.push_back( \
	MenuTogglableItem(PxVisualizationParameter::var, title))

	MENU_PUSHV(eCOLLISION_AABBS,		"Collision AABBs"			);
	MENU_PUSHV(eCOLLISION_SHAPES,		"Collision Shapes"			);
	MENU_PUSHV(eCOLLISION_AXES,			"Collision Axes"			);
	MENU_PUSHV(eCOLLISION_FNORMALS,		"Collision Face Normals"	);
	MENU_PUSHV(eCOLLISION_EDGES,		"Collision Edges"			);
	MENU_PUSHV(eCONTACT_POINT,			"Contact Points"			);
	MENU_PUSHV(eCONTACT_NORMAL,			"Contact Normals"			);
	MENU_PUSHV(eCONTACT_ERROR,			"Contact Error"				);
	MENU_PUSHV(eCONTACT_FORCE,			"Contact Force"				);
	MENU_PUSHV(eACTOR_AXES,				"Actor Axes"				);
	MENU_PUSHV(eBODY_AXES,				"Body Axes"					);
	MENU_PUSHV(eBODY_LIN_VELOCITY,		"Linear Velocity"			);
	MENU_PUSHV(eBODY_ANG_VELOCITY,		"Angular Velocity"			);
	MENU_PUSHV(eBODY_MASS_AXES,			"Mass Axes"					);
	MENU_PUSHV(eJOINT_LIMITS,			"Joint Limits"				);
	MENU_PUSHV(eJOINT_LOCAL_FRAMES,		"Joint Local Frames"		);
	MENU_PUSHV(ePARTICLE_SYSTEM_BROADPHASE_BOUNDS, "Particle System Broadphase Bounds");
	MENU_PUSHV(ePARTICLE_SYSTEM_GRID,	"Particle System Grid"		);	
	MENU_PUSHV(ePARTICLE_SYSTEM_BOUNDS, "Particle System Bounds"	);	
	MENU_PUSHV(ePARTICLE_SYSTEM_COLLISION_NORMAL, "Particle System Collision Normals");		
	MENU_PUSHV(eCULL_BOX,				"Culling Box"				);
	//
	MENU_PUSHV(eCOLLISION_STATIC,		"Static pruning structure"	);
	MENU_PUSHV(eCOLLISION_DYNAMIC,		"Dynamic pruning structure"	);
	MENU_PUSHV(eCOLLISION_COMPOUNDS,	"Compounds"					);
	//
	MENU_PUSHV(eCLOTH_VERTICAL,			"Cloth Vertical Phases"		);
	MENU_PUSHV(eCLOTH_HORIZONTAL,		"Cloth Horizontal Phases"	);
	MENU_PUSHV(eCLOTH_BENDING,			"Cloth Bending Phases"		);
	MENU_PUSHV(eCLOTH_SHEARING,			"Cloth Shearing Phases"		);
	MENU_PUSHV(eCLOTH_VIRTUAL_PARTICLES, "Cloth Virtual Particles"  );

    mSelected = getSampleTreeRoot().getFirstTest();

    setPvdParams(cmdline);
}

PhysXSampleApplication::~PhysXSampleApplication()
{
	DELETESINGLE(mConsole);
	DELETESINGLE(mInputEventBuffer);

	PX_ASSERT(!mLights.size());
}

void PhysXSampleApplication::execute()
{
	if(isOpen())
        onOpen();
	while(!quitIsSignalled())
	{
		if(isOpen() && !isCloseRequested())
		{
			updateEngine();
		}
		else 
			break;
#if defined(RENDERER_ANDROID)
		if(!SamplePlatform::platform()->isOpen())
			break;
#endif
	}

	mInputMutex.lock();
	if (isOpen() || isCloseRequested())
	{
		close();
	}
	mInputMutex.unlock();

	quit();
}

void PhysXSampleApplication::updateEngine()
{
	if(mInputEventBuffer)
		mInputEventBuffer->flush();
	if(mSwitchSample)
		switchSample();
	onDraw();
}

const char* PhysXSampleApplication::inputInfoMsg(const char* firstPart,const char* secondPart, PxI32 inputEventId1, PxI32 inputEventId2)
{
	const char* keyNames1[5];

	PxU32 inputMask = 0;
	if(m_platform->getSampleUserInput()->gamepadSupported() && m_platform->getSampleUserInput()->keyboardSupported())
	{
		inputMask = KEYBOARD_INPUT | GAMEPAD_INPUT | TOUCH_PAD_INPUT | MOUSE_INPUT;
	}
	else
	{
		if(m_platform->getSampleUserInput()->gamepadSupported())
		{
			inputMask = GAMEPAD_INPUT | TOUCH_PAD_INPUT | MOUSE_INPUT;
		}

		if(m_platform->getSampleUserInput()->keyboardSupported())
		{
			inputMask = KEYBOARD_INPUT | TOUCH_PAD_INPUT | MOUSE_INPUT;
		}
	}

	PxU16 numNames1 = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventId1,keyNames1,5,inputMask);

	const char* keyNames2[5];
	PxU16 numNames2 = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventId2,keyNames2,5,inputMask);

	if(!numNames1 && !numNames2)
		return NULL;

	strcpy(m_Msg, firstPart);
	if(numNames1 && numNames2)
	{
		for (PxU16 i = 0; i < numNames1; i++)
		{
			strcat(m_Msg, keyNames1[i]);
			strcat(m_Msg, " or ");
		}		

		for (PxU16 i = 0; i < numNames2; i++)
		{
			strcat(m_Msg, keyNames2[i]);
			if(i != numNames2 - 1)
				strcat(m_Msg, " or ");
		}				
	}
	else
	{
		if(numNames1)
		{
			for (PxU16 i = 0; i < numNames1; i++)
			{
				strcat(m_Msg, keyNames1[i]);
				if(i != numNames1 - 1)
					strcat(m_Msg, " or ");
			}		
		}
		else
		{
			for (PxU16 i = 0; i < numNames2; i++)
			{
				strcat(m_Msg, keyNames2[i]);
				if(i != numNames2 - 1)
					strcat(m_Msg, " or ");
			}		
		}
	}

	strcat(m_Msg, secondPart);
	return m_Msg;
}

const char* PhysXSampleApplication::inputInfoMsg_Aor_BandC(const char* firstPart,const char* secondPart, PxI32 inputEventIdA, PxI32 inputEventIdB, PxI32 inputEventIdC)
{
	PxU32 inputMask = TOUCH_PAD_INPUT | MOUSE_INPUT;
	if(m_platform->getSampleUserInput()->gamepadSupported())
		inputMask |= GAMEPAD_INPUT;
	if(m_platform->getSampleUserInput()->keyboardSupported())
		inputMask |= KEYBOARD_INPUT;

	const char* keyNamesA[5];
	PxU16 numNamesA = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventIdA,keyNamesA,5,inputMask);

	const char* keyNamesB[5];
	PxU16 numNamesB = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventIdB,keyNamesB,5,inputMask);

	const char* keyNamesC[5];
	PxU16 numNamesC = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventIdC,keyNamesC,5,inputMask);

	PX_ASSERT((numNamesB!=0) == (numNamesC!=0));

	strcpy(m_Msg, firstPart);
	{

		for (PxU16 i = 0; i < numNamesA; i++)
		{
			strcat(m_Msg, keyNamesA[i]);
			if(i != numNamesA - 1)
				strcat(m_Msg, " or ");
		}	

		if(numNamesB && numNamesC)
		{
			if(numNamesA)
				strcat(m_Msg, " or (");

			for (PxU16 i = 0; i < numNamesB; i++)
			{
				strcat(m_Msg, keyNamesB[i]);
				if(i != numNamesB - 1)
					strcat(m_Msg, " or ");
			}		

			strcat(m_Msg, " and ");

			for (PxU16 i = 0; i < numNamesC; i++)
			{
				strcat(m_Msg, keyNamesC[i]);
				if(i != numNamesC - 1)
					strcat(m_Msg, " or ");
			}		
			if(numNamesA)
				strcat(m_Msg, ")");
		}		
	}

	strcat(m_Msg, secondPart);
	return m_Msg;
}


const char* PhysXSampleApplication::inputMoveInfoMsg(const char* firstPart,const char* secondPart, PxI32 inputEventId1, PxI32 inputEventId2,PxI32 inputEventId3,PxI32 inputEventId4)
{
	const char* keyNames1[5];
	const char* keyNames2[5];
	const char* keyNames3[5];
	const char* keyNames4[5];

	PxU32 inputMask = 0;
	if(m_platform->getSampleUserInput()->gamepadSupported() && m_platform->getSampleUserInput()->keyboardSupported())
	{
		inputMask = KEYBOARD_INPUT | GAMEPAD_INPUT | TOUCH_PAD_INPUT | MOUSE_INPUT;
	}
	else
	{
		if(m_platform->getSampleUserInput()->gamepadSupported())
		{
			inputMask = GAMEPAD_INPUT | TOUCH_PAD_INPUT | MOUSE_INPUT;
		}

		if(m_platform->getSampleUserInput()->keyboardSupported())
		{
			inputMask = KEYBOARD_INPUT | TOUCH_PAD_INPUT | MOUSE_INPUT;
		}
	}

	PxU16 numNames1 = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventId1,keyNames1,5,inputMask);
	PxU16 numNames2 = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventId2,keyNames2,5,inputMask);
	PxU16 numNames3 = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventId3,keyNames3,5,inputMask);
	PxU16 numNames4 = getPlatform()->getSampleUserInput()->getUserInputKeys(inputEventId4,keyNames4,5,inputMask);


	if(!numNames1 && !numNames2 && !numNames3 && !numNames4)
		return NULL;

	bool firstVal = true;

	strcpy(m_Msg, firstPart);
	if(numNames1)
	{
		for (PxU16 i = 0 ; i < numNames1; i++)
		{
			if(!firstVal)
			{
				strcat(m_Msg, ",");				
			}

			firstVal = false;
			strcat(m_Msg, keyNames1[i]);
		}		
	}
	if(numNames2)
	{
		for (PxU16 i = 0 ; i < numNames2; i++)
		{
			if(!firstVal)
			{
				strcat(m_Msg, ",");				
			}

			firstVal = false;
			strcat(m_Msg, keyNames2[i]);
		}		
	}
	if(numNames3)
	{
		for (PxU16 i = 0 ; i < numNames3; i++)
		{
			if(!firstVal)
			{
				strcat(m_Msg, ",");				
			}

			firstVal = false;
			strcat(m_Msg, keyNames3[i]);
		}		
	}
	if(numNames4)
	{
		for (PxU16 i = 0 ; i < numNames4; i++)
		{
			if(!firstVal)
			{			
				strcat(m_Msg, ",");				
			}

			firstVal = false;
			strcat(m_Msg, keyNames4[i]);
		}		
	}

	strcat(m_Msg, secondPart);
	return m_Msg;
}

void PhysXSampleApplication::collectInputEvents(std::vector<const InputEvent*>& inputEvents)
{	
	//digital keyboard events
	DIGITAL_INPUT_EVENT_DEF(MENU_SAMPLES,			WKEY_RETURN,	XKEY_RETURN,	X1KEY_RETURN,	PS3KEY_RETURN,	PS4KEY_RETURN,	AKEY_UNKNOWN,	OSXKEY_RETURN,	IKEY_UNKNOWN,	LINUXKEY_RETURN,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_QUICK_UP,			WKEY_UP,		XKEY_UP,		X1KEY_UP,		PS3KEY_UP,		PS4KEY_UP,		AKEY_UNKNOWN,	OSXKEY_UP,		IKEY_UNKNOWN,	LINUXKEY_UP,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_QUICK_DOWN,		WKEY_DOWN,		XKEY_DOWN,		X1KEY_DOWN,		PS3KEY_DOWN,	PS4KEY_DOWN,	AKEY_UNKNOWN,	OSXKEY_DOWN,	IKEY_UNKNOWN,	LINUXKEY_DOWN,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_QUICK_LEFT,		WKEY_LEFT,		XKEY_LEFT,		X1KEY_LEFT,		PS3KEY_LEFT,	PS4KEY_LEFT,	AKEY_UNKNOWN,	OSXKEY_LEFT,	IKEY_UNKNOWN,	LINUXKEY_LEFT,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_QUICK_RIGHT,		WKEY_RIGHT,		XKEY_RIGHT,		X1KEY_RIGHT,	PS3KEY_RIGHT,	PS4KEY_RIGHT,	AKEY_UNKNOWN,	OSXKEY_RIGHT,	IKEY_UNKNOWN,	LINUXKEY_RIGHT,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_SELECT,			WKEY_RETURN,	XKEY_RETURN,	X1KEY_RETURN,	PS3KEY_RETURN,	PS4KEY_RETURN,	AKEY_UNKNOWN,	OSXKEY_RETURN,	IKEY_UNKNOWN,	LINUXKEY_RETURN,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_ESCAPE,			WKEY_ESCAPE,	XKEY_ESCAPE,	X1KEY_ESCAPE,	PS3KEY_ESCAPE,	PS4KEY_ESCAPE,	AKEY_UNKNOWN,	OSXKEY_ESCAPE,	IKEY_UNKNOWN,	LINUXKEY_ESCAPE,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(QUIT,					WKEY_ESCAPE,	XKEY_ESCAPE,	X1KEY_ESCAPE,	PS3KEY_ESCAPE,	PS4KEY_ESCAPE,	AKEY_UNKNOWN,	OSXKEY_ESCAPE,	IKEY_UNKNOWN,	LINUXKEY_ESCAPE,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(SHOW_EXTENDED_HELP,		WKEY_F1,		XKEY_F1,		X1KEY_F1,		PS3KEY_F1,		PS4KEY_F1,		AKEY_UNKNOWN,	OSXKEY_F1,		IKEY_UNKNOWN,	LINUXKEY_F1,		WIIUKEY_UNKNOWN);
#if PX_SUPPORT_GPU_PHYSX || PX_XBOXONE
	DIGITAL_INPUT_EVENT_DEF(TOGGLE_CPU_GPU,			WKEY_F2,		XKEY_UNKNOWN,	X1KEY_F2,		PS3KEY_UNKNOWN,	PS4KEY_UNKNOWN, AKEY_UNKNOWN,	OSXKEY_UNKNOWN,	IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	WIIUKEY_UNKNOWN);
#endif
	DIGITAL_INPUT_EVENT_DEF(MENU_VISUALIZATIONS,	WKEY_F3,		XKEY_F3,		X1KEY_F3,		PS3KEY_F3,		PS4KEY_F3,		AKEY_UNKNOWN,	OSXKEY_F3,		IKEY_UNKNOWN,	LINUXKEY_F3,		WIIUKEY_UNKNOWN);
	
	//digital mouse events
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_BUTTON, MOUSE_BUTTON_LEFT, XKEY_UNKNOWN,	X1KEY_UNKNOWN,	PS3KEY_UNKNOWN, PS4KEY_UNKNOWN, AKEY_UNKNOWN, MOUSE_BUTTON_LEFT, IKEY_UNKNOWN,	MOUSE_BUTTON_LEFT,	WIIUKEY_UNKNOWN);

	//digital gamepad events
	DIGITAL_INPUT_EVENT_DEF(MENU_SAMPLES,	GAMEPAD_START,		GAMEPAD_START,		GAMEPAD_START,		GAMEPAD_START,		GAMEPAD_START,		AKEY_UNKNOWN,	GAMEPAD_START,		IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_START);
	DIGITAL_INPUT_EVENT_DEF(MENU_UP,		GAMEPAD_DIGI_UP,	GAMEPAD_DIGI_UP,	GAMEPAD_DIGI_UP,	GAMEPAD_DIGI_UP,	GAMEPAD_DIGI_UP,	AKEY_UNKNOWN,	GAMEPAD_DIGI_UP,	IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_DIGI_UP);
	DIGITAL_INPUT_EVENT_DEF(MENU_DOWN,		GAMEPAD_DIGI_DOWN,	GAMEPAD_DIGI_DOWN,	GAMEPAD_DIGI_DOWN,	GAMEPAD_DIGI_DOWN,	GAMEPAD_DIGI_DOWN,	AKEY_UNKNOWN,	GAMEPAD_DIGI_DOWN,	IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_DIGI_DOWN);
	DIGITAL_INPUT_EVENT_DEF(MENU_LEFT,		GAMEPAD_DIGI_LEFT,	GAMEPAD_DIGI_LEFT,	GAMEPAD_DIGI_LEFT,	GAMEPAD_DIGI_LEFT,	GAMEPAD_DIGI_LEFT,	AKEY_UNKNOWN,	GAMEPAD_DIGI_LEFT,	IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_DIGI_LEFT);	
	DIGITAL_INPUT_EVENT_DEF(MENU_RIGHT,		GAMEPAD_DIGI_RIGHT,	GAMEPAD_DIGI_RIGHT,	GAMEPAD_DIGI_RIGHT,	GAMEPAD_DIGI_RIGHT,	GAMEPAD_DIGI_RIGHT,	AKEY_UNKNOWN,	GAMEPAD_DIGI_RIGHT,	IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_DIGI_RIGHT);
	DIGITAL_INPUT_EVENT_DEF(MENU_SELECT,	GAMEPAD_SOUTH,		GAMEPAD_SOUTH,		GAMEPAD_SOUTH,		GAMEPAD_SOUTH,		GAMEPAD_SOUTH,		AKEY_UNKNOWN,	GAMEPAD_SOUTH,		IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_SOUTH);
	DIGITAL_INPUT_EVENT_DEF(MENU_ESCAPE,	GAMEPAD_EAST,		GAMEPAD_EAST,		GAMEPAD_EAST,		GAMEPAD_EAST,		GAMEPAD_EAST,		AKEY_UNKNOWN,	GAMEPAD_EAST,		IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_EAST);

	//analog mouse events
	ANALOG_INPUT_EVENT_DEF(CAMERA_MOUSE_LOOK,GAMEPAD_DEFAULT_SENSITIVITY, MOUSE_MOVE, XKEY_UNKNOWN, X1KEY_UNKNOWN,	PS3KEY_UNKNOWN, PS4KEY_UNKNOWN, AKEY_UNKNOWN, MOUSE_MOVE, IKEY_UNKNOWN, MOUSE_MOVE, WIIUKEY_UNKNOWN);

	// touchscreen events for sampleall
	if(mRunning->getParent()->numChildren() > 1)
	{
		TOUCH_INPUT_EVENT_DEF(RUN_NEXT_SAMPLE,		">>",  ASELECTOR_BUTTON2,	ISELECTOR_BUTTON2);
		TOUCH_INPUT_EVENT_DEF(RUN_PREVIOUS_SAMPLE,	"<<",  ASELECTOR_BUTTON1,	ISELECTOR_BUTTON1);
	}

	if(mCurrentCameraController)
	{
		mCurrentCameraController->collectInputEvents(inputEvents);
	}

	if(mConsole)
	{
		mConsole->collectInputEvents(inputEvents);
	}

	if(mSample)
	{
		mSample->collectInputEvents(inputEvents);
	}
}

void PhysXSampleApplication::refreshVisualizationMenuState(PxVisualizationParameter::Enum p)
{
	PxScene& scene = mSample->getActiveScene();
	
	for(PxU32 i=0; i < mMenuVisualizations.size(); i++)
	{
		if(mMenuVisualizations[i].toggleCommand == (PxU32)p)
		{
			mMenuVisualizations[i].toggleState = !!scene.getVisualizationParameter(p);
			break;
		}
	}
}

void PhysXSampleApplication::applyDefaultVisualizationSettings()
{
	PxScene& scene = mSample->getActiveScene();
	PxReal debugRenderScale = mSample->getDebugRenderScale();

	for(PxU32 i=0; i < mMenuVisualizations.size(); i++)
	{
		bool enabled = mMenuVisualizations[i].toggleState;
		
		PxVisualizationParameter::Enum p = static_cast<PxVisualizationParameter::Enum>(mMenuVisualizations[i].toggleCommand);
		
		if (p != PxVisualizationParameter::eSCALE)
			scene.setVisualizationParameter(p, enabled ? 1.0f : 0.0f);
		else
			scene.setVisualizationParameter(p, enabled ? debugRenderScale: 0.0f); 		
	}
}

///////////////////////////////////////////////////////////////////////////////

void PhysXSampleApplication::customizeSample(SampleSetup& setup)
{
	setup.mName			= "PhysXSampleApplication";
	setup.mWidth		= SCREEN_WIDTH;
	setup.mHeight		= SCREEN_HEIGHT;
	setup.mFullscreen	= false;
}

///////////////////////////////////////////////////////////////////////////////

bool PhysXSampleApplication::initLogo()
{
/*	RAWTexture data;
	data.mName = "physx_logo.bmp";
	RenderTexture* logoTexture = createRenderTextureFromRawTexture(data);

	mRockMaterial = SAMPLE_NEW(RenderMaterial)(*getRenderer(), PxVec3(1.0f, 1.0f, 1.0f), 1.0f, false, 0xffffffff, rockTexture);
	mRenderMaterials.push_back(mRockMaterial);
*/
	return true;
}

float PhysXSampleApplication::tweakElapsedTime(float dtime)
{
	if(dtime>1.0f)
		dtime = 1.0f/60.0f;

	if(mOneFrameUpdate)
	{
		mPause = false;
		dtime = 1.0f/60.0f;
	}

/*	if(mOneFrameUpdate)
		mPause = false;

	if(mPause)
		return 0.0f;*/

	return dtime;
}

void PhysXSampleApplication::baseResize(PxU32 width, PxU32 height)
{
//	shdfnd::printFormatted("Resize: %d | %d\n", width, height);

	SampleApplication::onResize(width, height);

	updateCameraViewport(width, height);
}

void PhysXSampleApplication::updateCameraViewport(PxU32 clientWidth, PxU32 clientHeight)
{
//	PxU32 clientWidth, clientHeight;
//	renderer->getWindowSize(clientWidth, clientHeight);

#if defined(RENDERER_WINDOWS)
	PxU32 width, height;
	m_platform->getWindowSize(width, height);
//	const PxReal ratio = PxReal(width) / PxReal(height);
//	const PxReal ratio2 = PxReal(Rect.right - Rect.left) / PxReal(Rect.bottom - Rect.top);
	mCamera.setScreenSize(clientWidth, clientHeight, width, height);
#else
	mCamera.setScreenSize(clientWidth, clientHeight, clientWidth, clientHeight);
#endif
}

void PhysXSampleApplication::setPvdParams(const SampleCommandLine& cmdLine)
{
	if (cmdLine.hasSwitch("nonVizPvd"))
	{
		mPvdParams.useFullPvdConnection = false;
	}

	if (cmdLine.hasSwitch("pvdhost"))
	{
		const char* ipStr = cmdLine.getValue("pvdhost");
		if (ipStr)
			Ps::strlcpy(mPvdParams.ip, 256, ipStr);
	}

	if (cmdLine.hasSwitch("pvdport"))
	{	
		const char* portStr = cmdLine.getValue("pvdport");
		if (portStr)
			mPvdParams.port = atoi(portStr);	
	}

	if (cmdLine.hasSwitch("pvdtimeout"))
	{	
		const char* timeoutStr = cmdLine.getValue("pvdtimeout");
		if (timeoutStr)
			mPvdParams.timeout = atoi(timeoutStr);	
	}
}

void PhysXSampleApplication::onRender()
{
	Renderer* renderer = getRenderer();
	if(renderer)
	{
		mCamera.BuildFrustum();
#if 0
		PxScene* mScene = &mSample->getActiveScene();
		if(mScene)
		{
			const PxVec3* frustumVerts = mCamera.getFrustumVerts();
			PxBounds3 cameraBounds(frustumVerts[0], frustumVerts[0]);
			for(PxU32 i=0;i<8;i++)
				cameraBounds.include(frustumVerts[i]);
			mScene->setVisualizationCullingBox(cameraBounds);
		}
#endif
		renderer->clearBuffers();

		if(mDrawScreenQuad)
		{
			ScreenQuad sq;
			sq.mLeftUpColor		= mScreenQuadTopColor;
			sq.mRightUpColor	= mScreenQuadTopColor;
			sq.mLeftDownColor	= mScreenQuadBottomColor;
			sq.mRightDownColor	= mScreenQuadBottomColor;

			renderer->drawScreenQuad(sq);
		}

		for(PxU32 i=0; i<mLights.size(); i++)
		{
			renderer->queueLightForRender(*mLights[i]);
		}

		{
			PxVec3 camPos( mCamera.getPos() );
			PxVec3 camDir = mCamera.getViewDir();
			PxVec3 camUp = PxVec3( 0, 1, 0 );
			PxVec3 camTarget = camPos + camDir * 50.0f;
			PxPvdSceneClient* pvdClient = mSample->getActiveScene().getScenePvdClient();
			if(pvdClient)
				pvdClient->updateCamera( "SampleCamera", camPos, camUp, camTarget );
		}			
		// main scene render
		{
			mSample->render();
			renderer->render(mCamera.getViewMatrix(), mCamera.getProjMatrix());
		}

		// render debug lines and points with a small depth bias to avoid z-fighting
		{
			mSample->getDebugRenderer()->queueForRenderLine();
			mSample->getDebugRenderer()->queueForRenderPoint();

			// modify entry(3,3) of the projection matrix according to 
			// http://www.terathon.com/gdc07_lengyel.pdf
			// this applies a small constant depth bias in NDC
			SampleRenderer::RendererProjection proj = mCamera.getProjMatrix();
			proj.getPxMat44()(3,3) += 4.8e-3f;
			
			renderer->render(mCamera.getViewMatrix(), proj);
		}
	{
		const PxReal scale = 0.5f;
		const PxReal shadowOffset = 6.0f;

		Renderer* renderer = getRenderer();
		PxU32 x = 10;
		PxU32 y = (PxU32)(-8);
		const PxU32 yInc = 18;

		char strbuf[512] = "";
		if (mMenuExpand)
		{

			const RendererColor textColor(255, 255, 255, 255);
			const RendererColor highlightTextColor(255, 255, 0, 255);
			
			switch(mMenuType)
			{
				case MenuType::TESTS:
				{
					Test::TestGroup* parent = mSelected->getParent();
					parent->getPathName(strbuf, sizeof strbuf - 1, true);
					renderer->print(x, y += yInc, strbuf, scale, shadowOffset, textColor);

					for (Test::TestGroup* child = parent->getFirstChild(); child != NULL; child = parent->getNextChild(*child))
					{
						sprintf(strbuf, "%s%s", child->getName(), child->isTest() ? "" : "/...");
						renderer->print(30, y += yInc, strbuf, scale, shadowOffset, (mSelected == child ? highlightTextColor : textColor));
					}
				}
				break;
				case MenuType::VISUALIZATIONS:
				{
					const RendererColor color(0, 90, 90);
					ScreenQuad sq;
					sq.mLeftUpColor		= color;
					sq.mRightUpColor	= color;
					sq.mLeftDownColor	= color;
					sq.mRightDownColor	= color;
					sq.mAlpha			= 0.75;

					getRenderer()->drawScreenQuad(sq);

					for (PxU32 i = 0; i < mMenuVisualizations.size(); i++)
					{
						bool selected = mMenuVisualizationsIndexSelected == i;
						sprintf(strbuf, "%d  (%s)  %s", i+1, mMenuVisualizations[i].toggleState ? "ON " : "OFF", mMenuVisualizations[i].name);
						renderer->print(30, y += yInc, strbuf, scale, shadowOffset, (selected ? highlightTextColor : textColor));
					}
				}
				break;
				default: {}
			}

			//print minimal information
			{
				y += yInc;

				const RendererColor textColor(255, 255, 255, 255);
				const char* msg;

				msg = inputInfoMsg("Press "," for help", SHOW_HELP, -1);
				if(msg)
					renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);

				y+=yInc;
			}

			if (mTextAlphaHelp != 0.0f)
			{
				const RendererColor textColor(255, 255, 255, PxU32(mTextAlphaHelp*255.0f));
				const char* msg;

				if(m_platform->getSampleUserInput()->keyboardSupported() && m_platform->getSampleUserInput()->gamepadSupported()) 
					renderer->print(x, y += yInc, "Use arrow keys or D-Pad to navigate between the items", scale, shadowOffset, textColor);
				else
				{
					if(m_platform->getSampleUserInput()->keyboardSupported())
					{
						renderer->print(x, y += yInc, "Use arrow keys to navigate between the items", scale, shadowOffset, textColor);
					}
					else
					{
						if(m_platform->getSampleUserInput()->gamepadSupported())
						{
							renderer->print(x, y += yInc, "Use D-Pad to navigate between the items", scale, shadowOffset, textColor);
						}
					}
				}
				msg = inputInfoMsg("Press "," to run the selected sample", MENU_SELECT, -1);
				if(msg)
					renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);
				msg = inputInfoMsg("Press "," to exit sample selector", MENU_ESCAPE, -1);
				if(msg)
					renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);
			}
		}
		else
		{
			if (mShowExtendedHelp)
			{
				mSample->showExtendedInputEventHelp(x,y);
			}
			else
			{
				//print minimal information
				{
					const RendererColor highlightTextColor(255, 255, 0, 255);
					mRunning->getPathName(strbuf, sizeof strbuf - 1, true);

					if (mSample->isConnectedPvd())
						strncat(strbuf, "  <PVD>", 7);
					if (mPause) 
						strncat(strbuf, "  <PAUSED>", 10);

					renderer->print(x, y += yInc, strbuf, scale, shadowOffset, highlightTextColor);
					y += yInc;

					const RendererColor textColor(255, 255, 255, 255);
					const char* msg;

					msg = inputInfoMsg("Press "," for description", SHOW_DESCRIPTION, -1);
					if(msg)
						renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);

					msg = inputInfoMsg("Press "," for help", SHOW_HELP, -1);
					if(msg)
						renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);

					y+=yInc;
				}

				mSample->descriptionRender(x, y+=yInc, PxU8(mTextAlphaDesc*255.0f));
				
				//print help
				if (mTextAlphaHelp != 0.0f)
				{
		
					//print common help
					const RendererColor textColor(255, 255, 255, PxU8(mTextAlphaHelp*255.0f));
					const char* msg;

					msg = inputInfoMsg("Press "," to enter sample selector", MENU_SAMPLES, -1);
					if(msg)
						renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);

					msg = inputInfoMsg("Press "," to quit", QUIT,-1);
					if(msg)
						renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);

					//print sample specific help
					mSample->helpRender(x, y += yInc, PxU8(mTextAlphaHelp*255.0f));
				}
			}
		}

		// PT: "customizeRender" is NOT just for text render, it's a generic render callback that should be called all the time,
		// not just when "mTextAlpha" isn't 0.0
		mSample->customizeRender();
	}

	renderer->drawTouchControls();

	mSample->displayFPS();

	if(isConsoleActive())
		mConsole->render(getRenderer());
	}
}
/*
void PhysXSampleApplication::advanceSimulation(float dtime)
{
	const PxReal timestep = 1.0f/60.0f;
	while(dtime>0.0f)
	{
		const PxReal dt = dtime>=timestep ? timestep : dtime;
		mScene->simulate(dt);
		mScene->fetchResults(true);
		dtime -= dt;
	}
}
*/

///////////////////////////////////////////////////////////////////////////////

void PhysXSampleApplication::onAnalogInputEvent(const SampleFramework::InputEvent& ie, float val)
{
	if(mCurrentCameraController)
	{
		mCurrentCameraController->onAnalogInputEvent(ie,val);
	}

	SampleApplication::onAnalogInputEvent(ie,val);

	if(NULL != mSample)
		mSample->onAnalogInputEvent(ie,val);
}



///////////////////////////////////////////////////////////////////////////////

bool PhysXSampleApplication::isConsoleActive() const
{
	return mConsole && mConsole->isActive();
}

///////////////////////////////////////////////////////////////////////////////

const char* getSampleMediaFilename(const char* filename)
{
	return findPath(filename);
}
///////////////////////////////////////////////////////////////////////////////

void PhysXSampleApplication::baseTickPreRender(float dtime)
{
	if(mCurrentCameraController && !isConsoleActive())
		mCurrentCameraController->update(getCamera(), dtime);
}

void PhysXSampleApplication::baseTickPostRender(float dtime)
{
}

//float tweakElapsedTime(float dtime);
void PhysXSampleApplication::onTickPreRender(float dtime)
{
	if(!mShowHelp)
	{
		mTextAlphaHelp -= dtime;
		if(mTextAlphaHelp<0.0f)
			mTextAlphaHelp = 0.0f;
	}
	else if(0.0f==mTextAlphaDesc)
	{
		mTextAlphaHelp += dtime;
		if(mTextAlphaHelp>1.0f)
			mTextAlphaHelp = 1.0f;
	}

	if(!mShowDescription)
	{
		mTextAlphaDesc -= dtime;
		if(mTextAlphaDesc<0.0f)
			mTextAlphaDesc = 0.0f;
	}
	else if(0.0f==mTextAlphaHelp)
	{
		mTextAlphaDesc += dtime;
		if(mTextAlphaDesc>1.0f)
			mTextAlphaDesc = 1.0f;
	}

	if (mSample) mSample->onTickPreRender(dtime);
}

void PhysXSampleApplication::handleMouseVisualization()
{
    // hide cursor if mHideMouseCursor is set and the window has focus.
    showCursor(!mHideMouseCursor || !hasFocus());
}

void PhysXSampleApplication::onShutdown()
{
	Renderer* renderer = getRenderer();

	if (renderer)
	{
		renderer->finishRendering();
	}

	if (mSample)
	{
		mSample->onShutdown();
		delete mSample;
		mSample = NULL;
	}

	if (renderer)
	{
		renderer->closeScreenquad();
		renderer->closeTexter();
	}

	for(PxU32 i=0; i<mLights.size(); i++)
		mLights[i]->release();
	mLights.clear();
	for(PxU32 i=0;i<MATERIAL_COUNT;i++)
	{
		if(mManagedMaterials[i])
			mManagedMaterials[i]->release();
		mManagedMaterials[i] = NULL;
	}
	
	DELETESINGLE(mDebugRenderer);
}

void PhysXSampleApplication::onInit()
{
	Ps::MutexT<Ps::RawAllocator>::ScopedLock lock(mInputMutex);

	Renderer* renderer = getRenderer();

	getPlatform()->getSampleUserInput()->setRenderer(renderer);
	getPlatform()->getSampleUserInput()->registerInputEventListerner(mInputEventBuffer);

	PxU32 clientWidth, clientHeight;
	renderer->getWindowSize(clientWidth, clientHeight);
	updateCameraViewport(clientWidth, clientHeight);

	RendererDirectionalLightDesc lightdesc;
	lightdesc.intensity = 1;

	lightdesc.color     = RendererColor(250, 250, 250, 255);
	lightdesc.direction = PxVec3(-4.0f, -5.0f, -3.0f);
	lightdesc.direction.normalizeFast();
	
	mLights.push_back(renderer->createLight(lightdesc));

	renderer->initTexter();	
	renderer->initScreenquad();
	
	mDebugRenderer = SAMPLE_NEW(RenderPhysX3Debug)(*renderer, *getAssetManager());
	
	// Create managed materials
	{
		const PxReal c = 0.75f;
		const PxReal opacity = 1.0f;
		const bool doubleSided = false;
		const PxU32 id = 0xffffffff;

		mManagedMaterials[MATERIAL_GREY]	= SAMPLE_NEW(RenderMaterial)(*renderer, PxVec3(0.5f, 0.5f, 0.5f),	opacity, doubleSided, id, NULL);
		mManagedMaterials[MATERIAL_RED]		= SAMPLE_NEW(RenderMaterial)(*renderer, PxVec3(c, 0.0f, 0.0f),		opacity, doubleSided, id, NULL);
		mManagedMaterials[MATERIAL_GREEN]	= SAMPLE_NEW(RenderMaterial)(*renderer, PxVec3(0.0f, c, 0.0f),		opacity, doubleSided, id, NULL);
		mManagedMaterials[MATERIAL_BLUE]	= SAMPLE_NEW(RenderMaterial)(*renderer, PxVec3(0.0f, 0.0f, c),		opacity, doubleSided, id, NULL);
		mManagedMaterials[MATERIAL_YELLOW]	= SAMPLE_NEW(RenderMaterial)(*renderer, PxVec3(c, c, 0.0f),			opacity, doubleSided, id, NULL);
		mManagedMaterials[MATERIAL_FLAT]	= SAMPLE_NEW(RenderMaterial)(*renderer, PxVec3(0.5f, 0.5f, 0.5f),	opacity, doubleSided, id, NULL, true, true);
	}

	getNextSample();
	if (mSample)
	{
		mSample->onInit(false);
		mSample->registerInputEvents();
	}
}

void PhysXSampleApplication::onTickPostRender(float dtime)
{
	if (mSample)
		mSample->onTickPostRender(dtime);
}

void PhysXSampleApplication::showCursor(bool show)
{ 
	if(m_platform) 
		m_platform->showCursor(show); 
}

void PhysXSampleApplication::setMouseCursorHiding(bool hide)
{ 
	if(hide != mHideMouseCursor)
	{
		mHideMouseCursor = hide;
	}						
}

void PhysXSampleApplication::setMouseCursorRecentering(bool recenter)
{
	PX_ASSERT(SamplePlatform::platform());
	SamplePlatform::platform()->setMouseCursorRecentering(recenter);
}

void PhysXSampleApplication::onPointerInputEvent(const InputEvent& ie, PxU32 x, PxU32 y, PxReal dx, PxReal dy, bool val)
{
	SampleApplication::onPointerInputEvent(ie,x,y,dx,dy,val);

	if(mSample)
	{
		mSample->onPointerInputEvent(ie,x,y,dx,dy,val);
	}

	if(mCurrentCameraController)
	{
		mCurrentCameraController->onPointerInputEvent(ie,x,y,dx,dy,val);
	}
}

void PhysXSampleApplication::onKeyDownEx(SampleFramework::SampleUserInput::KeyCode keyCode, PxU32 wParam)
{ 
	if(mSample) 
		mSample->onKeyDownEx(keyCode, wParam);

	if(mConsole)
		mConsole->onKeyDown(keyCode, wParam);

}

void PhysXSampleApplication::onResize(PxU32 width, PxU32 height)										
{ 
	if(mSample)
		mSample->onResize(width, height); 
}

///////////////////////////////////////////////////////////////////////////////
void PhysXSampleApplication::onDigitalInputEvent(const SampleFramework::InputEvent& ie, bool val)
{ 
	if(mCurrentCameraController)
	{
		mCurrentCameraController->onDigitalInputEvent(ie,val);
	}

	SampleApplication::onDigitalInputEvent(ie,val);

	if (mConsole)
	{
		mConsole->onDigitalInputEvent(ie,val);
		if(mConsole->isActive())
			return;			
	}

	MenuKey::Enum menuKey = MenuKey::NONE;

	switch (ie.m_Id)
	{
	case RUN_NEXT_SAMPLE:
		{
			if(val)
			{
				handleMenuKey(MenuKey::NAVI_DOWN);
				handleMenuKey(MenuKey::SELECT);
			}
		}
		break;
	case RUN_PREVIOUS_SAMPLE:
		{
			if(val)
			{
				handleMenuKey(MenuKey::NAVI_UP);
				if(!handleMenuKey(MenuKey::SELECT))
					return;
			}
		}
		break;
	default:
			break;
	}

	if(val)
	{		
		switch (ie.m_Id)
		{
		case MENU_UP:			if(mMenuExpand)		menuKey = MenuKey::NAVI_UP;		break;
		case MENU_DOWN:			if(mMenuExpand)		menuKey = MenuKey::NAVI_DOWN;	break;
		case MENU_LEFT:			if(mMenuExpand)		menuKey = MenuKey::NAVI_LEFT;	break;
		case MENU_RIGHT:		if(mMenuExpand)		menuKey = MenuKey::NAVI_RIGHT;	break;
		case MENU_SELECT:		if(mMenuExpand)		menuKey = MenuKey::SELECT;		break;		
		case MENU_ESCAPE:		if(mMenuExpand)		menuKey = MenuKey::ESCAPE;		break;		
		case MENU_QUICK_UP:		mMenuExpand = true; menuKey = MenuKey::NAVI_UP;		break;
		case MENU_QUICK_DOWN:	mMenuExpand = true; menuKey = MenuKey::NAVI_DOWN;	break;
		case MENU_QUICK_LEFT:	mMenuExpand = true; menuKey = MenuKey::NAVI_LEFT;	break;
		case MENU_QUICK_RIGHT:	mMenuExpand = true; menuKey = MenuKey::NAVI_RIGHT;	break;
		}

		if (mMenuExpand)
		{
			if (ie.m_Id == SHOW_HELP)
			{
				mShowHelp = !mShowHelp;
				return;
			}
			if(mMenuType == MenuType::TESTS)
			{
				if(!handleMenuKey(menuKey))
					return;
			}
			else
			{
				handleSettingMenuKey(menuKey);
			}
			return;
		}

		switch (ie.m_Id)
		{
		case SHOW_EXTENDED_HELP:
			{	
				mShowExtendedHelp = !mShowExtendedHelp;
				if(mSample)
					mSample->resetExtendedHelpText();
				break;
			}
		case MENU_VISUALIZATIONS:
			{
				mMenuExpand = true;	
				mMenuType = MenuType::VISUALIZATIONS;
				break;
			}		
		case MENU_SAMPLES:
			{
				mMenuExpand = true;	
				mMenuType = MenuType::TESTS;
				break;
			}
		case QUIT:
			{
				requestToClose();
				break;
			}
		default:
			if(NULL != mSample)
			{
				mSample->onDigitalInputEvent(ie,val);
			}
			break;
		}
	}
	else
	{
		if (mMenuExpand)
		{
			if (MENU_ESCAPE == ie.m_Id)
			{
				mMenuExpand = false;
				mMenuType = MenuType::TESTS;
			}
		}
		if(mSample)
		{
			mSample->onDigitalInputEvent(ie,val);
		}
	}
}

void PhysXSampleApplication::toggleDebugRenderer()
{
	PxScene& scene = mSample->getActiveScene();
	scene.setVisualizationParameter(PxVisualizationParameter::eSCALE, mSample->getDebugRenderScale());
	mMenuVisualizations[0].toggleState = true;
}

void PhysXSampleApplication::handleSettingMenuKey(MenuKey::Enum menuKey)
{
	size_t numEntries = mMenuVisualizations.size();
	switch(menuKey)
	{
	case MenuKey::NAVI_LEFT:
	case MenuKey::NAVI_UP:
		mMenuVisualizationsIndexSelected = (mMenuVisualizationsIndexSelected > 0) ? (mMenuVisualizationsIndexSelected - 1) : numEntries - 1;
		break;
	case MenuKey::NAVI_RIGHT:
	case MenuKey::NAVI_DOWN:
		mMenuVisualizationsIndexSelected = (mMenuVisualizationsIndexSelected + 1) % numEntries;
		break;
	case MenuKey::SELECT:
		{
			MenuTogglableItem& togglableItem = mMenuVisualizations[mMenuVisualizationsIndexSelected];
			PxU32 menuVisIndex = togglableItem.toggleCommand;
			PX_ASSERT(menuVisIndex < PxVisualizationParameter::eNUM_VALUES);
			PxScene& scene = mSample->getActiveScene();
			PxVisualizationParameter::Enum p = static_cast<PxVisualizationParameter::Enum>(menuVisIndex);
			scene.setVisualizationParameter(p, (scene.getVisualizationParameter(p) != 0.0f) ? 0.0f : 1.0f);
			
			bool enabled = scene.getVisualizationParameter(p) != 0.0f;
			if(enabled && scene.getVisualizationParameter(PxVisualizationParameter::eSCALE) == 0.0f)
			{
				toggleDebugRenderer();
			}					
			togglableItem.toggleState = scene.getVisualizationParameter(p) != 0.0f;
		}
		break;
	default:
		break;			
	}
}

bool PhysXSampleApplication::handleMenuKey(MenuKey::Enum menuKey)
{
	if (!mSelected)
		return false;

	Test::TestGroup* parent = mSelected->getParent(), *child = NULL;
	PX_ASSERT(parent);
	
	switch (menuKey)
	{
	case MenuKey::NAVI_LEFT:
		if (NULL != parent->getParent())
			mSelected = parent;
		break;
	case MenuKey::NAVI_RIGHT:
		if (NULL != (child = mSelected->getFirstChild()))
			mSelected = child;
		break;
	case MenuKey::NAVI_UP:
		if (NULL != (child = parent->getPreviousChild(*mSelected)))
			mSelected = child;
		else if (NULL != (child = parent->getLastChild()))
			mSelected = child;
		break;
	case MenuKey::NAVI_DOWN:
		if (NULL != (child = parent->getNextChild(*mSelected)))
			mSelected = child;
		else if (NULL != (child = parent->getFirstChild()))
			mSelected = child;
		break;
	case MenuKey::SELECT:
		if (mSelected->isTest())
		{
			mMenuExpand = false;
			mSwitchSample = true;
			mInputEventBuffer->clear();
			return false;
		}
		else
		{
			if (NULL != (child = mSelected->getFirstChild()))
				mSelected = child;
		}
		break;
	default:
		//mSelected = mRunning;
		break;
	}

	return true;
}

void PhysXSampleApplication::switchSample()
{
	Ps::MutexT<Ps::RawAllocator>::ScopedLock lock(mInputMutex);
	if(mInputEventBuffer)
		mInputEventBuffer->clear();

	bool isRestart = mRunning == mSelected;

	Renderer* renderer = getRenderer();
	if (renderer)
	{
		renderer->finishRendering();
	}

	if (mSample)
	{
		mSample->onShutdown();
		delete mSample;
		mSample = NULL;
	}

	if(!isRestart)
	{
		setDefaultCameraController();
		resetDefaultCameraController();
	}
	if (getNextSample())
	{	
		mSample->onInit(isRestart);
		mSample->registerInputEvents();
		if (mCurrentCameraController)
			mCurrentCameraController->update(getCamera(), 0.0f);
		mSample->onTickPreRender(0.0f);
		mSample->onSubstep(0.0f);
		mSample->initRenderObjects();
	}

	mSwitchSample = false;
}

//=============================================================================
// PhysXSampleManager
//-----------------------------------------------------------------------------
Test::TestGroup* PhysXSampleApplication::mSampleTreeRoot = NULL;

Test::TestGroup& PhysXSampleApplication::getSampleTreeRoot()
{
	if (NULL == mSampleTreeRoot)
	{
		mSampleTreeRoot = new Test::TestGroup("");
	}
	return *mSampleTreeRoot;
}

bool PhysXSampleApplication::addSample(Test::TestGroup &root, SampleCreator creator, const char *fullPath)
{
	PX_ASSERT(fullPath);

	do
	{
		if ('\0' == fullPath[0] || '/' == fullPath[0])
		{
			shdfnd::printFormatted("invalid name: %s\n", fullPath);
			break;
		}

		const char* p = fullPath;
		while ('\0' != *p && '/' != *p)
			++p;

		if ('\0' == *p) // test name
		{
			if (root.getChildByName(fullPath))	// duplicated name
			{
				shdfnd::printFormatted("test \"%s\" exists.\n", fullPath);
				break;
			}
			root.addTest(creator, fullPath);
		}
		else // group name
		{
			Test::TestGroup* group = root.getChildByName(fullPath, p - fullPath);
			if (group)
				return addSample(*group, creator, p + 1);

			group = new Test::TestGroup(fullPath, p - fullPath);
			if (!addSample(*group, creator, p + 1))
			{
				delete group;
				break;
			}
			root.addGroup(group);
		}

		return true;
	} while (false);
	return false;
}

bool PhysXSampleApplication::getNextSample()
{
	Test::TestGroup& root = PhysXSampleApplication::getSampleTreeRoot();

	if (NULL == mSelected)
		mSelected = root.getFirstTest();

	if (NULL == mSelected)
		return false;

	mRunning = mSelected;
	mSample = (*mRunning->getCreator())(*this);
	return true;
}

void PhysXSampleApplication::saveCameraState()
{
	mSavedView = getCamera().getViewMatrix();
}

void PhysXSampleApplication::restoreCameraState()
{
	getCamera().setView(mSavedView);
}

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

#ifndef PHYSX_SAMPLE_APPLICATION_H
#define PHYSX_SAMPLE_APPLICATION_H

#include "SamplePreprocessor.h"
#include "SampleApplication.h"
#include "SampleCamera.h"
#include "SampleCameraController.h"
#include "SampleAllocator.h"
#include "PxFiltering.h"
#include "RawLoader.h"
#include "RendererMeshContext.h"
#include "PxTkFPS.h"
#include "PxVisualizationParameter.h"
#include "PxTkRandom.h"

#include <PsThread.h>
#include <PsSync.h>
#include "PsHashMap.h"
#include "PsUtilities.h"
#include "SampleArray.h"

#if PX_WINDOWS
#include "task/PxTask.h"
#endif

#define	PHYSX_VERSION_STRING	PX_STRINGIZE(PX_PHYSICS_VERSION_MAJOR) "." PX_STRINGIZE(PX_PHYSICS_VERSION_MINOR)

#define	SAMPLE_MEDIA_DIR		"/PhysX/" PHYSX_VERSION_STRING "/Samples"
#define	SAMPLE_OUTPUT_DIR		"media" SAMPLE_MEDIA_DIR "/user"

namespace physx
{
	class PxDefaultCpuDispatcher;
	class PxPhysics;
	class PxCooking;
	class PxScene;
	class PxGeometry;
	class PxMaterial;
	class PxRigidActor;
};

	class RenderPhysX3Debug;
	class RenderBaseActor;
	class RenderMaterial;
	class RenderMeshActor;
	class RenderTexture;
	class Stepper;
	class Console;

	class PhysXSampleApplication;
	class PhysXSample;
	class InputEventBuffer;

	namespace Test
	{
		class TestGroup;
	}

	class PhysXSampleCreator
	{
	public:
		virtual ~PhysXSampleCreator() {}
		virtual PhysXSample* operator()(PhysXSampleApplication& app) const = 0;
	};

	typedef PhysXSampleCreator *SampleCreator;
	typedef PhysXSample* (*FunctionCreator)(PhysXSampleApplication& app);
//	typedef PhysXSample* (*SampleCreator)(PhysXSampleApplication& app);

	struct SampleSetup
	{
		SampleSetup() : 
			mName		(NULL),
			mWidth		(0),
			mHeight		(0),
			mFullscreen	(false)
		{
		}
		const char*	mName;
		PxU32		mWidth;
		PxU32		mHeight;
		bool		mFullscreen;
	};
		// Default materials created by PhysXSampleApplication
		enum MaterialIndex
		{
			MATERIAL_GREY,
			MATERIAL_RED,
			MATERIAL_GREEN,
			MATERIAL_BLUE,
			MATERIAL_YELLOW,
			MATERIAL_FLAT,
			MATERIAL_COUNT,
		};

template <typename Container>
void releaseAll(Container& container)
{
	for (PxU32 i = 0; i < container.size(); ++i)
		container[i]->release();
	container.clear();
}


class PhysXSampleApplication :	public SampleFramework::SampleApplication, public SampleAllocateable, public Ps::ThreadT<Ps::RawAllocator>
	{
	public:
		using SampleAllocateable::operator new;
		using SampleAllocateable::operator delete;
	private:
		friend class PhysXSample;
		struct PvdParameters
		{
			char							ip[256];
			PxU32							port;
			PxU32							timeout;
			bool							useFullPvdConnection;

			PvdParameters()
			: port(5425)
			, timeout(10)
			, useFullPvdConnection(true)
			{
				Ps::strlcpy(ip, 256, "127.0.0.1");
			}
		};
		struct MenuKey
		{
			enum Enum
			{
				NONE,
				ESCAPE,
				SELECT,
				NAVI_UP,
				NAVI_DOWN,
				NAVI_LEFT,
				NAVI_RIGHT
			};
		};

		// menu events
		struct MenuType
		{
			enum Enum
			{
				NONE,
				HELP,
				SETTINGS,
				VISUALIZATIONS,
				TESTS
			};
		};
		
		struct MenuTogglableItem
		{
			MenuTogglableItem(PxU32 c, const char* n) : toggleCommand(c), toggleState(false), name(n)  {}
			PxU32 toggleCommand; 
			bool toggleState;
			const char* name;
		};

		public:
														PhysXSampleApplication(const SampleFramework::SampleCommandLine& cmdline);
		virtual											~PhysXSampleApplication();

		///////////////////////////////////////////////////////////////////////////////
		// PsThread interface
		virtual			void							execute();

		///////////////////////////////////////////////////////////////////////////////

		// Implements SampleApplication/RendererWindow
		virtual			void							onInit();
		virtual			void							onShutdown();

		virtual			float							tweakElapsedTime(float dtime);
		virtual			void							onTickPreRender(float dtime);
		virtual			void							onRender();
		virtual			void							onTickPostRender(float dtime);
		
						void							onKeyDownEx(SampleFramework::SampleUserInput::KeyCode keyCode, PxU32 wParam);
						void							onAnalogInputEvent(const SampleFramework::InputEvent& , float val);
						void							onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);        
						void							onPointerInputEvent(const SampleFramework::InputEvent&, PxU32 x, PxU32 y, PxReal dx, PxReal dy, bool val);

		virtual			void							onResize(PxU32 width, PxU32 height);

						void							baseTickPreRender(float dtime);
						void							baseTickPostRender(float dtime);

						void							baseResize(PxU32 width, PxU32 height);

		///////////////////////////////////////////////////////////////////////////////

						void							customizeSample(SampleSetup&);
//						void							onSubstep(float dtime);

		///////////////////////////////////////////////////////////////////////////////
						void							applyDefaultVisualizationSettings();
						void							saveCameraState();
						void							restoreCameraState();

		// Camera functions
		PX_FORCE_INLINE	void							setDefaultCameraController()				{ mCurrentCameraController = &mCameraController; mCameraController = DefaultCameraController();}
		PX_FORCE_INLINE	void							resetDefaultCameraController()				{ mCameraController = DefaultCameraController(); }
		PX_FORCE_INLINE	void							setCameraController(CameraController* c)	{ mCurrentCameraController = c;						}

		PX_FORCE_INLINE	PxReal							getTextAlpha1()						const	{ return mTextAlphaHelp;								}
		PX_FORCE_INLINE	PxReal							getTextAlpha2()						const	{ return mTextAlphaDesc;								}
		PX_FORCE_INLINE	bool							isPaused()							const	{ return mPause;									}
		PX_FORCE_INLINE	Camera&							getCamera()									{ return mCamera;									}
		PX_FORCE_INLINE	RenderPhysX3Debug*				getDebugRenderer()					const	{ return mDebugRenderer;							}
		PX_FORCE_INLINE	Ps::MutexT<Ps::RawAllocator>&	getInputMutex()								{ return mInputMutex;								}
                        bool							isConsoleActive()					const;
						void							showCursor(bool show);
						void							setMouseCursorHiding(bool hide);
						void							setMouseCursorRecentering(bool recenter);
						void							handleMouseVisualization();
						void							updateEngine();
						void							setPvdParams(const SampleFramework::SampleCommandLine& cmdLine);
		PX_FORCE_INLINE	void							registerLight(SampleRenderer::RendererLight* light)	{ mLights.push_back(light);					}
						void							collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);
						const char*						inputInfoMsg(const char* firstPart,const char* secondPart, PxI32 inputEventId1, PxI32 inputEventId2);
						const char*						inputInfoMsg_Aor_BandC(const char* firstPart,const char* secondPart, PxI32 inputEventIdA, PxI32 inputEventIdB, PxI32 inputEventIdC);
						const char*						inputMoveInfoMsg(const char* firstPart,const char* secondPart, PxI32 inputEventId1, PxI32 inputEventId2,PxI32 inputEventId3,PxI32 inputEventId4);
						void							requestToClose() { mIsCloseRequested = true; }
						bool							isCloseRequested() { return mIsCloseRequested; }
						
		private:
						PxReal							mTextAlphaHelp;
						PxReal							mTextAlphaDesc;
						MenuType::Enum					mMenuType;
						std::vector<MenuTogglableItem>	mMenuVisualizations;
						size_t							mMenuVisualizationsIndexSelected;
						void							setMenuVisualizations(MenuTogglableItem& togglableItem);
						char							m_Msg[512];

						PxTransform						mSavedView;
						bool							mIsCloseRequested;
										
		protected:
						Console*						mConsole;

						Camera							mCamera;
						DefaultCameraController			mCameraController;
						CameraController*				mCurrentCameraController;

						std::vector<SampleRenderer::RendererLight*>		mLights;
						RenderMaterial*									mManagedMaterials[MATERIAL_COUNT];

						RenderPhysX3Debug*				mDebugRenderer;


						bool							mPause;
						bool							mOneFrameUpdate;
						bool							mSwitchSample;

						bool							mShowHelp;
						bool							mShowDescription;
						bool							mShowExtendedHelp;
						volatile bool					mHideMouseCursor;
						InputEventBuffer*				mInputEventBuffer;
						Ps::MutexT<RawAllocator>		mInputMutex;
        
						bool							mDrawScreenQuad;
						SampleRenderer::RendererColor	mScreenQuadTopColor;
						SampleRenderer::RendererColor	mScreenQuadBottomColor;						
						
						PvdParameters					mPvdParams;
						void							updateCameraViewport(PxU32 w, PxU32 h);
						bool							initLogo();

	private:
		bool handleMenuKey(MenuKey::Enum menuKey);
		void handleSettingMenuKey(MenuKey::Enum menuKey);
		void refreshVisualizationMenuState(PxVisualizationParameter::Enum p);
		void toggleDebugRenderer();
		
//-----------------------------------------------------------------------------
// PhysXSampleManager
//-----------------------------------------------------------------------------
	public:
		static bool registerSample(SampleCreator creator, const char* fullPath)
		{
			return addSample(getSampleTreeRoot(), creator, fullPath);
		}
		bool getNextSample();
		void switchSample();
	protected:
		static Test::TestGroup*	mSampleTreeRoot;
		static Test::TestGroup& getSampleTreeRoot();
		static bool addSample(Test::TestGroup& root, SampleCreator creator, const char* fullPath);
	public:
		bool											mMenuExpand;
		Test::TestGroup*								mRunning;
		Test::TestGroup*								mSelected;
		PhysXSample*									mSample;
		const char*										mDefaultSamplePath;
	};

	const char*				getSampleMediaFilename(const char* filename);
	PxToolkit::BasicRandom& getSampleRandom();
	PxErrorCallback&		getSampleErrorCallback();

//=============================================================================
// macro REGISTER_SAMPLE
//-----------------------------------------------------------------------------
	class SampleFunctionCreator : public PhysXSampleCreator
	{
	public:
		SampleFunctionCreator(FunctionCreator func) : mFunc(func) {}
		virtual PhysXSample* operator()(PhysXSampleApplication& app) const { return (*mFunc)(app); }
	private:
		FunctionCreator	mFunc;
	};

#define SAMPLE_CREATOR(className)	create##className
#define SAMPLE_STARTER(className)	className##Starter

#define SAMPLE_CREATOR_VAR(className)	g##className##creator

#define REGISTER_SAMPLE(className, fullPath)													\
	static PhysXSample* SAMPLE_CREATOR(className)(PhysXSampleApplication& app) {				\
		return SAMPLE_NEW(className)(app);														\
	}																							\
	static SampleFunctionCreator SAMPLE_CREATOR_VAR(className)(SAMPLE_CREATOR(className));		\
	struct SAMPLE_STARTER(className) {															\
		SAMPLE_STARTER(className)() {															\
			PhysXSampleApplication::registerSample(&SAMPLE_CREATOR_VAR(className), fullPath);	\
		}																						\
	} g##className##Starter;


///////////////////////////////////////////////////////////////////////////////
#endif

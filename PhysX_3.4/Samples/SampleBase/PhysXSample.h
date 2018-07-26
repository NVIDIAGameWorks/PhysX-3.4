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

#ifndef PHYSX_SAMPLE_H
#define PHYSX_SAMPLE_H

#include "PhysXSampleApplication.h"
#include "SampleStepper.h"
#include "SampleDirManager.h"
#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include "RenderParticleSystemActor.h"
#include "RenderClothActor.h"

namespace SampleFramework
{
	class SampleInputAsset;
}

namespace physx
{
	class Picking;	
}

struct PhysXShape
{
	PxRigidActor*	mActor;
	PxShape*		mShape;

	PhysXShape(PxRigidActor* actor, PxShape* shape) : mActor(actor), mShape(shape) {}
	PhysXShape(const PhysXShape& shape) : mActor(shape.mActor), mShape(shape.mShape) {}
	bool operator<(const PhysXShape& shape) const { return mActor == shape.mActor ? mShape < shape.mShape : mActor < shape.mActor; }
};

enum StepperType
{
	DEFAULT_STEPPER,
	FIXED_STEPPER,
	VARIABLE_STEPPER
};

class PhysXSample	: public RAWImportCallback
					, public SampleAllocateable
					, public PxDeletionListener
{
	typedef std::map<PhysXShape, RenderBaseActor*> PhysXShapeToRenderActorMap;

public:
															PhysXSample(PhysXSampleApplication& app, PxU32 maxSubSteps=8);
	virtual													~PhysXSample();

public:
					void									render();
					void									displayFPS();

					SampleFramework::SampleAsset*			getAsset(const char* relativePath, SampleFramework::SampleAsset::Type type, bool abortOnError = true);
					void									importRAWFile(const char* relativePath, PxReal scale, bool recook=false);
					void									removeActor(PxRigidActor* actor);
					void									removeRenderObject(RenderBaseActor *renderAcotr);
#if PX_USE_CLOTH_API
					void									createRenderObjectsFromCloth(const PxCloth& cloth, const PxClothMeshDesc &meshDesc, RenderMaterial* material = NULL, const PxVec2* uvs = NULL, bool enableDebugRender = true, PxReal scale = 1.0f);
					void									removeRenderClothActor(RenderClothActor& renderActor);
#endif
					void									createRenderObjectsFromActor(PxRigidActor* actor, RenderMaterial* material=NULL);
					RenderBaseActor*						createRenderBoxFromShape(PxRigidActor* actor, PxShape* shape, RenderMaterial* material=NULL, const PxReal* uvs=NULL);
					RenderBaseActor*						createRenderObjectFromShape(PxRigidActor* actor, PxShape* shape, RenderMaterial* material=NULL);
					RenderMeshActor*						createRenderMeshFromRawMesh(const RAWMesh& data, PxShape* shape = NULL);
					RenderTexture*							createRenderTextureFromRawTexture(const RAWTexture& data);
					RenderMaterial*							createRenderMaterialFromTextureFile(const char* filename);
					PxCloth*								createClothFromMeshDesc(const PxClothMeshDesc &meshDesc, const PxTransform &pose, const PxVec3& gravityDir = PxVec3(0,0,-1), 
																const PxVec2* uv = 0, const char *textureFile = 0, PxReal scale = 1.0f);
					PxCloth*								createClothFromObj(const char *objFileName, const PxTransform& pose, const char* textureFileName);
					PxCloth*                                createGridCloth(PxReal sizeX, PxReal sizeY, PxU32 numX, PxU32 numY,	const PxTransform &pose, const char* filename = NULL);
					PxRigidActor*							createGrid(RenderMaterial* material=NULL);
					PxRigidDynamic*							createBox(const PxVec3& pos, const PxVec3& dims, const PxVec3* linVel=NULL, RenderMaterial* material=NULL, PxReal density=1.0f);
					PxRigidDynamic*							createSphere(const PxVec3& pos, PxReal radius, const PxVec3* linVel=NULL, RenderMaterial* material=NULL, PxReal density=1.0f);
					PxRigidDynamic*							createCapsule(const PxVec3& pos, PxReal radius, PxReal halfHeight, const PxVec3* linVel=NULL, RenderMaterial* material=NULL, PxReal density=1.0f);
					PxRigidDynamic*							createConvex(const PxVec3& pos, const PxVec3* linVel=NULL, RenderMaterial* material=NULL, PxReal density=1.0f);
					PxRigidDynamic*							createCompound(const PxVec3& pos, const std::vector<PxTransform>& localPoses, const std::vector<const PxGeometry*>& geometries, const PxVec3* linVel=NULL, RenderMaterial* material=NULL, PxReal density=1.0f);
					PxRigidDynamic*							createTestCompound(const PxVec3& pos, PxU32 nbBoxes, float boxSize, float amplitude, const PxVec3* linVel, RenderMaterial* material, PxReal density, bool makeSureVolumeEmpty = false);
#if PX_USE_PARTICLE_SYSTEM_API
					RenderParticleSystemActor*				createRenderObjectFromParticleSystem(ParticleSystem& ps, RenderMaterial* material = NULL, 
																bool useMeshInstancing = false, bool useFading = false, PxReal fadingPeriod = 1.0f, PxReal instancingScale = 1.0f);
#endif
					void									createRenderObjectsFromScene();
					
					void									addRenderParticleSystemActor(RenderParticleSystemActor& renderActor);
					void									removeRenderParticleSystemActor(RenderParticleSystemActor& renderActor);

					void									setSubStepper(const PxReal stepSize, const PxU32 maxSteps) { getStepper()->setSubStepper(stepSize, maxSteps); }
					void									togglePause();
					void									toggleFlyCamera();
					void									initRenderObjects();

					// project from world coords to screen coords (can be used for text rendering)
					void									project(const PxVec3& v, int& x, int& y, float& depth);

public:
	virtual			void									onInit();
	virtual			void									onInit(bool restart) { onInit(); }
	virtual			void									onShutdown();

	// called after simulate() has completed
	virtual			void									onSubstep(float dtime) {}

	// called after simulate() has completed, but before fetchResult() is called
	virtual			void									onSubstepPreFetchResult() {}

	// called before simulate() is called
	virtual			void									onSubstepSetup(float dtime, PxBaseTask* cont) {}
	// called after simulate() has started
	virtual			void									onSubstepStart(float dtime) {}

	virtual			void									onTickPreRender(float dtime);
	virtual			void									customizeRender() {}
	virtual			void									helpRender(PxU32 x, PxU32 y, PxU8 textAlpha) {}
	virtual			void									descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha) {}
	virtual			void									onTickPostRender(float dtime);

	virtual			void									onAnalogInputEvent(const SampleFramework::InputEvent& , float val);
	virtual			void									onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);
	virtual			void									onPointerInputEvent(const SampleFramework::InputEvent& ie, PxU32 x, PxU32 y, PxReal dx, PxReal dy, bool val);

	virtual			void									onKeyDownEx(SampleFramework::SampleUserInput::KeyCode keyCode, PxU32 param);

	virtual			void									onResize(PxU32 width, PxU32 height);

	virtual			void									newMaterial(const RAWMaterial&);
	virtual			void									newMesh(const RAWMesh&);
	virtual			void									newShape(const RAWShape&);
	virtual			void									newHelper(const RAWHelper&);
	virtual			void									newTexture(const RAWTexture&);
					void									unregisterInputEvents();
					void									registerInputEvents(bool ignoreSaved = false);					
	virtual			void									collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);
	virtual			SampleFramework::SampleDirManager&		getSampleOutputDirManager();

	// delete listener
	virtual			void									onRelease(const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent);


protected:
	// Let samples override this
	virtual			void									getDefaultSceneDesc(PxSceneDesc&)		{}
	virtual			void									customizeSceneDesc(PxSceneDesc&)		{}
	virtual			void									customizeTolerances(PxTolerancesScale&)	{}
	virtual			void									renderScene() {}
	// this lets samples customize the debug objects
	enum DebugObjectType
	{
		DEBUG_OBJECT_BOX		=	(1 << 0),
		DEBUG_OBJECT_SPHERE		=	(1 << 1),
		DEBUG_OBJECT_CAPSULE	=	(1 << 2),
		DEBUG_OBJECT_CONVEX		=	(1 << 3),
		DEBUG_OBJECT_COMPOUND	=	(1 << 4),
		DEBUG_OBJECT_ALL		=	(DEBUG_OBJECT_BOX | DEBUG_OBJECT_SPHERE | DEBUG_OBJECT_CAPSULE | DEBUG_OBJECT_CONVEX | DEBUG_OBJECT_COMPOUND)
	};
	virtual			PxU32									getDebugObjectTypes()				const	{ return DEBUG_OBJECT_ALL;			}
	virtual			PxReal									getDebugObjectsVelocity()			const	{ return 20.0f;						}
	virtual			PxVec3									getDebugBoxObjectExtents()			const	{ return PxVec3(0.3f, 0.3f, 1.0f);	}
	virtual			PxReal									getDebugSphereObjectRadius()		const	{ return 0.3f;						}
	virtual			PxReal									getDebugCapsuleObjectRadius()		const	{ return 0.3f;						}
	virtual			PxReal									getDebugCapsuleObjectHalfHeight()	const	{ return 1.0f;						}
	virtual			PxReal									getDebugConvexObjectScale()			const	{ return 0.3f;						}
	virtual			void									onDebugObjectCreation(PxRigidDynamic* actor){									}
					Stepper*								getStepper();

	void													prepareInputEventUserInputInfo(const char* sampleName,PxU32 &userInputCS, PxU32 &inputEventCS);

private:
	///////////////////////////////////////////////////////////////////////////////

	PhysXSample&	operator= (const PhysXSample&);		

public:	// Helpers from PhysXSampleApplication
	PX_FORCE_INLINE	void									fatalError(const char* msg)					{ mApplication.fatalError(msg); }
	PX_FORCE_INLINE	void									setCameraController(CameraController* c)	{ mApplication.setCameraController(c); }

	PX_FORCE_INLINE	void									toggleVisualizationParam(PxScene& scene, PxVisualizationParameter::Enum param)
															{
																PxSceneWriteLock scopedLock(scene);
                                                                const bool visualization = scene.getVisualizationParameter(param) == 1.0f;
                                                                scene.setVisualizationParameter(param, visualization ? 0.0f : 1.0f);
																mApplication.refreshVisualizationMenuState(param);						
															}

public:	// getter & setter
	PX_FORCE_INLINE	void									setDefaultMaterial(PxMaterial* material)	{ mMaterial = material; }
	PX_FORCE_INLINE	void									setFilename(const char* name)				{ mFilename = name; }

	PX_FORCE_INLINE	PhysXSampleApplication&					getApplication()					const	{ return mApplication; }
	PX_FORCE_INLINE	PxPhysics&								getPhysics()						const	{ return *mPhysics; }
	PX_FORCE_INLINE	PxCooking&								getCooking()						const	{ return *mCooking; }
	PX_FORCE_INLINE	PxScene&								getActiveScene()					const	{ return *mScene; }
	PX_FORCE_INLINE	PxMaterial&								getDefaultMaterial()				const	{ return *mMaterial; }
					RenderMaterial*							getMaterial(PxU32 materialID);

	PX_FORCE_INLINE	Camera&									getCamera()							const	{ return mApplication.getCamera(); }
	PX_FORCE_INLINE	SampleRenderer::Renderer*				getRenderer()						const	{ return mApplication.getRenderer(); }
	PX_FORCE_INLINE	RenderPhysX3Debug*						getDebugRenderer()					const	{ return mApplication.getDebugRenderer(); }
	PX_FORCE_INLINE	Console*								getConsole()						const	{ return mApplication.mConsole; }
	PX_FORCE_INLINE	CameraController*						getCurrentCameraController()		const	{ return mApplication.mCurrentCameraController; }
	PX_FORCE_INLINE	DefaultCameraController&				getDefaultCameraController()		const   { return mCameraController;						}
	PX_FORCE_INLINE const PxMat44&							getEyeTransform(void)				const	{ return mApplication.m_worldToView.getInverseTransform();}
	PX_FORCE_INLINE	PxReal									getSimulationTime()					const	{ return mSimulationTime; }
	PX_FORCE_INLINE	PxReal									getDebugRenderScale()				const	{ return mDebugRenderScale; }

	PX_FORCE_INLINE	bool									isPaused()							const	{ return mApplication.isPaused(); }
	PX_FORCE_INLINE	bool									isConnectedPvd()					const	{ return mPvd ? mPvd->isConnected() : false; }
#if PX_SUPPORT_GPU_PHYSX
	PX_FORCE_INLINE	bool									isGpuSupported()					const	{ return mCudaContextManager && mCudaContextManager->contextIsValid(); }
#else
	PX_FORCE_INLINE	bool									isGpuSupported()					const	{ return false; }
#endif
	PX_FORCE_INLINE void									setMenuExpandState(bool menuExpand)		 { mApplication.mMenuExpand = menuExpand; }
	PX_FORCE_INLINE void									setEyeTransform(const PxVec3& pos, const PxVec3& rot) {mApplication.setEyeTransform(pos, rot); }
	PX_FORCE_INLINE void									resetExtendedHelpText()					{ mExtendedHelpPage = 0; }
	PX_FORCE_INLINE void									addPhysicsActors(PxRigidActor* actor)	{ mPhysicsActors.push_back(actor); }
					void									unlink(RenderBaseActor* renderActor, PxShape* shape, PxRigidActor* actor);
					void									link(RenderBaseActor* renderActor, PxShape* shape, PxRigidActor* actor);
					RenderBaseActor*						getRenderActor(PxRigidActor* actor, PxShape* shape);
					const char*								getSampleOutputFilePath(const char* inFilePath, const char* outExtension);
					void									showExtendedInputEventHelp(PxU32 x, PxU32 y);

					void									freeDeletedActors();
	PX_FORCE_INLINE	StepperType								getStepperType()					const	{ return mStepperType; }

protected:
					void									updateRenderObjectsFromRigidActor(PxRigidActor& actor, RenderMaterial* mat = NULL);
					void									updateRenderObjectsFromArticulation(PxArticulation& articulation);					

protected:
					void									togglePvdConnection();
					void									createPvdConnection();					

					void									bufferActiveTransforms();
					void									updateRenderObjectsDebug(float dtime); // update of render actors debug draw information, will be called while the simulation is NOT running
					void									updateRenderObjectsSync(float dtime);  // update of render objects while the simulation is NOT running (for particles, cloth etc. because data is not double buffered)
					void									updateRenderObjectsAsync(float dtime); // update of render objects, potentially while the simulation is running (for rigid bodies etc. because data is double buffered)

					void									saveUserInputs();
					void									saveInputEvents(const std::vector<const SampleFramework::InputEvent*>& );					
					void									parseSampleOutputAsset(const char* sampleName, PxU32 , PxU32 );
					void									spawnDebugObject();
					void									removeRenderActorsFromPhysicsActor(const PxRigidActor* actor);

protected:	// configurations
					bool									mInitialDebugRender;
					bool									mCreateCudaCtxManager;
					bool									mCreateGroundPlane;
					StepperType								mStepperType;
					PxU32									mMaxNumSubSteps;
					PxU32									mNbThreads;
					PxReal									mDefaultDensity;

protected:	// control
					bool									mDisplayFPS;

					bool&									mPause;
					bool&									mOneFrameUpdate;
					bool&									mShowHelp;
					bool&									mShowDescription;
					bool&									mShowExtendedHelp;
					bool									mHideGraphics;
					bool									mEnableAutoFlyCamera;

					DefaultCameraController&				mCameraController;
					DefaultCameraController					mFlyCameraController;
					PhysXSampleApplication::PvdParameters&	mPvdParams;

protected:
					PhysXSampleApplication&					mApplication;
					PxFoundation*							mFoundation;
					PxPhysics*								mPhysics;
					PxCooking*								mCooking;
					PxScene*								mScene;
					PxMaterial*								mMaterial;
					PxDefaultCpuDispatcher*					mCpuDispatcher;
					physx::PxPvd*                           mPvd;
					physx::PxPvdTransport*                  mTransport;
					physx::PxPvdInstrumentationFlags        mPvdFlags;

#if PX_SUPPORT_GPU_PHYSX
					PxCudaContextManager*				    mCudaContextManager;
#endif

					std::vector<PxRigidActor*>				mPhysicsActors;
					std::vector<RenderBaseActor*>			mDeletedRenderActors;
					std::vector<RenderBaseActor*>			mRenderActors;
					std::vector<RenderParticleSystemActor*>	mRenderParticleSystemActors;
					std::vector<RenderClothActor*>			mRenderClothActors;
					

					std::vector<RenderTexture*>					mRenderTextures;
					std::vector<SampleFramework::SampleAsset*>	mManagedAssets;
					std::vector<RenderMaterial*>				mRenderMaterials;

					RenderMaterial*								(&mManagedMaterials)[MATERIAL_COUNT];

					SampleFramework::SampleInputAsset*		mSampleInputAsset;
					
					PxActiveTransform*						mBufferedActiveTransforms;
					std::vector<PxActor*>					mDeletedActors;
					PxU32									mActiveTransformCount;
					PxU32									mActiveTransformCapacity;
					bool									mIsFlyCamera;
					PhysXShapeToRenderActorMap				mPhysXShapeToRenderActorMap;

private:
					PxU32									mMeshTag;
					const char*								mFilename;
					PxReal									mScale;

					PxReal									mDebugRenderScale;
protected:
					bool									mWaitForResults;
private:
					PxToolkit::FPS							mFPS;

					CameraController*						mSavedCameraController;

					DebugStepper							mDebugStepper;
					FixedStepper							mFixedStepper;
					VariableStepper							mVariableStepper;
					bool									mWireFrame;

					PxReal									mSimulationTime;
					Picking*								mPicking;
					bool									mPicked;
					physx::PxU8								mExtendedHelpPage;
					physx::PxU32							mDebugObjectType;

					static const PxU32						SCRATCH_BLOCK_SIZE = 1024*128;
					void*									mScratchBlock;
};

PxToolkit::BasicRandom& getSampleRandom();
PxErrorCallback& getSampleErrorCallback();

#endif // PHYSX_SAMPLE_H

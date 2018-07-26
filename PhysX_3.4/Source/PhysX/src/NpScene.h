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


#ifndef PX_PHYSICS_NP_SCENE
#define PX_PHYSICS_NP_SCENE

#include "foundation/PxProfiler.h"
#include "PsUserAllocated.h"
#include "PsSync.h"
#include "PsArray.h"
#include "PsThread.h"
#include "PsHashSet.h"
#include "PxPhysXConfig.h"

#if PX_SUPPORT_GPU_PHYSX
#include "device/PhysXIndicator.h"
#endif

#include "NpSceneQueries.h"

namespace physx
{

class PhysicsThread;
class PxBatchQueryDesc;
class NpMaterial;
class NpVolumeCache;
class NpScene;

namespace Sc
{
	class Joint;
	class ConstraintBreakEvent;
}

namespace Sq
{
	class SceneQueryManager;
}

class NpObjectFactory;
class NpRigidStatic;
class NpRigidDynamic;
class NpParticleSystem;
class NpParticleFluid;
class NpConstraint;
class NpArticulationLink;
class NpCloth;
class NpShapeManager;
class NpBatchQuery;

class PxBatchQuery;

enum NpProfileZones
{
	NpScene_checkResults,
	NpScene_reportContacts,
	NpScene_reportProfiling,
	NpScene_reportTriggers,
	NpScene_stats,

	NpPrNumZones
};


class NpContactCallbackTask : public physx::PxLightCpuTask
{
	NpScene*	mScene;
	const PxContactPairHeader* mContactPairHeaders;
	uint32_t mNbContactPairHeaders;

public:

	void setData(NpScene* scene, const PxContactPairHeader* contactPairHeaders, const uint32_t nbContactPairHeaders);

	virtual void run();

	virtual const char* getName() const
	{
		return "NpContactCallbackTask";
	}
};

class NpScene : public NpSceneQueries, public Ps::UserAllocated
{
	//virtual interfaces:

	PX_NOCOPY(NpScene)
	public:

	virtual			void							release();

	virtual			void							setFlag(PxSceneFlag::Enum flag, bool value);
	virtual			PxSceneFlags					getFlags() const;

	// implement PxScene:

	virtual			void							setGravity(const PxVec3&);
	virtual			PxVec3							getGravity() const;

	virtual			void							setBounceThresholdVelocity(const PxReal t);
	virtual			PxReal							getBounceThresholdVelocity() const;

	virtual			PxReal							getFrictionOffsetThreshold() const;

	virtual			void							setLimits(const PxSceneLimits& limits);
	virtual			PxSceneLimits					getLimits() const;

	virtual			void							addActor(PxActor& actor);
	virtual			void							removeActor(PxActor& actor, bool wakeOnLostTouch);

	virtual			PxU32							getNbConstraints() const;
	virtual			PxU32							getConstraints(PxConstraint** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	virtual			void							addArticulation(PxArticulation&);
	virtual			void							removeArticulation(PxArticulation&, bool wakeOnLostTouch);
	virtual			PxU32							getNbArticulations() const;
	virtual			PxU32							getArticulations(PxArticulation** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

// PX_AGGREGATE
    virtual			void							addAggregate(PxAggregate&);
	virtual			void							removeAggregate(PxAggregate&, bool wakeOnLostTouch);
	virtual			PxU32							getNbAggregates()	const;
	virtual			PxU32							getAggregates(PxAggregate** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;
//~PX_AGGREGATE
	
	virtual			void							addCollection(const PxCollection& collection);

	// Groups
	virtual			void							setDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2, const PxDominanceGroupPair& dominance);
	virtual			PxDominanceGroupPair			getDominanceGroupPair(PxDominanceGroup group1, PxDominanceGroup group2) const;

	// Actors
	virtual			PxU32							getNbActors(PxActorTypeFlags types) const;
	virtual			PxU32							getActors(PxActorTypeFlags types, PxActor** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;
	virtual			const PxActiveTransform*		getActiveTransforms(PxU32& nbTransformsOut, PxClientID client);
	virtual			PxActor**						getActiveActors(PxU32& nbActorsOut, PxClientID client);

	// Run
	virtual			void							getSimulationStatistics(PxSimulationStatistics& s) const;

	// Multiclient 
	virtual			PxClientID						createClient();
	virtual			void							setClientBehaviorFlags(PxClientID client, PxClientBehaviorFlags clientBehaviorFlags); 
	virtual			PxClientBehaviorFlags			getClientBehaviorFlags(PxClientID client) const;

	// FrictionModel
	virtual			void							setFrictionType(PxFrictionType::Enum frictionType);
	virtual			PxFrictionType::Enum			getFrictionType() const;

#if PX_USE_CLOTH_API
	// Cloth
	virtual			void							setClothInterCollisionDistance(PxF32 distance);
	virtual			PxF32							getClothInterCollisionDistance() const;
	virtual			void							setClothInterCollisionStiffness(PxF32 stiffness); 
	virtual			PxF32							getClothInterCollisionStiffness() const;
	virtual			void							setClothInterCollisionNbIterations(PxU32 nbIterations);
	virtual			PxU32							getClothInterCollisionNbIterations() const;
#endif

	// Callbacks
	virtual			void							setSimulationEventCallback(PxSimulationEventCallback* callback, PxClientID client);
	virtual			PxSimulationEventCallback*		getSimulationEventCallback(PxClientID client)	const;
	virtual			void							setContactModifyCallback(PxContactModifyCallback* callback);
	virtual			PxContactModifyCallback*		getContactModifyCallback()	const;
	virtual			void							setCCDContactModifyCallback(PxCCDContactModifyCallback* callback);
	virtual			PxCCDContactModifyCallback*		getCCDContactModifyCallback()	const;
	virtual			void							setBroadPhaseCallback(PxBroadPhaseCallback* callback, PxClientID client);
	virtual			PxBroadPhaseCallback*			getBroadPhaseCallback(PxClientID client)		const;

	//CCD passes
	virtual			void							setCCDMaxPasses(PxU32 ccdMaxPasses);
	virtual			PxU32							getCCDMaxPasses()	const;

	// Collision filtering
	virtual			void							setFilterShaderData(const void* data, PxU32 dataSize);
	virtual			const void*						getFilterShaderData() const;
	virtual			PxU32							getFilterShaderDataSize() const;
	virtual			PxSimulationFilterShader		getFilterShader() const;
	virtual			PxSimulationFilterCallback*		getFilterCallback() const;
	virtual			void							resetFiltering(PxActor& actor);
	virtual			void							resetFiltering(PxRigidActor& actor, PxShape*const* shapes, PxU32 shapeCount);

	// Get Physics SDK
	virtual			PxPhysics&						getPhysics();

	// new API methods
	virtual			void							simulate(PxReal elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation);
	virtual			void							advance(physx::PxBaseTask* completionTask);
	virtual			void							collide(PxReal elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation = true);
	virtual			bool							checkResults(bool block);
	virtual			bool							checkCollision(bool block);
	virtual			bool							fetchCollision(bool block);
	virtual			bool							fetchResults(bool block, PxU32* errorState);
	virtual			bool							fetchResultsStart(const PxContactPairHeader*& contactPairs, PxU32& nbContactPairs, bool block = false);
	virtual			void							processCallbacks(physx::PxBaseTask* continuation);
	virtual			void							fetchResultsFinish(PxU32* errorState = 0);



	virtual			void							flush(bool sendPendingReports) { flushSimulation(sendPendingReports); }
	virtual			void							flushSimulation(bool sendPendingReports);
	virtual			void							flushQueryUpdates();
	virtual			const PxRenderBuffer&			getRenderBuffer();

	virtual			PxBatchQuery*					createBatchQuery(const PxBatchQueryDesc& desc);
					void							releaseBatchQuery(PxBatchQuery* bq);
	virtual			PxVolumeCache*					createVolumeCache(PxU32 maxStaticShapes, PxU32 maxDynamicShapes);
					void							releaseVolumeCache(NpVolumeCache* cache);
	virtual			void							setDynamicTreeRebuildRateHint(PxU32 dynamicTreeRebuildRateHint);
	virtual			PxU32							getDynamicTreeRebuildRateHint() const;
	virtual			void							forceDynamicTreeRebuild(bool rebuildStaticStructure, bool rebuildDynamicStructure);
	virtual			void							sceneQueriesUpdate(physx::PxBaseTask* completionTask, bool controlSimulation);
	virtual			bool							checkQueries(bool block);
	virtual			bool							fetchQueries(bool block);
	virtual			void							setSceneQueryUpdateMode(PxSceneQueryUpdateMode::Enum updateMode);
	virtual			PxSceneQueryUpdateMode::Enum	getSceneQueryUpdateMode() const;

	virtual			void							setSolverBatchSize(PxU32 solverBatchSize);
	virtual			PxU32							getSolverBatchSize(void) const;

	virtual			bool							setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value);
	virtual			PxReal							getVisualizationParameter(PxVisualizationParameter::Enum param) const;

	virtual			void							setVisualizationCullingBox(const PxBounds3& box);
	virtual			PxBounds3						getVisualizationCullingBox() const;

	virtual			PxTaskManager*					getTaskManager()	{ return mTaskManager; }
					void							checkBeginWrite() const {}
					
	virtual         void							setNbContactDataBlocks(PxU32 numBlocks);
	virtual         PxU32							getNbContactDataBlocksUsed() const;
	virtual         PxU32							getMaxNbContactDataBlocksUsed() const;

	virtual			PxU32							getContactReportStreamBufferSize() const;

	virtual			PxU32							getTimestamp()	const;
	virtual			PxU32							getSceneQueryStaticTimestamp()	const;

	virtual			PxCpuDispatcher*				getCpuDispatcher() const;
	virtual			PxGpuDispatcher*				getGpuDispatcher() const;

	virtual			PxPruningStructureType::Enum	getStaticStructure() const;
	virtual			PxPruningStructureType::Enum	getDynamicStructure() const;
	
	virtual			PxBroadPhaseType::Enum			getBroadPhaseType()									const;
	virtual			bool							getBroadPhaseCaps(PxBroadPhaseCaps& caps)			const;
	virtual			PxU32							getNbBroadPhaseRegions()							const;
	virtual			PxU32							getBroadPhaseRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;
	virtual			PxU32							addBroadPhaseRegion(const PxBroadPhaseRegion& region, bool populateRegion);
	virtual			bool							removeBroadPhaseRegion(PxU32 handle);

	virtual			void							addActors(PxActor*const* actors, PxU32 nbActors);
	virtual			void							addActors(const PxPruningStructure& prunerStructure);
	virtual			void							removeActors(PxActor*const* actors, PxU32 nbActors, bool wakeOnLostTouch);

	virtual			void							lockRead(const char* file=NULL, PxU32 line=0);
	virtual			void							unlockRead();

	virtual			void							lockWrite(const char* file=NULL, PxU32 line=0);
	virtual			void							unlockWrite();

	virtual			PxReal							getWakeCounterResetValue() const;

	virtual			void							shiftOrigin(const PxVec3& shift);

	virtual         PxPvdSceneClient*				getScenePvdClient();

	//internal public methods:
	public:
													NpScene(const PxSceneDesc& desc);
													~NpScene();

	PX_FORCE_INLINE	PxTaskManager*					getTaskManager()			const	{ return mTaskManager;			}

	PX_FORCE_INLINE Sc::SimulationStage::Enum		getSimulationStage()		const	{ return mScene.getSimulationStage(); }
	PX_FORCE_INLINE void							setSimulationStage(Sc::SimulationStage::Enum stage) { mScene.setSimulationStage(stage); }

					void							addActorInternal(PxActor& actor);
					void							removeActorInternal(PxActor& actor, bool wakeOnLostTouch, bool removeFromAggregate);
					void							addActorsInternal(PxActor*const* PX_RESTRICT actors, PxU32 nbActors, const Sq::PruningStructure* ps = NULL);

					void							addArticulationInternal(PxArticulation&);
					void							removeArticulationInternal(PxArticulation&, bool wakeOnLostTouch,  bool removeFromAggregate);
	// materials
					void							addMaterial(const NpMaterial& mat);
					void							updateMaterial(const NpMaterial& mat);
					void							removeMaterial(const NpMaterial& mat);

					void							executeScene(PxBaseTask* continuation);
					void							executeCollide(PxBaseTask* continuation);
					void							executeAdvance(PxBaseTask* continuation);
					void							constraintBreakEventNotify(PxConstraint *const *constraints, PxU32 count);

					bool							loadFromDesc(const PxSceneDesc&);

					void							removeFromRigidActorList(const PxU32&);
	PX_FORCE_INLINE	void							removeFromArticulationList(PxArticulation&);
	PX_FORCE_INLINE	void							removeFromAggregateList(PxAggregate&);

	PX_FORCE_INLINE	void							addToConstraintList(PxConstraint&);
	PX_FORCE_INLINE	void							removeFromConstraintList(PxConstraint&);

#if PX_USE_PARTICLE_SYSTEM_API
	PX_FORCE_INLINE void							removeFromParticleBaseList(PxParticleBase&);
#endif

					void							addArticulationLink(NpArticulationLink& link);
					void							addArticulationLinkBody(NpArticulationLink& link);
					void							addArticulationLinkConstraint(NpArticulationLink& link);
					void							removeArticulationLink(NpArticulationLink& link, bool wakeOnLostTouch);

					struct StartWriteResult
					{
						enum Enum { eOK, eNO_LOCK, eIN_FETCHRESULTS, eRACE_DETECTED };
					};

					StartWriteResult::Enum			startWrite(bool allowReentry);
					void							stopWrite(bool allowReentry);

					bool							startRead() const;
					void							stopRead() const;

					PxU32							getReadWriteErrorCount() const { return PxU32(mConcurrentErrorCount); }

#if PX_CHECKED
					void							checkPositionSanity(const PxRigidActor& a, const PxTransform& pose, const char* fnName) const;
#endif

#if PX_USE_CLOTH_API
					void							addCloth(NpCloth&);
					void							removeCloth(NpCloth&);
#endif

#if PX_SUPPORT_GPU_PHYSX
					void							updatePhysXIndicator();
#else
	PX_FORCE_INLINE	void							updatePhysXIndicator() {}
#endif

	PX_FORCE_INLINE PxReal							getWakeCounterResetValueInteral() const { return mScene.getWakeCounterResetValue(); }

private:
					bool							checkResultsInternal(bool block);
					bool							checkCollisionInternal(bool block);
					bool							checkSceneQueriesInternal(bool block);
					void							simulateOrCollide(PxReal elapsedTime, physx::PxBaseTask* completionTask, void* scratchBlock, PxU32 scratchBlockSize, bool controlSimulation, const char* invalidCallMsg, Sc::SimulationStage::Enum simStage);

					void							addRigidStatic(NpRigidStatic& , bool hasPrunerStructure = false);
					void							removeRigidStatic(NpRigidStatic&, bool wakeOnLostTouch, bool removeFromAggregate);
					void							addRigidDynamic(NpRigidDynamic& , bool hasPrunerStructure = false);
					void							removeRigidDynamic(NpRigidDynamic&, bool wakeOnLostTouch, bool removeFromAggregate);

					bool							addRigidActorsInternal(PxU32 nbActors, PxActor** PX_RESTRICT actors);

#if PX_USE_PARTICLE_SYSTEM_API
					void							addParticleSystem(NpParticleSystem&);
					void							removeParticleSystem(NpParticleSystem&);
					void							addParticleFluid(NpParticleFluid&);
					void							removeParticleFluid(NpParticleFluid&);
#endif

#if PX_USE_CLOTH_API
	PX_FORCE_INLINE	void							removeFromClothList(PxCloth&);
#endif

					void							visualize();

					void							updateDirtyShaders();

					void							fireOutOfBoundsCallbacks();
					void							fetchResultsPreContactCallbacks();
					void							fetchResultsPostContactCallbacks();



					void							updateScbStateAndSetupSq(const PxRigidActor& rigidActor, Scb::Actor& actor, NpShapeManager& shapeManager, bool actorDynamic, const PxBounds3* bounds, bool hasPrunerStructure);
	PX_FORCE_INLINE	void							updateScbStateAndSetupSq(const PxRigidActor& rigidActor, Scb::Body& body, NpShapeManager& shapeManager, bool actorDynamic, const PxBounds3* bounds, bool hasPrunerStructure);

					Cm::RenderBuffer				mRenderBuffer;

					Ps::CoalescedHashSet<PxConstraint*> mConstraints;
					Ps::Array<PxRigidActor*>		mRigidActors;  // no hash set used because it would be quite a bit slower when adding a large number of actors
					Ps::CoalescedHashSet<PxArticulation*> mArticulations;
					Ps::CoalescedHashSet<PxAggregate*> mAggregates;
					Ps::HashSet<NpVolumeCache*>		mVolumeCaches;
					Ps::Array<NpBatchQuery*>		mBatchQueries;

#if PX_USE_PARTICLE_SYSTEM_API
					Ps::CoalescedHashSet<PxParticleBase*> mPxParticleBaseSet;
#endif

#if PX_USE_CLOTH_API
					Ps::CoalescedHashSet<PxCloth*>	mPxCloths;
#endif

					PxBounds3						mSanityBounds;
#if PX_SUPPORT_GPU_PHYSX
					PhysXIndicator					mPhysXIndicator;
#endif

					Ps::Sync						mPhysicsDone;		// physics thread signals this when update ready
					Ps::Sync						mCollisionDone;		// physics thread signals this when all collisions ready
					Ps::Sync						mSceneQueriesDone;	// physics thread signals this when all scene queries update ready


		//legacy timing settings:
					PxReal							mElapsedTime;		//needed to transfer the elapsed time param from the user to the sim thread.

					PxU32							mNbClients;		// Tracks reserved clients for multiclient support.
					Ps::Array<PxU32>				mClientBehaviorFlags;// Tracks behavior bits for clients.


					struct SceneCompletion : public Cm::Task
					{
						SceneCompletion(PxU64 contextId, Ps::Sync& sync) : Cm::Task(contextId), mSync(sync){}
						virtual void runInternal() {}
						//ML: As soon as mSync.set is called, and the scene is shutting down,
						//the scene may be deleted. That means this running task may also be deleted.
						//As such, we call mSync.set() inside release() to avoid a crash because the v-table on this
						//task might be deleted between the call to runInternal() and release() in the worker thread.
						virtual void release() 
						{ 
							//We cache the continuation pointer because this class may be deleted 
							//as soon as mSync.set() is called if the application releases the scene.
							PxBaseTask* c = mCont; 
							//once mSync.set(), fetchResults() will be allowed to run.
							mSync.set(); 
							//Call the continuation task that we cached above. If we use mCont or 
							//any other member variable of this class, there is a small chance
							//that the variables might have become corrupted if the class
							//was deleted.
							if(c) c->removeReference(); 
						}
						virtual const char* getName() const { return "NpScene.completion"; }

						//	//This method just is called in the split sim approach as a way to set continuation after the task has been initialized
						void setDependent(PxBaseTask* task){PX_ASSERT(mCont == NULL); mCont = task; if(task)task->addReference();}
						Ps::Sync& mSync;
					private:
						SceneCompletion& operator=(const SceneCompletion&);
					};

					typedef Cm::DelegateTask<NpScene, &NpScene::executeScene> SceneExecution;
					typedef Cm::DelegateTask<NpScene, &NpScene::executeCollide> SceneCollide;
					typedef Cm::DelegateTask<NpScene, &NpScene::executeAdvance> SceneAdvance;
					

					PxTaskManager*					mTaskManager;
					SceneCompletion					mSceneCompletion;
					SceneCompletion					mCollisionCompletion;
					SceneCompletion					mSceneQueriesCompletion;
					SceneExecution					mSceneExecution;
					SceneCollide					mSceneCollide;
					SceneAdvance					mSceneAdvance;
					bool                            mControllingSimulation;

					PxU32							mSimThreadStackSize;

					volatile PxI32					mConcurrentWriteCount;
					mutable volatile PxI32			mConcurrentReadCount;					
					mutable volatile PxI32			mConcurrentErrorCount;

					// TLS slot index, keeps track of re-entry depth for this thread
					PxU32							mThreadReadWriteDepth;
					Ps::Thread::Id					mCurrentWriter;
					Ps::ReadWriteLock				mRWLock;

					bool							mSceneQueriesUpdateRunning;

					bool							mHasSimulatedOnce;
					bool							mBetweenFetchResults;
};


PX_FORCE_INLINE	void NpScene::addToConstraintList(PxConstraint& constraint)
{
	mConstraints.insert(&constraint);
}


PX_FORCE_INLINE	void NpScene::removeFromConstraintList(PxConstraint& constraint)
{
	const bool exists = mConstraints.erase(&constraint);
	PX_ASSERT(exists);
	PX_UNUSED(exists);
}


PX_FORCE_INLINE void NpScene::removeFromArticulationList(PxArticulation& articulation)
{
	const bool exists = mArticulations.erase(&articulation);
	PX_ASSERT(exists);
	PX_UNUSED(exists);
}


PX_FORCE_INLINE void NpScene::removeFromAggregateList(PxAggregate& aggregate)
{
	const bool exists = mAggregates.erase(&aggregate);
	PX_ASSERT(exists);
	PX_UNUSED(exists);
}


#if PX_USE_PARTICLE_SYSTEM_API
PX_FORCE_INLINE void NpScene::removeFromParticleBaseList(PxParticleBase& p)
{
	const bool exists = mPxParticleBaseSet.erase(&p);
	PX_ASSERT(exists);
	PX_UNUSED(exists);
}
#endif


#if PX_USE_CLOTH_API
PX_FORCE_INLINE void NpScene::removeFromClothList(PxCloth& cloth)
{
	const bool exists = mPxCloths.erase(&cloth);
	PX_ASSERT(exists);
	PX_UNUSED(exists);
}
#endif  // PX_USE_CLOTH_API

}

#endif

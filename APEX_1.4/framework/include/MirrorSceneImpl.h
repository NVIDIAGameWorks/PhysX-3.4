/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MIRROR_SCENE_IMPL_H

#define MIRROR_SCENE_IMPL_H

#include "PhysXSDKVersion.h"

#if PX_PHYSICS_VERSION_MAJOR == 3

#include "MirrorScene.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PxSimulationEventCallback.h"
#include "PxClient.h"
#include "PsHashMap.h"
#include "PxDeletionListener.h"
#include "PxTransform.h"
#include "PsArray.h"

namespace nvidia
{
	namespace apex
	{
		class MirrorSceneImpl;
		class MirrorActor;

		enum MirrorCommandType
		{
			MCT_CREATE_ACTOR,
			MCT_RELEASE_ACTOR,
			MCT_UPDATE_POSE,
			MCT_LAST
		};

		class MirrorCommand
		{
		public:
			MirrorCommand(MirrorCommandType type,MirrorActor *ma)
			{
				mType = type;
				mMirrorActor = ma;
			}
			MirrorCommand(MirrorCommandType type,MirrorActor *ma,const PxTransform &pose)
			{
				mType = type;
				mMirrorActor = ma;
				mPose = pose;
			}
			MirrorCommandType	mType;
			MirrorActor			*mMirrorActor;
			PxTransform	mPose;
		};

		class MirrorActor : public shdfnd::UserAllocated
		{
		public:

			// The constructor is only ever called from the PrimaryScene thread
			MirrorActor(size_t actorHash,physx::PxRigidActor &actor,MirrorSceneImpl &parentScene);
			// The destructor is only ever called from the MirrorSceneImpl thread
			virtual ~MirrorActor(void);

			// Increments the reference count for the number of shapes on this actor
			// currently inside the trigger volume.
			// Only ever called from the primary scene thread
			void addShape(void)
			{
				mShapeCount++;
			}

			// Decrements the reference count for the number of shapes on this actor
			// which are currently in the trigger volume. 
			// If the reference count goes to zero, then no part of this actor is 
			// any longer inside the trigger volume and it's mirror should in turn be released
			// This is only ever called from the PrimaryScene thread
			bool removeShape(void)
			{
				mShapeCount--;
				return mShapeCount == 0;
			}

			// This method is called when the reference count goes to zero and/or the primary
			// actor is released.
			// This method posts on the MirrorSceneImpl thread queue for this object to be deleted
			// the next time it does an update.
			// At this point the primary scene should remove this actor from the hash table.
			void release(void);


			// Required by the PxObserver class we inherited
			virtual	uint32_t getObjectSize()const
			{
				return sizeof(MirrorActor);
			}
	
			// This method is only called by the PrimaryScene thread
			// If the pose of the actor we are mirroring has 
			void synchronizePose(void); // called from the primary scene thread; see if the pose of the mirrored actor has changed.

			void createActor(PxScene &scene);
			void updatePose(const PxTransform &pose);

			MirrorSceneImpl		&mMirrorScene;
			uint32_t			mShapeCount;
			uint32_t			mMirrorShapeCount;
			PxRigidActor	*mPrimaryActor;
			PxRigidActor	*mMirrorActor;
			bool			mReleasePosted;
			PxTransform		mPrimaryGlobalPose;
			size_t	mActorHash;	// hash in primary scene.

		private:
			MirrorActor& operator=(const MirrorActor&);
		};

		typedef nvidia::Array< MirrorCommand > MirrorCommandArray;

		class MirrorSceneImpl : public MirrorScene, public shdfnd::UserAllocated,
			public physx::PxSimulationEventCallback, physx::PxDeletionListener
		{
		public:

			typedef shdfnd::HashMap< size_t, MirrorActor * > ActorHash;
			typedef shdfnd::HashMap< size_t, MirrorActor * > ShapeHash;

			enum ActorChange
			{
				AC_DELETED,
				AC_FOUND,
				AC_LOST
			};

			MirrorSceneImpl(physx::PxScene &primaryScene,
				physx::PxScene &mirrorScene,
				MirrorScene::MirrorFilter &mirrorFilter,
				float mirrorStaticDistance,
				float mirrorDynamicDistance,
				float mirrorDistanceThreshold);

			MirrorScene::MirrorFilter & getMirrorFilter(void) const
			{
				return mMirrorFilter;
			}

			virtual void synchronizePrimaryScene(const PxVec3 &cameraPos);
			virtual void synchronizeMirrorScene(void);

			virtual void release(void);

			void postCommand(const MirrorCommand &mc);


			/**
			physx::PxSimulationEventCallback interface
			*/
			virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, uint32_t count);
			virtual void onWake(PxActor** actors, uint32_t count);
			virtual void onSleep(PxActor** actors, uint32_t count);
			virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, uint32_t nbPairs);
			virtual void onTrigger(physx::PxTriggerPair* pairs, uint32_t count);
			virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count);

			// This is a notification even that the PrimayScene actor we are mirroring has been
			// deleted.  If this is the case, we need to zero out PrimaryActor pointer so we
			// no longer attempt to access it.
			// However...we do not call release, because we should get trigger events which cause
			// the reference count to go to zero.
			virtual void onRelease(const PxBase* observed,
				void* userData,
				physx::PxDeletionEventFlag::Enum deletionEvent);



		protected:
			virtual ~MirrorSceneImpl(void);
		private:

			void mirrorShape(const physx::PxTriggerPair &tp);

			void createTriggerActor(const PxVec3 &cameraPosition);

			physx::PxScene				&mPrimaryScene;
			physx::PxScene				&mMirrorScene;
			MirrorScene::MirrorFilter &mMirrorFilter;
			float				mMirrorStaticDistance;
			float				mMirrorDynamicDistance;
			float				mMirrorDistanceThreshold;
			PxVec3				mLastCameraLocation;
			physx::PxRigidDynamic		*mTriggerActor;
			PxMaterial			*mTriggerMaterial;
			physx::PxShape				*mTriggerShapeStatic;
			physx::PxShape				*mTriggerShapeDynamic;
			ActorHash					mActors;
			ShapeHash					mShapes;
			shdfnd::Mutex		mMirrorCommandMutex;
			MirrorCommandArray			mMirrorCommands;
			physx::PxSimulationEventCallback	*mSimulationEventCallback;
			nvidia::Array< physx::PxTriggerPair > mTriggerPairs;
		};

	}; // end of apex namespace
}; // end of physx namespace

#endif // PX_PHYSICS_VERSION_MAJOR

#endif // MIRROR_SCENE_IMPL_H

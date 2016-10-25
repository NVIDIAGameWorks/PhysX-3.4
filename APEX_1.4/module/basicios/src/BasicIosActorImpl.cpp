/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "SceneIntl.h"
#include "ApexSDKIntl.h"
#include "ScopedPhysXLock.h"

#include "BasicIosActor.h"
#include "BasicIosActorImpl.h"
#include "BasicIosAssetImpl.h"
#include "IofxAsset.h"
#include "IofxActor.h"
#include "ModuleBasicIosImpl.h"
#include "BasicIosScene.h"
#include "RenderDebugInterface.h"
#include "AuthorableObjectIntl.h"
#include "ModuleIofxIntl.h"
#include "FieldSamplerManagerIntl.h"
#include "FieldSamplerQueryIntl.h"
#include "ApexMirroredArray.h"
#include "ApexResourceHelper.h"
#include "PsAsciiConversion.h"

#include "PxTask.h"

#include <PxScene.h>
#include <PxConvexMesh.h>

#include <PxShape.h>
#include <geometry/PxBoxGeometry.h>
#include <geometry/PxSphereGeometry.h>
#include <geometry/PxCapsuleGeometry.h>
#include <geometry/PxPlaneGeometry.h>
#include <geometry/PxTriangleMeshGeometry.h>
#include <geometry/PxTriangleMesh.h>

#include <PxMaterial.h>

#include <PxRigidActor.h>
#include <PxRigidBody.h>
#include <extensions/PxShapeExt.h>

#include <PsAsciiConversion.h>

#include "Lock.h"

namespace nvidia
{
	namespace basicios
	{
		using namespace physx;

		class BasicIosInjectTask : public PxTask, public UserAllocated
		{
		public:
			BasicIosInjectTask(BasicIosActorImpl* actor) : mActor(actor) {}

			const char* getName() const
			{
				return "BasicIosActorImpl::InjectTask";
			}
			void run()
			{
				mActor->injectNewParticles();
			}

		protected:
			BasicIosActorImpl* mActor;
		};

		void BasicIosActorImpl::initStorageGroups(InplaceStorage& storage)
		{
			mSimulationStorageGroup.init(storage);
		}

		BasicIosActorImpl::BasicIosActorImpl(
			ResourceList& list,
			BasicIosAssetImpl& asset,
			BasicIosScene& scene,
			nvidia::apex::IofxAsset& iofxAsset,
			bool isDataOnDevice)
			: mAsset(&asset)
			, mBasicIosScene(&scene)
			, mIofxMgr(NULL)
			, mTotalElapsedTime(0.0f)
			, mParticleCount(0)
			, mParticleBudget(UINT_MAX)
			, mInjectedCount(0)
			, mLastActiveCount(0)
			, mLastBenefitSum(0)
			, mLastBenefitMin(+FLT_MAX)
			, mLastBenefitMax(-FLT_MAX)
			, mLifeSpan(scene.getApexScene(), PX_ALLOC_INFO("mLifeSpan", PARTICLES))
			, mLifeTime(scene.getApexScene(), PX_ALLOC_INFO("mLifeTime", PARTICLES))
			, mInjector(scene.getApexScene(), PX_ALLOC_INFO("mInjector", PARTICLES))
			, mBenefit(scene.getApexScene(), PX_ALLOC_INFO("mBenefit", PARTICLES))
			, mConvexPlanes(scene.getApexScene(), PX_ALLOC_INFO("mConvexPlanes", PARTICLES))
			, mConvexVerts(scene.getApexScene(), PX_ALLOC_INFO("mConvexVerts", PARTICLES))
			, mConvexPolygonsData(scene.getApexScene(), PX_ALLOC_INFO("mConvexPolygonsData", PARTICLES))
			, mTrimeshVerts(scene.getApexScene(), PX_ALLOC_INFO("mTrimeshVerts", PARTICLES))
			, mTrimeshIndices(scene.getApexScene(), PX_ALLOC_INFO("mTrimeshIndices", PARTICLES))
			, mInjectorsCounters(scene.getApexScene(), PX_ALLOC_INFO("mInjectorsCounters", PARTICLES))
			, mGridDensityGrid(scene.getApexScene(), PX_ALLOC_INFO("mGridDensityGrid", PARTICLES))
			, mGridDensityGridLowPass(scene.getApexScene(), PX_ALLOC_INFO("mGridDensityGridLowPass", PARTICLES))
			, mFieldSamplerQuery(NULL)
			, mField(scene.getApexScene(), PX_ALLOC_INFO("mField", PARTICLES))
			, mDensityOrigin(0.f,0.f,0.f)
			, mOnStartCallback(NULL)
			, mOnFinishCallback(NULL)
		{
			list.add(*this);

			mMaxParticleCount = mAsset->mParams->maxParticleCount;
			mParticleBudget = mMaxParticleCount;
			float maxInjectCount = mAsset->mParams->maxInjectedParticleCount;
			mMaxTotalParticleCount = mMaxParticleCount + uint32_t(maxInjectCount <= 1.0f ? mMaxParticleCount * maxInjectCount : maxInjectCount);


			IofxManagerDescIntl desc;
			desc.iosAssetName         = mAsset->getName();
			desc.iosSupportsDensity   = mAsset->getSupportsDensity();
			desc.iosSupportsCollision = true;
			desc.iosSupportsUserData  = true;
			desc.iosOutputsOnDevice   = isDataOnDevice;
			desc.maxObjectCount       = mMaxParticleCount;
			desc.maxInputCount        = mMaxTotalParticleCount;
			desc.maxInStateCount      = mMaxTotalParticleCount;

			ModuleIofxIntl* moduleIofx = mAsset->mModule->getInternalModuleIofx();
			if (moduleIofx)
			{
				mIofxMgr = moduleIofx->createActorManager(*mBasicIosScene->mApexScene, iofxAsset, desc);
				mIofxMgr->createSimulationBuffers(mBufDesc);
			}

			mCollisionFilterData = ApexResourceHelper::resolveCollisionGroup128(mAsset->mParams->fieldSamplerFilterDataName);

			FieldSamplerManagerIntl* fieldSamplerManager = mBasicIosScene->getInternalFieldSamplerManager();
			if (fieldSamplerManager)
			{
				FieldSamplerQueryDescIntl queryDesc;
				queryDesc.maxCount = mMaxParticleCount;
				queryDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(mAsset->mParams->fieldSamplerFilterDataName);

				mFieldSamplerQuery = fieldSamplerManager->createFieldSamplerQuery(queryDesc);

				if (isDataOnDevice)
				{
#if APEX_CUDA_SUPPORT
					mField.reserve(mMaxParticleCount, ApexMirroredPlace::GPU);
#endif
				}
				else
				{
					mField.reserve(mMaxParticleCount, ApexMirroredPlace::CPU);
				}
			}

			mInjectTask = PX_NEW(BasicIosInjectTask)(this);

			// Pull Grid Density Parameters
			{
				if(mBufDesc.pmaDensity)
				{
					BasicIOSAssetParam* gridParams = (BasicIOSAssetParam*)(mAsset->getAssetNvParameterized());
					mGridDensityParams.Enabled = gridParams->GridDensity.Enabled;
					mGridDensityParams.GridSize = gridParams->GridDensity.GridSize;
					mGridDensityParams.GridMaxCellCount = gridParams->GridDensity.MaxCellCount;
					mGridDensityParams.GridResolution = general_string_parsing2::PxAsc::strToU32(&gridParams->GridDensity.Resolution[4],NULL);
					mGridDensityParams.DensityOrigin = mDensityOrigin;
				}		
				else
				{
					mGridDensityParams.Enabled = false;
					mGridDensityParams.GridSize = 1.f;
					mGridDensityParams.GridMaxCellCount = 1u;
					mGridDensityParams.GridResolution = 8;
					mGridDensityParams.DensityOrigin = mDensityOrigin;
				}
			}

			addSelfToContext(*scene.mApexScene->getApexContext());		// add self to Scene
			addSelfToContext(*DYNAMIC_CAST(ApexContext*)(&scene));		// add self to BasicIosScene
		}

		BasicIosActorImpl::~BasicIosActorImpl()
		{
			PX_DELETE(mInjectTask);
		}

		void BasicIosActorImpl::release()
		{
			if (mInRelease)
			{
				return;
			}
			mInRelease = true;
			mAsset->releaseIosActor(*this);
		}

		void BasicIosActorImpl::destroy()
		{
			ApexActor::destroy();

			setPhysXScene(NULL);

			// remove ourself from our asset's resource list, in case releasing our emitters
			// causes our asset's resource count to reach zero and for it to be released.
			ApexResource::removeSelf();

			// Release all injectors, releasing all emitters and their IOFX asset references
			while (mInjectorList.getSize())
			{
				BasicParticleInjector* inj = DYNAMIC_CAST(BasicParticleInjector*)(mInjectorList.getResource(mInjectorList.getSize() - 1));
				inj->release();
			}

			if (mIofxMgr)
			{
				mIofxMgr->release();
			}
			if (mFieldSamplerQuery)
			{
				mFieldSamplerQuery->release();
			}

			delete this;
		}

		void BasicIosActorImpl::setPhysXScene(PxScene* scene)
		{
			if (scene)
			{
				putInScene(scene);
			}
			else
			{
				removeFromScene();
			}
		}
		PxScene* BasicIosActorImpl::getPhysXScene() const
		{
			return NULL;
		}
		void BasicIosActorImpl::putInScene(PxScene* scene)
		{
			SCOPED_PHYSX_LOCK_READ(scene);
			PxVec3 gravity = scene->getGravity();
			setGravity(gravity);
		}

		void BasicIosActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
		{
			PX_UNUSED(min);
			PX_UNUSED(max);
			PX_UNUSED(intOnly);
			APEX_INVALID_OPERATION("not implemented");
		}


		float BasicIosActorImpl::getActiveLod() const
		{
			APEX_INVALID_OPERATION("BasicIosActor does not support this operation");
			return -1.0f;
		}


		void BasicIosActorImpl::forceLod(float lod)
		{
			PX_UNUSED(lod);
			APEX_INVALID_OPERATION("not implemented");
		}


		void BasicIosActorImpl::removeFromScene()
		{
			mParticleCount = 0;
		}


		const PxVec3* BasicIosActorImpl::getRecentPositions(uint32_t& count, uint32_t& stride) const
		{
			APEX_INVALID_OPERATION("not implemented");

			count = 0;
			stride = 0;
			return NULL;
		}

		PxVec3 BasicIosActorImpl::getGravity() const
		{
			return mGravityVec;
		}

		void BasicIosActorImpl::setGravity(PxVec3& gravity)
		{
			mGravityVec = gravity;
			mUp = mGravityVec;

			// apply asset's scene gravity scale and external acceleration
			mUp *= mAsset->getSceneGravityScale();
			mUp += mAsset->getExternalAcceleration();

			mGravity = mUp.magnitude();
			if (!PxIsFinite(mGravity))
			{
				// and they could set both to 0,0,0
				mUp = PxVec3(0.0f, -1.0f, 0.0f);
				mGravity = 1.0f;
			}
			mUp *= -1.0f;

			mIofxMgr->setSimulationParameters(getObjectRadius(), mUp, mGravity, getObjectDensity());
		}

		IosInjectorIntl* BasicIosActorImpl::allocateInjector(IofxAsset* iofxAsset)
		{
			BasicParticleInjector* inj = 0;
			//createInjector
			{
				uint32_t injectorID = mBasicIosScene->getInjectorAllocator().allocateInjectorID();
				if (injectorID != BasicIosInjectorAllocator::NULL_INJECTOR_INDEX)
				{
					inj = PX_NEW(BasicParticleInjector)(mInjectorList, *this, injectorID);
				}
			}
			if (inj == 0)
			{
				APEX_INTERNAL_ERROR("Failed to create new BasicIos injector.");
				return NULL;
			}

			inj->init(iofxAsset);
			return inj;
		}

		void BasicIosActorImpl::releaseInjector(IosInjectorIntl& injector)
		{
			BasicParticleInjector* inj = DYNAMIC_CAST(BasicParticleInjector*)(&injector);

			//destroyInjector
			{
				//set mLODBias to FLT_MAX to mark released injector
				//all particles from released injectors will be removed in simulation
				InjectorParams injParams;
				mBasicIosScene->fetchInjectorParams(inj->mInjectorID, injParams);
				injParams.mLODBias = FLT_MAX;
				mBasicIosScene->updateInjectorParams(inj->mInjectorID, injParams);

				mBasicIosScene->getInjectorAllocator().releaseInjectorID(inj->mInjectorID);
				inj->destroy();
			}

			if (mInjectorList.getSize() == 0)
			{
				//if we have no injectors - release self
				release();
			}
		}


		void BasicIosActorImpl::visualize()
		{
			if ( !mEnableDebugVisualization ) return;
#ifndef WITHOUT_DEBUG_VISUALIZE
			RenderDebugInterface* renderer = mBasicIosScene->mDebugRender;
			const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(renderer)->getPoseTyped();
			RENDER_DEBUG_IFACE(renderer)->setIdentityPose();
			if(mBasicIosScene->mBasicIosDebugRenderParams->VISUALIZE_BASIC_IOS_GRID_DENSITY)
			{
				if(mGridDensityParams.Enabled)
				{					
					RENDER_DEBUG_IFACE(renderer)->setCurrentColor(0x0000ff);
					float factor = PxMin((float)(mGridDensityParams.GridResolution-4) / (mGridDensityParams.GridResolution),0.75f);
					uint32_t onScreenRes = (uint32_t)(factor*mGridDensityParams.GridResolution);
					for (uint32_t i = 0 ; i <= onScreenRes; i++)
					{     
						float u = 2.f*((float)i/(onScreenRes))-1.f;
						PxVec4 a = mDensityDebugMatInv.transform(PxVec4(u,-1.f,0.1f,1.f));
						PxVec4 b = mDensityDebugMatInv.transform(PxVec4(u, 1.f,0.1f,1.f));
						PxVec4 c = mDensityDebugMatInv.transform(PxVec4(-1.f,u,0.1f,1.f));
						PxVec4 d = mDensityDebugMatInv.transform(PxVec4( 1.f,u,0.1f,1.f));
						RENDER_DEBUG_IFACE(renderer)->debugLine(PxVec3(a.getXYZ()/a.w), PxVec3(b.getXYZ()/b.w));
						RENDER_DEBUG_IFACE(renderer)->debugLine(PxVec3(c.getXYZ()/c.w), PxVec3(d.getXYZ()/d.w));
					}
				}
			}
			if(mBasicIosScene->mBasicIosDebugRenderParams->VISUALIZE_BASIC_IOS_COLLIDE_SHAPES)
			{
				INPLACE_STORAGE_GROUP_SCOPE(mSimulationStorageGroup);
				SimulationParams simParams;
				mSimulationParamsHandle.fetch(_storage_, simParams);

				RenderDebugInterface* renderer = mBasicIosScene->mDebugRender;
				RENDER_DEBUG_IFACE(renderer)->setCurrentColor(RENDER_DEBUG_IFACE(renderer)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));

				for(uint32_t i = 0; i < (uint32_t)simParams.boxes.getSize(); ++i)
				{
					CollisionBoxData boxData;
					simParams.boxes.fetchElem(_storage_, boxData, i);

					const PxTransform& pose = boxData.pose;  
					PxVec3 position = pose.p;
					PxVec3 halfSize = boxData.halfSize;
					RENDER_DEBUG_IFACE(renderer)->setPose(pose);
					RENDER_DEBUG_IFACE(renderer)->debugBound(PxBounds3(-halfSize, halfSize));
				}

				for(uint32_t i = 0; i < (uint32_t)simParams.spheres.getSize(); ++i)
				{
					CollisionSphereData sphereData;
					simParams.spheres.fetchElem(_storage_, sphereData, i);

					float r = sphereData.radius;
					PxVec3 pos = sphereData.pose.p;
					RENDER_DEBUG_IFACE(renderer)->debugSphere(pos, r);
				}

				for(uint32_t i = 0; i < (uint32_t)simParams.capsules.getSize(); ++i)
				{
					CollisionCapsuleData capsuleData;
					simParams.capsules.fetchElem(_storage_, capsuleData, i);

					float r = capsuleData.radius, h = capsuleData.halfHeight;
					const PxTransform& pose = capsuleData.pose;
					RENDER_DEBUG_IFACE(renderer)->setPose(pose);
					RENDER_DEBUG_IFACE(renderer)->debugCapsule(r, 2 * h);
				}
			}
			RENDER_DEBUG_IFACE(renderer)->setPose(savedPose);
#endif
		}

		void BasicIosActorImpl::FillCollisionData(CollisionData& baseData, PxShape* shape)
		{
			PxTransform actorGlobalPose = shape->getActor()->getGlobalPose();

			if (PxRigidBody* pxBody = shape->getActor()->is<physx::PxRigidBody>())
			{
				baseData.bodyCMassPosition = actorGlobalPose.transform(pxBody->getCMassLocalPose().p);
				baseData.bodyLinearVelocity = pxBody->getLinearVelocity();
				baseData.bodyAngluarVelocity = pxBody->getAngularVelocity();

				baseData.materialRestitution = mAsset->mParams->restitutionForDynamicShapes;
			}
			else
			{
				baseData.bodyCMassPosition = actorGlobalPose.p;
				baseData.bodyLinearVelocity = PxVec3(0, 0, 0);
				baseData.bodyAngluarVelocity = PxVec3(0, 0, 0);

				baseData.materialRestitution = mAsset->mParams->restitutionForStaticShapes;
			}
			//PxMaterial* PxMaterial = shape->getMaterialFromInternalFaceIndex(0);
			//PX_ASSERT(PxMaterial);
			//baseData->materialRestitution = PxMaterial->getRestitution();
		}

		void BasicIosActorImpl::submitTasks()
		{
			PxTaskManager* tm = mBasicIosScene->getApexScene().getTaskManager();
			tm->submitUnnamedTask(*mInjectTask);

			//compile a list of actually colliding objects and process them to be used by the simulation
			INPLACE_STORAGE_GROUP_SCOPE(mSimulationStorageGroup);

			SimulationParams simParams;
			if (mSimulationParamsHandle.allocOrFetch(_storage_, simParams))
			{
				//one time initialization on alloc
				simParams.collisionThreshold = mAsset->mParams->collisionThreshold;
				simParams.collisionDistance = mAsset->mParams->particleRadius * mAsset->mParams->collisionDistanceMultiplier;
#if APEX_CUDA_SUPPORT
#define			GET_PTR(x) x.getGpuPtr() ? x.getGpuPtr() : x.getPtr()
#else
#define			GET_PTR(x) x.getPtr()
#endif
				simParams.convexPlanes = GET_PTR(mConvexPlanes);
				simParams.convexVerts = GET_PTR(mConvexVerts);
				simParams.convexPolygonsData = GET_PTR(mConvexPolygonsData);

				simParams.trimeshVerts = GET_PTR(mTrimeshVerts);
				simParams.trimeshIndices = GET_PTR(mTrimeshIndices);
			}

			uint32_t numBoxes = 0;
			uint32_t numSpheres = 0;
			uint32_t numCapsules = 0;
			uint32_t numHalfSpaces = 0;
			uint32_t numConvexMeshes = 0;
			uint32_t numTriMeshes = 0;

			mConvexPlanes.setSize(0);
			mConvexVerts.setSize(0);
			mConvexPolygonsData.setSize(0);

			mTrimeshVerts.setSize(0);
			mTrimeshIndices.setSize(0);

			const float collisionRadius = simParams.collisionThreshold + simParams.collisionDistance;
			uint32_t numCollidingObjects = 0;
			PxBounds3 bounds = mIofxMgr->getBounds();
			if ((mAsset->mParams->staticCollision || mAsset->mParams->dynamicCollision) && !bounds.isEmpty())
			{
				PX_ASSERT(!bounds.isEmpty());
				bounds.fattenFast(collisionRadius);

				const uint32_t maxCollidingObjects = mAsset->mParams->maxCollidingObjects;

				mOverlapHits.resize(maxCollidingObjects);
				PxBoxGeometry overlapGeom(bounds.getExtents());
				PxTransform overlapPose(bounds.getCenter());

				PxQueryFilterData overlapFilterData;
				overlapFilterData.data = mCollisionFilterData;

				overlapFilterData.flags = PxQueryFlag::eNO_BLOCK | PxQueryFlag::ePREFILTER;
				if (mAsset->mParams->staticCollision) overlapFilterData.flags |= PxQueryFlag::eSTATIC;
				if (mAsset->mParams->dynamicCollision) overlapFilterData.flags |= PxQueryFlag::eDYNAMIC;

				SCOPED_PHYSX_LOCK_READ(mBasicIosScene->getModulePhysXScene());

				class OverlapFilter : public physx::PxQueryFilterCallback
				{
				public:
					virtual PxQueryHitType::Enum preFilter(
						const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, physx::PxHitFlags& queryFlags)
					{
						PX_UNUSED(queryFlags);

						const PxFilterData shapeFilterData = shape->getQueryFilterData();
						const physx::PxFilterObjectAttributes iosAttributes = PxFilterObjectType::ePARTICLE_SYSTEM;
						physx::PxFilterObjectAttributes actorAttributes = actor->getType();
						if (const PxRigidBody* rigidBody = actor->is<physx::PxRigidBody>())
						{
							if (rigidBody->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
							{
								actorAttributes |= PxFilterObjectFlag::eKINEMATIC;
							}
						}

						physx::PxPairFlags pairFlags;
						physx::PxFilterFlags filterFlags = mFilterShader(iosAttributes, filterData, actorAttributes, shapeFilterData, pairFlags, mFilterShaderData, mFilterShaderDataSize);
						return (filterFlags & (PxFilterFlag::eKILL | PxFilterFlag::eSUPPRESS)) ? PxQueryHitType::eNONE : PxQueryHitType::eTOUCH;
					}

					virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
					{
						PX_UNUSED(filterData);
						PX_UNUSED(hit);
						return PxQueryHitType::eNONE;
					}

					OverlapFilter(PxScene* scene)
					{
						mFilterShader = scene->getFilterShader();
						mFilterShaderData = scene->getFilterShaderData();
						mFilterShaderDataSize = scene->getFilterShaderDataSize();
					}

				private:
					physx::PxSimulationFilterShader mFilterShader;
					const void*						mFilterShaderData;
					uint32_t						mFilterShaderDataSize;

				} overlapFilter(mBasicIosScene->getModulePhysXScene());

				physx::PxOverlapBuffer ovBuffer(&mOverlapHits[0], mOverlapHits.size());
				mBasicIosScene->getModulePhysXScene()->overlap(overlapGeom, overlapPose, ovBuffer, overlapFilterData, &overlapFilter);
				numCollidingObjects = ovBuffer.getNbTouches();

				for (uint32_t iShape = 0; iShape < numCollidingObjects; iShape++)
				{
					PxShape* shape = mOverlapHits[iShape].shape;

					switch (shape->getGeometryType())
					{
					case PxGeometryType::eBOX:
						{
							++numBoxes;
						}
						break;
					case PxGeometryType::eSPHERE:
						{
							++numSpheres;
						}
						break;
					case PxGeometryType::eCAPSULE:
						{
							++numCapsules;
						}
						break;
					case PxGeometryType::ePLANE:
						{
							++numHalfSpaces;
						}
						break;
					case PxGeometryType::eCONVEXMESH:
						{
							if (mAsset->mParams->collisionWithConvex)
							{
								++numConvexMeshes;
							}
						}
						break;
					case PxGeometryType::eTRIANGLEMESH:
						{
							if (mAsset->mParams->collisionWithTriangleMesh)
							{
								++numTriMeshes;
							}
						}
						break;
					default:
						break;
					}
				}
			}

			simParams.boxes.resize(_storage_ , numBoxes);
			simParams.spheres.resize(_storage_ , numSpheres);
			simParams.capsules.resize(_storage_ , numCapsules);
			simParams.halfSpaces.resize(_storage_ , numHalfSpaces);
			simParams.convexMeshes.resize(_storage_ , numConvexMeshes);
			simParams.trimeshes.resize(_storage_ , numTriMeshes);

			numBoxes = 0;
			numSpheres = 0;
			numCapsules = 0;
			numHalfSpaces = 0;
			numConvexMeshes = 0;
			numTriMeshes = 0;

			if (numCollidingObjects > 0)
			{
				SCOPED_PHYSX_LOCK_READ(mBasicIosScene->getModulePhysXScene());
				for (uint32_t iShape = 0; iShape < numCollidingObjects; iShape++)
				{
					PxShape* shape = mOverlapHits[iShape].shape;
					PxRigidActor* actor = mOverlapHits[iShape].actor;

					PxBounds3 shapeWorldBounds = PxShapeExt::getWorldBounds(*shape, *actor);
					PxTransform actorGlobalPose = shape->getActor()->getGlobalPose();
					PxTransform shapeGlobalPose = actorGlobalPose.transform(shape->getLocalPose());

					PX_ASSERT(!shapeWorldBounds.isEmpty());
					shapeWorldBounds.fattenFast(collisionRadius);

					switch (shape->getGeometryType())
					{
					case PxGeometryType::eBOX:
						{
							PxBoxGeometry boxGeom;
							shape->getBoxGeometry(boxGeom);

							CollisionBoxData data;

							data.aabb = shapeWorldBounds;
							data.pose = shapeGlobalPose;
							data.inversePose = data.pose.getInverse();

							data.halfSize = boxGeom.halfExtents;

							FillCollisionData(data, shape);
							simParams.boxes.updateElem(_storage_, data, numBoxes++); 
						}
						break;
					case PxGeometryType::eSPHERE:
						{
							PxSphereGeometry sphereGeom;
							shape->getSphereGeometry(sphereGeom);

							CollisionSphereData data;

							data.aabb = shapeWorldBounds;
							data.pose = shapeGlobalPose;
							data.inversePose = data.pose.getInverse();

							data.radius = sphereGeom.radius;
							//extend
							data.radius += simParams.collisionDistance;

							FillCollisionData(data, shape);
							simParams.spheres.updateElem(_storage_, data, numSpheres++);
						}
						break;
					case PxGeometryType::eCAPSULE:
						{
							PxCapsuleGeometry capsuleGeom;
							shape->getCapsuleGeometry(capsuleGeom);

							CollisionCapsuleData data;

							data.aabb = shapeWorldBounds;
							data.pose = shapeGlobalPose;
							data.inversePose = data.pose.getInverse();

							data.halfHeight = capsuleGeom.halfHeight;
							data.radius = capsuleGeom.radius;
							//extend
							data.radius += simParams.collisionDistance;

							FillCollisionData(data, shape);
							simParams.capsules.updateElem(_storage_, data, numCapsules++); 
						}
						break;
					case PxGeometryType::ePLANE:
						{
							CollisionHalfSpaceData data;

							data.origin = shapeGlobalPose.p;
							data.normal = shapeGlobalPose.rotate(PxVec3(1, 0, 0));
							//extend
							data.origin += data.normal * simParams.collisionDistance;

							FillCollisionData(data, shape);
							simParams.halfSpaces.updateElem(_storage_, data, numHalfSpaces++);
						}
						break;
					case PxGeometryType::eCONVEXMESH:
						{
							if (mAsset->mParams->collisionWithConvex)
							{
								PxConvexMeshGeometry convexGeom;
								shape->getConvexMeshGeometry(convexGeom);

								CollisionConvexMeshData data;

								data.aabb = shapeWorldBounds;
								data.pose = shapeGlobalPose;
								data.inversePose = data.pose.getInverse();

								//get ConvexMesh
								const PxConvexMesh* convexMesh = convexGeom.convexMesh;

								uint32_t numPolygons = convexMesh->getNbPolygons();
								uint32_t numVertices = convexMesh->getNbVertices();
								uint32_t polygonsDataSize = 0;
								for (uint32_t i = 0; i < numPolygons; i++)
								{
									physx::PxHullPolygon polygon;
									bool polygonDataTest = convexMesh->getPolygonData(i, polygon);
									PX_UNUSED( polygonDataTest );
									PX_ASSERT( polygonDataTest );

									polygonsDataSize += (1 + polygon.mNbVerts);
								}

								if (mConvexPlanes.getSize() + numPolygons <= mConvexPlanes.getCapacity() &&
									mConvexVerts.getSize() + numVertices <= mConvexVerts.getCapacity() &&
									mConvexPolygonsData.getSize() + polygonsDataSize <= mConvexPolygonsData.getCapacity())
								{
									data.numPolygons = numPolygons;
									data.firstPlane = mConvexPlanes.getSize();
									data.firstVertex = mConvexVerts.getSize();
									data.polygonsDataOffset = mConvexPolygonsData.getSize();

									PxPlane* convexPlanes = mConvexPlanes.getPtr() + data.firstPlane;
									PxVec4*  convexVerts = mConvexVerts.getPtr() + data.firstVertex;
									uint32_t*   convexPolygonsData = mConvexPolygonsData.getPtr() + data.polygonsDataOffset;

									mConvexPlanes.setSize(data.firstPlane + numPolygons);
									mConvexVerts.setSize(data.firstVertex + numVertices);
									mConvexPolygonsData.setSize(data.polygonsDataOffset + polygonsDataSize);

									const PxMeshScale convexScaleInv( convexGeom.scale.getInverse() );
									//copy Convex Planes & Polygon Data
									const uint8_t* srcIndices = convexMesh->getIndexBuffer();
									for (uint32_t i = 0; i < numPolygons; i++)
									{
										physx::PxHullPolygon polygon;
										bool polygonDataTest = convexMesh->getPolygonData(i, polygon);
										PX_UNUSED( polygonDataTest );
										PX_ASSERT( polygonDataTest );
										PxPlane plane(polygon.mPlane[0], polygon.mPlane[1], polygon.mPlane[2], polygon.mPlane[3]);
										plane.n = convexScaleInv.transform(plane.n);
										plane.normalize();
										*convexPlanes++ = plane;

										const uint32_t polygonVertCount = polygon.mNbVerts;
										const uint8_t* polygonIndices = srcIndices + polygon.mIndexBase;

										*convexPolygonsData++ = polygonVertCount;
										for (uint32_t j = 0; j < polygonVertCount; ++j)
										{
											*convexPolygonsData++ = *polygonIndices++; 
										}
									}

									//copy Convex Vertices
									const PxVec3* srcVertices = convexMesh->getVertices();
									for (uint32_t i = 0; i < numVertices; i++)
									{
										*convexVerts++ = PxVec4(convexGeom.scale.transform(*srcVertices++), 0);
									}
								}
								else
								{
									APEX_DEBUG_WARNING("BasicIosActorImpl: out of memory to store Convex data");

									data.numPolygons = 0;
									data.firstPlane = 0;
									data.firstVertex = 0;
									data.polygonsDataOffset = 0;
								}

								FillCollisionData(data, shape);
								simParams.convexMeshes.updateElem(_storage_, data, numConvexMeshes++);
							}
							break;
						}
					case PxGeometryType::eTRIANGLEMESH:
						{
							if (mAsset->mParams->collisionWithTriangleMesh)
							{
								PxTriangleMeshGeometry trimeshGeom;
								shape->getTriangleMeshGeometry(trimeshGeom);

								CollisionTriMeshData data;

								data.aabb = shapeWorldBounds;
								PX_ASSERT(!data.aabb.isEmpty());
								data.aabb.fattenFast( simParams.collisionDistance + simParams.collisionThreshold );
								data.pose = shapeGlobalPose;
								data.inversePose = data.pose.getInverse();

								//triangle mesh data
								const PxTriangleMesh* trimesh = trimeshGeom.triangleMesh;

								const uint32_t numTrimeshIndices = trimesh->getNbTriangles() * 3;
								const uint32_t numTrimeshVerts = trimesh->getNbVertices();

								if (mTrimeshIndices.getSize() + numTrimeshIndices <= mTrimeshIndices.getCapacity() &&
									mTrimeshVerts.getSize() + numTrimeshVerts <= mTrimeshVerts.getCapacity())
								{
									data.numTriangles = trimesh->getNbTriangles();
									data.firstIndex = mTrimeshIndices.getSize();
									data.firstVertex = mTrimeshVerts.getSize();

									mTrimeshIndices.setSize(data.firstIndex + numTrimeshIndices);
									//copy TriangleMesh indices
									uint32_t* trimeshIndices = mTrimeshIndices.getPtr() + data.firstIndex;

									const bool has16BitIndices = (trimesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES);
									if (has16BitIndices)
									{
										const uint16_t* srcIndices = static_cast<const uint16_t*>(trimesh->getTriangles());
										for( uint32_t i = 0; i < numTrimeshIndices; i++)
										{
											trimeshIndices[i] = srcIndices[i];
										}
									}
									else
									{
										const uint32_t* srcIndices = static_cast<const uint32_t*>(trimesh->getTriangles());
										for( uint32_t i = 0; i < numTrimeshIndices; i++)
										{
											trimeshIndices[i] = srcIndices[i];
										}
									}

									mTrimeshVerts.setSize(data.firstVertex + numTrimeshVerts);
									//copy TriangleMesh vertices
									PxVec4* trimeshVerts = mTrimeshVerts.getPtr() + data.firstVertex;
									const PxVec3* srcVertices = trimesh->getVertices();
									for( uint32_t i = 0; i < numTrimeshVerts; i++)
									{
										trimeshVerts[i] = PxVec4(trimeshGeom.scale.transform(srcVertices[i]), 0);
									}
								}
								else
								{
									APEX_DEBUG_WARNING("BasicIosActorImpl: out of memory to store TriangleMesh data");

									data.numTriangles = 0;
									data.firstIndex = 0;
									data.firstVertex = 0;
								}

								FillCollisionData(data, shape);
								simParams.trimeshes.updateElem(_storage_, data, numTriMeshes++);
							}
							break;
						}
					default:
						break;
					}

				}

			}
			mSimulationParamsHandle.update(_storage_, simParams);
		}

		void BasicIosActorImpl::setTaskDependencies(PxTask* iosTask, bool isDataOnDevice)
		{
			PxTaskManager* tm = mBasicIosScene->getApexScene().getTaskManager();

			iosTask->startAfter(mInjectTask->getTaskID());

			if (mFieldSamplerQuery != NULL)
			{
				float deltaTime = mBasicIosScene->getApexScene().getPhysXSimulateTime();

				FieldSamplerQueryDataIntl queryData;
				queryData.timeStep = deltaTime;
				queryData.count = mParticleCount;
				queryData.isDataOnDevice = isDataOnDevice;
				queryData.positionStrideBytes = sizeof(PxVec4);
				queryData.velocityStrideBytes = sizeof(PxVec4);
				queryData.massStrideBytes = sizeof(PxVec4);
				queryData.pmaInIndices = 0;
				if (isDataOnDevice)
				{
#if APEX_CUDA_SUPPORT
					queryData.pmaInPosition = (float*)mBufDesc.pmaPositionMass->getGpuPtr();
					queryData.pmaInVelocity = (float*)mBufDesc.pmaVelocityLife->getGpuPtr();
					queryData.pmaInMass = &mBufDesc.pmaPositionMass->getGpuPtr()->w;
					queryData.pmaOutField = mField.getGpuPtr();
#endif
				}
				else
				{
					queryData.pmaInPosition = (float*)mBufDesc.pmaPositionMass->getPtr();
					queryData.pmaInVelocity = (float*)mBufDesc.pmaVelocityLife->getPtr();
					queryData.pmaInMass = &mBufDesc.pmaPositionMass->getPtr()->w;
					queryData.pmaOutField = mField.getPtr();
				}
				mFieldSamplerQuery->submitFieldSamplerQuery(queryData, iosTask->getTaskID());
			}

			PxTaskID postIofxTaskID = tm->getNamedTask(AST_PHYSX_FETCH_RESULTS);
			PxTaskID iofxTaskID = mIofxMgr->getUpdateEffectsTaskID(postIofxTaskID);
			if (iofxTaskID == (PxTaskID)0xFFFFFFFF)
			{
				iofxTaskID = postIofxTaskID;
			}
			iosTask->finishBefore(iofxTaskID);
		}

		void BasicIosActorImpl::fetchResults()
		{
			for(uint32_t i = 0; i < mInjectorList.getSize(); ++i)
			{
				BasicParticleInjector* inj = DYNAMIC_CAST(BasicParticleInjector*)(mInjectorList.getResource(i));
				inj->assignSimParticlesCount(mInjectorsCounters.get(i));
			}
		}

		void BasicIosActorImpl::injectNewParticles()
		{
			mInjectedBenefitSum = 0;
			mInjectedBenefitMin = +FLT_MAX;
			mInjectedBenefitMax = -FLT_MAX;

			uint32_t maxInjectCount = (mMaxTotalParticleCount - mParticleCount);

			uint32_t injectCount = 0;
			uint32_t lastInjectCount = 0;
			do
			{
				lastInjectCount = injectCount;
				for (uint32_t i = 0; i < mInjectorList.getSize(); i++)
				{
					BasicParticleInjector* inj = DYNAMIC_CAST(BasicParticleInjector*)(mInjectorList.getResource(i));
					if (inj->mInjectedParticles.size() == 0)
					{
						continue;
					}

					if (injectCount < maxInjectCount)
					{
						IosNewObject obj;
						if (inj->mInjectedParticles.popFront(obj))
						{
							uint32_t injectIndex = mParticleCount + injectCount;

							float particleMass = mAsset->getParticleMass();
							mBufDesc.pmaPositionMass->get(injectIndex) = PxVec4(obj.initialPosition.x, obj.initialPosition.y, obj.initialPosition.z, particleMass);
							mBufDesc.pmaVelocityLife->get(injectIndex) = PxVec4(obj.initialVelocity.x, obj.initialVelocity.y, obj.initialVelocity.z, 1.0f);
							mBufDesc.pmaActorIdentifiers->get(injectIndex) = obj.iofxActorID;

							mBufDesc.pmaUserData->get(injectIndex) = obj.userData;

							mLifeSpan[injectIndex] = obj.lifetime;
							mInjector[injectIndex] = inj->mInjectorID;
							mBenefit[injectIndex] = obj.lodBenefit;

							mInjectedBenefitSum += obj.lodBenefit;
							mInjectedBenefitMin = PxMin(mInjectedBenefitMin, obj.lodBenefit);
							mInjectedBenefitMax = PxMax(mInjectedBenefitMax, obj.lodBenefit);

							++injectCount;
						}
					}
				}
			}
			while (injectCount > lastInjectCount);

			mInjectedCount = injectCount;

			//clear injectors FIFO
			for (uint32_t i = 0; i < mInjectorList.getSize(); i++)
			{
				BasicParticleInjector* inj = DYNAMIC_CAST(BasicParticleInjector*)(mInjectorList.getResource(i));

				IosNewObject obj;
				while (inj->mInjectedParticles.popFront(obj))
				{
					;
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////////

		BasicParticleInjector::BasicParticleInjector(ResourceList& list, BasicIosActorImpl& actor, uint32_t injectorID)
			: mIosActor(&actor)
			, mIofxClient(NULL)
			, mVolume(NULL)
			, mLastRandomID(0)
			, mVolumeID(IofxActorIDIntl::NO_VOLUME)
			, mInjectorID(injectorID)
			, mSimulatedParticlesCount(0)
		{
			mRand.setSeed(actor.mBasicIosScene->getApexScene().getSeed());

			list.add(*this);

			setLODWeights(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

			mInjectedParticles.reserve(actor.mMaxTotalParticleCount);
		}

		BasicParticleInjector::~BasicParticleInjector()
		{
		}

		void BasicParticleInjector::setListIndex(ResourceList& list, uint32_t index)
		{
			m_listIndex = index;
			m_list = &list;

			InjectorParams injParams;
			mIosActor->mBasicIosScene->fetchInjectorParams(mInjectorID, injParams);

			injParams.mLocalIndex = index;

			mIosActor->mBasicIosScene->updateInjectorParams(mInjectorID, injParams);
		}


		/* Emitter calls this function to adjust their particle weights with respect to other emitters */
		void BasicParticleInjector::setLODWeights(float maxDistance, float distanceWeight, float speedWeight, float lifeWeight, float separationWeight, float bias)
		{
			PX_UNUSED(separationWeight);

			InjectorParams injParams;
			mIosActor->mBasicIosScene->fetchInjectorParams(mInjectorID, injParams);

			//normalize weights
			float totalWeight = distanceWeight + speedWeight + lifeWeight;
			if (totalWeight > PX_EPS_F32)
			{
				distanceWeight /= totalWeight;
				speedWeight /= totalWeight;
				lifeWeight /= totalWeight;
			}

			injParams.mLODMaxDistance = maxDistance;
			injParams.mLODDistanceWeight = distanceWeight;
			injParams.mLODSpeedWeight = speedWeight;
			injParams.mLODLifeWeight = lifeWeight;
			injParams.mLODBias = bias;

			mIosActor->mBasicIosScene->updateInjectorParams(mInjectorID, injParams);
		}


		PxTaskID BasicParticleInjector::getCompletionTaskID() const
		{
			return mIosActor->mInjectTask->getTaskID();
		}

		void BasicParticleInjector::setObjectScale(float objectScale)
		{
			PX_ASSERT(mIofxClient);
			IofxManagerClientIntl::Params params;
			mIofxClient->getParams(params);
			params.objectScale = objectScale;
			mIofxClient->setParams(params);
		}

		void BasicParticleInjector::init(IofxAsset* iofxAsset)
		{
			mIofxClient = mIosActor->mIofxMgr->createClient(iofxAsset, IofxManagerClientIntl::Params());

			/* add this injector to the IOFX asset's context (so when the IOFX goes away our ::release() is called) */
			iofxAsset->addDependentActor(this);

			mRandomActorClassIDs.clear();
			if (iofxAsset->getMeshAssetCount() < 2)
			{
				mRandomActorClassIDs.pushBack(mIosActor->mIofxMgr->getActorClassID(mIofxClient, 0));
				return;
			}

			/* Cache actorClassIDs for this asset */
			physx::Array<uint16_t> temp;
			for (uint32_t i = 0 ; i < iofxAsset->getMeshAssetCount() ; i++)
			{
				uint32_t w = iofxAsset->getMeshAssetWeight(i);
				uint16_t acid = mIosActor->mIofxMgr->getActorClassID(mIofxClient, (uint16_t) i);
				for (uint32_t j = 0 ; j < w ; j++)
				{
					temp.pushBack(acid);
				}
			}

			mRandomActorClassIDs.reserve(temp.size());
			while (temp.size())
			{
				uint32_t index = (uint32_t)mRand.getScaled(0, (float)temp.size());
				mRandomActorClassIDs.pushBack(temp[ index ]);
				temp.replaceWithLast(index);
			}
		}


		void BasicParticleInjector::release()
		{
			if (mInRelease)
			{
				return;
			}
			mInRelease = true;
			mIosActor->releaseInjector(*this);
		}

		void BasicParticleInjector::destroy()
		{
			ApexActor::destroy();

			mIosActor->mIofxMgr->releaseClient(mIofxClient);

			delete this;
		}

		void BasicParticleInjector::setPreferredRenderVolume(nvidia::apex::RenderVolume* volume)
		{
			mVolume = volume;
			mVolumeID = mVolume ? mIosActor->mIofxMgr->getVolumeID(mVolume) : IofxActorIDIntl::NO_VOLUME;
		}

		/* Emitter calls this virtual injector API to insert new particles.  It is safe for an emitter to
		* call this function at any time except for during the IOS::fetchResults().  Since
		* ParticleScene::fetchResults() is single threaded, it should be safe to call from
		* emitter::fetchResults() (destruction may want to do this because of contact reporting)
		*/
		void BasicParticleInjector::createObjects(uint32_t count, const IosNewObject* createList)
		{
			PX_PROFILE_ZONE("BasicIosCreateObjects", GetInternalApexSDK()->getContextId());

			if (mRandomActorClassIDs.size() == 0)
			{
				return;
			}

			PxVec3 eyePos;
			{
				SceneIntl& apexScene = mIosActor->mBasicIosScene->getApexScene();
				READ_LOCK(apexScene);
				eyePos = apexScene.getEyePosition();
			}
			InjectorParams injParams;
			mIosActor->mBasicIosScene->fetchInjectorParams(mInjectorID, injParams);
			// Append new objects to our FIFO.  We do copies because we must perform buffering for the
			// emitters.  We have to hold these new objects until there is room in the TurbulenceFS and the
			// injector's virtID range to emit them.
			for (uint32_t i = 0 ; i < count ; i++)
			{
				if (mInjectedParticles.size() == mInjectedParticles.capacity())
				{
					break;
				}

				IosNewObject obj = *createList++;

				obj.lodBenefit = calcParticleBenefit(injParams, eyePos, obj.initialPosition, obj.initialVelocity, 1.0f);
				obj.iofxActorID.set(mVolumeID, mRandomActorClassIDs[ mLastRandomID++ ]);
				mLastRandomID = mLastRandomID == mRandomActorClassIDs.size() ? 0 : mLastRandomID;
				//mInjectedParticleBenefit += obj.lodBenefit;
				mInjectedParticles.pushBack(obj);
			}
		}

#if APEX_CUDA_SUPPORT
		void BasicParticleInjector::createObjects(ApexMirroredArray<const IosNewObject>& createArray)
		{
			PX_UNUSED(createArray);

			// An emitter will call this API when it has filled a host or device buffer.  The injector
			// should trigger a copy to the location it would like to see the resulting data when the
			// IOS is finally ticked.

			PX_ALWAYS_ASSERT(); /* Not yet supported */
		}
#endif

	}
} // namespace nvidia


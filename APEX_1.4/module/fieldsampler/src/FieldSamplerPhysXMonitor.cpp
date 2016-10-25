/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"

#if PX_PHYSICS_VERSION_MAJOR == 3

#include "ScopedPhysXLock.h"

#include "PsSort.h"
#include "FieldSamplerPhysXMonitor.h"
#include "FieldSamplerScene.h"
#include "FieldSamplerManager.h"
#include "FieldSamplerQueryIntl.h"

#include "extensions/PxShapeExt.h"

namespace nvidia
{
namespace fieldsampler
{

	using namespace physx;

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

FieldSamplerPhysXMonitor::FieldSamplerPhysXMonitor(FieldSamplerScene& scene)
	: mFieldSamplerScene(&scene)
	, mNumPS(0)
	, mNumRB(0)
	, mEnable(false)
	, mTaskRunAfterActorUpdate(*this)
{
	mFilterData.setToDefault();

	mScene = mFieldSamplerScene->getModulePhysXScene();
	mParams = static_cast<FieldSamplerPhysXMonitorParams*>(GetInternalApexSDK()->getParameterizedTraits()->createNvParameterized(FieldSamplerPhysXMonitorParams::staticClassName()));
	mFieldSamplerManager = DYNAMIC_CAST(FieldSamplerManager*)(mFieldSamplerScene->getManager());

	mRBIndex.reserve(mParams->maxRBCount);
	mParticleSystems.resize(mParams->maxPSCount);

	mPSOutField.resize(mParams->maxParticleCount);
	mOutVelocities.resize(mParams->maxParticleCount);
	mOutIndices.resize(mParams->maxParticleCount);

	mRBOutField.resize(mParams->maxRBCount);

	mRBActors.resize(mParams->maxRBCount);
}


FieldSamplerPhysXMonitor::~FieldSamplerPhysXMonitor()
{
	for (uint32_t i = 0; i < mPSFieldSamplerQuery.size(); i++)
	{
		if (mPSFieldSamplerQuery[i])
		{
			mPSFieldSamplerQuery[i]->release();
		}
	}
	for (uint32_t i = 0; i < mRBFieldSamplerQuery.size(); i++)
	{
		if (mRBFieldSamplerQuery[i])
		{
			mRBFieldSamplerQuery[i]->release();
		}
	}
	if(mParams)
	{
		mParams->destroy();
	}
}


void FieldSamplerPhysXMonitor::setPhysXScene(PxScene* scene)
{
	mScene = scene;
}


void FieldSamplerPhysXMonitor::getParticles(uint32_t taskId)
{
	SCOPED_PHYSX_LOCK_READ(&mFieldSamplerScene->getApexScene());
	float deltaTime = mFieldSamplerScene->getApexScene().getPhysXSimulateTime();
	mPCount = 0;
	mNumPS = mScene->getActors(physx::PxActorTypeFlag::ePARTICLE_SYSTEM, &mParticleSystems[0], mParams->maxPSCount);
	for(uint32_t i = 0; i < mNumPS; i++)
	if (!mFieldSamplerManager->isUnhandledParticleSystem(mParticleSystems[i]) && mParticleSystems[i]->is<PxParticleBase>())
	{
		if (mPSFieldSamplerQuery.size() == i)
		{
			FieldSamplerQueryDescIntl queryDesc;
			queryDesc.maxCount = mParams->maxParticleCount;
			queryDesc.samplerFilterData = mFilterData;
			mPSFieldSamplerQuery.pushBack( mFieldSamplerManager->createFieldSamplerQuery(queryDesc) );
			mPSFieldSamplerTaskID.pushBack(0);
			mParticleReadData.pushBack(0);
			mPSMass.pushBack(0.f);
		}

		PxParticleSystem* particleSystem = DYNAMIC_CAST(PxParticleSystem*)((mParticleSystems[i]));
		mParticleReadData[i] = particleSystem->lockParticleReadData();
		uint32_t numParticles;
		if (mParticleReadData[i])
		{
			numParticles = mParticleReadData[i]->validParticleRange;
			if(mPCount + numParticles >= mParams->maxParticleCount) break;
		
			FieldSamplerQueryDataIntl queryData;
			queryData.timeStep = deltaTime;
			queryData.count = numParticles;
			queryData.isDataOnDevice = false;
			
			//hack for PhysX particle stride calculation
			physx::PxStrideIterator<const PxVec3> positionIt(mParticleReadData[i]->positionBuffer);
#ifdef WIN64
			queryData.positionStrideBytes = (uint32_t)(-(int64_t)&*positionIt + (int64_t)&*(++positionIt));
#else
			queryData.positionStrideBytes = (uint32_t)(-(int32_t)&*positionIt + (int32_t)&*(++positionIt));
#endif
			queryData.velocityStrideBytes = queryData.positionStrideBytes;
			queryData.massStrideBytes = 0;
			queryData.pmaInPosition = (float*)&*(mParticleReadData[i]->positionBuffer);
			queryData.pmaInVelocity = (float*)&*(mParticleReadData[i]->velocityBuffer);
			queryData.pmaInIndices = 0;
			mPSMass[i] = particleSystem->getParticleMass();
			queryData.pmaInMass = &mPSMass[i];
			queryData.pmaOutField = &mPSOutField[mPCount];
			mPSFieldSamplerTaskID[i] = mPSFieldSamplerQuery[i]->submitFieldSamplerQuery(queryData, taskId);

			mPCount += numParticles;
		}
	}
}


void FieldSamplerPhysXMonitor::updateParticles()
{
	uint32_t pCount = 0;
	SCOPED_PHYSX_LOCK_WRITE(mFieldSamplerScene->getApexScene().getPhysXScene());
	for(uint32_t i = 0; i < mNumPS; i++)
	if (!mFieldSamplerManager->isUnhandledParticleSystem(mParticleSystems[i]))
	{
		PxParticleSystem* particleSystem = DYNAMIC_CAST(PxParticleSystem*)((mParticleSystems[i]));
		uint32_t numParticles = PxMin(mParticleReadData[i]->validParticleRange, mParams->maxParticleCount);

		uint32_t numUpdates = 0;

		if (numParticles > 0)
		{
			for (uint32_t w = 0; w <= (mParticleReadData[i]->validParticleRange-1) >> 5; w++)
			{
				for (uint32_t b = mParticleReadData[i]->validParticleBitmap[w]; b; b &= b-1) 
				{
					uint32_t index = (w << 5 | shdfnd::lowestSetBit(b));

					PxVec3 diffVel = mPSOutField[pCount + index].getXYZ();
					if (!diffVel.isZero())
					{
						const PxVec3& sourceVelocity = mParticleReadData[i]->velocityBuffer[index];
						mOutVelocities[numUpdates] = sourceVelocity + diffVel;
						mOutIndices[numUpdates] = index;
						numUpdates++;
					}
				}
			}
		}
		pCount += numParticles;
		// return ownership of the buffers back to the SDK
		mParticleReadData[i]->unlock();
	
		if(pCount <= mParams->maxParticleCount && numUpdates > 0)
		{
			physx::PxStrideIterator<uint32_t> indices(&mOutIndices[0]);
			physx::PxStrideIterator<PxVec3> outVelocities(&mOutVelocities[0]);
			particleSystem->setVelocities(numUpdates, indices, outVelocities);
		}		
	}
}


void FieldSamplerPhysXMonitor::getRigidBodies(uint32_t taskId)
{
	SCOPED_PHYSX_LOCK_READ(&mFieldSamplerScene->getApexScene());
	float deltaTime = mFieldSamplerScene->getApexScene().getPhysXSimulateTime();

	FieldSamplerQueryDataIntl queryData;
	queryData.timeStep = deltaTime;
	queryData.pmaInIndices = 0;

	uint32_t rbCount = mScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, &mRBActors[0], mParams->maxRBCount);
	Array<PxShape*> shapes;
	mNumRB = 0;
	mRBIndex.clear();
	float weight = 1.f;

	for (uint32_t i = 0; i < rbCount; i++)
	{
		physx::PxRigidDynamic* rb = (physx::PxRigidDynamic*)mRBActors[i];
		if (rb->getRigidBodyFlags() == PxRigidBodyFlag::eKINEMATIC) 
		{
			continue;
		}
		const PxVec3& cmPos = rb->getGlobalPose().p;
		const PxVec3& velocity = rb->getLinearVelocity();
		const PxVec3& rotation = rb->getAngularVelocity();
		float mass = rb->getMass();

		const uint32_t numShapes = rb->getNbShapes();
		shapes.resize(numShapes);
		if (numShapes == 0) 
		{
			continue;
		}
		rb->getShapes(&shapes[0], numShapes);
		for (uint32_t j = 0; j < numShapes && mNumRB < mParams->maxRBCount; j++)
		{
			PxFilterData filterData = shapes[j]->getQueryFilterData();
			if (mFieldSamplerManager->getFieldSamplerGroupsFiltering(mFilterData, filterData, weight))
			{
				PxFilterData* current = mRBFilterData.find(filterData);
				if (current == mRBFilterData.end())
				{
					mRBFilterData.pushBack(filterData);
					current = &mRBFilterData.back();
					FieldSamplerQueryDescIntl queryDesc;
					queryDesc.maxCount = mParams->maxParticleCount;
					queryDesc.samplerFilterData = filterData;
					mRBFieldSamplerQuery.pushBack( mFieldSamplerManager->createFieldSamplerQuery(queryDesc) );
				}

				ShapeData* sd = PX_NEW(ShapeData)();
				sd->fdIndex = (uint32_t)(current - &mRBFilterData[0]);
				sd->rbIndex = i;
				sd->mass = mass / numShapes;
				sd->pos = PxShapeExt::getWorldBounds(*shapes[j], *rb).getCenter();
				sd->vel = velocity + rotation.cross(sd->pos - cmPos);
				mRBIndex.pushBack(sd);
				++mNumRB;
			}
		}		
	}

	if (mNumRB == 0) 
	{
		return;
	}

	
	sort(&mRBIndex[0], mNumRB, ShapeData::sortPredicate);

	mRBInPosition.resize(mNumRB);
	mRBInVelocity.resize(mNumRB);

	uint32_t current(mRBIndex[0]->fdIndex);
	uint32_t currentCount = 0;
	uint32_t fdCount = 0;
	for (uint32_t i = 0; i <= mNumRB; i++)
	{
		if (i == mNumRB || current != mRBIndex[i]->fdIndex)
		{
			queryData.count = currentCount;
			queryData.isDataOnDevice = false;
			queryData.massStrideBytes = sizeof(PxVec4);
			queryData.positionStrideBytes = sizeof(PxVec4);
			queryData.velocityStrideBytes = queryData.positionStrideBytes;

			queryData.pmaInPosition = (float*)(&mRBInPosition[fdCount]);
			queryData.pmaInVelocity = (float*)(&mRBInVelocity[fdCount]);
			queryData.pmaInMass = &(mRBInPosition[fdCount].w);
			queryData.pmaOutField = &mRBOutField[fdCount];
	
			mRBFieldSamplerQuery[current]->submitFieldSamplerQuery(queryData, taskId);
			
			fdCount += currentCount;
			if (i != mNumRB) 
			{
				current = mRBIndex[i]->fdIndex;
				currentCount = 1;
			}
		}
		else
		{
			currentCount++;
		}
		if (i < mNumRB)
		{
			mRBInPosition[i] = PxVec4 (mRBIndex[i]->pos, mRBIndex[i]->mass);
			mRBInVelocity[i] = PxVec4 (mRBIndex[i]->vel, 0.f);
		}
	}
}

void FieldSamplerPhysXMonitor::updateRigidBodies()
{
	SCOPED_PHYSX_LOCK_WRITE(&mFieldSamplerScene->getApexScene());

	for(uint32_t i = 0; i < mNumRB; i++)
	{
		physx::PxRigidDynamic* rb = (physx::PxRigidDynamic*)mRBActors[mRBIndex[i]->rbIndex];
		const PxVec3 velocity = mRBOutField[i].getXYZ();
		const PxVec3 rotation = mRBOutField[i].getXYZ().cross(mRBInPosition[i].getXYZ() - rb->getGlobalPose().p);
		if (!velocity.isZero() || !rotation.isZero())
		{
			rb->setLinearVelocity(rb->getLinearVelocity() + velocity);
			rb->setAngularVelocity(rb->getAngularVelocity() + rotation);
		}

		PX_DELETE(mRBIndex[i]);
	}
}


void FieldSamplerPhysXMonitor::update()
{	
	PxTaskManager* tm = mFieldSamplerScene->getApexScene().getTaskManager();
	uint32_t taskId = tm->getNamedTask(FSST_PHYSX_MONITOR_LOAD);
	if(mScene)
	{
		getParticles(taskId);

		getRigidBodies(taskId);

		//getCloth(task);
	}
	if(mNumPS > 0 || mNumRB > 0)
	{	
		tm->submitNamedTask(&mTaskRunAfterActorUpdate, FSST_PHYSX_MONITOR_UPDATE);
		mTaskRunAfterActorUpdate.startAfter(tm->getNamedTask(FSST_PHYSX_MONITOR_FETCH));
		mTaskRunAfterActorUpdate.finishBefore(tm->getNamedTask(AST_PHYSX_SIMULATE));
	}	
}

void FieldSamplerPhysXMonitor::updatePhysX()
{
	updateParticles();
	updateRigidBodies();
}


}
} // end namespace nvidia::apex

#endif


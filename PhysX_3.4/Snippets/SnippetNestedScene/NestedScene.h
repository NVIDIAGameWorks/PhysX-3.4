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



extern PxPhysics*				gPhysics;
extern PxDefaultCpuDispatcher*	gDispatcher;
extern PxMaterial*				gMaterial;

#ifdef RENDER_SNIPPET
void renderScene(physx::PxScene *, physx::PxRigidActor * frame, const physx::PxVec3 & color);
#endif

class NestedScene : public PxSimulationEventCallback
{
public:
	NestedScene(PxRigidDynamic* containingActor) : mContainingActor(containingActor), lastLVel(PxZero), lastAVel(PxZero)
	{
		//create contained scene
		PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
		sceneDesc.cpuDispatcher = gDispatcher;
		sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
		sceneDesc.simulationEventCallback = this;	
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		mScene = gPhysics->createScene(sceneDesc);

		//create the static environment of the contained scene out of the containing actor's geometry
		//this could be any other arbitrary geometry too though


		{
			//the truck bed
			PxShape* shape = gPhysics->createShape(PxBoxGeometry(1.8f, 0.25f, 4.5f), *gMaterial);
			PxRigidStatic * staticActor = gPhysics->createRigidStatic(PxTransform(PxVec3(0.0f, 1.0f, 0.0f)));
			staticActor->attachShape(*shape);
			mScene->addActor(*staticActor);

		}

		{

			//the cabin of the truck
			PxShape* shape = gPhysics->createShape(PxBoxGeometry(1.7f, 0.5f, 0.9f), *gMaterial);
			PxRigidStatic * staticActor = gPhysics->createRigidStatic(PxTransform(PxVec3(0.0f, 1.75f, 2.5f)));
			staticActor->attachShape(*shape);
			mScene->addActor(*staticActor);
		}


		{
			//some detail thingie on the cabin
			PxShape* shape = gPhysics->createShape(PxSphereGeometry(0.5f), *gMaterial);
			PxRigidStatic * staticActor = gPhysics->createRigidStatic(PxTransform(PxVec3(0.6f, 2.25f, 2.75f)));
			staticActor->attachShape(*shape);
			mScene->addActor(*staticActor);
		}

		{
			//a trigger shape around the truck bed.  If shapes leave this, they move to the main scene.
			PxShape* shape = gPhysics->createShape(PxBoxGeometry(1.8f, 3.0f, 4.5f), *gMaterial);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			PxRigidStatic * staticActor = gPhysics->createRigidStatic(PxTransform(PxVec3(0.0f, 1.0f, 0.0f)));
			staticActor->attachShape(*shape);
			mScene->addActor(*staticActor);

		}





	}

	~NestedScene()
	{

	}


	void createBoxStack()
	{
		//create a box stack inside the scene

		createStack(PxTransform(PxVec3(0.0f, 3.0f, 0.0f)), 3, 0.4f);

		//TODO
	}

	void simulate(PxScene * , PxF32 timeStep)
	{
		//1) mirror reference actor accelerations into scene

		if (!mContainingActor->isSleeping())
		{
			PxVec3 lvel = mContainingActor->getLinearVelocity();
			PxVec3 avel = mContainingActor->getAngularVelocity();

			//note that this is actually -acc.
			PxVec3 lacc = lastLVel - lvel;
			//PxVec3 aacc = lastAVel - avel;

			lastLVel = lvel;
			lastAVel = avel;
	

			//special sauce:
			//filter out vertical movement due to suspension travel, otherwise the ride is too bumpy
			lacc.y = 0.0f;
			//decrease the rest of the acceleration by a bit
			lacc *= 0.7f;


			if (lacc.magnitudeSquared() > 0.01f)	//let things sleep if the accel is too small
			{
				PxU32 nbActors = mScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
				if(nbActors)
				{
					std::vector<PxActor*> actors(nbActors);
					mScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, &actors[0], nbActors);

					for(PxU32 i=0;i<nbActors;i++)
					{
						actors[i]->is<PxRigidBody>()->addForce(lacc, PxForceMode::eVELOCITY_CHANGE);
					}
				}
			}



		}
		else
		{
			lastLVel = PxVec3(PxZero);
			lastAVel = PxVec3(PxZero);
		}

		//2) simulate scene
		mScene->simulate(timeStep);
		mScene->fetchResults(true);

		//move actors that have dropped off the truck out into the main scene

		for(PxU32 i=0;i<removalQueue.size();i++)
		{
			mScene->removeActor(*removalQueue[i]);

			//transform to the parent frame:
			PxTransform localPose = removalQueue[i]->getGlobalPose();
			PxTransform parentFrame = mContainingActor->getGlobalPose();
			removalQueue[i]->setGlobalPose(parentFrame * localPose);
			mContainingActor->getScene()->addActor(*removalQueue[i]);
		
		}
		removalQueue.clear();

		

	}

	void render()
	{
#ifdef RENDER_SNIPPET
		renderScene(mScene, mContainingActor, PxVec3(0.54f, 0.85f, 0.1f));
#endif
	}

	//simulation event callback -- we only need trigger:
	void onConstraintBreak(PxConstraintInfo* , PxU32 )		{  }
	void onWake(PxActor** , PxU32 )							{ }
	void onSleep(PxActor** , PxU32 )						{  }
	void onContact(const PxContactPairHeader& , const PxContactPair* , PxU32 ) { }
	void onTrigger(PxTriggerPair* pairs, PxU32 count)		
	{
		for(PxU32 i=0; i < count; i++)
		{
			// ignore pairs when shapes have been deleted
			if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				//we're not allowed to make changes in the callback, so save it for later
				removalQueue.push_back(pairs[i].otherActor);
			}

		}

	}
	void onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}

private:

	void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
	{
		PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);

		PxFilterData simFilterData;
		simFilterData.word0 = snippetvehicle::COLLISION_FLAG_OBSTACLE;
		simFilterData.word1 = snippetvehicle::COLLISION_FLAG_OBSTACLE_AGAINST;
		shape->setSimulationFilterData(simFilterData);

		for(PxU32 i=0; i<size;i++)
		{
			for(PxU32 j=0;j<size-i;j++)
			{
				PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size-i), PxReal(i*2+1), 0) * halfExtent);
				PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
				body->attachShape(*shape);
				PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
				mScene->addActor(*body);
			}
		}
		shape->release();
	}



	PxScene* mScene;
	PxRigidDynamic* mContainingActor;
	PxVec3 lastLVel;
	PxVec3 lastAVel;
	std::vector<PxRigidActor*> removalQueue;
};


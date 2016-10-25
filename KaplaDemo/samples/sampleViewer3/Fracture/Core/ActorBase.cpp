#include "RTdef.h"
#if RT_COMPILE
#include "ActorBase.h"

#include <foundation/PxMat44.h>
#include "PxRigidBodyExt.h"

#include "PxScene.h"
#include "SimSceneBase.h"
#include "CompoundBase.h"

namespace physx
{
namespace fracture
{
namespace base
{

Actor::Actor(SimScene* scene):
	mScene(scene),
	mMinConvexSize(scene->mMinConvexSize),
	mDepthLimit(100),
	mDestroyIfAtDepthLimit(false)
{

}

Actor::~Actor()
{
	mScene->getScene()->lockWrite();
	clear();
	mScene->getScene()->unlockWrite();
	mScene->removeActor(this);
}

void Actor::clear()
{
	for (int i = 0; i < (int)mCompounds.size(); i++) {
		PX_DELETE(mCompounds[i]);
	}
	mCompounds.clear();
}

void Actor::addCompound(Compound *c)
{
	mCompounds.pushBack(c);
	PxRigidDynamic *a = c->getPxActor();
#if 1
    if (a) {
//		a->setContactReportFlags(Px_NOTIFY_ON_TOUCH_FORCE_THRESHOLD | Px_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD);
		a->setContactReportThreshold(mScene->mFractureForceThreshold);
    }
#endif
	c->mActor = this;
	++(mScene->mSceneVersion);
}

void Actor::removeCompound(Compound *c)
{
	int num = 0;
	for (int i = 0; i < (int)mCompounds.size(); i++) {
		if (mCompounds[i] != c) {
			mCompounds[num] = mCompounds[i];
			num++;
		}
	}
	if (mScene->mPickActor == c->getPxActor())
		mScene->mPickActor = NULL;

	c->clear();
	//delCompoundList.push_back(c);
	//delete c;
	mScene->delCompoundList.pushBack(c);
	mCompounds.resize(num);
	++mScene->mSceneVersion;
}

void Actor::preSim(float dt)
{
	int num = 0;
	for (int i = 0; i < (int)mCompounds.size(); i++) {
		mCompounds[i]->step(dt);
		if (mCompounds[i]->getLifeFrames() == 0) {
			mCompounds[i]->clear();
			//delCompoundList.push_back(mCompounds[i]);
			//delete mCompounds[i];
			mScene->delCompoundList.pushBack(mCompounds[i]);
		}
		else {
			mCompounds[num] = mCompounds[i];
			num++;
		}
	}
	mCompounds.resize(num);
}

void Actor::postSim(float /*dt*/)
{
}

bool Actor::rayCast(const PxVec3 &orig, const PxVec3 &dir, float &dist, int &compoundNr, int &convexNr, PxVec3 &normal) const
{
	dist = PX_MAX_F32;
	compoundNr = -1;
	convexNr = -1;

	for (int i = 0; i < (int)mCompounds.size(); i++) {
		float d;
		int cNr;
		PxVec3 n;
		if (mCompounds[i]->rayCast(orig, dir, d, cNr, n)) {
			if (d < dist) {
				dist = d;
				compoundNr = i;
				convexNr = cNr;
				normal = n;
			}
		}
	}
	return compoundNr >= 0;
}

bool Actor::findCompound(const Compound* c, int& compoundNr)
{
	for(int i = 0; i < (int)mCompounds.size(); i++)
	{
		if(mCompounds[i] == c)
		{
			compoundNr = i;
			return true;
		}
	}
	return false;
}

}
}
}
#endif
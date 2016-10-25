#include "RTdef.h"
#if RT_COMPILE
#ifndef ACTOR_BASE_H
#define ACTOR_BASE_H

#include <PsArray.h>
#include <PsUserAllocated.h>

namespace physx
{
namespace fracture
{
namespace base
{

class Compound;

class Actor : public ::physx::shdfnd::UserAllocated
{
	friend class SimScene;
	friend class Compound;
protected:
	Actor(SimScene* scene);
public:
	virtual ~Actor();

	void clear();
	void addCompound(Compound *m);
	void removeCompound(Compound *m);

	bool findCompound(const Compound* c, int& compoundNr);

	void preSim(float dt);
	void postSim(float dt);

	bool rayCast(const PxVec3 &orig, const PxVec3 &dir, float &dist, int &compoundNr, int &convexNr, PxVec3 &normal) const;

	shdfnd::Array<Compound*> getCompounds() { return mCompounds; }

protected:
	SimScene* mScene;
	shdfnd::Array<Compound*> mCompounds;

	PxF32 mMinConvexSize;
	PxU32 mDepthLimit;
	bool mDestroyIfAtDepthLimit;
};

}
}
}

#endif
#endif
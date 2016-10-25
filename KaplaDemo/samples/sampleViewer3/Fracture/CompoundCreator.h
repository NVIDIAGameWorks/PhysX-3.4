#ifndef COMPOUND_CREATOR_H
#define COMPOUND_CREATOR_H

// Matthias Müller-Fischer

#include <foundation/PxVec3.h>
#include <foundation/PxTransform.h>
#include <PsArray.h>

#include "CompoundCreatorBase.h"

using namespace physx::fracture;

class CompoundCreator : public base::CompoundCreator
{
	friend class SimScene;
public:
	virtual void debugDraw();
protected:
	CompoundCreator(SimScene* scene): base::CompoundCreator((base::SimScene*)scene) {}
};

#endif

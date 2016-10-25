#include "Actor.h"

#include <foundation/PxMat44.h>
#include "PxRigidBodyExt.h"
#include "PxPhysics.h"
#include "PxCooking.h"
#include "PxShape.h"

#include "SimScene.h"
#include "Compound.h"

Actor::Actor(base::SimScene* scene):
	base::Actor(scene)
{

}

Actor::~Actor()
{

}
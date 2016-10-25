#ifndef RT_ACTOR_H
#define RT_ACTOR_H

//#include <PxFoundation.h>

#include "ActorBase.h"

class RegularCell3D;

using namespace physx;
using namespace physx::fracture;

class Actor : public base::Actor
{
	friend class SimScene;
protected:
	Actor(base::SimScene* scene);
public:
	virtual ~Actor();

protected:

};

#endif
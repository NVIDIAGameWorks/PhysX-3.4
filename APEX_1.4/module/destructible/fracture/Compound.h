/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef COMPOUND_H
#define COMPOUND_H

#include "CompoundBase.h"

namespace nvidia
{
namespace fracture
{

class Compound : public base::Compound
{
	friend class SimScene;
	friend class Actor;
protected:
	Compound(SimScene* scene, const base::FracturePattern *pattern, const base::FracturePattern *secondaryPattern = NULL, float contactOffset = 0.005f, float restOffset = -0.001f):
		nvidia::fracture::base::Compound((base::SimScene*)scene,pattern,secondaryPattern,contactOffset,restOffset) {}
public:
	virtual void applyShapeTemplate(PxShape* shape);
	virtual ::nvidia::destructible::DestructibleActorImpl* getDestructibleActor() const;
};

}
}


#endif
#endif
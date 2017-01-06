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
#ifndef RT_ACTOR_H
#define RT_ACTOR_H

#include "ActorBase.h"

namespace nvidia
{
namespace destructible
{
	class DestructibleActorImpl;
	struct DamageEvent;
	struct FractureEvent;
}

using namespace destructible;
namespace fracture
{

class Compound;
class FracturePattern;
class Renderable;

class Actor : public base::Actor
{
	friend class SimScene;
	friend class Renderable;
protected:
	Actor(base::SimScene* scene, DestructibleActorImpl* actor);
public:
	virtual ~Actor();

	Compound* createCompound();

	Compound* createCompoundFromChunk(const destructible::DestructibleActorImpl& destructibleActor, uint32_t partIndex);

	bool patternFracture(const PxVec3& hitLocation, const PxVec3& dir, float scale = 1.f, float vel = 0.f, float radius = 0.f);
	bool patternFracture(const DamageEvent& damageEvent);
	bool patternFracture(const FractureEvent& fractureEvent, bool fractureOnLoad = true);

	bool rayCast(const PxVec3& orig, const PxVec3& dir, float &dist) const;

	DestructibleActorImpl* getDestructibleActor() {return mActor;}

protected:
	void attachBasedOnFlags(base::Compound* c);

	FracturePattern* mDefaultFracturePattern;
	DestructibleActorImpl* mActor;
	bool mRenderResourcesDirty;

	float mMinRadius;
	float mRadiusMultiplier;
	float mImpulseScale;
	bool mSheetFracture;

	struct AttachmentFlags
	{
		AttachmentFlags() :
			posX(0), negX(0), posY(0), negY(0), posZ(0), negZ(0) {}

		uint32_t posX : 1;
		uint32_t negX : 1;
		uint32_t posY : 1;
		uint32_t negY : 1;
		uint32_t posZ : 1;
		uint32_t negZ : 1;
	}mAttachmentFlags;

	struct MyDamageEvent
	{
		PxVec3	position;
		PxVec3	direction;
		float	damage;
		float	radius;
	};
	MyDamageEvent mDamageEvent;

};

}
}

#endif
#endif
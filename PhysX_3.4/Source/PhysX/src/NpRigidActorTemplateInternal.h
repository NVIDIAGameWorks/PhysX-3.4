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

#ifndef PX_PHYSICS_NP_RIGIDACTOR_TEMPLATE_INTERNAL
#define PX_PHYSICS_NP_RIGIDACTOR_TEMPLATE_INTERNAL

namespace physx
{

template<class T, class T2>
static PX_FORCE_INLINE void releaseActorT(NpRigidActorTemplate<T>* actor, T2& scbActor)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*actor));

	NpPhysics::getInstance().notifyDeletionListenersUserRelease(actor, actor->userData);

	Scb::Scene* s = scbActor.getScbSceneForAPI();

	const bool noSim = scbActor.isSimDisabledInternally();
	// important to check the non-buffered flag because it tells what the current internal state of the object is
	// (someone might switch to non-simulation and release all while the sim is running). Reading is fine even if 
	// the sim is running because actor flags are read-only internally.
	if(s && noSim)
	{
		// need to do it here because the Np-shape buffer will not be valid anymore after the release below
		// and unlike simulation objects, there is no shape buffer in the simulation controller
		actor->getShapeManager().clearShapesOnRelease(*s, *actor);
	}

	actor->NpRigidActorTemplate<T>::release();

	if(s)
	{
		s->removeActor(scbActor, true, noSim);
		static_cast<NpScene*>(s->getPxScene())->removeFromRigidActorList(actor->getRigidActorArrayIndex());
	}

	scbActor.destroy();
}

}

#endif

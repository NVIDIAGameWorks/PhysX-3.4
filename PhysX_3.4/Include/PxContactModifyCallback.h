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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_CONTACT_MODIFY_CALLBACK
#define PX_CONTACT_MODIFY_CALLBACK
/** \addtogroup physics
@{
*/

#include "PxPhysXConfig.h"
#include "PxShape.h"
#include "PxContact.h"
#include "foundation/PxTransform.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxShape;

/**
\brief An array of contact points, as passed to contact modification.

The word 'set' in the name does not imply that duplicates are filtered in any 
way.  This initial set of contacts does potentially get reduced to a smaller 
set before being passed to the solver.

You can use the accessors to read and write contact properties.  The number of 
contacts is immutable, other than being able to disable contacts using ignore().

@see PxContactModifyCallback, PxModifiableContact
*/
class PxContactSet
{
public:
	/**
	\brief Get the position of a specific contact point in the set.

	@see PxModifiableContact.point
	*/
	PX_FORCE_INLINE		const PxVec3& getPoint(PxU32 i) const			{ return mContacts[i].contact;		}

	/**
	\brief Alter the position of a specific contact point in the set.

	@see PxModifiableContact.point
	*/
	PX_FORCE_INLINE		void setPoint(PxU32 i, const PxVec3& p)			{ mContacts[i].contact = p; }

	/**
	\brief Get the contact normal of a specific contact point in the set.

	@see PxModifiableContact.normal
	*/
	PX_FORCE_INLINE		const PxVec3& getNormal(PxU32 i) const			{ return mContacts[i].normal;	}

	/**
	\brief Alter the contact normal of a specific contact point in the set.

	\note Changing the normal can cause contact points to be ignored.

	@see PxModifiableContact.normal
	*/
	PX_FORCE_INLINE		void setNormal(PxU32 i, const PxVec3& n)		
	{ 
		PxContactPatch* patch = getPatch();
		patch->internalFlags |= PxContactPatch::eREGENERATE_PATCHES;
		mContacts[i].normal = n;
	}

	/**
	\brief Get the separation of a specific contact point in the set.

	@see PxModifiableContact.separation
	*/
	PX_FORCE_INLINE		PxReal getSeparation(PxU32 i) const				{ return mContacts[i].separation;	}

	/**
	\brief Alter the separation of a specific contact point in the set.

	@see PxModifiableContact.separation
	*/
	PX_FORCE_INLINE		void setSeparation(PxU32 i, PxReal s)			{ mContacts[i].separation = s; }

	/**
	\brief Get the target velocity of a specific contact point in the set.

	@see PxModifiableContact.targetVelocity

	*/
	PX_FORCE_INLINE		const PxVec3& getTargetVelocity(PxU32 i) const	{ return mContacts[i].targetVelocity;	}

	/**
	\brief Alter the target velocity of a specific contact point in the set.

	@see PxModifiableContact.targetVelocity
	*/
	PX_FORCE_INLINE		void setTargetVelocity(PxU32 i, const PxVec3& v)
	{ 
		PxContactPatch* patch = getPatch();
		patch->internalFlags |= PxContactPatch::eHAS_TARGET_VELOCITY;
		mContacts[i].targetVelocity = v;
	}

	/**
	\brief Get the face index with respect to the first shape of the pair for a specific contact point in the set.

	@see PxModifiableContact.internalFaceIndex0
	*/
	PX_FORCE_INLINE		PxU32 getInternalFaceIndex0(PxU32 i)			{ PX_UNUSED(i); return PXC_CONTACT_NO_FACE_INDEX; }

	/**
	\brief Get the face index with respect to the second shape of the pair for a specific contact point in the set.

	@see PxModifiableContact.internalFaceIndex1
	*/
	PX_FORCE_INLINE		PxU32 getInternalFaceIndex1(PxU32 i)			
	{
		PxContactPatch* patch = getPatch();
		if (patch->internalFlags & PxContactPatch::eHAS_FACE_INDICES)
		{
			return reinterpret_cast<PxU32*>(mContacts + mCount)[mCount + i];
		}
		return PXC_CONTACT_NO_FACE_INDEX;
	}

	/**
	\brief Get the maximum impulse for a specific contact point in the set.

	@see PxModifiableContact.maxImpulse
	*/
	PX_FORCE_INLINE		PxReal getMaxImpulse(PxU32 i) const				{ return mContacts[i].maxImpulse;	}

	/**
	\brief Alter the maximum impulse for a specific contact point in the set.

	\note Must be nonnegative. If set to zero, the contact point will be ignored

	@see PxModifiableContact.maxImpulse
	*/
	PX_FORCE_INLINE		void setMaxImpulse(PxU32 i, PxReal s)			
	{
		PxContactPatch* patch = getPatch();
		patch->internalFlags |= PxContactPatch::eHAS_MAX_IMPULSE;
		mContacts[i].maxImpulse = s; 
	}

	/**
	\brief Get the restitution coefficient for a specific contact point in the set.

	@see PxModifiableContact.restitution
	*/
	PX_FORCE_INLINE		PxReal getRestitution(PxU32 i) const			{ return mContacts[i].restitution; }

	/**
	\brief Alter the restitution coefficient for a specific contact point in the set.

	\note Valid ranges [0,1]

	@see PxModifiableContact.restitution
	*/
	PX_FORCE_INLINE		void setRestitution(PxU32 i, PxReal r)		
	{
		PxContactPatch* patch = getPatch();
		patch->internalFlags |= PxContactPatch::eREGENERATE_PATCHES;
		mContacts[i].restitution = r; 
	}

	/**
	\brief Get the static friction coefficient for a specific contact point in the set.

	@see PxModifiableContact.staticFriction
	*/
	PX_FORCE_INLINE		PxReal getStaticFriction(PxU32 i) const { return mContacts[i].staticFriction; }

	/**
	\brief Alter the static friction coefficient for a specific contact point in the set.

	@see PxModifiableContact.staticFriction
	*/
	PX_FORCE_INLINE		void setStaticFriction(PxU32 i, PxReal f) 
	{ 
		PxContactPatch* patch = getPatch();
		patch->internalFlags |= PxContactPatch::eREGENERATE_PATCHES;
		mContacts[i].staticFriction = f; 
	}

	/**
	\brief Get the static friction coefficient for a specific contact point in the set.

	@see PxModifiableContact.dynamicFriction
	*/
	PX_FORCE_INLINE		PxReal getDynamicFriction(PxU32 i) const { return mContacts[i].dynamicFriction; }

	/**
	\brief Alter the static dynamic coefficient for a specific contact point in the set.

	@see PxModifiableContact.dynamic
	*/
	PX_FORCE_INLINE		void setDynamicFriction(PxU32 i, PxReal f) 
	{
		PxContactPatch* patch = getPatch();
		patch->internalFlags |= PxContactPatch::eREGENERATE_PATCHES; 
		mContacts[i].dynamicFriction = f; 
	}

	/**
	\brief Ignore the contact point.

	If a contact point is ignored then no force will get applied at this point. This can be used to disable collision in certain areas of a shape, for example.
	*/
	PX_FORCE_INLINE		void ignore(PxU32 i)							{ mContacts[i].maxImpulse = 0.f; }

	/**
	\brief The number of contact points in the set.
	*/
	PX_FORCE_INLINE		PxU32 size() const								{ return mCount; }

	/**
	\brief Returns the invMassScale of body 0

	A value < 1.0 makes this contact treat the body as if it had larger mass. A value of 0.f makes this contact
	treat the body as if it had infinite mass. Any value > 1.f makes this contact treat the body as if it had smaller mass.
	*/
	PX_FORCE_INLINE		PxReal getInvMassScale0() const					
	{ 
		PxContactPatch* patch = getPatch();
		return patch->mMassModification.mInvMassScale0;
	}

	/**
	\brief Returns the invMassScale of body 1

	A value < 1.0 makes this contact treat the body as if it had larger mass. A value of 0.f makes this contact
	treat the body as if it had infinite mass. Any value > 1.f makes this contact treat the body as if it had smaller mass.
	*/
	PX_FORCE_INLINE		PxReal getInvMassScale1() const					
	{ 
		PxContactPatch* patch = getPatch();
		return patch->mMassModification.mInvMassScale1;
	}

	/**
	\brief Returns the invInertiaScale of body 0

	A value < 1.0 makes this contact treat the body as if it had larger inertia. A value of 0.f makes this contact
	treat the body as if it had infinite inertia. Any value > 1.f makes this contact treat the body as if it had smaller inertia.
	*/
	PX_FORCE_INLINE		PxReal getInvInertiaScale0() const					
	{ 
		PxContactPatch* patch = getPatch();
		return patch->mMassModification.mInvInertiaScale0;
	}

	/**
	\brief Returns the invInertiaScale of body 1

	A value < 1.0 makes this contact treat the body as if it had larger inertia. A value of 0.f makes this contact
	treat the body as if it had infinite inertia. Any value > 1.f makes this contact treat the body as if it had smaller inertia.
	*/
	PX_FORCE_INLINE		PxReal getInvInertiaScale1() const					
	{ 
		PxContactPatch* patch = getPatch();
		return patch->mMassModification.mInvInertiaScale1;
	}

	/**
	\brief Sets the invMassScale of body 0

	This can be set to any value in the range [0, PX_MAX_F32). A value < 1.0 makes this contact treat the body as if it had larger mass. A value of 0.f makes this contact
	treat the body as if it had infinite mass. Any value > 1.f makes this contact treat the body as if it had smaller mass.
	*/
	PX_FORCE_INLINE		void setInvMassScale0(const PxReal scale)					
	{ 
		PxContactPatch* patch = getPatch();
		patch->mMassModification.mInvMassScale0 = scale;
		patch->internalFlags |= PxContactPatch::eHAS_MODIFIED_MASS_RATIOS;
	}

	/**
	\brief Sets the invMassScale of body 1

	This can be set to any value in the range [0, PX_MAX_F32). A value < 1.0 makes this contact treat the body as if it had larger mass. A value of 0.f makes this contact
	treat the body as if it had infinite mass. Any value > 1.f makes this contact treat the body as if it had smaller mass.
	*/
	PX_FORCE_INLINE		void setInvMassScale1(const PxReal scale)					
	{ 
		PxContactPatch* patch = getPatch();
		patch->mMassModification.mInvMassScale1 = scale;
		patch->internalFlags |= PxContactPatch::eHAS_MODIFIED_MASS_RATIOS;
	}

	/**
	\brief Sets the invInertiaScale of body 0

	This can be set to any value in the range [0, PX_MAX_F32). A value < 1.0 makes this contact treat the body as if it had larger inertia. A value of 0.f makes this contact
	treat the body as if it had infinite inertia. Any value > 1.f makes this contact treat the body as if it had smaller inertia.
	*/
	PX_FORCE_INLINE		void setInvInertiaScale0(const PxReal scale)					
	{ 
		PxContactPatch* patch = getPatch();
		patch->mMassModification.mInvInertiaScale0 = scale;
		patch->internalFlags |= PxContactPatch::eHAS_MODIFIED_MASS_RATIOS;
	}

	/**
	\brief Sets the invInertiaScale of body 1

	This can be set to any value in the range [0, PX_MAX_F32). A value < 1.0 makes this contact treat the body as if it had larger inertia. A value of 0.f makes this contact
	treat the body as if it had infinite inertia. Any value > 1.f makes this contact treat the body as if it had smaller inertia.
	*/
	PX_FORCE_INLINE		void setInvInertiaScale1(const PxReal scale)					
	{ 
		PxContactPatch* patch = getPatch();
		patch->mMassModification.mInvInertiaScale1 = scale;
		patch->internalFlags |= PxContactPatch::eHAS_MODIFIED_MASS_RATIOS;
	}

protected:

	PX_FORCE_INLINE PxContactPatch* getPatch() const
	{
		const size_t headerOffset = sizeof(PxContactPatch)*mCount;
		return reinterpret_cast<PxContactPatch*>(reinterpret_cast<PxU8*>(mContacts) - headerOffset);
	}

	PxU32					mCount;			//!< Number of contact points in the set
	PxModifiableContact*	mContacts;		//!< The contact points of the set
};



/**
\brief An array of instances of this class is passed to PxContactModifyCallback::onContactModify().

@see PxContactModifyCallback
*/

class PxContactModifyPair
{
public:

	/**
	\brief The actors which make up the pair in contact. 
	
	Note that these are the actors as seen by the simulation, and may have been deleted since the simulation step started.
	*/

	const PxRigidActor*		actor[2];
	/**
	\brief The shapes which make up the pair in contact. 
	
	Note that these are the shapes as seen by the simulation, and may have been deleted since the simulation step started.
	*/
	
	const PxShape*			shape[2];

	/**
	\brief The shape to world transforms of the two shapes. 
	
	These are the transforms as the simulation engine sees them, and may have been modified by the application
	since the simulation step started.
	
	*/

	PxTransform 			transform[2];

	/**
	\brief An array of contact points between these two shapes.
	*/

	PxContactSet			contacts;
};


/**
\brief An interface class that the user can implement in order to modify contact constraints.

<b>Threading:</b> It is <b>necessary</b> to make this class thread safe as it will be called in the context of the
simulation thread. It might also be necessary to make it reentrant, since some calls can be made by multi-threaded
parts of the physics engine.

You can enable the use of this contact modification callback by raising the flag PxPairFlag::eMODIFY_CONTACTS in
the filter shader/callback (see #PxSimulationFilterShader) for a pair of rigid body objects.

Please note: 
+ Raising the contact modification flag will not wake the actors up automatically.
+ It is not possible to turn off the performance degradation by simply removing the callback from the scene, the
  filter shader/callback has to be used to clear the contact modification flag.
+ The contacts will only be reported as long as the actors are awake. There will be no callbacks while the actors are sleeping.

@see PxScene.setContactModifyCallback() PxScene.getContactModifyCallback()
*/
class PxContactModifyCallback
{
public:

	/**
	\brief Passes modifiable arrays of contacts to the application.

	The initial contacts are as determined fresh each frame by collision detection.
	
	The number of contacts can not be changed, so you cannot add your own contacts.  You may however
	disable contacts using PxContactSet::ignore().

	@see PxContactModifyPair
	*/
	virtual void onContactModify(PxContactModifyPair* const pairs, PxU32 count) = 0;

protected:
	virtual ~PxContactModifyCallback(){}
};

/**
\brief An interface class that the user can implement in order to modify CCD contact constraints.

<b>Threading:</b> It is <b>necessary</b> to make this class thread safe as it will be called in the context of the
simulation thread. It might also be necessary to make it reentrant, since some calls can be made by multi-threaded
parts of the physics engine.

You can enable the use of this contact modification callback by raising the flag PxPairFlag::eMODIFY_CONTACTS in
the filter shader/callback (see #PxSimulationFilterShader) for a pair of rigid body objects.

Please note: 
+ Raising the contact modification flag will not wake the actors up automatically.
+ It is not possible to turn off the performance degradation by simply removing the callback from the scene, the
  filter shader/callback has to be used to clear the contact modification flag.
+ The contacts will only be reported as long as the actors are awake. There will be no callbacks while the actors are sleeping.

@see PxScene.setContactModifyCallback() PxScene.getContactModifyCallback()
*/
class PxCCDContactModifyCallback
{
public:

	/**
	\brief Passes modifiable arrays of contacts to the application.

	The initial contacts are as determined fresh each frame by collision detection.
	
	The number of contacts can not be changed, so you cannot add your own contacts.  You may however
	disable contacts using PxContactSet::ignore().

	@see PxContactModifyPair
	*/
	virtual void onCCDContactModify(PxContactModifyPair* const pairs, PxU32 count) = 0;

protected:
	virtual ~PxCCDContactModifyCallback(){}
};


#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif

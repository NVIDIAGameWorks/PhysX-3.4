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


#include "NpMaterial.h"
#include "NpPhysics.h"
#include "CmUtils.h"

using namespace physx;

NpMaterial::NpMaterial(const Sc::MaterialCore& desc)
: PxMaterial(PxConcreteType::eMATERIAL, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
, mMaterial(desc)
{
	mMaterial.setNxMaterial(this);  // back-reference	
}

NpMaterial::~NpMaterial()
{
	NpPhysics::getInstance().removeMaterialFromTable(*this);
}

// PX_SERIALIZATION
void NpMaterial::resolveReferences(PxDeserializationContext&)
{
	// ### this one could be automated if NpMaterial would inherit from MaterialCore
	// ### well actually in that case the pointer would not even be needed....
	mMaterial.setNxMaterial(this);	// Resolve MaterialCore::mNxMaterial

	// Maybe not the best place to do it but it has to be done before the shapes resolve material indices
	// since the material index translation table is needed there. This requires that the materials have
	// been added to the table already.
	NpPhysics::getInstance().addMaterial(this);
}

void NpMaterial::onRefCountZero()
{
	void* ud = userData;	

	if(getBaseFlags() & PxBaseFlag::eOWNS_MEMORY)
		NpFactory::getInstance().releaseMaterialToPool(*this);
	else
		this->~NpMaterial();

	NpPhysics::getInstance().notifyDeletionListenersMemRelease(this, ud);
}

NpMaterial* NpMaterial::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpMaterial* obj = new (address) NpMaterial(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpMaterial);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

void NpMaterial::release()
{
	decRefCount();
}

void NpMaterial::acquireReference()
{
	incRefCount();
}

PxU32 NpMaterial::getReferenceCount() const
{
	return getRefCount();
}

PX_INLINE void NpMaterial::updateMaterial()
{
	NpPhysics::getInstance().updateMaterial(*this);
}

///////////////////////////////////////////////////////////////////////////////

void NpMaterial::setDynamicFriction(PxReal x)
{
	PX_CHECK_AND_RETURN(PxIsFinite(x), "PxMaterial::setDynamicFriction: invalid float");
	mMaterial.dynamicFriction = x;

	updateMaterial();
}

PxReal NpMaterial::getDynamicFriction() const
{
	return mMaterial.dynamicFriction;
}

///////////////////////////////////////////////////////////////////////////////

void NpMaterial::setStaticFriction(PxReal x)
{
	PX_CHECK_AND_RETURN(PxIsFinite(x), "PxMaterial::setStaticFriction: invalid float");
	mMaterial.staticFriction = x;

	updateMaterial();
}

PxReal NpMaterial::getStaticFriction() const
{
	return mMaterial.staticFriction;
}

///////////////////////////////////////////////////////////////////////////////

void NpMaterial::setRestitution(PxReal x)
{
	PX_CHECK_AND_RETURN(PxIsFinite(x), "PxMaterial::setRestitution: invalid float");
	PX_CHECK_MSG(((x >= 0.0f) && (x <= 1.0f)), "PxMaterial::setRestitution: Restitution value has to be in [0,1]!");
	if ((x < 0.0f) || (x > 1.0f))
	{
		PxClamp(x, 0.0f, 1.0f);
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxMaterial::setRestitution: Invalid value %f was clamped to [0,1]!", PxF64(x));
	}
	mMaterial.restitution = x;

	updateMaterial();
}

PxReal NpMaterial::getRestitution() const
{
	return mMaterial.restitution;
}

/////////////////////////////////////////////////////////////////////////////////

void NpMaterial::setFlag(PxMaterialFlag::Enum flag, bool value)
{
	if (value)
		mMaterial.flags |= flag;
	else
		mMaterial.flags &= ~PxMaterialFlags(flag);

	updateMaterial();
}

void NpMaterial::setFlags(PxMaterialFlags inFlags)
{
	mMaterial.flags = inFlags;
	updateMaterial();
}

PxMaterialFlags NpMaterial::getFlags() const
{
	return mMaterial.flags;
}

///////////////////////////////////////////////////////////////////////////////

void NpMaterial::setFrictionCombineMode(PxCombineMode::Enum x)
{
	mMaterial.setFrictionCombineMode(x);

	updateMaterial();
}

PxCombineMode::Enum NpMaterial::getFrictionCombineMode() const
{
	return mMaterial.getFrictionCombineMode();
}

///////////////////////////////////////////////////////////////////////////////

void NpMaterial::setRestitutionCombineMode(PxCombineMode::Enum x)
{
	mMaterial.setRestitutionCombineMode(x);
	updateMaterial();
}

PxCombineMode::Enum NpMaterial::getRestitutionCombineMode() const
{
	return mMaterial.getRestitutionCombineMode();
}

///////////////////////////////////////////////////////////////////////////////

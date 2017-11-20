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


#include "PxvConfig.h"
#include "DyCorrelationBuffer.h"
#include "PxsMaterialManager.h"
#include "PsUtilities.h"

using namespace physx;
using namespace Gu;

namespace physx
{

namespace Dy
{

namespace 
{
PX_FORCE_INLINE void initContactPatch(CorrelationBuffer::ContactPatchData& patch, PxU16 index, PxReal restitution, PxReal staticFriction, PxReal dynamicFriction,
	PxU8 flags)
{
	patch.start = index;
	patch.count = 1;
	patch.next = 0;
	patch.flags = flags;
	patch.restitution = restitution;
	patch.staticFriction = staticFriction;
	patch.dynamicFriction = dynamicFriction;
}

PX_FORCE_INLINE void initFrictionPatch(FrictionPatch& p, const PxVec3& worldNormal, const PxTransform& body0Pose, const PxTransform& body1Pose, 
	PxReal restitution, PxReal staticFriction, PxReal dynamicFriction, PxU8 materialFlags)
{
	p.body0Normal = body0Pose.rotateInv(worldNormal);
	p.body1Normal = body1Pose.rotateInv(worldNormal);
	p.anchorCount = 0;
	p.broken = 0;
	p.staticFriction = staticFriction;
	p.dynamicFriction = dynamicFriction;
	p.restitution = restitution;
	p.materialFlags = materialFlags;
}
}


bool createContactPatches(CorrelationBuffer& fb, const Gu::ContactPoint* cb, PxU32 contactCount, PxReal normalTolerance)
{

	// PT: this rewritten version below doesn't have LHS

	PxU32 contactPatchCount = fb.contactPatchCount;
	if(contactPatchCount == Gu::ContactBuffer::MAX_CONTACTS)
		return false;
	if(contactCount>0)
	{
		CorrelationBuffer::ContactPatchData* currentPatchData = fb.contactPatches + contactPatchCount;
		const Gu::ContactPoint* PX_RESTRICT contacts = cb;

		PxU8 count=1;

		initContactPatch(fb.contactPatches[contactPatchCount++], Ps::to16(0), contacts[0].restitution, 
			contacts[0].staticFriction, contacts[0].dynamicFriction, PxU8(contacts[0].materialFlags));

		PxBounds3 bounds(contacts[0].point, contacts[0].point);

		PxU32 patchIndex = 0;

		for (PxU32 i = 1; i<contactCount; i++)
		{
			const Gu::ContactPoint& curContact = contacts[i];
			const Gu::ContactPoint& preContact = contacts[patchIndex];

			if(curContact.staticFriction == preContact.staticFriction
				&& curContact.dynamicFriction == preContact.dynamicFriction
				&& curContact.restitution == preContact.restitution
				&& curContact.normal.dot(preContact.normal)>=normalTolerance)
			{
				bounds.include(curContact.point);
				count++;
			}
			else
			{
				if(contactPatchCount == Gu::ContactBuffer::MAX_CONTACTS)
					return false;
				patchIndex = i;
				currentPatchData->count = count;
				count = 1;
				currentPatchData->patchBounds = bounds;
				currentPatchData = fb.contactPatches + contactPatchCount;

				initContactPatch(fb.contactPatches[contactPatchCount++], Ps::to16(i), curContact.restitution,
					curContact.staticFriction, curContact.dynamicFriction, PxU8(curContact.materialFlags));

				
				bounds = PxBounds3(curContact.point, curContact.point);
			}
		}
		if(count!=1)
			currentPatchData->count = count;

		currentPatchData->patchBounds = bounds;
	}
	fb.contactPatchCount = contactPatchCount;
	return true;
}

bool correlatePatches(CorrelationBuffer& fb, 
					  const Gu::ContactPoint* cb,
					  const PxTransform& bodyFrame0,
					  const PxTransform& bodyFrame1,
					  PxReal normalTolerance,
					  PxU32 startContactPatchIndex,
					  PxU32 startFrictionPatchIndex)
{
	bool overflow = false;
	PxU32 frictionPatchCount = fb.frictionPatchCount;

	for(PxU32 i=startContactPatchIndex;i<fb.contactPatchCount;i++)
	{
		CorrelationBuffer::ContactPatchData &c = fb.contactPatches[i];
		const PxVec3 patchNormal = cb[c.start].normal;

		PxU32 j=startFrictionPatchIndex;
		for(;j<frictionPatchCount && ((patchNormal.dot(fb.frictionPatchWorldNormal[j]) < normalTolerance) 
			|| fb.frictionPatches[j].restitution != c.restitution|| fb.frictionPatches[j].staticFriction != c.staticFriction || 
			fb.frictionPatches[j].dynamicFriction != c.dynamicFriction);j++)
			;

		if(j==frictionPatchCount)
		{
			overflow |= j==CorrelationBuffer::MAX_FRICTION_PATCHES;
			if(overflow)
				continue;

			initFrictionPatch(fb.frictionPatches[frictionPatchCount], patchNormal, bodyFrame0, bodyFrame1, c.restitution, c.staticFriction, c.dynamicFriction, c.flags);
			fb.frictionPatchWorldNormal[j] = patchNormal;
			fb.frictionPatchContactCounts[frictionPatchCount] = c.count;
			fb.contactID[frictionPatchCount][0] = 0xffff;
			fb.patchBounds[frictionPatchCount] = c.patchBounds;
			fb.contactID[frictionPatchCount++][1] = 0xffff;
			c.next = CorrelationBuffer::LIST_END;
		}
		else
		{
			fb.patchBounds[j].include(c.patchBounds);
			fb.frictionPatchContactCounts[j] += c.count;
			c.next = Ps::to16(fb.correlationListHeads[j]);
		}

		fb.correlationListHeads[j] = i;
	}

	fb.frictionPatchCount = frictionPatchCount;

	return overflow;
}

// run over the friction patches, trying to find two anchors per patch. If we already have
// anchors that are close, we keep them, which gives us persistent spring behavior

void growPatches(CorrelationBuffer& fb,
				 const ContactPoint* cb,
				 const PxTransform& bodyFrame0,
				 const PxTransform& bodyFrame1,
				 PxReal correlationDistance,
				 PxU32 frictionPatchStartIndex,
				 PxReal frictionOffsetThreshold)
{
	for(PxU32 i=frictionPatchStartIndex;i<fb.frictionPatchCount;i++)
	{
		FrictionPatch& fp = fb.frictionPatches[i];

		if (fp.anchorCount == 2 || fb.correlationListHeads[i] == CorrelationBuffer::LIST_END)
		{
			const PxReal frictionPatchDiagonalSq = fb.patchBounds[i].getDimensions().magnitudeSquared();
			const PxReal anchorSqDistance = (fp.body0Anchors[0] - fp.body0Anchors[1]).magnitudeSquared();

			//If the squared distance between the anchors is more than a quarter of the patch diagonal, we can keep, 
			//otherwise the anchors are potentially clustered around a corner so force a rebuild of the patch
			if(fb.frictionPatchContactCounts[i] == 0 || (anchorSqDistance * 4.f) >= frictionPatchDiagonalSq)
				continue;

			fp.anchorCount = 0;
		}

		PxVec3 worldAnchors[2];
		PxU16 anchorCount = 0;
		PxReal pointDistSq = 0.0f, dist0, dist1;

		// if we have an anchor already, keep it
		if(fp.anchorCount == 1)
		{
			worldAnchors[anchorCount++] = bodyFrame0.transform(fp.body0Anchors[0]);
		}

		for(PxU32 patch = fb.correlationListHeads[i]; 
			patch!=CorrelationBuffer::LIST_END; 
			patch = fb.contactPatches[patch].next)
		{
			CorrelationBuffer::ContactPatchData& cp = fb.contactPatches[patch];
			for(PxU16 j=0;j<cp.count;j++)
			{
				const PxVec3& worldPoint = cb[cp.start+j].point;

				if(cb[cp.start+j].separation < frictionOffsetThreshold)
				{

					switch(anchorCount)
					{
					case 0:
						fb.contactID[i][0] = PxU16(cp.start+j);
						worldAnchors[0] = worldPoint;
						anchorCount++;
						break;
					case 1:
						pointDistSq = (worldPoint-worldAnchors[0]).magnitudeSquared(); 
						if (pointDistSq > (correlationDistance * correlationDistance))
						{
							fb.contactID[i][1] = PxU16(cp.start+j);
							worldAnchors[1] = worldPoint;
							anchorCount++;
						}
						break;
					default: //case 2
						dist0 = (worldPoint-worldAnchors[0]).magnitudeSquared();
						dist1 = (worldPoint-worldAnchors[1]).magnitudeSquared();
						if (dist0 > dist1)
						{
							if(dist0 > pointDistSq)
							{
								fb.contactID[i][1] = PxU16(cp.start+j);
								worldAnchors[1] = worldPoint;
								pointDistSq = dist0;
							}
						}
						else if (dist1 > pointDistSq)
						{
							fb.contactID[i][0] = PxU16(cp.start+j);
							worldAnchors[0] = worldPoint;
							pointDistSq = dist1;
						}
					}
				}
			}
		}

		//PX_ASSERT(anchorCount > 0);

		// add the new anchor(s) to the patch
		for(PxU32 j = fp.anchorCount; j < anchorCount; j++)
		{
			fp.body0Anchors[j] = bodyFrame0.transformInv(worldAnchors[j]);
			fp.body1Anchors[j] = bodyFrame1.transformInv(worldAnchors[j]);
		}

		// the block contact solver always reads at least one anchor per patch for performance reasons even if there are no valid patches,
		// so we need to initialize this in the unexpected case that we have no anchors

		if(anchorCount==0)
			fp.body0Anchors[0] = fp.body1Anchors[0] = PxVec3(0);

		fp.anchorCount = anchorCount;
	}
}

}

}


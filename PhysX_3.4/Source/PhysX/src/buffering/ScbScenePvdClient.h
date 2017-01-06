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

#ifndef SCB_SCENE_PVD_CLIENT_H
#define SCB_SCENE_PVD_CLIENT_H

#include "PxPhysXConfig.h"

#if PX_SUPPORT_PVD

#include "foundation/PxStrideIterator.h"
#include "pvd/PxPvdTransport.h"

#include "PxPvdSceneClient.h"
#include "PvdMetaDataPvdBinding.h"

#include "CmBitMap.h"

#include "PxPvdClient.h"
#include "PxPvdUserRenderer.h"
#include "PsPvd.h"

namespace physx
{
class PxScene;
class PxActor;
class PxShape;
class PxGeometryHolder;
class PxArticulationLink;
class PxRenderBuffer;

namespace Scb
{
class Scene;
class Actor;
class Body;
class RigidStatic;
class RigidObject;
class Shape;
class ParticleSystem;
class Constraint;
class Articulation;
class ArticulationJoint;
class Cloth;
class Aggregate;
}

namespace Sc
{
class MaterialCore;
class ConstraintCore;
class ParticleSystemCore;
}

namespace Vd
{
class ScbScenePvdClient : public PxPvdSceneClient, public PvdClient, public PvdVisualizer
{
	PX_NOCOPY(ScbScenePvdClient)
  public:
							ScbScenePvdClient(Scb::Scene& scene);
	virtual					~ScbScenePvdClient();

	// PxPvdSceneClient
	virtual	void			setScenePvdFlag(PxPvdSceneFlag::Enum flag, bool value);
	virtual	void			setScenePvdFlags(PxPvdSceneFlags flags)				{ mFlags = flags;	}
	virtual	PxPvdSceneFlags	getScenePvdFlags()							const	{ return mFlags;	}
	virtual	void			updateCamera(const char* name, const PxVec3& origin, const PxVec3& up, const PxVec3& target);
	virtual	void			drawPoints(const PvdDebugPoint* points, PxU32 count);
	virtual	void			drawLines(const PvdDebugLine* lines, PxU32 count);
	virtual	void			drawTriangles(const PvdDebugTriangle* triangles, PxU32 count);
	virtual	void			drawText(const PvdDebugText& text);
	virtual	PvdClient*		getClientInternal()									{ return this;		}
	//~PxPvdSceneClient
	
	// pvdClient	
	virtual	PvdDataStream*		getDataStream()			{ return mPvdDataStream;	}
	virtual	PvdMetaDataBinding*	getMetaDataBinding()	{ return &mMetaDataBinding;	}
	virtual	PvdUserRenderer*	getUserRender()			{ return mUserRender;		}
	virtual bool                isConnected()	const	{ return mIsConnected;		}
	virtual void                onPvdConnected();
	virtual void                onPvdDisconnected();
	virtual void                flush()					{}
	//~pvdClient

	PX_FORCE_INLINE bool checkPvdDebugFlag()	const
	{
		return mIsConnected && (mPvd->getInstrumentationFlags() & PxPvdInstrumentationFlag::eDEBUG);
	}

	PX_FORCE_INLINE	PxPvdSceneFlags	getScenePvdFlagsFast() const	{ return mFlags;	}
	PX_FORCE_INLINE	void             setPsPvd(PsPvd* pvd)			{ mPvd = pvd;		}

	void frameStart(PxReal simulateElapsedTime);
	void frameEnd();

	void updatePvdProperties();
	void releasePvdInstance();

	void createPvdInstance	(const PxActor* actor);          // temporary for deformables and particle systems
	void updatePvdProperties(const PxActor* actor);
	void releasePvdInstance	(const PxActor* actor); // temporary for deformables and particle systems

	void createPvdInstance	(const Scb::Actor* actor); // temporary for deformables and particle systems
	void updatePvdProperties(const Scb::Actor* actor);
	void releasePvdInstance	(const Scb::Actor* actor); // temporary for deformables and particle systems

	void createPvdInstance		(const Scb::Body* body);
	void updatePvdProperties	(const Scb::Body* body);
	void updateKinematicTarget	(const Scb::Body* body, const PxTransform& p);

	void createPvdInstance		(const Scb::RigidStatic* rigidStatic);
	void updatePvdProperties	(const Scb::RigidStatic* rigidStatic);

	void releasePvdInstance		(const Scb::RigidObject* rigidObject);

	void createPvdInstance	(const Scb::Constraint* constraint);
	void updatePvdProperties(const Scb::Constraint* constraint);
	void releasePvdInstance	(const Scb::Constraint* constraint);

	void createPvdInstance	(const Scb::Articulation* articulation);
	void updatePvdProperties(const Scb::Articulation* articulation);
	void releasePvdInstance	(const Scb::Articulation* articulation);

	void createPvdInstance	(const Scb::ArticulationJoint* articulationJoint);
	void updatePvdProperties(const Scb::ArticulationJoint* articulationJoint);
	void releasePvdInstance	(const Scb::ArticulationJoint* articulationJoint);

	void createPvdInstance	(const Sc::MaterialCore* materialCore);
	void updatePvdProperties(const Sc::MaterialCore* materialCore);
	void releasePvdInstance	(const Sc::MaterialCore* materialCore);

	void createPvdInstance			(const Scb::Shape* shape, PxActor& owner);
	void updateMaterials			(const Scb::Shape* shape);
	void updatePvdProperties		(const Scb::Shape* shape);
	void releaseAndRecreateGeometry	(const Scb::Shape* shape);
	void releasePvdInstance			(const Scb::Shape* shape, PxActor& owner);
	void addBodyAndShapesToPvd		(Scb::Body& b);
	void addStaticAndShapesToPvd	(Scb::RigidStatic& s);

	void createPvdInstance		(const Scb::Aggregate* aggregate);
	void updatePvdProperties	(const Scb::Aggregate* aggregate);
	void attachAggregateActor	(const Scb::Aggregate* aggregate, Scb::Actor* actor);
	void detachAggregateActor	(const Scb::Aggregate* aggregate, Scb::Actor* actor);
	void releasePvdInstance		(const Scb::Aggregate* aggregate);

	void createPvdInstance			(const Scb::Cloth* cloth);
	void sendSimpleProperties		(const Scb::Cloth* cloth);
	void sendMotionConstraints		(const Scb::Cloth* cloth);
	void sendCollisionSpheres		(const Scb::Cloth* cloth);
	void sendCollisionCapsules		(const Scb::Cloth* cloth);
	void sendCollisionTriangles		(const Scb::Cloth* cloth);
	void sendVirtualParticles		(const Scb::Cloth* cloth);
	void sendSeparationConstraints	(const Scb::Cloth* cloth);
	void sendParticleAccelerations	(const Scb::Cloth* cloth);
	void sendSelfCollisionIndices	(const Scb::Cloth* cloth);
	void sendRestPositions			(const Scb::Cloth* cloth);
	void releasePvdInstance			(const Scb::Cloth* cloth);

	void originShift(PxVec3 shift);
	void updateJoints();
	void updateContacts();
	void updateSceneQueries();

	// PvdVisualizer
	void visualize(PxArticulationLink& link);
	void visualize(const PxRenderBuffer& debugRenderable);

  private:

	template <typename TPropertyType>
	void				sendArray(	const void* instance, const char* propName, const Cm::BitMap* bitMap, PxU32 nbValidParticles,
								   PxStrideIterator<const TPropertyType>& iterator);

	void				sendStateDatas(Sc::ParticleSystemCore* psCore);
	void				sendEntireScene();
	void				updateConstraint(const Sc::ConstraintCore& scConstraint, PxU32 updateType);
	void				setCreateContactReports(bool b);

	PxPvdSceneFlags			mFlags;
	PsPvd*					mPvd;
	Scb::Scene&				mScbScene;
	
	PvdDataStream*			mPvdDataStream;
	PvdMetaDataBinding		mMetaDataBinding;
	PvdUserRenderer*		mUserRender;
	RendererEventClient*	mRenderClient;
	bool					mIsConnected;
};

} // pvd

} // physx
#endif // PX_SUPPORT_PVD

#endif // SCB_SCENE_PVD_CLIENT_H

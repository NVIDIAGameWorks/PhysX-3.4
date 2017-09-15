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


#ifndef PX_PHYSICS_CONSTRAINTCORE
#define PX_PHYSICS_CONSTRAINTCORE

#include "CmPhysXCommon.h"
#include "PxConstraintDesc.h"
#include "PsAllocator.h"
#include "PxConstraint.h"

namespace physx
{

class PxConstraint;

namespace Sc
{
	class ConstraintCore;
	class ConstraintSim;
	class RigidCore;


	class ConstraintCore : public Ps::UserAllocated	
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
											ConstraintCore(const PxEMPTY) : mFlags(PxEmpty), mConnector(NULL), mSim(NULL)	{}
	PX_FORCE_INLINE	void					setConstraintFunctions(PxConstraintConnector& n,
																   const PxConstraintShaderTable& shaders)
											{ 
												mConnector = &n;	
												mSolverPrep = shaders.solverPrep;
												mProject = shaders.project;
												mVisualize = shaders.visualize;
											}
		static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
											ConstraintCore(PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize);
											~ConstraintCore();

					// The two-step protocol here allows us to unlink the constraint prior to deleting
					// the actors when synchronizing the scene, then set the bodies after new actors have been inserted

					void					prepareForSetBodies();
					void					setBodies(RigidCore* r0v, RigidCore* r1v);

					PxConstraint*			getPxConstraint();
					const PxConstraint*		getPxConstraint()									const;
	PX_FORCE_INLINE	PxConstraintConnector*	getPxConnector()									const	{ return mConnector;	}

	PX_FORCE_INLINE	PxConstraintFlags		getFlags()											const	{ return mFlags;	}
					void					setFlags(PxConstraintFlags flags);

					void					getForce(PxVec3& force, PxVec3& torque)				const;

					bool					updateConstants(void* addr); 

					void					setBreakForce(PxReal linear, PxReal angular);
					void					getBreakForce(PxReal& linear, PxReal& angular)	const;

					void					setMinResponseThreshold(PxReal threshold);
					PxReal					getMinResponseThreshold()							const	{ return mMinResponseThreshold; }

					void					breakApart();

	PX_FORCE_INLINE	PxConstraintVisualize	getVisualize()										const	{ return mVisualize;				}
	PX_FORCE_INLINE	PxConstraintProject		getProject()										const	{ return mProject;					}
	PX_FORCE_INLINE	PxConstraintSolverPrep	getSolverPrep()										const	{ return mSolverPrep;				}
	PX_FORCE_INLINE	PxU32					getConstantBlockSize()								const	{ return mDataSize;					}

	PX_FORCE_INLINE	void					setSim(ConstraintSim* sim)
											{
												PX_ASSERT((sim==0) ^ (mSim == 0));
												mSim = sim;
											}
	PX_FORCE_INLINE	ConstraintSim*			getSim()											const	{ return mSim;	}
	private:
					PxConstraintFlags		mFlags;
					PxU16					mPaddingFromFlags;	// PT: because flags are PxU16

					PxVec3					mAppliedForce;
					PxVec3					mAppliedTorque;

					PxConstraintConnector*	mConnector;
					PxConstraintProject		mProject;
					PxConstraintSolverPrep	mSolverPrep;
					PxConstraintVisualize	mVisualize;
					PxU32					mDataSize;
					PxReal					mLinearBreakForce;
					PxReal					mAngularBreakForce;
					PxReal					mMinResponseThreshold;

					ConstraintSim*			mSim;
	};

} // namespace Sc

}

#endif

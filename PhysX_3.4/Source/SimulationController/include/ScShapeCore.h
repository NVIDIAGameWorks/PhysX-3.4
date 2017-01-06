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


#ifndef PX_PHYSICS_SCP_SHAPECORE
#define PX_PHYSICS_SCP_SHAPECORE

#include "PsUserAllocated.h"
#include "GuGeometryUnion.h"
#include "PxvGeometry.h"
#include "PsUtilities.h"
#include "PxFiltering.h"
#include "PxShape.h"

namespace physx
{
class PxShape;

namespace Sc
{
	class Scene;
	class RigidCore;
	class BodyCore;
	class ShapeSim;
	class MaterialCore;

	class ShapeCore : public Ps::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
													ShapeCore(const PxEMPTY);
						void						exportExtraData(PxSerializationContext& stream);
						void						importExtraData(PxDeserializationContext& context);
						void						resolveReferences(PxDeserializationContext& context);
		static			void						getBinaryMetaData(PxOutputStream& stream);
		                void                        resolveMaterialReference(PxU32 materialTableIndex, PxU16 materialIndex);
//~PX_SERIALIZATION

													ShapeCore(const PxGeometry& geometry, 
															  PxShapeFlags shapeFlags,
															  const PxU16* materialIndices, 
															  PxU16 materialCount);

													~ShapeCore();

		PX_FORCE_INLINE	PxGeometryType::Enum		getGeometryType()							const	{ return mCore.geometry.getType();			}
						PxShape*					getPxShape();
						const PxShape*				getPxShape()								const;

		PX_FORCE_INLINE	const Gu::GeometryUnion&	getGeometryUnion()							const	{ return mCore.geometry;					}
		PX_FORCE_INLINE	const PxGeometry&			getGeometry()								const	{ return mCore.geometry.getGeometry();		}
						void						setGeometry(const PxGeometry& geom);

						PxU16						getNbMaterialIndices()						const;
						const PxU16*				getMaterialIndices()						const;
						void						setMaterialIndices(const PxU16* materialIndices, PxU16 materialIndexCount);

		PX_FORCE_INLINE	const PxTransform&			getShape2Actor()							const	{ return mCore.transform;					}
		PX_FORCE_INLINE	void						setShape2Actor(const PxTransform& s2b)				{ mCore.transform = s2b;					}
		
		PX_FORCE_INLINE	const PxFilterData&			getSimulationFilterData()					const	{ return mSimulationFilterData;				}
		PX_FORCE_INLINE	void						setSimulationFilterData(const PxFilterData& data)	{ mSimulationFilterData = data;				}

		// PT: this one doesn't need double buffering
		PX_FORCE_INLINE	const PxFilterData&			getQueryFilterData()						const	{ return mQueryFilterData;					}
		PX_FORCE_INLINE	void						setQueryFilterData(const PxFilterData& data)		{ mQueryFilterData = data;					}

		PX_FORCE_INLINE	PxReal						getContactOffset()							const	{ return mCore.contactOffset;				}
		PX_FORCE_INLINE	void						setContactOffset(PxReal offset)						{ mCore.contactOffset = offset;				}

		PX_FORCE_INLINE	PxReal						getRestOffset()								const	{ return mRestOffset;						}
		PX_FORCE_INLINE	void						setRestOffset(PxReal offset)						{ mRestOffset = offset;						}

		PX_FORCE_INLINE	PxShapeFlags				getFlags()									const	{ return PxShapeFlags(mCore.mShapeFlags);	}
		PX_FORCE_INLINE	void						setFlags(PxShapeFlags f)							{ mCore.mShapeFlags = f;					}

		PX_FORCE_INLINE const PxsShapeCore&			getCore()									const	{ return mCore;								}

		static PX_FORCE_INLINE ShapeCore&			getCore(PxsShapeCore& core)			
		{ 
			size_t offset = PX_OFFSET_OF(ShapeCore, mCore);
			return *reinterpret_cast<ShapeCore*>(reinterpret_cast<PxU8*>(&core) - offset); 
		}	

	protected:
						PxFilterData				mQueryFilterData;		// Query filter data PT: TODO: consider moving this to SceneQueryShapeData
						PxFilterData				mSimulationFilterData;	// Simulation filter data
						PxsShapeCore				PX_ALIGN(16, mCore);	
						PxReal						mRestOffset;			// same as the API property of the same name
	};

} // namespace Sc


}

#endif

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


#ifndef PX_PHYSICS_NP_SCENEQUERY
#define PX_PHYSICS_NP_SCENEQUERY
/** \addtogroup physics 
@{ */

#include "PxBatchQuery.h"
#include "PsArray.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PsSync.h"

namespace physx
{

class NpSceneQueryManager;
struct BatchStreamHeader;
class NpScene;

namespace Sq
{
class SceneQueryManager;
}

struct BatchQueryStream : Ps::Array<char>
{
	BatchQueryStream() { rewind(); }

	void rewind() { mPosition = 0; }

	PX_FORCE_INLINE PxI32 getPos() { return PxI32(mPosition); } // signed to avoid casts elsewhere

	// write an object of type T to the stream, copying by value
	template<typename T>
	PX_FORCE_INLINE void write(const T* val, PxU32 count = 1)
	{
		PX_COMPILE_TIME_ASSERT(sizeof(T) > 0);
		PxU32 newPosition = mPosition + sizeof(T)*count;
		if (newPosition > capacity())
			reserve(newPosition+(newPosition<<1));
		resizeUninitialized(newPosition);
		T* dest = reinterpret_cast<T*>(begin() + mPosition);
		for (PxU32 i = 0; i < count; i++)
		{
			*dest = *(val+i);
			dest++;
		}
		mPosition = newPosition;
	}

	template<typename T>
	PX_FORCE_INLINE void write(const T& val)
	{
		write(&val, 1);
	}

	PX_FORCE_INLINE bool atEnd() const { return mPosition >= size(); }

protected:
	mutable PxU32 mPosition;
};

struct BatchQueryStreamReader
{
	BatchQueryStreamReader(char* buffer) : mBuffer(buffer), mReadPos(0) {}

	// read an object of type T from the stream (simply returns a pointer without copying)
	template<typename T>
	PX_FORCE_INLINE T* read(PxU32 count = 1)
	{
		//PX_ASSERT(mPosition+sizeof(T)*count <= size());
		T* result = reinterpret_cast<T*>(mBuffer+mReadPos);
		mReadPos += sizeof(T)*count;
		return result;
	}

	char* mBuffer;
	PxU32 mReadPos;
};

class NpBatchQuery : public PxBatchQuery, public Ps::UserAllocated
{
public:
											NpBatchQuery(NpScene& owner, const PxBatchQueryDesc& d);
	virtual									~NpBatchQuery();

	// PxBatchQuery interface
	virtual	void							execute();
	virtual void							release();
	virtual	PxBatchQueryPreFilterShader		getPreFilterShader() const;
	virtual	PxBatchQueryPostFilterShader	getPostFilterShader() const;
	virtual	const void*						getFilterShaderData() const;
	virtual	PxU32							getFilterShaderDataSize() const;
	virtual PxClientID						getOwnerClient() const;
	virtual void							setUserMemory(const PxBatchQueryMemory& );
	virtual const PxBatchQueryMemory&		getUserMemory();

	virtual void							raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal distance, PxU16 maxTouchHits,
													PxHitFlags hitFlags, const PxQueryFilterData& filterData,
													void* userData, const PxQueryCache* cache);

	virtual void							overlap(const PxGeometry& geometry, const PxTransform& pose, PxU16 maxTouchHits,
													const PxQueryFilterData& filterData, void* userData,
													const PxQueryCache* cache);

	virtual void							sweep(const PxGeometry& geometry, const PxTransform& pose,
												const PxVec3& unitDir, const PxReal distance, PxU16 maxTouchHits,
												PxHitFlags hitFlags, const PxQueryFilterData& filterData,
												void* userData, const PxQueryCache* cache, const PxReal inflation);

	PxBatchQueryDesc&						getDesc() { return mDesc; }
	virtual const PxBatchQueryDesc&			getDesc() const { return mDesc; }

	enum { eTERMINAL = PxU32(-16) }; // -16 so it's aligned to avoid SPU checks

	// sync object for batch query completion wait
	shdfnd::Sync							mSync;
private:
			void							resetResultBuffers();
			void							finalizeExecute();
			void							writeBatchHeader(const BatchStreamHeader& h);

						NpScene*			mNpScene;
						BatchQueryStream	mStream;
						PxU32				mNbRaycasts, mNbOverlaps, mNbSweeps;
			volatile	PxI32				mBatchQueryIsRunning;
						PxBatchQueryDesc	mDesc;
	// offset in mStream of the offset to the next query for the last header written by BQ query functions
						PxU32				mPrevOffset;
						bool				mHasMtdSweep;

	friend class physx::Sq::SceneQueryManager;
};

}

/** @} */
#endif

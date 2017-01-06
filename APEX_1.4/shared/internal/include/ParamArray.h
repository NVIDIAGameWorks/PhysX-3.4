/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PARAM_ARRAY_H
#define PARAM_ARRAY_H

#include "nvparameterized/NvParameterized.h"
#include "PsUserAllocated.h"
#include "PxAssert.h"

namespace nvidia
{
namespace apex
{

struct ParamDynamicArrayStruct
{
	void* buf;
	bool isAllocated;
	int elementSize;
	int arraySizes[1];
};


template <class ElemType>
class ParamArray : public physx::UserAllocated
{
public:

	ParamArray() : mParams(NULL), mArrayHandle(0), mArrayStruct(NULL) {}

	ParamArray(NvParameterized::Interface* params, const char* arrayName, ParamDynamicArrayStruct* arrayStruct) :
		mParams(params),
		mArrayHandle(*params),
		mArrayStruct(arrayStruct)
	{
		PX_ASSERT(mParams);

		mParams->getParameterHandle(arrayName, mArrayHandle);

		PX_ASSERT(mArrayStruct->elementSize == sizeof(ElemType));
	}

	ParamArray(NvParameterized::Interface* params, const NvParameterized::Handle& handle, ParamDynamicArrayStruct* arrayStruct) :
		mParams(params),
		mArrayHandle(handle),
		mArrayStruct(arrayStruct)
	{
		PX_ASSERT(mArrayStruct->elementSize == sizeof(ElemType));
	}

	PX_INLINE bool init(NvParameterized::Interface* params, const char* arrayName, ParamDynamicArrayStruct* arrayStruct)
	{
		if (mParams == NULL && mArrayStruct == NULL)
		{
			mParams = params;
			mArrayStruct = arrayStruct;
			mArrayHandle.setInterface(mParams);
			mParams->getParameterHandle(arrayName, mArrayHandle);

			PX_ASSERT(mArrayStruct->elementSize == sizeof(ElemType));

			return true;
		}
		return false;
	}

	PX_INLINE uint32_t size() const
	{
		// this only works for fixed structs
		//return (uint32_t)mArrayHandle.parameterDefinition()->arraySize(0);
		int outSize = 0;
		if (mParams != NULL)
		{
			PX_ASSERT(mArrayHandle.getConstInterface() == mParams);
			mArrayHandle.getArraySize(outSize);
		}
		return (uint32_t)outSize;
	}

	/**
	Returns a constant reference to an element in the sequence.
	*/
	PX_INLINE const ElemType& operator[](unsigned int n) const
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
#if _DEBUG
		uint32_t NxParamArraySize = 0;
		mArrayHandle.getArraySize((int&)NxParamArraySize);
		PX_ASSERT(NxParamArraySize > n);
#endif
		return ((ElemType*)mArrayStruct->buf)[n];
	}

	/**
	Returns a reference to an element in the sequence.
	*/
	PX_INLINE ElemType& operator[](unsigned int n)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		//NvParameterized::Handle indexHandle;
		//arrayHandle.getChildHandle( n, indexHandle );
#if _DEBUG
		uint32_t NxParamArraySize = 0;
		mArrayHandle.getArraySize((int&)NxParamArraySize);
		PX_ASSERT(NxParamArraySize > n);
#endif
		return ((ElemType*)mArrayStruct->buf)[n];
	}

	// resize is marginally useful because the ElemType doesn't have proper
	// copy constructors, and if strings are withing ElemType that doesn't work well
	PX_INLINE void resize(unsigned int n)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		PX_ASSERT(mParams == mArrayHandle.getConstInterface());
		mArrayHandle.resizeArray((int32_t)n);
	}

	// pushBack is marginally useful because the ElemType doesn't have proper
	// copy constructors, and if strings are withing ElemType that doesn't work well
	PX_INLINE void pushBack(const ElemType& x)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);

		int32_t paramArraySize = 0;

		mArrayHandle.getArraySize(paramArraySize);
		mArrayHandle.resizeArray(paramArraySize + 1);

		((ElemType*)mArrayStruct->buf)[(uint32_t)paramArraySize] = x;
	}

	PX_INLINE ElemType& pushBack()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);

		int32_t paramArraySize = 0;

		mArrayHandle.getArraySize(paramArraySize);
		mArrayHandle.resizeArray(paramArraySize + 1);

		return ((ElemType*)mArrayStruct->buf)[(uint32_t)paramArraySize];
	}

	PX_INLINE void replaceWithLast(unsigned position)
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);

		uint32_t arraySize = size();
		PX_ASSERT(position < arraySize);
		if (position != arraySize - 1)
		{
			// TODO should we call the destructor here or not?
			//(*this)[position].~ElemType();

			ElemType elem = back();

			// put the replaced one in the back (possibly to be deleted)
			(*this)[arraySize - 1] = (*this)[position];

			(*this)[position] = elem;
		}
		popBack();
	}

	PX_INLINE bool isEmpty() const
	{
		return size() == 0;
	}

	PX_INLINE ElemType* begin()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return &((ElemType*)mArrayStruct->buf)[0];
	}

	PX_INLINE ElemType* end()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return &((ElemType*)mArrayStruct->buf)[size()];
	}

	PX_INLINE ElemType& front()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return ((ElemType*)mArrayStruct->buf)[0];
	}

	PX_INLINE ElemType& back()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		return ((ElemType*)mArrayStruct->buf)[size() - 1];
	}

	PX_INLINE void clear()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		resize(0);
	}

	PX_INLINE void popBack()
	{
		PX_ASSERT(mParams != NULL && mArrayStruct != NULL);
		resize(size() - 1);
	}

private:
	NvParameterized::Interface*	mParams;
	NvParameterized::Handle		mArrayHandle;
	ParamDynamicArrayStruct*	mArrayStruct;
};

}
} // end namespace nvidia::apex

#endif


/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_INTEROPABLE_BUFFER_H__
#define __APEX_INTEROPABLE_BUFFER_H__

typedef struct CUgraphicsResource_st *CUgraphicsResource;

namespace physx
{
	class PxCudaContextManager;
}

namespace nvidia
{
namespace apex
{

class ApexInteropableBuffer
{
public:
	ApexInteropableBuffer(bool mustBeRegistered = false, PxCudaContextManager *interopContext = NULL)
		: m_mustBeRegisteredInCUDA(mustBeRegistered)
		, m_registeredInCUDA(false)
		, m_interopContext(interopContext)
		, m_InteropHandle(NULL)
	{
	}

	virtual bool getInteropResourceHandle(CUgraphicsResource &handle)
	{
		if(m_registeredInCUDA && m_InteropHandle)
		{
			handle = m_InteropHandle;

			return true;
		}

		return false;
	}

protected:

	bool		 m_mustBeRegisteredInCUDA;
	bool		 m_registeredInCUDA;
	PxCudaContextManager *m_interopContext;
	CUgraphicsResource	m_InteropHandle;
};


}
} // end namespace nvidia::apex


#endif // __APEX_INTEROPABLE_BUFFER_H__

/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CUDA_KERNEL_MANAGER__
#define __APEX_CUDA_KERNEL_MANAGER__

#include "ApexDefs.h"
#include "CudaProfileManager.h"

#include "PsMemoryBuffer.h"
#include "ApexString.h"
#include "SceneIntl.h"
#include "PsMutex.h"

namespace nvidia
{
namespace apex
{

class ApexCudaObj;
class ApexCudaFunc;
class ApexCudaProfileManager;

class ApexCudaProfileSession
{
	struct ProfileData
	{
		uint32_t id;
		void* start;
		void* stop;
	};
public:
	ApexCudaProfileSession();
	~ApexCudaProfileSession();

	PX_INLINE void	init(ApexCudaProfileManager* manager)
	{
		mManager = manager;
	}
	void		nextFrame();
	void		start();
	bool		stopAndSave();
	uint32_t		getProfileId(const char* name, const char* moduleName);

	void		onFuncStart(uint32_t id, void* stream);
	void		onFuncFinish(uint32_t id, void* stream);

protected:
	float		flushProfileInfo(ProfileData& pd);

	ApexCudaProfileManager* mManager;
	void*			mTimer;
	nvidia::PsMemoryBuffer	mMemBuf;
	nvidia::Mutex	mLock;
	Array <ProfileData> mProfileDataList;
	float	mFrameStart;
	float	mFrameFinish;
};

/** 
 */
class ApexCudaProfileManager : public CudaProfileManager, public UserAllocated
{
public:
	struct KernelInfo
	{
		ApexSimpleString functionName;
		ApexSimpleString moduleName;
		uint32_t id;
		
		KernelInfo(const char* functionName, const char* moduleName, uint32_t id = 0) 
			: functionName(functionName), moduleName(moduleName), id(id) {}

		bool operator!= (const KernelInfo& ki)
		{
			return		(this->functionName != "*" && this->functionName != ki.functionName) 
					|| 	(this->moduleName != ki.moduleName);
		}
	};

	ApexCudaProfileManager();

	virtual ~ApexCudaProfileManager();

	PX_INLINE void setInternalApexScene(SceneIntl* scene)
	{
		mApexScene = scene;
	}
	void nextFrame();

	// interface for CudaProfileManager
	PX_INLINE void setPath(const char* path)
	{
		mPath = ApexSimpleString(path);
		enable(false);
	}
	void setKernel(const char* functionName, const char* moduleName);
	PX_INLINE void setTimeFormat(TimeFormat tf)
	{
		mTimeFormat = tf;
	}
	void enable(bool state);
	PX_INLINE bool isEnabled() const
	{
		return mState;
	}
		
private:
	bool			mState;
	uint32_t			mSessionCount;
	TimeFormat		mTimeFormat;
	uint32_t			mReservedId;
	ApexSimpleString			mPath;
	Array <KernelInfo>			mKernels;
	ApexCudaProfileSession		mSession;
	SceneIntl*	mApexScene;
	friend class ApexCudaProfileSession;
};

}
} // namespace nvidia::apex

#endif // __APEX_CUDA_KERNEL_MANAGER__

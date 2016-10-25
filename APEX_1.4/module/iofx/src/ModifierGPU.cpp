/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ModifierImpl.h"

#if APEX_CUDA_SUPPORT

#include "ApexCudaWrapper.h"

#include "ModifierData.h"

namespace nvidia
{
namespace apex
{

#define MODIFIER_DECL
#define CURVE_TYPE nvidia::iofx::Curve
#define EVAL_CURVE(curve, value) 0
#define PARAMS_NAME(name) name ## ParamsGPU

#include "ModifierSrc.h"

#undef MODIFIER_DECL
#undef CURVE_TYPE
#undef EVAL_CURVE
//#undef PARAMS_NAME
}
namespace iofx
{

class ModifierParamsMapperGPU_Adapter
{
private:
	ModifierParamsMapperGPU_Adapter& operator=(const ModifierParamsMapperGPU_Adapter&);

	ModifierParamsMapperGPU& _mapper;
	InplaceStorage& _storage;
	uint8_t* _params;

public:
	ModifierParamsMapperGPU_Adapter(ModifierParamsMapperGPU& mapper)
		: _mapper(mapper), _storage(mapper.getStorage()), _params(0) {}

	PX_INLINE InplaceStorage& getStorage()
	{
		return _storage;
	}

	PX_INLINE void beginParams(void* params, size_t , size_t , uint32_t)
	{
		_params = (uint8_t*)params;
	}
	PX_INLINE void endParams()
	{
		_params = 0;
	}

	template <typename T>
	PX_INLINE void mapValue(size_t offset, T value)
	{
		PX_ASSERT(_params != 0);
		*(T*)(_params + offset) = value;
	}

	PX_INLINE void mapCurve(size_t offset, const nvidia::apex::Curve* nxCurve)
	{
		PX_ASSERT(_params != 0);
		Curve& curve = *(Curve*)(_params + offset);

		uint32_t numPoints;
		const Vec2R* nxPoints = nxCurve->getControlPoints(numPoints);

		curve.resize(_storage, numPoints);
		for (uint32_t i = 0; i < numPoints; ++i)
		{
			const Vec2R& nxPoint = nxPoints[i];
			curve.setPoint(_storage, CurvePoint(nxPoint.x, nxPoint.y), i);
		}
	}
};

#define _MODIFIER(name) \
	void name ## ModifierImpl :: mapParamsGPU(ModifierParamsMapperGPU& mapper) const \
	{ \
		ModifierParamsMapperGPU_Adapter adapter(mapper); \
		InplaceHandle< PARAMS_NAME(name) > paramsHandle; \
		paramsHandle.alloc( adapter.getStorage() ); \
		PARAMS_NAME(name) params; \
		mapParams( adapter, &params ); \
		paramsHandle.update( adapter.getStorage(), params ); \
		mapper.onParams( paramsHandle, PARAMS_NAME(name)::RANDOM_COUNT ); \
	} \
	 
#include "ModifierList.h"

}
} // namespace nvidia

#endif

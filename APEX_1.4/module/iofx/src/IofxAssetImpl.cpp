/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexUsingNamespace.h"
#include "IofxAssetImpl.h"
#include "IofxActorImpl.h"
#include "IofxScene.h"
#include "ModuleIofxImpl.h"
#include "Modifier.h"
#include "ModifierImpl.h"
#include "SceneIntl.h"
#include "IofxManagerIntl.h"
//#include "ApexSharedSerialization.h"
#include "CurveImpl.h"

namespace nvidia
{
namespace iofx
{


IofxAssetImpl::IofxAssetImpl(ModuleIofxImpl* module, ResourceList& list, const char* name) :
	mRenderMeshAssetTracker(module->mSdk, RENDER_MESH_AUTHORING_TYPE_NAME),
	mSpriteMaterialAssetTracker(module->mSdk),
	mModule(module),
	mName(name),
	mSpriteSemanticBitmap(0),
	mMeshSemanticBitmap(0)
{
	using namespace IofxAssetParametersNS;

	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = (IofxAssetParameters*)traits->createNvParameterized(IofxAssetParameters::staticClassName());

	PX_ASSERT(mParams);

	// LRR: Hmmmm, these will never be setup for authors, it's a good thing we can't create actors from authors!
	mRenderMeshList = NULL;
	mSpriteParams = NULL;

	list.add(*this);
}

IofxAssetImpl::IofxAssetImpl(ModuleIofxImpl* module,
                     ResourceList& list,
                     NvParameterized::Interface* params,
                     const char* name) :
	mRenderMeshAssetTracker(module->mSdk, RENDER_MESH_AUTHORING_TYPE_NAME),
	mSpriteMaterialAssetTracker(module->mSdk),
	mParams((IofxAssetParameters*)params),
	mSpriteParams(NULL),
	mModule(module),
	mName(name),
	mSpriteSemanticBitmap(0),
	mMeshSemanticBitmap(0)
{
	using namespace MeshIofxParametersNS;

	if (mParams->iofxType &&
	        !nvidia::strcmp(mParams->iofxType->className(), MeshIofxParameters::staticClassName()))
	{
		MeshIofxParameters* mIofxParams = static_cast<MeshIofxParameters*>(mParams->iofxType);

		ParamDynamicArrayStruct* tmpStructPtr = (ParamDynamicArrayStruct*) & (mIofxParams->renderMeshList);
		mRenderMeshList = PX_NEW(ParamArray<meshProperties_Type>)(mIofxParams, "renderMeshList", tmpStructPtr);
	}
	else
	{
		mRenderMeshList = NULL;
	}

	if (mParams->iofxType &&
	        !nvidia::strcmp(mParams->iofxType->className(), SpriteIofxParameters::staticClassName()))
	{
		mSpriteParams = static_cast<SpriteIofxParameters*>(mParams->iofxType);
	}


	// call this now to "initialize" the asset
	postDeserialize();

	list.add(*this);
}

IofxAssetImpl::~IofxAssetImpl()
{
	for (uint32_t i = mSpawnModifierStack.size(); i--;)
	{
		delete mSpawnModifierStack[i];
	}

	for (uint32_t i = mContinuousModifierStack.size(); i--;)
	{
		delete mContinuousModifierStack[i];
	}
}

void IofxAssetImpl::destroy()
{
	/* Assets that were forceloaded or loaded by actors will be automatically
	 * released by the ApexAssetTracker member destructors.
	 */
	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}

	if (mRenderMeshList)
	{
		PX_DELETE(mRenderMeshList);
	}

#if IOFX_SLOW_COMPOSITE_MODIFIERS
	// destroy any color vs life params that we may have laying around
	for (uint32_t i = 0; i < mCompositeParams.size(); i++)
	{
		mCompositeParams[i]->destroy();
	}
	mCompositeParams.clear();
#endif

	// this will release all of this asset's IOFX actors, which is necessary, otherwise
	// the IOFX actors will have their APEX Render Meshes ripped out from underneath them
	ResourceList::clear();

	delete this;
}

void IofxAssetImpl::release()
{
	mModule->mSdk->releaseAsset(*this);
}

void IofxAssetImpl::removeAllActors()
{
	ApexContext::removeAllActors();
}

void IofxAssetImpl::addDependentActor(ApexActor* actor)
{
	WRITE_ZONE();
	if (actor)
	{
		actor->addSelfToContext(*this);
	}
}



void IofxAssetImpl::preSerialize(void* userData_)
{
	PX_UNUSED(userData_);
}

void IofxAssetImpl::postDeserialize(void* userData_)
{
	PX_UNUSED(userData_);

	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();

	PX_ASSERT(mParams->iofxType);
	if (!mParams->iofxType)
	{
		return;
	}

	initializeAssetNameTable();

	// destroy the old modifier stacks
	for (uint32_t i = mSpawnModifierStack.size(); i--;)
	{
		delete mSpawnModifierStack[i];
	}
	mSpawnModifierStack.resize(0);

	for (uint32_t i = mContinuousModifierStack.size(); i--;)
	{
		delete mContinuousModifierStack[i];
	}
	mContinuousModifierStack.resize(0);

#if IOFX_SLOW_COMPOSITE_MODIFIERS
	// destroy any color vs life params that we may have laying around
	for (uint32_t i = 0; i < mCompositeParams.size(); i++)
	{
		mCompositeParams[i]->destroy();
	}
	mCompositeParams.clear();
#endif

	const char* handleNameList[] = { "spawnModifierList", "continuousModifierList" };

// 'param' and 'newMod' are defined in the local scope where this macro is used
#define _MODIFIER(Type) \
	else if( !nvidia::strcmp(param->className(), #Type "ModifierParams") )	\
	{																\
		newMod = PX_NEW( Type##ModifierImpl )( (Type##ModifierParams *)param );	\
	}

	for (uint32_t i = 0; i < 2; i++)
	{
		NvParameterized::Handle h(*mParams->iofxType);
		mParams->iofxType->getParameterHandle(handleNameList[i], h);
		int listSize = 0;
		h.getArraySize(listSize);

		for (int j = 0; j < listSize; j++)
		{
			NvParameterized::Interface* param = 0;
			NvParameterized::Handle ih(*mParams->iofxType);
			Modifier* newMod = 0;

			h.getChildHandle(j, ih);
			ih.getParamRef(param);
			PX_ASSERT(param);
			if (!param)
			{
				continue;
			}

			if (newMod != 0)  //this'll never happen :-)
				{}

#include "ModifierList.h"

#if IOFX_SLOW_COMPOSITE_MODIFIERS
			// LRR: there is a better way to do this, but I don't have time at the moment to
			// make this any cleaner

			// special case, we don't currently have an initial color modifier, just insert
			// a colorVsLife modifier for each color channel
			else if (!nvidia::strcmp(param->className(), "InitialColorModifierParams"))
			{
				InitialColorModifierParams* icParams = (InitialColorModifierParams*)param;

				for (uint32_t colorIdx = 0; colorIdx < 4; colorIdx++)
				{
					ColorVsLifeModifierParams* p = (ColorVsLifeModifierParams*)
					                               traits->createNvParameterized(ColorVsLifeModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ColorVsLifeModifierImpl* cvlMod = PX_NEW(ColorVsLifeModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the color channel
					cvlMod->setColorChannel((ColorChannel)colorIdx);

					// set the curve
					CurveImpl curve;
					curve.addControlPoint(Vec2R(0.0f, icParams->color[colorIdx]));
					curve.addControlPoint(Vec2R(1.0f, icParams->color[colorIdx]));

					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ColorVsLifeCompositeModifierParams"))
			{
				// setup an array so we can get the control points easily
				ColorVsLifeCompositeModifierParams* cParams = (ColorVsLifeCompositeModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (uint32_t colorIdx = 0; colorIdx < 4; colorIdx++)
				{

					ColorVsLifeModifierParams* p = (ColorVsLifeModifierParams*)
					                               traits->createNvParameterized(ColorVsLifeModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ColorVsLifeModifierImpl* cvlMod = PX_NEW(ColorVsLifeModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the color channel
					cvlMod->setColorChannel((ColorChannel)colorIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{

						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].lifeRemaining,
						                              cParams->controlPoints.buf[cpIdx].color[colorIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ColorVsDensityCompositeModifierParams"))
			{
				ColorVsDensityCompositeModifierParams* cParams = (ColorVsDensityCompositeModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (uint32_t colorIdx = 0; colorIdx < 4; colorIdx++)
				{

					ColorVsDensityModifierParams* p = (ColorVsDensityModifierParams*)
					                                  traits->createNvParameterized(ColorVsDensityModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ColorVsDensityModifierImpl* cvlMod = PX_NEW(ColorVsDensityModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the color channel
					cvlMod->setColorChannel((ColorChannel)colorIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].density,
						                              cParams->controlPoints.buf[cpIdx].color[colorIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ColorVsVelocityCompositeModifierParams"))
			{
				// setup an array so we can get the control points easily
				ColorVsVelocityCompositeModifierParams* cParams = (ColorVsVelocityCompositeModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (uint32_t colorIdx = 0; colorIdx < 4; colorIdx++)
				{

					ColorVsVelocityModifierParams* p = (ColorVsVelocityModifierParams*)
					                               traits->createNvParameterized(ColorVsVelocityModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ColorVsVelocityModifierImpl* cvlMod = PX_NEW(ColorVsVelocityModifierImpl)(p);
					PX_ASSERT(cvlMod);

					cvlMod->setVelocity0(cParams->velocity0);
					cvlMod->setVelocity1(cParams->velocity1);

					// set the color channel
					cvlMod->setColorChannel((ColorChannel)colorIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{

						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].velocity,
						                              cParams->controlPoints.buf[cpIdx].color[colorIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ScaleVsLife2DModifierParams"))
			{
				// setup an array so we can get the control points easily
				ScaleVsLife2DModifierParams* cParams = (ScaleVsLife2DModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (int32_t axisIdx = 0; axisIdx < 2; axisIdx++)
				{
					ScaleVsLifeModifierParams* p = (ScaleVsLifeModifierParams*)
					                               traits->createNvParameterized(ScaleVsLifeModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ScaleVsLifeModifierImpl* cvlMod = PX_NEW(ScaleVsLifeModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the scale axis
					cvlMod->setScaleAxis((ScaleAxis)axisIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].lifeRemaining,
						                              cParams->controlPoints.buf[cpIdx].scale[axisIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ScaleVsLife3DModifierParams"))
			{
				// setup an array so we can get the control points easily
				ScaleVsLife3DModifierParams* cParams = (ScaleVsLife3DModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (uint32_t axisIdx = 0; axisIdx < 3; axisIdx++)
				{
					ScaleVsLifeModifierParams* p = (ScaleVsLifeModifierParams*)
					                               traits->createNvParameterized(ScaleVsLifeModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ScaleVsLifeModifierImpl* cvlMod = PX_NEW(ScaleVsLifeModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the scale axis
					cvlMod->setScaleAxis((ScaleAxis)axisIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].lifeRemaining,
						                              cParams->controlPoints.buf[cpIdx].scale[axisIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ScaleVsDensity2DModifierParams"))
			{
				// setup an array so we can get the control points easily
				ScaleVsDensity2DModifierParams* cParams = (ScaleVsDensity2DModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (int32_t axisIdx = 0; axisIdx < 2; axisIdx++)
				{
					ScaleVsDensityModifierParams* p = (ScaleVsDensityModifierParams*)
					                                  traits->createNvParameterized(ScaleVsDensityModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ScaleVsDensityModifierImpl* cvlMod = PX_NEW(ScaleVsDensityModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the scale axis
					cvlMod->setScaleAxis((ScaleAxis)axisIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].density,
						                              cParams->controlPoints.buf[cpIdx].scale[axisIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ScaleVsDensity3DModifierParams"))
			{
				// setup an array so we can get the control points easily
				ScaleVsDensity3DModifierParams* cParams = (ScaleVsDensity3DModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (uint32_t axisIdx = 0; axisIdx < 3; axisIdx++)
				{
					ScaleVsDensityModifierParams* p = (ScaleVsDensityModifierParams*)
					                                  traits->createNvParameterized(ScaleVsDensityModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ScaleVsDensityModifierImpl* cvlMod = PX_NEW(ScaleVsDensityModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the scale axis
					cvlMod->setScaleAxis((ScaleAxis)axisIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].density,
						                              cParams->controlPoints.buf[cpIdx].scale[axisIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ScaleVsCameraDistance2DModifierParams"))
			{
				// setup an array so we can get the control points easily
				ScaleVsCameraDistance2DModifierParams* cParams = (ScaleVsCameraDistance2DModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (int32_t axisIdx = 0; axisIdx < 2; axisIdx++)
				{
					ScaleVsCameraDistanceModifierParams* p = (ScaleVsCameraDistanceModifierParams*)
					        traits->createNvParameterized(ScaleVsCameraDistanceModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ScaleVsCameraDistanceModifierImpl* cvlMod = PX_NEW(ScaleVsCameraDistanceModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the scale axis
					cvlMod->setScaleAxis((ScaleAxis)axisIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].cameraDistance,
						                              cParams->controlPoints.buf[cpIdx].scale[axisIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}

			else if (!nvidia::strcmp(param->className(), "ScaleVsCameraDistance3DModifierParams"))
			{
				// setup an array so we can get the control points easily
				ScaleVsCameraDistance3DModifierParams* cParams = (ScaleVsCameraDistance3DModifierParams*)param;

				if (cParams->controlPoints.arraySizes[0] == 0)
				{
					continue;
				}

				for (uint32_t axisIdx = 0; axisIdx < 3; axisIdx++)
				{
					ScaleVsCameraDistanceModifierParams* p = (ScaleVsCameraDistanceModifierParams*)
					        traits->createNvParameterized(ScaleVsCameraDistanceModifierParams::staticClassName());

					// save these parameterized classes off for later, we'll need to destroy them
					mCompositeParams.pushBack(p);

					ScaleVsCameraDistanceModifierImpl* cvlMod = PX_NEW(ScaleVsCameraDistanceModifierImpl)(p);
					PX_ASSERT(cvlMod);

					// set the scale axis
					cvlMod->setScaleAxis((ScaleAxis)axisIdx);

					// set the curve
					CurveImpl curve;
					for (int32_t cpIdx = 0; cpIdx < cParams->controlPoints.arraySizes[0]; cpIdx++)
					{
						curve.addControlPoint(Vec2R(cParams->controlPoints.buf[cpIdx].cameraDistance,
						                              cParams->controlPoints.buf[cpIdx].scale[axisIdx]));
					}
					cvlMod->setFunction(&curve);

					// save which sprite render semantics this modifier will update
					setSpriteSemanticsUsed(cvlMod->getModifierSpriteSemantics());
					setMeshSemanticsUsed(cvlMod->getModifierMeshSemantics());
					ModifierStack& activeStack = getModifierStack(i);
					activeStack.pushBack(cvlMod);
				}
			}
#endif /* IOFX_SLOW_COMPOSITE_MODIFIERS */

			if (newMod)
			{
				// save which sprite render semantics this modifier will update
				setSpriteSemanticsUsed(newMod->getModifierSpriteSemantics());
				setMeshSemanticsUsed(newMod->getModifierMeshSemantics());

				ModifierStack& activeStack = getModifierStack(i);
				activeStack.pushBack(newMod);
			}

		}
	}

	bool useFloat4Color = mParams->renderOutput.useFloat4Color;
	if (!useFloat4Color)
	{
		//detect if some Color modifier has a color component value greater than 1
		for (uint32_t modStage = 0; modStage < 2; modStage++)
		{
			const ModifierStack& activeStack = getModifierStack(modStage);

			uint32_t modCount = activeStack.size();
			for (uint32_t modIndex = 0; modIndex < modCount; modIndex++)
			{
				const nvidia::apex::Curve* pColorCurve = 0;

				const Modifier* pModifier = activeStack[modIndex];
				switch (pModifier->getModifierType())
				{
					case ModifierType_ColorVsLife:
						pColorCurve = static_cast<const ColorVsLifeModifier*>(pModifier)->getFunction();
						break;
					case ModifierType_ColorVsDensity:
						pColorCurve = static_cast<const ColorVsDensityModifier*>(pModifier)->getFunction();
						break;
					default:
						break;
				}

				if (pColorCurve)
				{
					uint32_t pointsCount;
					const Vec2R* pPoints = pColorCurve->getControlPoints(pointsCount);

					for (uint32_t i = 0; i < pointsCount; i++)
					{
						if (pPoints[i].y > 1)
						{
							useFloat4Color = true;
							break;
						}
					}
				}
				if (useFloat4Color)
				{
					break;
				}
			}
			if (useFloat4Color)
			{
				break;
			}
		}
	}

	if (mParams->renderOutput.useUserSemantic)
	{
		mMeshSemanticBitmap |= (1 << IofxRenderSemantic::USER_DATA);
		mSpriteSemanticBitmap |= (1 << IofxRenderSemantic::USER_DATA);
	}
}

const char* IofxAssetImpl::getSpriteMaterialName() const
{
	READ_ZONE();
	if (mSpriteParams &&
	        mSpriteParams->spriteMaterialName &&
	        mSpriteParams->spriteMaterialName->name() &&
	        mSpriteParams->spriteMaterialName->name()[0])
	{
		return mSpriteParams->spriteMaterialName->name();
	}
	else
	{
		return NULL;
	}
}

const char* IofxAssetImpl::getMeshAssetName(uint32_t index) const
{
	READ_ZONE();
	if (mRenderMeshList && index < (*mRenderMeshList).size() && (*mRenderMeshList)[index].meshAssetName)
	{
		return (*mRenderMeshList)[index].meshAssetName->name();
	}
	else
	{
		return NULL;
	}
}

uint32_t IofxAssetImpl::getMeshAssetWeight(uint32_t index) const
{
	READ_ZONE();
	if (mRenderMeshList && index < (*mRenderMeshList).size())
	{
		return (*mRenderMeshList)[index].weight;
	}
	else
	{
		return 0;
	}
}

bool IofxAssetImpl::isOpaqueMesh(uint32_t index) const
{
	if (mRenderMeshList && index < (*mRenderMeshList).size() && (*mRenderMeshList)[index].meshAssetName)
	{
		return nvidia::strcmp((*mRenderMeshList)[index].meshAssetName->className(), "ApexOpaqueMesh") == 0;
	}
	else
	{
		return false;
	}
}

uint32_t IofxAssetImpl::forceLoadAssets()
{
	WRITE_ZONE();
	uint32_t assetLoadedCount = 0;

	assetLoadedCount += mRenderMeshAssetTracker.forceLoadAssets();
	assetLoadedCount += mSpriteMaterialAssetTracker.forceLoadAssets();

	return assetLoadedCount;
}

void IofxAssetImpl::initializeAssetNameTable()
{
	// clean up asset tracker list
	mRenderMeshAssetTracker.removeAllAssetNames();
	mSpriteMaterialAssetTracker.removeAllAssetNames();

	if (mRenderMeshList)
	{
		/* initialize the rendermesh asset name to resID tables */
		for (uint32_t i = 0; i < (*mRenderMeshList).size(); i++)
		{
			const char* name = "";
			if ((*mRenderMeshList)[i].meshAssetName)
			{
				name = (*mRenderMeshList)[i].meshAssetName->name();
			}
			mRenderMeshAssetTracker.addAssetName(name, isOpaqueMesh(i));
		}
	}
	else
	{
		/* initialize the sprite material name to resID tables */
		if (getSpriteMaterialName())
		{
			mSpriteMaterialAssetTracker.addAssetName(getSpriteMaterialName(), false);
		}
	}
}

void IofxAssetImpl::setSpriteSemanticsUsed(uint32_t spriteSemanticsBitmap)
{
	mSpriteSemanticBitmap |= spriteSemanticsBitmap;
}
void IofxAssetImpl::setMeshSemanticsUsed(uint32_t meshSemanticsBitmap)
{
	mMeshSemanticBitmap |= meshSemanticsBitmap;
}

uint32_t IofxAssetImpl::getPubStateSize() const
{
	return mSpriteParams ? sizeof(SpritePublicState) : sizeof(MeshPublicState);
}

uint32_t IofxAssetImpl::getPrivStateSize() const
{
	return mSpriteParams ? sizeof(SpritePrivateState) : sizeof(MeshPrivateState);
}

bool IofxAssetImpl::isSortingEnabled() const
{
	const ModifierStack& stack = getModifierStack(ModifierStage_Continuous);
	for (ModifierStack::ConstIterator it = stack.begin(); it != stack.end(); ++it)
	{
		if ((*it)->getModifierType() == ModifierType_ViewDirectionSorting)
		{
			return true;
		}
	}

	return false;
}

#ifndef WITHOUT_APEX_AUTHORING

void IofxAssetAuthoringImpl::release()
{
	delete this;
}

const char* IofxAssetAuthoringImpl::getObjTypeName() const
{
	return IofxAssetImpl::getObjTypeName();
}

#if IOFX_AUTHORING_API_ENABLED

void IofxAssetAuthoringImpl::setMeshAssetCount(const uint32_t meshCount)
{
	(*mRenderMeshList).resize(meshCount);
}

void IofxAssetAuthoringImpl::setSpriteMaterialName(const char* inSpriteMaterialName)
{
	if ((*mRenderMeshList).size())
	{
		APEX_INVALID_OPERATION("Unable to set Sprite Material Name (%s). Systems can be either Mesh or Sprite, but not both. This system is already a Mesh.", getSpriteMaterialName());
		return;
	}

	NvParameterized::Handle h(*mParams);
	NvParameterized::Interface* refParam;

	mParams->getParameterHandle("spriteMaterialName", h);
	mParams->initParamRef(h, NULL, true);
	mParams->getParamRef(h, refParam);

	PX_ASSERT(refParam);
	if (refParam)
	{
		refParam->setName(inSpriteMaterialName);
	}
}

void IofxAssetAuthoringImpl::setMeshAssetName(const char* meshAssetName, uint32_t meshIndex)
{
	if (getSpriteMaterialName())
	{
		APEX_INVALID_OPERATION("Unable to set Mesh Material Name (%s). Systems can be either Mesh or Sprite, but not both. This system is already a Sprite.", meshAssetName);
		return;
	}

	PX_ASSERT(meshIndex < (*mRenderMeshList).size());

	NvParameterized::Handle arrayHandle(*mParams), indexHandle(*mParams), childHandle(*mParams);
	NvParameterized::Interface* refPtr;

	mParams->getParameterHandle("renderMeshList", arrayHandle);
	arrayHandle.getChildHandle(meshIndex, indexHandle);

	indexHandle.getChildHandle(mParams, "meshAssetName", childHandle);
	mParams->initParamRef(childHandle, NULL, true);
	mParams->getParamRef(childHandle, refPtr);
	PX_ASSERT(refPtr);
	if (refPtr)
	{
		refPtr->setName(meshAssetName);
	}
}

void IofxAssetAuthoringImpl::setMeshAssetWeight(const uint32_t weight, uint32_t meshIndex)
{
	if (getSpriteMaterialName())
	{
		APEX_INVALID_OPERATION("Unable to set Mesh Material Weight (%d). Systems can be either Mesh or Sprite, but not both. This system is already a Sprite.", weight);
		return;
	}

	PX_ASSERT(meshIndex < (*mRenderMeshList).size());
	(*mRenderMeshList)[meshIndex].weight = weight;
}


Modifier* IofxAssetAuthoringImpl::createModifier(uint32_t modStage, uint32_t modType)
{
	// create a modifier to go in this particular modifier stage
	// we'll need a handle to the correct mod stage list, pass that and the Param Obj to CreateModifier,
	// it will resize the array (+1), then pass the correct string to initParamRef()
	// Then it can pass the new ref param to the mod constructor
	NvParameterized::Handle h(*mParams), ih(*mParams);
	int modStackSize = 0;

	getModifierStack(modStage, h);

	// resize the array
	mParams->getArraySize(h, modStackSize);
	mParams->resizeArray(h, ++modStackSize);

	h.getChildHandle(modStackSize - 1, ih);

	Modifier* retVal = CreateModifier((ModifierTypeEnum)modType, mParams, ih);

	uint32_t assetTarget = getAssetTarget();
	// We don't yet know what type this is, which is invalid.
	if (assetTarget == 0)
	{
		// delete the new param ref and re-resize array
		NvParameterized::Interface* refParam = 0;
		mParams->getParamRef(ih, refParam);
		PX_ASSERT(refParam);
		if (refParam)
		{
			refParam->destroy();
		}
		mParams->resizeArray(h, modStackSize - 1);

		delete retVal;
		retVal = 0;
		APEX_INVALID_OPERATION("Specifying modifiers before specifying the mesh or sprite asset is invalid");
		return retVal;
	}

	// The modifier only supports certain semantics, so let's test those now.
	const uint32_t supportedUsage = retVal->getModifierUsage();

	if ((supportedUsage & assetTarget) != assetTarget)
	{
		// delete the new param ref and re-resize array
		NvParameterized::Interface* refParam = 0;
		mParams->getParamRef(ih, refParam);
		PX_ASSERT(refParam);
		if (refParam)
		{
			refParam->destroy();
		}
		mParams->resizeArray(h, modStackSize - 1);

		delete retVal;
		retVal = 0;
		APEX_INVALID_OPERATION("The specified modifier doesn't work on that system type (e.g. Sprite ModifierImpl on a Mesh System or vice-versa).");
		return retVal;
	}

	uint32_t stageBits = ModifierUsageFromStage(ModifierStage(modStage));
	if ((supportedUsage & stageBits) != stageBits)
	{
		// delete the new param ref and re-resize array
		NvParameterized::Interface* refParam = 0;
		mParams->getParamRef(ih, refParam);
		PX_ASSERT(refParam);
		if (refParam)
		{
			refParam->destroy();
		}
		mParams->resizeArray(h, modStackSize - 1);

		delete retVal;
		retVal = 0;
		APEX_INVALID_OPERATION("The specified modifier doesn't work in that stage.");
		return retVal;
	}

	ModifierStack& activeStack = getModifierStack(modStage);

	activeStack.pushBack(retVal);
	return retVal;
}

void IofxAssetAuthoringImpl::removeModifier(uint32_t modStage, uint32_t position)
{
	if (modStage >= ModifierStage_Count)
	{
		APEX_INVALID_OPERATION("Invalid modifier stage");
		return;
	}

	ModifierStack& activeStack = getModifierStack(modStage);

	if (position >= activeStack.size())
	{
		APEX_INVALID_OPERATION("position %d is greater than modifier stack size: %d", position, activeStack.size());
		return;
	}

	// remove from ModifierStack (runtime)
	activeStack.replaceWithLast(position);

	// remove from NvParameterized data
	NvParameterized::Handle h(*mParams), ih(*mParams);
	NvParameterized::Interface* modParams = 0;
	int modStackSize = 0;

	// 1. get the correct stack
	getModifierStack(modStage, h);

	// 2. get the NvParameterized::Interface* for the position
	mParams->getArraySize(h, modStackSize);
	h.getChildHandle(position, ih);
	mParams->getParamRef(ih, modParams);
	PX_ASSERT(modParams);

	// 3. destroy it
	modParams->destroy();

	// 4. copy the last member of the stack to position
	if (position != (uint32_t)modStackSize - 1)
	{
		NvParameterized::Handle lastH(*mParams);
		h.getChildHandle(modStackSize - 1, lastH);
		mParams->getParamRef(lastH, modParams);
		mParams->setParamRef(ih, modParams);
	}

	// 5. resize the stack
	mParams->resizeArray(h, modStackSize - 1);
}

uint32_t IofxAssetAuthoringImpl::getModifierCount(uint32_t modStage) const
{
	if (modStage >= ModifierStage_Count)
	{
		APEX_INVALID_OPERATION("Invalid modifier stage");
		return 0;
	}

	const ModifierStack& activeStack = getModifierStack(modStage);
	return activeStack.size();
}

uint32_t IofxAssetAuthoringImpl::findModifier(uint32_t modStage, Modifier* modifier)
{
	if (modStage >= ModifierStage_Count)
	{
		APEX_INVALID_OPERATION("Invalid modifier stage");
		return 0;
	}

	const ModifierStack& activeStack = getModifierStack(modStage);

	for (uint32_t i = 0; i < activeStack.size(); i++)
	{
		if (activeStack[i] == modifier)
		{
			return i;
		}
	}

	return 0xffffffff;
}

Modifier* IofxAssetAuthoringImpl::getModifier(uint32_t modStage, uint32_t position) const
{
	if (modStage >= ModifierStage_Count)
	{
		APEX_INVALID_OPERATION("Invalid modifier stage");
		return 0;
	}

	const ModifierStack& activeStack = getModifierStack(modStage);

	if (position >= activeStack.size())
	{
		APEX_INVALID_OPERATION("position %d is greater than modifier stack size: %d", position, activeStack.size());
		return 0;
	}

	// remove from ModifierStack (runtime)
	return activeStack[ position ];
}

uint32_t IofxAssetAuthoringImpl::getAssetTarget() const
{
	uint32_t retVal = 0;
	if (getSpriteMaterialName())
	{
		retVal |= ModifierUsage_Sprite;
	}
	else if ((*mRenderMeshList).size())
	{
		retVal |= ModifierUsage_Mesh;
	}

	return retVal;
}

#endif /* IOFX_AUTHORING_API_ENABLED */

#endif

}
} // namespace nvidia

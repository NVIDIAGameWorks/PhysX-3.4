/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SampleUIController.h"
#include "SampleSceneController.h"
#include "CommonUIController.h"

SampleUIController::SampleUIController(SampleSceneController* s, CommonUIController* c) : mScene(s), mCommonUIController(c)
{
}

void SampleUIController::onInitialize()
{
	TwBar* sampleBar = TwNewBar("Sample");
	PX_UNUSED(sampleBar);
	TwDefine("Sample color='19 25 59' alpha=128 text=light size='200 150' iconified=false valueswidth=150 position='12 480' label='Select Asset'");

	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_POSITION);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_VELOCITY);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_COLLISION_NORMAL);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_BOUNDS);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_GRID);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_BROADPHASE_BOUNDS);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::ePARTICLE_SYSTEM_MAX_MOTION_DISTANCE);

	mCommonUIController->addApexDebugRenderParam("VISUALIZE_TURBULENCE_FS_VELOCITY", "TurbulenceFS", 1.0f, "TurbulenceFS velocities");
	mCommonUIController->addApexDebugRenderParam("VISUALIZE_TURBULENCE_FS_BBOX", "TurbulenceFS", 1.0f, "TurbulenceFS BBOX");
	mCommonUIController->addApexDebugRenderParam("VISUALIZE_TURBULENCE_FS_ACTOR_NAME", "TurbulenceFS", 1.0f, "TurbulenceFS name");
	mCommonUIController->addApexDebugRenderParam("VISUALIZE_TURBULENCE_FS_VELOCITY_FIELD", "TurbulenceFS", 1.0f, "TurbulenceFS velocity field");
	mCommonUIController->addApexDebugRenderParam("VISUALIZE_TURBULENCE_FS_STREAMLINES", "TurbulenceFS", 1.0f, "TurbulenceFS streamlines");
	mCommonUIController->addApexDebugRenderParam("VISUALIZE_IOFX_ACTOR", "Iofx", 1.0f, "IOFX actor");
	mCommonUIController->addApexDebugRenderParam("apexEmitterParameters.VISUALIZE_APEX_EMITTER_ACTOR", "Emitter", 1.0f, "Emitter actor");
}

LRESULT SampleUIController::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PX_UNUSED(hWnd);
	PX_UNUSED(uMsg);
	PX_UNUSED(wParam);
	PX_UNUSED(lParam);;

	return 1;
}

void SampleUIController::BackBufferResized(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	PX_UNUSED(pDevice);

	mWidth = pBackBufferSurfaceDesc->Width;
	mHeight = pBackBufferSurfaceDesc->Height;
}

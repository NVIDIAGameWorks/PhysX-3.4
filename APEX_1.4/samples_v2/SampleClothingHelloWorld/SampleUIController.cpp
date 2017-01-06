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

	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eBODY_ANG_VELOCITY);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eBODY_LIN_VELOCITY);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eBODY_MASS_AXES);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_AABBS);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_SHAPES);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_AXES);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_COMPOUNDS);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_FNORMALS);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_EDGES);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_STATIC);
	mCommonUIController->addPhysXDebugRenderParam(PxVisualizationParameter::eCOLLISION_DYNAMIC);

	mCommonUIController->addApexDebugRenderParam("LodBenefits");
	mCommonUIController->addApexDebugRenderParam("RenderNormals");
	mCommonUIController->addApexDebugRenderParam("RenderTangents");
	mCommonUIController->addApexDebugRenderParam("Bounds");
	mCommonUIController->addApexDebugRenderParam("PhysicsMeshWire", "Clothing", 0.9f, "Clothing Physics Mesh(Wire)");
	mCommonUIController->addApexDebugRenderParam("SolverMode", "Clothing", 1.0f, "SolverMode");
	mCommonUIController->addApexDebugRenderParam("Wind", "Clothing", 0.1f, "Wind");
	mCommonUIController->addApexDebugRenderParam("Velocities", "Clothing", 0.1f, "Velocities");
	mCommonUIController->addApexDebugRenderParam("SkinnedPositions", "Clothing", 1.0f, "Clothing Skinned Positions");
	mCommonUIController->addApexDebugRenderParam("Backstop", "Clothing", 1.0f, "Backstop");
	mCommonUIController->addApexDebugRenderParam("MaxDistance", "Clothing", 1.0f, "MaxDistance");
	mCommonUIController->addApexDebugRenderParam("LengthFibers", "Clothing", 1.0f, "LengthFibers");
	mCommonUIController->addApexDebugRenderParam("CrossSectionFibers", "Clothing", 1.0f, "CrossSectionFibers");
	mCommonUIController->addApexDebugRenderParam("TethersActive", "Clothing", 1.0f, "TethersActive");
	mCommonUIController->addApexDebugRenderParam("SelfCollision", "Clothing", 1.0f, "SelfCollision");
	mCommonUIController->addApexDebugRenderParam("CollisionShapes", "Clothing", 1.0f, "Solid Clothing Collision");
}

LRESULT SampleUIController::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PX_UNUSED(hWnd);
	PX_UNUSED(wParam);
	PX_UNUSED(lParam);

	if(uMsg == WM_LBUTTONDOWN)
	{
		mScene->throwSphere();
	}

	return 1;
}

void SampleUIController::BackBufferResized(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	PX_UNUSED(pDevice);

	mWidth = pBackBufferSurfaceDesc->Width;
	mHeight = pBackBufferSurfaceDesc->Height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//									UI Callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

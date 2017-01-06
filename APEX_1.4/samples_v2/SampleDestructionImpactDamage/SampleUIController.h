/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SAMPLE_UI_CONTROLLER_H
#define SAMPLE_UI_CONTROLLER_H

#include "SampleManager.h"
#include <DirectXMath.h>
#include "AntTweakBar.h"

class SampleSceneController;
class CommonUIController;

class SampleUIController : public ISampleController
{
  public:
	SampleUIController(SampleSceneController* s, CommonUIController* c);

	virtual void onInitialize();
	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Render(ID3D11Device*, ID3D11DeviceContext*, ID3D11RenderTargetView*, ID3D11DepthStencilView*);
	virtual void BackBufferResized(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

	static void TW_CALL setForceToDamage(const void* value, void* clientData);
	static void TW_CALL getForceToDamage(void* value, void* clientData);

	static void TW_CALL setDamageThreshold(const void* value, void* clientData);
	static void TW_CALL getDamageThreshold(void* value, void* clientData);

	static void TW_CALL setDamageCap(const void* value, void* clientData);
	static void TW_CALL getDamageCap(void* value, void* clientData);

  private:
	SampleSceneController* mScene;
	CommonUIController* mCommonUIController;
	TwBar* mSettingsBar;

	UINT mWidth;
	UINT mHeight;
};

#endif
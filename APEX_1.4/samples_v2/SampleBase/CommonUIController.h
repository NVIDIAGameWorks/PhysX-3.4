/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef COMMON_UI_CONTROLLER_H
#define COMMON_UI_CONTROLLER_H

#include "SampleManager.h"
#include <DirectXMath.h>
#include "AntTweakBar.h"
#pragma warning(push)
#pragma warning(disable : 4350)
#include <string>
#include <list>
#pragma warning(pop)
#include "PxPhysicsAPI.h"

class CFirstPersonCamera;
class ApexRenderer;
class ApexController;

class CommonUIController : public ISampleController
{
  public:
	CommonUIController(CFirstPersonCamera* cam, ApexRenderer* r, ApexController* a);
	virtual ~CommonUIController() {};

	virtual HRESULT DeviceCreated(ID3D11Device* pDevice);
	virtual void DeviceDestroyed();
	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Animate(double fElapsedTimeSeconds);
	virtual void Render(ID3D11Device*, ID3D11DeviceContext*, ID3D11RenderTargetView*, ID3D11DepthStencilView*);
	virtual void BackBufferResized(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

	void addHintLine(std::string hintLine);

	void addApexDebugRenderParam(std::string name, std::string module = "", float value = 1.0f, std::string uiName = "");
	void addPhysXDebugRenderParam(physx::PxVisualizationParameter::Enum parameter);

	static void TW_CALL setWireframeEnabled(const void* value, void* clientData);
	static void TW_CALL getWireframeEnabled(void* value, void* clientData);

	static void TW_CALL setDebugRenderParam(const void* value, void* clientData);
	static void TW_CALL getDebugRenderParam(void* value, void* clientData);

	static void TW_CALL onReloadShadersButton(void* clientData);

	static void TW_CALL setFixedTimestepEnabled(const void* value, void* clientData);
	static void TW_CALL getFixedTimestepEnabled(void* value, void* clientData);

	static void TW_CALL setFixedSimFrequency(const void* value, void* clientData);
	static void TW_CALL getFixedSimFrequency(void* value, void* clientData);

private:
	void toggleCameraSpeed(bool overspeed);

	CFirstPersonCamera* mCamera;
	ApexRenderer* mApexRenderer;
	ApexController* mApexController;
	TwBar* mSettingsBar;

	UINT mWidth;
	UINT mHeight;

	std::list<std::string> mHintOnLines;
	std::list<std::string> mHintOffLines;
	bool mShowHint;

	class IDebugRenderParam
	{
	public:
		virtual ~IDebugRenderParam() {}
		virtual bool isParamEnabled() = 0;
		virtual void setParamEnabled(bool) = 0;
	};

	class ApexDebugRenderParam : public IDebugRenderParam
	{
	public:
		ApexDebugRenderParam(CommonUIController* controller, std::string name, std::string module, float value)
			: mController(controller), mName(name), mModule(module), mValue(value) {}

		virtual bool isParamEnabled();
		virtual void setParamEnabled(bool);

	private:
		CommonUIController* mController;
		std::string mName;
		std::string mModule;
		float mValue;
	};

	class PhysXDebugRenderParam : public IDebugRenderParam
	{
	public:
		PhysXDebugRenderParam(CommonUIController* controller, physx::PxVisualizationParameter::Enum parameter)
			: mController(controller), mParameter(parameter) {}

		virtual bool isParamEnabled();
		virtual void setParamEnabled(bool);

	private:
		CommonUIController* mController;
		physx::PxVisualizationParameter::Enum mParameter;
	};

	std::list<IDebugRenderParam*> mDebugRenderParams;
};

#endif
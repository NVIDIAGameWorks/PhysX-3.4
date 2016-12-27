#include "Utils.h"

#include <DirectXMath.h>
#include "XInput.h"
#include "DXUTMisc.h"
#include "DXUTCamera.h"


#include "ApexController.h"
#include "ApexRenderer.h"
#include "CommonUIController.h"
#include "SampleUIController.h"
#include "SampleSceneController.h"

#include "SampleManager.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	PX_UNUSED(hInstance);
	PX_UNUSED(hPrevInstance);
	PX_UNUSED(lpCmdLine);
	PX_UNUSED(nCmdShow);

// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
#endif

	SampleManager* sampleManager = new SampleManager(L"APEX Particles Sample: Impact Particles");

	CFirstPersonCamera camera;

	auto apexController = ApexController(PxDefaultSimulationFilterShader, &camera);
	auto apexRender = ApexRenderer(&camera, apexController);
	auto sceneController = SampleSceneController(&camera, apexController);
	auto commonUiController = CommonUIController(&camera, &apexRender, &apexController);
	auto sampleUIController = SampleUIController(&sceneController, &commonUiController);

	sampleManager->addControllerToFront(&apexController);
	sampleManager->addControllerToFront(&apexRender);
	sampleManager->addControllerToFront(&sceneController);
	sampleManager->addControllerToFront(&sampleUIController);
	sampleManager->addControllerToFront(&commonUiController);

	int result = sampleManager->run();

	delete sampleManager;

	return result;
}

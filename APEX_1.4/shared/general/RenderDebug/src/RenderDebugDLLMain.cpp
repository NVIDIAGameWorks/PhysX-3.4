#include "RenderDebug.h"


extern "C" 
#if PX_WINDOWS_FAMILY != 0
__declspec(dllexport)
#endif
RENDER_DEBUG::RenderDebug* createRenderDebugExport(RENDER_DEBUG::RenderDebug::Desc &desc)
{
	return createRenderDebug(desc);
}

#if PX_WIN32 || PX_WIN64

#include "windows/PsWindowsInclude.h"
#pragma warning(disable:4100 4127)


BOOL APIENTRY DllMain( HANDLE ,
					  DWORD ul_reason_for_call,
					  LPVOID )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif


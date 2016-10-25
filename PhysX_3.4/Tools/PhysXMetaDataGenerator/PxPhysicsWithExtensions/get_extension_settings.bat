@echo off

rem Find SDK_ROOT by searching backwards from cwd for SDKs
set SDK_ROOT=%~p0
:sdkloop
if exist "%SDK_ROOT%\Source" goto :havesdkpath
set SDK_ROOT=%SDK_ROOT%..\
goto :sdkloop

:havesdkpath

call "%SDK_ROOT%\Tools\PhysXMetaDataGenerator\get_common_settings.bat"

set SRCPATH=PxPhysicsWithExtensionsAPI.h

if not defined TARGETDIR (set TARGETDIR="%SDK_ROOT%\Source\PhysXMetaData")



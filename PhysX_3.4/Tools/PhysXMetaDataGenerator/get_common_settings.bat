@echo off

set EXTERNALS=%~p0
:externalsLoop
if exist "%EXTERNALS%\externals" goto :haveExternalsPath
set EXTERNALS=%EXTERNALS%..\
goto :externalsLoop

:haveExternalsPath

rem Find SDK_ROOT by searching backwards from cwd for SDKs
set SDK_ROOT=%~p0
:sdkloop
if exist "%SDK_ROOT%\Source" goto :havesdkpath
set SDK_ROOT=%SDK_ROOT%..\
goto :sdkloop

:havesdkpath

if exist %SDK_ROOT%\..\PhysX_3.4 goto :distro
set PHYSX_ROOT=%~p0
:rootloop
if exist %PHYSX_ROOT%\PhysXSDK goto :haverootpath
set PHYSX_ROOT=%PHYSX_ROOT%..\
goto :rootloop

:haverootpath

rem We should read the paths from the buildconf!
set PX_SHARED=%PHYSX_ROOT%/PxShared/1.0/trunk
goto :havesharedpath

:distro

set PX_SHARED=%SDK_ROOT%/../PxShared/
set BOILERPLATE_FILE=%SDK_ROOT%/Tools/PhysXMetaDataGenerator/PxBoilerPlate.h

:havesharedpath

set INCLUDES=-I"%SDK_ROOT%/Include/foundation"
set INCLUDES=%INCLUDES% -I"%PX_SHARED%/include"
set INCLUDES=%INCLUDES% -I"%PX_SHARED%/src/foundation/include"
set INCLUDES=%INCLUDES% -I"%PX_SHARED%/src/pvd/include"
set INCLUDES=%INCLUDES% -I"%PX_SHARED%/src/fastxml/include"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/common"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/geometry"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/pvd"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/particles"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/cloth"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include/gpu"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Include"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysXCommon/src"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/GeomUtils/headers"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/GeomUtils/src"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/GeomUtils/Opcode"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysX/src"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysX/src/buffering"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysX/src/particles"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysX/src/cloth"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/SimulationController/src"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/SimulationController/src/framework"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/SimulationController/include"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysXCooking/include"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/SceneQuery"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysXMetaData/core/include"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Source/PhysXGpu/include"
set INCLUDES=%INCLUDES% -I"%SDK_ROOT%/Tools/PhysXMetaDataGenerator"

rem stddef.h doesn't compile with VS10 and -std=c++0x
set WINDOWS_SYS_PATH=-isystem"%VS110COMNTOOLS%/../../VC/include"

set COMMON_FLAGS=-cc1 -x c++-header -fms-extensions -w -nobuiltininc -boilerplate-file %BOILERPLATE_FILE%

set WINDOWS_DEFINES=    -DNDEBUG -DPX_VC=11 -D_WIN32                        -std=c++0x

set EXE="%EXTERNALS%\externals\clang\3.3.3\win32\bin\clang.exe"

if exist %EXE% goto :theEnd
echo Not found %EXE%
pause

:theEnd

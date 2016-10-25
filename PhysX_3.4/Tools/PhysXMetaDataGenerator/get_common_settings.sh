#!/bin/bash
#search sdk root

SCRIPT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ ! "$SDK_ROOT" ]; then
	sdk_root=$SCRIPT_DIR;
	while [ ! -e $sdk_root"/Source" ]; do
		sdk_root="$sdk_root""/.."
	done
	export SDK_ROOT="$sdk_root"
fi

echo SDK_ROOT is $SDK_ROOT

# distro
if [ -e $sdk_root"/../PhysX_3.4" ]; then
	export PHYSX_ROOT="$sdk_root/.."
	export EXTERNALS="$PHYSX_ROOT/Externals"
fi

if [ ! "$EXTERNALS" ]; then
externals=$SCRIPT_DIR;
	while [ ! -e $externals"/externals" ]; do
		externals="$externals""/.."
	done
	export EXTERNALS="$externals/externals"
fi

#search physx root
if [ ! "$PHYSX_ROOT" ]; then
	physx_root=$SCRIPT_DIR;
	while [ ! -e $physx_root"/PhysXSDK" ]; do
		physx_root="$physx_root""/.."
	done
	export PHYSX_ROOT="$physx_root";
	export DEVREL="$physx_root/../devrel"
fi

export PX_SHARED="$PHYSX_ROOT/PxShared/1.0/trunk"

# distro
if [ -e $sdk_root"/../PhysX_3.4" ]; then
	export PX_SHARED="$sdk_root/../PxShared"
	export BOILERPLATE_FILE="$sdk_root/Tools/PhysXMetaDataGenerator/PxBoilerPlate.h"
fi

INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/foundation""
INCLUDES="$INCLUDES -I"$PX_SHARED/include""
INCLUDES="$INCLUDES -I"$PX_SHARED/src/foundation/include""
INCLUDES="$INCLUDES -I"$PX_SHARED/src/pvd/include""
INCLUDES="$INCLUDES -I"$PX_SHARED/src/fastxml/include""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/common""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/geometry""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/pvd""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/particles""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/cloth""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include/gpu""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Include""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysXCommon/src""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/GeomUtils/headers""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/GeomUtils/src""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/GeomUtils/Opcode""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysX/src""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysX/src/buffering""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysX/src/particles""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysX/src/cloth""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/SimulationController/src""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/SimulationController/src/framework""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/SimulationController/include""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysXCooking/include""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/SceneQuery""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysXMetaData/core/include""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Source/PhysXGpu/include""
INCLUDES="$INCLUDES -I"$SDK_ROOT/Tools/PhysXMetaDataGenerator""

COMMON_FLAGS="-fms-extensions -w -boilerplate-file $BOILERPLATE_FILE"
LINUX_DEFINES="-x c++-header -DNDEBUG -std=c++0x"
OSX_DEFINES="-x c++-header -DNDEBUG -std=c++0x"

export OSX_EXE="$EXTERNALS/clang/3.3.3/osx/bin/clang"

export LINUX_EXE="$EXTERNALS/clang/3.3.3/linux32/bin/clang"

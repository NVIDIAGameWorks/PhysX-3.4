#!/bin/bash

SCRIPT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ ! "$SDK_ROOT" ]; then
	sdk_root=$SCRIPT_DIR;
	while [ ! -e $sdk_root"/Source" ]; do
		sdk_root="$sdk_root""/.."
	done
	export SDK_ROOT="$sdk_root"
fi

source "$SDK_ROOT/Tools/PhysXMetaDataGenerator/get_common_settings.sh"

SRCPATH=PxPhysicsWithExtensionsAPI.h

if [ ! "$TARGETDIR" ]; then
  TARGETDIR="$SDK_ROOT/Source/PhysXMetaData"
fi

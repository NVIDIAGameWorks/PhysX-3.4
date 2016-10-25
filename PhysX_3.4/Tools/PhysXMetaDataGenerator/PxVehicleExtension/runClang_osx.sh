#!/bin/bash

export SDK_ROOT=""
export EXTERNALS=""

sdkparam=`xcodebuild -showsdks | awk '/^$/{p=0};p; /OS X SDKs:/{p=1}' | tail -1 | cut -f3`
sdkpath=`xcodebuild -version $sdkparam Path`

source get_extension_settings.sh
OSX_SYS_PATH=-isysroot"$sdkpath"
CMD="$OSX_EXE $COMMON_FLAGS $OSX_DEFINES $INCLUDES $OSX_SYS_PATH $SRCPATH -o $TARGETDIR"

echo $CMD>>clangCommandLine_osx.txt
$CMD

exit $?



#!/bin/bash

export SDK_ROOT=""
export EXTERNALS=""

source get_extension_settings.sh

CMD="$LINUX_EXE $COMMON_FLAGS $LINUX_DEFINES $INCLUDES $SRCPATH -o $TARGETDIR"

echo $CMD>>clangCommandLine_linux.txt
$CMD

exit $?

#!/usr/bin/env bash

# See https://github.com/musescore/musescore_devtools/tree/main/include-what-you-use

BUILD_DIR=$1
OUT=$2

IWYU_TOOL=$(which iwyu_tool.py)
IWYU_TOOL_DIR=$(dirname $IWYU_TOOL)
IWYU_IMP=$IWYU_TOOL_DIR/../imp/default.imp

echo "$IWYU_TOOL -p $BUILD_DIR -- -Xiwyu --mapping_file=$IWYU_IMP --transitive_includes_only --no_comments --cxx17ns > $OUT"
$IWYU_TOOL -p $BUILD_DIR -- -Xiwyu --mapping_file=$IWYU_IMP -Xiwyu --transitive_includes_only -Xiwyu --no_comments -Xiwyu --cxx17ns > $OUT

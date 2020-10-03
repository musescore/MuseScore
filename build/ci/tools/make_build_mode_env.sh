#!/usr/bin/env bash

ARTIFACTS_DIR="build.artifacts"

EVENT=""
BUILD_MODE=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -e|--event) EVENT="$2"; shift ;;
        -m|--mode) BUILD_MODE="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_MODE" ]; then 
    if [ -z "$EVENT" ]; then echo "error: not set EVENT"; exit 1; fi

    if [ "$EVENT" == "pull_request" ]; then BUILD_MODE=devel_build; fi
    if [ "$EVENT" == "schedule" ]; then BUILD_MODE=nightly_build; fi
    if [ "$EVENT" == "workflow_dispatch" ]; then echo "error: event workflow_dispatch, but not set BUILD_MODE"; exit 1; fi
fi

echo "EVENT: $EVENT"

MODE_IS_VALID=0
if [ "$BUILD_MODE" == "devel_build" ]; then MODE_IS_VALID=1; fi
if [ "$BUILD_MODE" == "nightly_build" ]; then MODE_IS_VALID=1; fi
if [ "$BUILD_MODE" == "testing_build" ]; then MODE_IS_VALID=1; fi
if [ "$BUILD_MODE" == "stable_build" ]; then MODE_IS_VALID=1; fi

if [ $MODE_IS_VALID -ne 1 ] ; then
    echo "error: Not valid build mode: $BUILD_MODE"
    echo "valid modes: "
    echo "  devel_build - build for developers, usually builds on pull request"
    echo "  nightly_build - build for testing current development, usually builds once a day"
    echo "  testing_build - build for testing before release (alpha, beta, rc), usually builds manually"
    echo "  stable_build - build for release, usually builds manually"
    exit 1
fi

mkdir $ARTIFACTS_DIR
mkdir $ARTIFACTS_DIR/env

echo $BUILD_MODE > $ARTIFACTS_DIR/env/build_mode.env
echo "BUILD_MODE: $(cat $ARTIFACTS_DIR/env/build_mode.env)"


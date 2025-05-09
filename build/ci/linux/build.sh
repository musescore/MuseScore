#!/usr/bin/env bash
echo "############################## Build Linux MuseScore (build.sh) ##############################"
trap 'echo build.sh failed; exit 1' ERR

df -h .

# PARAM

#   optional
TELEMETRY_TRACK_ID=""
VTESTPREFIX=''
#   consume
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --telemetry) TELEMETRY_TRACK_ID="$2"; shift ;;
        -p) VTESTPREFIX="$2"; shift ;;
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --build_mode) BUILD_MODE="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done
#   required
if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$BUILD_MODE" ]; then echo "error: not set BUILD_MODE"; exit 1; fi

# INIT and MAIN

if [[ -n "$VTESTPREFIX" ]]; then
  BUILD_TOOLS=$HOME/$VTESTPREFIX/build_tools
else
  BUILD_TOOLS=$HOME/build_tools
fi
ENV_FILE=$BUILD_TOOLS/environment.sh
source "$ENV_FILE"
ARTIFACTS_DIR=build.artifacts
BUILDTYPE=portable # portable build is the default build
MUSESCORE_BUILD_CONFIG=dev
SUFFIX="" # appended to `mscore` command name to avoid conflicts (e.g. `mscore-dev`)
OPTIONS=""

case "${BUILD_MODE}" in
"devel")   MUSESCORE_BUILD_CONFIG=dev; SUFFIX=-dev;;
"nightly") MUSESCORE_BUILD_CONFIG=dev; SUFFIX=-nightly;;
"testing") MUSESCORE_BUILD_CONFIG=testing; SUFFIX=-testing;;
"stable")  MUSESCORE_BUILD_CONFIG=release; SUFFIX="";;
"mtests")  MUSESCORE_BUILD_CONFIG=dev; BUILDTYPE=installdebug; OPTIONS="USE_SYSTEM_FREETYPE=ON UPDATE_CACHE=FALSE PREFIX=$ARTIFACTS_DIR/software";;
"vtests")  MUSESCORE_BUILD_CONFIG=dev; BUILDTYPE=installdebug; OPTIONS="COVERAGE=ON DOWNLOAD_SOUNDFONT=OFF PREFIX=$HOME/$VTESTPREFIX";;
esac

if [ "${BUILDTYPE}" == "portable" ]; then
  SUFFIX="-portable${SUFFIX}" # special value needed for CMakeLists.txt
fi

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "TELEMETRY_TRACK_ID: $TELEMETRY_TRACK_ID"
echo "BUILD_MODE: $BUILD_MODE"
echo "BUILDTYPE: $BUILDTYPE"
echo "OPTIONS: $OPTIONS"
echo " "
cat "$ENV_FILE"
${CXX} --version
${CC} --version
echo " "
cmake --version
echo " "

echo "=== BUILD ==="

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

if [[ -n $VTESTPREFIX ]]; then mkdir -p $VTESTPREFIX; fi

make CPUS=4 $OPTIONS \
     MUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG \
     MUSESCORE_REVISION=$MUSESCORE_REVISION \
     BUILD_NUMBER=$BUILD_NUMBER \
     TELEMETRY_TRACK_ID=$TELEMETRY_TRACK_ID \
     SUFFIX=$SUFFIX \
     $BUILDTYPE

if [[ "$BUILD_MODE" != "vtests" ]]; then
  bash ./build/ci/tools/make_release_channel_env.sh -c $MUSESCORE_BUILD_CONFIG
  bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
  bash ./build/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
  bash ./build/ci/tools/make_branch_env.sh
  bash ./build/ci/tools/make_datetime_env.sh
fi

df -h .

echo "build.sh ended"
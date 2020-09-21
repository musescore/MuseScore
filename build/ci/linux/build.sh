#!/usr/bin/env bash

echo "Build Linux MuseScore AppImage"

# Go one-up from MuseScore root dir regardless of where script was run from:
cd "$(dirname "$(readlink -f "${0}")")/../../../.."

##########################################################################
# SETUP ENVIRONMENT
##########################################################################
echo "=== ENVIRONMENT === "

BUILD_MODE=$1

ENV_FILE=./musescore_environment.sh
cat ${ENV_FILE}
. ${ENV_FILE}

${CXX} --version 
${CC} --version
echo " "
cmake --version
echo "===================="

BUILD_UI_MU4=OFF
if [ ${BUILD_MODE} ] && [ ${BUILD_MODE} == "mu4" ] 
then 
    BUILD_UI_MU4=ON
fi 

##########################################################################
# BUILD MUSESCORE
##########################################################################

cd MuseScore
#rm -rf ./build.*
make revision
make "BUILD_UI_MU4=${BUILD_UI_MU4}" portable 
cd ..


#!/bin/bash

set -ex

PROJECT_NAME=libsndfile

# Clone the oss-fuzz repository
git clone https://github.com/google/oss-fuzz.git /tmp/ossfuzz

if [[ ! -d /tmp/ossfuzz/projects/${PROJECT_NAME} ]]
then
    echo "Could not find the ${PROJECT_NAME} project in ossfuzz"

    # Exit with a success code while the libsndfile project is not expected to exist
    # on oss-fuzz.
    exit 0
fi

# Work out which branch to clone from, inside Docker
BRANCH=${GITHUB_REF}

# Modify the oss-fuzz Dockerfile so that we're checking out the current reference on CI.
sed -i "s@RUN.*@RUN git config --global remote.origin.fetch '+refs/pull/*:refs/remotes/origin/pull/*' \&\& git clone https://github.com/erikd/libsndfile.git /src/libsndfile \&\& cd /src/libsndfile \&\& git checkout -b ${BRANCH}@" /tmp/ossfuzz/projects/${PROJECT_NAME}/Dockerfile

# Try and build the fuzzers
pushd /tmp/ossfuzz
python infra/helper.py build_image --pull ${PROJECT_NAME}
python infra/helper.py build_fuzzers ${PROJECT_NAME}
python infra/helper.py check_build ${PROJECT_NAME} --engine libfuzzer --sanitizer address --architecture x86_64
popd

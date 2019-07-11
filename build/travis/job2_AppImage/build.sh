#!/bin/bash
# Run this script from MuseScore's root directory

# Build portable Linux AppImages and upload them to Bintray. AppImages will
# always be uploaded unless a list of specific branches is passed in. e.g.:
#    $   build.sh  --upload-branches  master  my-branch-1  my-branch-2

set -e # exit on error
set -x # echo commands

env_error_msg="Required environment variable is empty or unset."

TARGET_ARCH="${TARGET_ARCH:?${env_error_msg}}" # env

date="$(date -u +%Y%m%d%H%M)"

branch="$TRAVIS_BRANCH"
[ "$branch" ] || branch="$(git rev-parse --abbrev-ref HEAD)"

revision="$(echo "$TRAVIS_COMMIT" | cut -c 1-7)"
[ "$revision" ] || revision="$(make revision && cat mscore/revision.h)"

docker_tag="latest" # Docker terminology for master branch
[ "$branch" == "master" ] || docker_tag="$branch" # other branches use branch name

function rebuild-docker-image() { # $1 is arch (e.g. x86_64)
  set +e # allow errors
  echo "TRAVIS_COMMIT_RANGE: ${TRAVIS_COMMIT_RANGE}" # https://github.com/travis-ci/travis-ci/issues/4596
  changed_files="$(git diff --name-only ${TRAVIS_COMMIT_RANGE/.../..})" # error on force push
  (($? == 0)) || changed_files="$(git diff --name-only HEAD~3)" # check recent commits
  if grep "^build/Linux+BSD/portable" <<<"${changed_files}"; then
    # Need to update image on Docker Cloud
    set +x # keep env secret
    echo "Triggering rebuild of $DOCKER_USER/musescore-$1$docker_tag on Docker Cloud."
    data="{\"source_type\": \"Branch\", \"source_name\": \"$branch\"}"
    url="https://registry.hub.docker.com/u/$DOCKER_USER/musescore-$1/trigger/$DOCKER_TRIGGER/"
    curl -H "Content-Type: application/json" --data "$data" -X POST "$url"
    set -x
  fi
  set -e # disallow errors
}

# Set AppImage name and automated update information according to
# https://github.com/AppImage/AppImageSpec/blob/master/draft.md
# Also set a label to distinguish when running multiple builds on one machine.
if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt
  if [ "${BINTRAY_REPO_OWNER}" == "musescore" ]
  then # This is a nightly build
    makefile_overrides="PREFIX='MuseScoreNightly-$date-$branch-$revision' \
                        SUFFIX='-portable-nightly' \
                        BUILD_NUMBER='${TRAVIS_BUILD_NUMBER}' \
                        LABEL='Portable Nightly Build'"
    cp -f build/travis/resources/splash-nightly.png mscore/data/splash.png
    update_info="" # automatic updates disabled for nightlies as it requires
    # changes on the server. See https://github.com/musescore/MuseScore/pull/4757.
  else
    # This is someone developing on their own fork
    export BINTRAY_VERSION="${date}-${branch}-${revision}"
    makefile_overrides="PREFIX='MuseScoreDev-${BINTRAY_VERSION}' \
                        SUFFIX='-portable-dev' \
                        LABEL='Unofficial Developer Build'"
    export BINTRAY_USER="${BINTRAY_USER:?${env_error_msg}}" # env, otherwise appimagetool will fail
    export BINTRAY_REPO="${BINTRAY_REPO:-MuseScore}"
    export BINTRAY_REPO_OWNER="${BINTRAY_REPO_OWNER:-${BINTRAY_USER}}" # env, or use $BINTRAY_USER
    export BINTRAY_PACKAGE="${BINTRAY_PACKAGE:-MuseScoreDev-${branch}}"
    update_info="bintray-zsync|${BINTRAY_REPO_OWNER}|${BINTRAY_REPO}|${BINTRAY_PACKAGE}|MuseScoreDev-_latestVersion-${TARGET_ARCH}.AppImage.zsync"
  fi
else
  # Build is STABLE so create a stable release!
  makefile_overrides="" # use Makefile defaults
  update_info="gh-releases-zsync|musescore|MuseScore|latest|MuseScore-*x86_64.AppImage.zsync"
fi

# Build AppImage depending on arch specified in $1 if cross-compiling, else default build x86_64
case "${TARGET_ARCH}" in

  armhf )
    # build MuseScore inside debian x86-64 multiarch image containing arm cross toolchain and libraries
    docker run -itv "${PWD}:/MuseScore" \
      ericfont/musescore:jessie-crosscompile-armhf \
      /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/RecipeDebian --build-only armhf $makefile_overrides"
    # then run inside fully emulated arm image for AppImage packing step (which has trouble inside multiarch image)
    docker run -i --privileged multiarch/qemu-user-static:register
    docker run -itv "${PWD}:/MuseScore" --privileged \
      ericfont/musescore:jessie-packaging-armhf \
      /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/RecipeDebian --package-only armhf"
    ;;

  i686 )
    # Build MuseScore AppImage inside 32-bit x86 Docker image
    (set +x; DOCKER_TRIGGER="$DOCKER_TRIGGER_X86_32" rebuild-docker-image x86_32)
    docker run -itv "${PWD}:/MuseScore" "$DOCKER_USER/musescore-x86_32:$docker_tag" /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/Recipe $makefile_overrides"
    ;;

  x86_64 )
    # Build MuseScore AppImage inside native (64-bit x86) Docker image
    (set +x; DOCKER_TRIGGER="$DOCKER_TRIGGER_X86_64" rebuild-docker-image x86_64)
    docker run -itv "${PWD}:/MuseScore" "$DOCKER_USER/musescore-x86_64:$docker_tag" /bin/bash -c \
      "UPDATE_INFORMATION='${update_info}' /MuseScore/build/Linux+BSD/portable/Recipe $makefile_overrides"
    ;;

  * )
    echo "$0: Unrecognised TARGET_ARCH '${TARGET_ARCH}'" >&2
    exit 1
    ;;

esac

# Should the AppImage be uploaded?
if [ "$1" == "--upload-branches" ] && [ "$2" != "ALL" ]; then
  # User passed in a list of zero or more branches so only upload those listed
  shift
  for upload_branch in "$@" ; do
    [ "$branch" == "$upload_branch" ] && upload=true || true # bypass `set -e`
  done
else
  # No list passed in (or specified "ALL"), so upload on every branch
  upload=true
fi

if [ "${upload}" ]; then
  if [[ "${TRAVIS_PULL_REQUEST}" ]] && [[ "${TRAVIS_PULL_REQUEST}" != "false" ]]; then
    # no env secrets available in pull requests
    echo "$0: Not uploading since this a pull request."
  else
    if [[ "${TRAVIS_REPO_SLUG}" == "musescore/MuseScore" ]]; then
      # this is an official build (stable or nightly)
      ./build/travis/job2_AppImage/osuosl.sh build.release/MuseScore*.AppImage
      if [[ -f build.release/MuseScore*.AppImage.zsync ]]; then
        # upload zsync delta for automatic updates
        ./build/travis/job2_AppImage/osuosl.sh build.release/MuseScore*.AppImage.zsync
      fi
    else
      # This is a developer building on their personal fork
      ./build/travis/job2_AppImage/bintray.sh build.release/MuseScore*.AppImage* # both AppImage and zsync
    fi
  fi
else
  echo "On branch '$branch' so AppImage will not be uploaded." >&2
fi

#!/bin/bash
# Run this script from MuseScore's root directory

# Build portable Linux AppImages and upload them to Bintray. AppImages will
# always be uploaded unless a list of specific branches is passed in. e.g.:
#    $   build.sh  --upload-branches  master  my-branch-1  my-branch-2
# Builds will be for the native architecture (64 bit) unless another is
# specified for cross-compiling. (e.g. build.sh --i686 or build.sh --armhf)

set -e # exit on error
set -x # echo commands

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

if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt
  if [ "${BINTRAY_REPO_OWNER}" == "musescore" ]
  then # This is a nightly build
    makefile_overrides="PREFIX='MuseScoreNightly-$date-$branch-$revision' \
                        SUFFIX='-portable-nightly' \
                        BUILD_NUMBER='${TRAVIS_BUILD_NUMBER}' \
                        LABEL='Portable Nightly Build'"
    cp -f build/travis/resources/splash-nightly.png mscore/data/splash.png
  else
    # This is someone developing on their own fork
    makefile_overrides="PREFIX='MuseScoreDev-$date-$branch-$revision' \
                        SUFFIX='-portable-dev' \
                        LABEL='Unofficial Developer Build'"
  fi
else
  # Build is STABLE so create a stable release!
  makefile_overrides="" # use Makefile defaults
fi

# Build AppImage depending on arch specified in $1 if cross-compiling, else default build x86_64
case "$1" in

  --armhf )
    shift
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

  --i686 )
    shift
    # Build MuseScore AppImage inside 32-bit x86 Docker image
    (set +x; DOCKER_TRIGGER="$DOCKER_TRIGGER_X86_32" rebuild-docker-image x86_32)
    docker run -itv "${PWD}:/MuseScore" "$DOCKER_USER/musescore-x86_32:$docker_tag" /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/Recipe $makefile_overrides"
    ;;

  * )
    [ "$1" == "--x86_64" ] && shift || true
    # Build MuseScore AppImage inside native (64-bit x86) Docker image
    (set +x; DOCKER_TRIGGER="$DOCKER_TRIGGER_X86_64" rebuild-docker-image x86_64)
    docker run -itv "${PWD}:/MuseScore" "$DOCKER_USER/musescore-x86_64:$docker_tag" /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/Recipe $makefile_overrides"
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
  # Upload AppImage to Bintray
  #./build/travis/job2_AppImage/bintray.sh build.release/MuseScore*.AppImage
  ./build/travis/job2_AppImage/osuosl.sh build.release/MuseScore*.AppImage
  if [[ -f build.release/MuseScore*.AppImage.zsync ]]; then
    # upload zsync delta for automatic updates
    ./build/travis/job2_AppImage/osuosl.sh build.release/MuseScore*.AppImage.zsync
  fi
else
  echo "On branch '$branch' so AppImage will not be uploaded." >&2
fi

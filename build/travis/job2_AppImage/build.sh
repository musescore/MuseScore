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

if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt
  if [ "${BINTRAY_REPO_OWNER}" == "musescore" ]
  then # This is a nightly build
    makefile_overrides="PREFIX='MuseScoreNightly-$date-$branch-$revision' \
                        SUFFIX='-portable-nightly' \
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
    docker run -i -v "${PWD}:/MuseScore" \
      ericfont/musescore:jessie-crosscompile-armhf \
      /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/RecipeDebian --build-only armhf $makefile_overrides"
    # then run inside fully emulated arm image for AppImage packing step (which has trouble inside multiarch image)
    docker run -i --privileged multiarch/qemu-user-static:register
    docker run -i -v "${PWD}:/MuseScore" --privileged \
      ericfont/musescore:jessie-packaging-armhf \
      /bin/bash -c \
      "/MuseScore/build/Linux+BSD/portable/RecipeDebian --package-only armhf"
    ;;

  --i686 )
    shift
    # Build MuseScore AppImage inside 32-bit x86 Docker image
    docker run -i -v "${PWD}:/MuseScore" toopher/centos-i386:centos6 /bin/bash -c \
      "linux32 --32bit i386 /MuseScore/build/Linux+BSD/portable/Recipe $makefile_overrides"
    ;;


  * )
    [ "$1" == "--x86_64" ] && shift || true
    # Build MuseScore AppImage inside native (64-bit x86) Docker image
    docker run -i -v "${PWD}:/MuseScore" library/centos:6 /bin/bash -c \
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
  ./build/travis/job2_AppImage/bintray.sh build.release/MuseScore*.AppImage
else
  echo "On branch '$branch' so AppImage will not be uploaded." >&2
fi

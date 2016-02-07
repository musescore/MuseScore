#!/bin/bash
# Run this script from MuseScore's root directory

# Build portable Linux AppImages and upload them to Bintray. AppImages will
# always be uploaded unless a list of specific branches is passed in. e.g.:
#    $   build.sh  --upload-branches  master  my-branch-1  my-branch-2

set -e # exit on error
set -x # echo commands

date="$(date -u +%Y%m%d%H%M)"

branch="$TRAVIS_BRANCH"
[ "$branch" ] || branch="$(git rev-parse --abbrev-ref HEAD)"

revision="$(echo "$TRAVIS_COMMIT" | cut -c 1-7)"
[ "$revision" ] || revision="$(make revision && cat mscore/revision.h)"

if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt so create a nightly build
  makefile_overrides="PREFIX='MuseScoreNightly-$date-$branch-$revision' \
                      SUFFIX='-portable-nightly' \
                      LABEL='Portable Nightly Build'"
  cp -f build/travis/job2_AppImage/splash-nightly.png mscore/data/splash.png
else
  # Build is STABLE so create an official release!
  makefile_overrides="" # use Makefile defaults
fi

# Build MuseScore AppImage inside Docker image
docker run -i -v "${PWD}:/MuseScore" library/centos:6 \
   /bin/bash -c "/MuseScore/build/Linux+BSD/portable/Recipe $makefile_overrides"

# Should the AppImage be uploaded?
if [ "$1" == "--upload-branches" ]; then
  shift
  for upload_branch in "$@" ; do
    [ "$branch" == "$upload_branch" ] && upload=true || true # bypass `set -e`
  done
else
  upload=true
fi

if [ "${upload}" ]; then
  # Upload AppImage to Bintray
  ./build/travis/job2_AppImage/bintray.sh build.release/MuseScore*.AppImage
else
  echo "On branch '$branch' so AppImage will not be uploaded." >&2
fi

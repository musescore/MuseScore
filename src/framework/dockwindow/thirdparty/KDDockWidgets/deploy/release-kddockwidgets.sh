#!/bin/bash

# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

#Exit if any undefined variable is used.
set -u
#Exit this script if it any subprocess exits non-zero.
set -e
#If any process in a pipeline fails, the return value is a failure.
set -o pipefail

PROJECT=kddockwidgets
FORMAL_PROJECT=KDDockWidgets #also used for the CMake options
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
TOP=$(dirname "$SCRIPT_DIR")

#function HELP
# print a help message and exit
HELP() {
  echo
  echo "Usage: $(basename "$0") [-f] X.Y.Z"
  echo
  echo "Create the tars/zips and sign for project release X.Y.Z"
  echo " Options:"
  echo "  -f  Force everything to run, even if the tag for X.Y.Z has already been pushed."
  echo
  exit 0
}

#git clone if needed, then update
GIT_UPDATE() {
  mkdir -p "$1"
  pushd "$1"
  git init
  set +e
  git remote add origin "$3"
  set -e
  git fetch
  git checkout master
  git submodule update --init --recursive
  popd
}

#compare 2 version strings
verlte() {
  printf '%s\n' "$1" "$2" | sort -C -V
}

#function SYNCECM: $1 is "KDAB" or "ECM"; $2 is the fullpath to the official version
SYNCECM() {
  set +e
  echo -n "Comparing $1 cmake modules to upstream: "
  savepwd=$(pwd)
  if (test ! -d "$TOP/cmake/$1/modules"); then
    echo "FAIL"
    echo " This project does not have the $1 CMake modules collected under cmake/$1/modules. Please deal with this first"
    exit 1
  fi
  cd "$TOP/cmake/$1/modules"
  whiteList="(ECMEnableSanitizers.cmake)"
  for m in *.cmake; do
    if [ -n "$whiteList" ]; then
      if [[ $m =~ $whiteList ]]; then
        echo "SKIPPING $m"
        continue
      fi
    fi
    if (test -f "$2/modules/$m"); then
      module="modules"
      diff "$m" "$2/modules/$m" 2>&1
      savestat=$?
    else
      if (test -f "$2/find-modules/$m"); then
        module="find-modules"
        diff "$m" "$2/find-modules/$m" 2>&1
        savestat=$?
      else
        echo "What is $m doing here?"
        exit 1
      fi
    fi
    if (test $savestat -ne 0); then
      echo "FAIL. Differences encountered in upstream $m"
      echo "  Upstream: $2/$module/$m"
      echo "  $PROJECT: cmake/$1/modules/$m"
      echo "Please sync the $PROJECT version before continuing (review changes first!)"
      exit 1
    fi
  done
  echo "OK"
  cd "$savepwd"
  set -e
}

options=$(getopt -o "hf" --long "help,force" -- "$@")
eval set -- "$options"
force=0
while true; do
  case "$1" in
  -h | --help)
    HELP
    ;;
  -f | --force)
    force=1
    shift
    ;;
  --)
    shift
    break
    ;;
  *)
    echo "Internal error!"
    exit 1
    ;;
  esac
done

if (test $# -ne 1); then
  HELP
fi

#compute X(major), Y(minor), Z(patchlevel)
if [[ ! $1 =~ ^[0-9]*\.[0-9]*\.[0-9]*$ ]]; then
  echo "\"$1\" is not a valid version string of the form X.Y.Z"
  exit 1
fi
X=$(echo "$1" | cut -d. -f1)
Y=$(echo "$1" | cut -d. -f2)
Z=$(echo "$1" | cut -d. -f3)

#set the branch and tag
branch=$X.$Y
tag=v$branch.$Z
release=$X.$Y.$Z

cd "$TOP" || exit 1
tbranch=$(sed -e 's,.*/,,' "$TOP/.git/HEAD")
if (test "$tbranch" != "$branch"); then
  echo "please git checkout $branch first"
  exit
fi

#Sanity Checking

# Update doxyfile
if ! command -v doxygen &>/dev/null; then
  echo "doxygen is not installed or not in your PATH. please fix."
  exit 1
fi

#CI uses 1.12.0 at this time
minDoxyVersion="1.12.0"
export PATH=/usr/local/opt/doxygen-$minDoxyVersion/bin:$PATH
doxyVersion=$(doxygen -version | awk '{print $1}')
if ! verlte "$minDoxyVersion" "$doxyVersion"; then
  echo "please install doxygen version $minDoxyVersion or higher"
  exit 1
fi

echo -n "Ensuring Doxyfile.cmake is up-to-date: "
doxygen -u docs/api/Doxyfile.cmake >/dev/null 2>&1
set +e
diff docs/api/Doxyfile.cmake docs/api/Doxyfile.cmake.bak >/dev/null 2>&1
if (test $? -ne 0); then
  echo "Doxyfile.cmake has been updated by 'doxygen -u'. Please deal with this first"
  exit 1
else
  echo "OK"
  rm -f docs/api/Doxyfile.cmake.bak
fi
set -e

### KDAB cmake modules are synced
kdabECM="$HOME/projects/kdecm"
GIT_UPDATE "$kdabECM" "master" "ssh://codereview.kdab.com:29418/kdab/extra-cmake-modules"
SYNCECM "KDAB" "$kdabECM"
### KDE cmake modules are synced
kdeECM="$HOME/projects/extra-cmake-modules"
GIT_UPDATE "$kdeECM" "master" "git@invent.kde.org:frameworks/extra-cmake-modules"
SYNCECM "ECM" "$kdeECM"

### pre-commit checking
echo "Pre-commit checking: "
pre-commit run --all-files
if (test $? -ne 0); then
  echo "There are pre-commit issues.  Please deal with this first"
  exit 1
else
  echo "OK"
fi

if (test "$(git tag -l | grep -c "$tag$")" -ne 1); then
  echo "please create the git tag $tag first:"
  echo "git tag -m \"$FORMAL_PROJECT $release\" $tag"
  exit
fi

if (test $force -eq 0 -a "$(git ls-remote --tags origin | grep -c "refs/tags/$tag$")" -gt 0); then
  echo "The tag for $tag has already been pushed."
  echo "Change the release number you provided on the command line."
  echo 'Or, if you simply want to re-create the tar and zips use the "-f" option.'
  exit
fi

# create the API documentation
rm -rf build-docs "$PROJECT-$release-doc.zip"
mkdir build-docs
cd build-docs || exit 1
cmake -G Ninja --warn-uninitialized -Werror=dev -D"$FORMAL_PROJECT"_DOCS=True ..
cmake --build . --target=docs
cd docs/api/html || exit 1
7z a "$TOP/$PROJECT-$release-doc.zip" .
cd "$TOP" || exit 1
rm -rf build-docs

git archive --format=tar --prefix="$PROJECT-$release/" "$tag" | gzip >"$PROJECT-$release.tar.gz"
git archive --format=zip --prefix="$PROJECT-$release/" "$tag" >"$PROJECT-$release.zip"

# sign the tarballs
gpg --yes --local-user "KDAB Products" --armor --detach-sign "$PROJECT-$release.tar.gz"
gpg --yes --local-user "KDAB Products" --armor --detach-sign "$PROJECT-$release.zip"

# final cleaning
#anything to clean?

# sanity
files="\
$PROJECT-$release.tar.gz \
$PROJECT-$release.tar.gz.asc \
$PROJECT-$release.zip \
$PROJECT-$release.zip.asc \
$PROJECT-$release-doc.zip \
"
for f in $files; do
  ls -l "$f"
done

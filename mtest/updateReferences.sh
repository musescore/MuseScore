#!/bin/bash

function showHelp() {
cat >&2 <<"EOF"

Update reference files in mtest based on test results in build.debug/mtest.

  Usage:    mtest/updateReferences.sh  mtest/$path

  Or:       cd  mtest  &&  ./updateReferences.sh  $path

Copies test files from build.debug/mtest/$path to mtest/$path. Test files are
all files named like *-test.* and they are renamed *-ref.* in the process.

EOF
}

path="$1"

[ "$(basename "${PWD}")" == "mtest" ] && path="mtest/${path}" && cd ..

# Some checks:

if [ "${path}" == "" ] || [ ! -d "${path}" ]; then
  showHelp
  exit 1
fi

if [ "$(ls "build.xcode/mtest/guitarpro/Debug/"*-test.*)" == "" ]; then
  echo "$0: No test files in 'build.debug/$path'. Have you run the tests?"
  exit 2
fi

# All good!

echo "Copy refs from 'build.debug/${path}' to '${path}'."

for file in build.xcode/mtest/guitarpro/*-test.*; do
  cp "$file" "${path}"/"$(basename "${file}" | sed "s|-test\.|-ref\.|")"
done

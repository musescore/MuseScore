#!/bin/sh
# Run this script from MuseScore's root directory

cd build.debug/mtest

# vnc is the only tested planform that allowed to run
# mscore executable in the used Travis environment.
export QT_QPA_PLATFORM=vnc

xvfb-run -a ctest -j2 --output-on-failure

PROC_RET=$?

if [ "$PROC_RET" -ne 0 ]; then
  killall Xvfb
  xvfb-run -a ./mtest
fi

# Searching for merge conflicts, by searching for the begin/end markers.
# Searching for the middle marker '=======)" won't work though as that is
# used elsewhere too.
# Haven't found a way without using an intermediate temp file.
rm -f /tmp/$$ # Cleanup, just in case of an older leftover
find ../.. -type f ! -name `basename $0` -print0 |
  xargs -0r egrep -n '<<<<<<< HEAD|>>>>>>> .*' |
  tee /tmp/$$
if [ -s /tmp/$$ ]; then # File not empty
  PROC_RET=`expr $PROC_RET + 1` # Store the error
  printf "\033[31m" # Set color to  (doesn't work on Travis though)
  echo
  echo "problem with a merge conflict"
  echo
  printf "\033[0m" # Reset color
fi
rm -f /tmp/$$ # Cleanup

#pwd == build.debug/mtest
cd ../../vtest
VTEST_BROWSER=ls xvfb-run ./gen
cd -

#make reporthtml
#REVISION=`git rev-parse --short HEAD`
#mv report/html $REVISION
#mv report/*.xml $REVISION
#chmod 755 $REVISION
#chmod 644 $REVISION/*
#zip -r $REVISION.zip $REVISION
#curl -F zip_file=@$REVISION.zip  http://prereleases.musescore.org/test/index.php
#echo "Test results: http://prereleases.musescore.org/test/$REVISION/"

cd ..
cd ..
if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then # Build is marked UNSTABLE inside CMakeLists.txt
  echo "Unstable version: do not upload source zip file"
else
  make clean
  rm -rf qt5
  rm -rf qt5.zip
  rm -rf share/sound/FluidR3Mono*
  rm -rf share/sound/README*
  rm -rf share/locale/*.qm
  make zip
  ./build/travis/job1_Tests/osuosl.sh MuseScore*.zip
fi


exit $PROC_RET

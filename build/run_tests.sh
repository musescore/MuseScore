#!/bin/sh
xvfb-run ctest
PROC_RET=$?

if [ "$PROC_RET" -ne 0 ]; then
killall Xvfb
xvfb-run ./mtest
fi

#pwd == build.debug/mtest
cd ../../vtest
xvfb-run ./gen
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

exit $PROC_RET
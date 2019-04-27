#!/usr/bin/env bash

BUILDPATH=./../../../build.debug/mtest/libmscore/midi

n=0;

for f in $BUILDPATH/*; do
    if [[ $f == *"-test"* ]]; then
        basename=$(basename $f)
        search=${basename//-test/-ref}
        if [ -f $search ]; then
            cp $f $search
            ((n++))
        fi
    fi
done

echo "Updated $n files"

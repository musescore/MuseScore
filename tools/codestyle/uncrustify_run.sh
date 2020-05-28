#!/usr/bin/env bash

DIR="$1"

SRC_DIRS="aeolus audio audiofile avsomr awl bww2mxml crashreporter effects fonttools global importexport \
libmscore main miditools mscore mtest omr telemetry"

START_TIME=$(date +%s)

for var in $SRC_DIRS
do
    uncrustify_run_dir.sh $DIR/$var
done

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo "time: $DIFF_TIME sec, complete all"

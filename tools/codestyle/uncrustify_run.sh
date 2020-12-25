#!/usr/bin/env bash

# Go to repository root directory regardless of where script was run from:
cd "${BASH_SOURCE%/*}/../.."

HERE="tools/codestyle" # path to dir that contains this script

SRC_DIRS=(
    # Alphabetical order please!
    bww2mxml
    fonttools
    miditools
    mtest
    src
)

START_TIME=$(date +%s)

for dir in "${SRC_DIRS[@]}"
do
    "${HERE}/uncrustify_run_dir.sh" "${dir}"
done

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo "time: $DIFF_TIME sec, complete all"

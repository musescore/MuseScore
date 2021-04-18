#!/usr/bin/env bash

HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

DIR="${1-.}" # use $1 or "." (current dir) if $1 is not defined

START_TIME=$(date +%s)

find $DIR -type f -regex '.*\.\(cpp\|h\|hpp\|cc\)$' | xargs -n 1 -P 16 \
    uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l CPP

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo ""
echo "time: $DIFF_TIME sec, complete: $DIR"
echo ""

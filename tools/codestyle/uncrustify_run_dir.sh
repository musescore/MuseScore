#!/usr/bin/env bash

DIR="$1"

START_TIME=$(date +%s)

find $DIR -type f -regex '.*\.\(cpp\|h\|hpp\|cc\)$' | xargs -n 1 -P 16 \
uncrustify -c uncrustify_musescore.cfg --no-backup -l CPP

END_TIME=$(date +%s)
DIFF_TIME=$(( $END_TIME - $START_TIME ))
echo ""
echo "time: $DIFF_TIME sec, complete: $DIR"
echo ""

#!/bin/sh

FILE="$1"

uncrustify -c uncrustify_musescore.cfg --no-backup -l CPP $FILE

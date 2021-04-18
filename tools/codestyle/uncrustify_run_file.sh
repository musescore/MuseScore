#!/usr/bin/env bash

HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

exec uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l CPP "$@"

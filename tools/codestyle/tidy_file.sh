#!/usr/bin/env bash

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$0: Error: Please upgrade Bash."; exit 1; }

set -euo pipefail

# Call the right command to tidy a file based on purely on its extension.
# For best performance, filter the list of files prior to calling this script.

# Editors, don't use `git` in this script as it can cause conflicts reading
# the git index when multiple instances of this script are run in parallel.

HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

function uncrustify_file()
{
    local file="$1" lang="$2" status
    if ! uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l "${lang}" "${file}"; then
        status=$?
        rm -f "${file}.uncrustify" # remove possible temporary file
        return ${status}
    fi
}

if (($# != 1)); then
    echo >&2 "$0: Error: Too many arguments. Please specify a single file."
    exit 255 # make xargs exit early and ensure non-zero status on macOS
fi

file="$1"
base="${file%.unstaged}" # remove possible '.unstaged' extension (see hooks/pre-commit)
ext="${base##*.}"

set +e
case "${ext,,}" in
c)          uncrustify_file "${file}" 'CPP' ;;
h|cpp|hpp)  uncrustify_file "${file}" 'CPP' ;;
m)          uncrustify_file "${file}" 'OC'  ;;
mm)         uncrustify_file "${file}" 'OC+' ;;
*)          echo >&2 "Skipping: ${file}" ;;
esac
status=$?
set -e

if ((${status} != 0)); then
    echo >&2 "$0: Error ${status} for ${file}"
    exit 255 # make xargs exit early and ensure non-zero status on macOS
fi

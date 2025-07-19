#!/usr/bin/env bash

FAIL_FAST=255 # exit code to make xargs terminate early (if this script is called by xargs)

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$0: Error: Please upgrade Bash."; exit ${FAIL_FAST}; }

set -uo pipefail

trap 'echo >&2 "$0: Error $?, line ${LINENO}, args: $*"; exit ${FAIL_FAST}' ERR

# Call the right command to tidy a file based on purely on its extension.
# For best performance, filter the list of files prior to calling this script.

# Editors, don't use `git` in this script as it can cause conflicts reading
# the git index when multiple instances of this script are run in parallel.

HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

function uncrustify_file()
{
    local file="$1" lang="$2" status
    uncrustify -c "${HERE}/uncrustify_musescore.cfg" --no-backup -l "${lang}" -f "${file}"
    status=$?
    rm -f "${file}.uncrustify" # remove possible temporary file
    return ${status}
}

if (($# != 1)); then
    echo >&2 "$0: Error: Wrong arguments. Please specify a single file."
    exit ${FAIL_FAST}
fi

file="$1"
base="${file%.unstaged}" # remove possible '.unstaged' extension (see hooks/pre-commit)
extension="${base##*.}"

case "${extension,,}" in
c)          tidy="$(uncrustify_file "${file}" 'C'  )" ;;
h|cpp|hpp)  tidy="$(uncrustify_file "${file}" 'CPP')" ;;
m)          tidy="$(uncrustify_file "${file}" 'OC' )" ;;
mm)         tidy="$(uncrustify_file "${file}" 'OC+')" ;;
*)          echo >&2 "Skipping: ${file}"; exit 0      ;;
esac

# Only update the original file if the tidy version is different.
if ! cmp --silent "${file}" - <<<"${tidy}"; then
    echo >&2 "Tidying: ${file}"
    printf '%s\n' "${tidy}" >"${file}" # restore final newline lost in command substitution
fi

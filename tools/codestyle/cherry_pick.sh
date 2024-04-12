#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Go to repository root directory regardless of where script was run from:
cd "${BASH_SOURCE%/*}/../.."
HERE="tools/codestyle" # path to dir that contains this script

# Cherry-pick a commit from a branch that uses a different coding style to the
# current branch. Ignore changes to code formatting and indentation.

COMMIT="$1" # SHA hash for commit we want to cherry-pick from another branch.

shift # remaining arguments passed through to `git cherry-pick`

# Which files did ${COMMIT} change?

# TODO: Add more options to `git show` to control special cases. Examples:
#   --diff-filter=AM to include only files that were added or modified.
#   --diff-filter=d to include all files except deleted files.
# You can play around with options like this if you have a PR that needs them.
# See `git help diff` and search for --diff-filter.

readarray -t changed_files < \
    <(git show --name-only --pretty="" "${COMMIT}")
# echo "changed_files:" "${changed_files[@]}"

# TODO: Ignore C++ files in thirdparty and other dirs that don't follow style.

readarray -t changed_cpp_files < \
    <(git show --name-only --pretty="" "${COMMIT}" '*.c' '*.h' '*.cpp' '*.hpp')
# echo "changed_cpp_files:" "${changed_cpp_files[@]}"

# Were any C++ files changed?

if ((${#changed_cpp_files} == 0)); then
    # Exit script and do a normal cherry-pick.
    echo "Handing over to 'git cherry-pick'..."
    exec git cherry-pick "${COMMIT}" "$@"
fi

# Checkout C++ files from other branch as they were prior to ${COMMIT}.

added_file="false"

for file in "${changed_cpp_files[@]}"; do
    ( # spawn subshell
    set -e # exit subshell on error (e.g. file did not exist before ${COMMIT})
    git checkout "${COMMIT}~1" "${file}" # file from commit before ${COMMIT}
    "${HERE}/uncrustify_run_file.sh" "${file}"
    git add "${file}"
    ) && added_file="true"
done

if [[ "${added_file}" == "true" ]]; then
  git commit -m "Put C++ files in state prior to ${COMMIT} but with new code style"
fi

# Checkout all files as they were after ${COMMIT} was applied.

for file in "${changed_files[@]}"; do
    git checkout "${COMMIT}" "${file}" # file from ${COMMIT}
    git add "${file}"
done

for file in "${changed_cpp_files[@]}"; do
    "${HERE}/uncrustify_run_file.sh" "${file}"
    git add "${file}"
done

git commit --reuse-message="${COMMIT}" # same commit but with new code style

if [[ "${added_file}" == "false" ]]; then
  exit 0 # nothing else to do
fi

COMMIT="$(git show --no-patch --pretty="%H")" # SHA hash has changed

# Revert changes then cherry-pick new commit

git reset --hard HEAD~2

echo "Handing over to 'git cherry-pick'..."
exec git cherry-pick "${COMMIT}" "$@"

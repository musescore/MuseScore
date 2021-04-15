#!/usr/bin/env bash
##
## // SPDX-License-Identifier: GPL-3.0-only
## // MuseScore-CLA-applies
## //=============================================================================
## //  MuseScore
## //  Music Composition & Notation
## //
## //  Copyright (C) 2021 MuseScore BVBA and others
## //
## //  This program is free software: you can redistribute it and/or modify
## //  it under the terms of the GNU General Public License version 3 as
## //  published by the Free Software Foundation.
## //
## //  This program is distributed in the hope that it will be useful,
## //  but WITHOUT ANY WARRANTY; without even the implied warranty of
## //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## //  GNU General Public License for more details.
## //
## //  You should have received a copy of the GNU General Public License
## //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
## //=============================================================================##

echo "Check code style"
trap 'echo Check code style failed; exit 1' ERR

brew install uncrustify findutils
export PATH="/usr/local/opt/findutils/libexec/gnubin:${PATH}"
tools/codestyle/uncrustify_run.sh

codestyle_failed_files="$(git diff --name-only)"
if [[ "${codestyle_failed_files}" ]]; then
cat >&2 <<EOF
Error: Code style is incorrect in these files:

${codestyle_failed_files}

Please run tools/codestyle/uncrustify_run_file.sh on these files and
then amend your commit.

$ git show --name-only '*.h' '*.cpp' | xargs tools/codestyle/uncrustify_run_file.sh
$ git show --name-only '*.h' '*.cpp' | xargs git add
$ git commit --amend --no-edit

If your PR contains multiple commits then you must do an interactive
rebase and amend each commit in turn.

$ git -c sequence.editor='sed -i s/^pick/edit/' rebase -i HEAD~\${NUM_COMMITS}

Where \${NUM_COMMITS} is the number of commits in your PR.

The required changes are...

$(git diff)
EOF
exit 1
fi

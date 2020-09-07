#!/usr/bin/env bash

echo "Check code style"

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
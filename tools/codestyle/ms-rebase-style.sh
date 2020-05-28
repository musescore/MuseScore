#!/usr/bin/env bash
set -e

STYLE_CHANGE_COMMIT="45b9887603f08523f9a663b164bd9f10b85f8b25"
CODESTYLE_CONFIG_COMMIT="88ec8b39cb63c1a6e2dee2f9d46aa5954323ba80"
TARGET_UNCRUSTIFY_VERSION="0.71.0"

function help() {
    echo "Usage:"
    echo "  $0 <new_base>"
    echo "or"
    echo "  $0 <new_base> <branch_to_rebase>"
    exit 0
}

function apply_style_change() {
    local FILE=$(readlink -e "$1")
    DIR="$PWD"
    cd "$TEMP_DIR"
    ./uncrustify_run_file.sh "$FILE"
    cd "$DIR"
}

case "$1" in
    "")
        help
        ;;
    "-h")
        help
        ;;
    "--help")
        help
        ;;
esac

if [ -n "$(git status --ignore-submodules --untracked-files=no --porcelain)" ]
then
    echo "Working directory is not clean: commit or stash your changes and launch the script again"
    exit 1
fi

TARGET="$1"

if [ -n "$2" ]
then
    SRC="$2"
else
    SRC=$(git rev-parse --abbrev-ref HEAD)
fi

UNCRUSTIFY_VERSION=$(uncrustify --version | grep -o '[0-9.]\+')
if [[ "$UNCRUSTIFY_VERSION" < "$TARGET_UNCRUSTIFY_VERSION" ]]
then
    echo "Uncrustify version is too old (current: $UNCRUSTIFY_VERSION, needed: $TARGET_UNCRUSTIFY_VERSION)"
    exit 1
fi

if git ls-files --error-unmatch "$0" 2> /dev/null
then
    echo "This copy of the script is under version control."
    echo "Please make a separate copy and run that one instead."
    echo "(This is necessary for the script to remain unchanged during the entire rebase process)"
    exit 1
fi

if git merge-base --is-ancestor "$STYLE_CHANGE_COMMIT" "$SRC"
then
    echo "$SRC branch is already in new style, use \"git rebase\" directly instead"
    exit 1
fi

# Make temporary copies of the necessary files to have them
# available regardless of the current source tree state.
git checkout "$CODESTYLE_CONFIG_COMMIT"
TEMP_DIR=$(mktemp -d)
cp tools/codestyle/uncrustify_run_file.sh "$TEMP_DIR"
cp tools/codestyle/uncrustify_musescore.cfg "$TEMP_DIR"

TMP_REBASE_BRANCH="ms_rebase_style_tmp_rebase_branch_$SRC"
REBASED_BRANCH="$SRC"_rebased_to_new_style
BEFORE_STYLE_CHANGE_COMMIT="$STYLE_CHANGE_COMMIT~1"

echo ">>> Rebasing to the point before style change (branch $TMP_REBASE_BRANCH)..."
git checkout "$SRC"
git checkout -b "$TMP_REBASE_BRANCH"
git rebase "$BEFORE_STYLE_CHANGE_COMMIT"

BRANCH_COMMITS=$(git log --format="%H" --reverse "$BEFORE_STYLE_CHANGE_COMMIT..$HEAD")

echo ">>> Rebasing to the point after style change..."
git checkout "$STYLE_CHANGE_COMMIT"
git branch "$REBASED_BRANCH"
LAST_REBASED_BRANCH_COMMIT="$STYLE_CHANGE_COMMIT"

for COMMIT in $BRANCH_COMMITS
do
    git checkout $COMMIT
    CHANGED_FILES=$(git diff --name-only $COMMIT~1 $COMMIT)

    for FILE in $CHANGED_FILES
    do
        case "$FILE" in
            *.cpp|*.cc|*.h|*.hpp)
                echo ">>> Applying style change to $FILE..."
                apply_style_change "$FILE"
                ;;
            *)
                echo ">>> Skipping $FILE..."
                ;;
        esac
    done

    COMMIT_DIFF=$(echo "$CHANGED_FILES" | xargs git diff $LAST_REBASED_BRANCH_COMMIT --)

    git checkout -- '*'
    git checkout "$REBASED_BRANCH"
    echo "$COMMIT_DIFF" | git apply --index -
    git commit --reuse-message=$COMMIT
    LAST_REBASED_BRANCH_COMMIT=$(git log -n1 --format="%H")
done

echo ">>> Cleaning up temporary branch ($TMP_REBASE_BRANCH)..."
git branch -D "$TMP_REBASE_BRANCH"

echo ">>> Rebasing to $TARGET"
git rebase "$TARGET" "$REBASED_BRANCH"

echo ">>> Cleaning up temporary directory..."
rm -rf "$TEMP_DIR"

echo ">>> Rebased $SRC to $TARGET (new branch name is $REBASED_BRANCH)"

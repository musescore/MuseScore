#!/usr/bin/env bash 


MF_ORIGIN_URL=https://github.com/musescore/muse_framework.git
MF_FORK_URL=""
TASK=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -f|--fork) MF_FORK_URL="$2"; shift ;;
        -t|--task) TASK="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$MF_FORK_URL" ]; then MF_FORK_URL=$(cat mf_fork_url.local); fi
if [ -z "$MF_FORK_URL" ]; then echo "error: not set MF_FORK_URL"; exit 1; fi

if [ -z "$TASK" ]; then echo "error: not set TASK"; exit 1; fi

echo "MF_FORK_URL: $MF_FORK_URL"
echo "TASK: $TASK"


function use_fork() {
    git submodule set-url muse $MF_FORK_URL
    cd muse
    git remote set-url origin $MF_FORK_URL
    git remote remove upstream
    git remote add upstream $MF_ORIGIN_URL
    git remote -v
    cd ..
}

function use_origin() {
    git submodule set-url muse $MF_ORIGIN_URL
    git submodule update --init
    cd muse
    git remote set-url origin $MF_ORIGIN_URL
    git remote remove upstream
    git remote -v
    cd ..
}

case "$TASK" in
    "use-fork")
        use_fork
        ;;
    "use-origin")
        use_origin
        ;;
    "sync-origin")
        use_origin
        git submodule update --init --remote ./muse
        ;;    
    *) echo "Unknown task: $TASK"; exit 1 ;;
esac
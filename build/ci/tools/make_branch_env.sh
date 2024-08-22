#!/usr/bin/env bash

ARTIFACTS_DIR="build.artifacts"

BRANCH=$1
if [ -z "$BRANCH" ]; then 
    BRANCH=$(git rev-parse --abbrev-ref HEAD)
fi

if [ -z "$BRANCH" ]; then echo "error: not set BRANCH"; exit 1; fi

BRANCH=$(echo "$BRANCH" | tr / _)

echo $BRANCH > $ARTIFACTS_DIR/env/build_branch.env
cat $ARTIFACTS_DIR/env/build_branch.env
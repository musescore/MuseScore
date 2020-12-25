#!/bin/bash

ENV_FILE=./../musescore_tx2s3_environment.sh
source $ENV_FILE

command -v lrelease >/dev/null 2>&1 || { echo "error: not found 'lrelease'" >&2; exit 1; }
command -v tx >/dev/null 2>&1 || { echo "error: not found 'tx'" >&2; exit 1; }
command -v s3cmd >/dev/null 2>&1 || { echo "error: not found 's3cmd'" >&2; exit 1; }

echo "lrelease: $(lrelease -version)"
echo "tx: $(tx --version)"
echo "s3cmd: $(s3cmd --version)"

echo "Updating translation on s3..."
SCRIPT_PATH=$(dirname $0)

# remove old ts files 
rm -rf share/locale/

python3 $SCRIPT_PATH/tx2s3.py
echo "Translation updated"

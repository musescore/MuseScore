#!/bin/bash

TRANSIFEX_USER=""
TRANSIFEX_PASSWORD=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -u|--user) TRANSIFEX_USER="$2"; shift ;;
        -p|--password) TRANSIFEX_PASSWORD="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$TRANSIFEX_USER" ]; then echo "error: not set TRANSIFEX_USER"; exit 1; fi
if [ -z "$TRANSIFEX_PASSWORD" ]; then echo "error: not set TRANSIFEX_PASSWORD"; exit 1; fi

apt install python3-setuptools
pip3 install transifex-client

cat > ~/.transifexrc <<EOL
[https://www.transifex.com]
hostname = https://www.transifex.com
password = $TRANSIFEX_PASSWORD
token =
username = $TRANSIFEX_USER
EOL

echo "tx version: $(tx --version)"
#tx push -s
#!/usr/bin/env bash

ARTIFACTS_DIR=build.artifacts
SYMBOLS_PATH=""
SENTRY_URL=""
SENTRY_AUTH_TOKEN=""
SENTRY_ORG=""
SENTRY_PROJECT=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -s|--symbols) SYMBOLS_PATH="$2"; shift ;;
        -u|--url) SENTRY_URL="$2"; shift ;;
        -t|--token) SENTRY_AUTH_TOKEN="$2"; shift ;;
        -o|--org) SENTRY_ORG="$2"; shift ;;
        -p|--project) SENTRY_PROJECT="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

# Default
if [ -z "$SYMBOLS_PATH" ]; then SYMBOLS_PATH=$ARTIFACTS_DIR/symbols; fi
if [ -z "$SENTRY_URL" ]; then SENTRY_URL=https://sentry.musescore.org; fi
if [ -z "$SENTRY_ORG" ]; then SENTRY_ORG=musescore; fi

# Check
if [ -z "$SYMBOLS_PATH" ]; then echo "error: not set SYMBOLS_PATH"; exit 1; fi
if [ -z "$SENTRY_URL" ]; then echo "error: not set SENTRY_URL"; exit 1; fi
if [ -z "$SENTRY_AUTH_TOKEN" ]; then echo "error: not set SENTRY_AUTH_TOKEN"; exit 1; fi
if [ -z "$SENTRY_ORG" ]; then echo "error: not set SENTRY_ORG"; exit 1; fi
if [ -z "$SENTRY_PROJECT" ]; then echo "error: not set SENTRY_PROJECT"; exit 1; fi

# Print
echo "SYMBOLS_PATH: $SYMBOLS_PATH"
echo "SENTRY_URL: $SENTRY_URL"
echo "SENTRY_AUTH_TOKEN: $SENTRY_AUTH_TOKEN"
echo "SENTRY_ORG: $SENTRY_ORG"
echo "SENTRY_PROJECT: $SENTRY_PROJECT"

# Install Sentry CLI
if ! command -v sentry-cli &> /dev/null
then
    curl -sL https://sentry.io/get-cli/ | bash
    echo "sentry-cli now installed"
else
    echo "sentry-cli already installed"
fi

# Upload symbols
export SENTRY_URL=$SENTRY_URL
export SENTRY_AUTH_TOKEN=$SENTRY_AUTH_TOKEN

sentry-cli upload-dif -o $SENTRY_ORG -p $SENTRY_PROJECT $SYMBOLS_PATH

if [ $? -eq 0 ]; then
    echo "Success symbols uploaded"
else
    echo "Failed symbols uploaded, code: $?"
fi
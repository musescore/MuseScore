#!/usr/bin/env bash

echo "Generate dump symbols"

GEN_SCRIPT=tools/crashdump/generate_syms.sh
DUMPSYMS_BIN=/c/breakpad_tools/dump_syms.exe
ARTIFACTS_DIR=build.artifacts
BUILD_DIR=build.release
SYMBOLS_DIR=$ARTIFACTS_DIR/symbols
MSCORE_BIN=$BUILD_DIR/src/main/MuseScore4.pdb

echo "GEN_SCRIPT: $GEN_SCRIPT"
echo "DUMPSYMS_BIN: $DUMPSYMS_BIN"
echo "BUILD_DIR: $BUILD_DIR"
echo "SYMBOLS_DIR: $SYMBOLS_DIR"
echo "MSCORE_BIN: $MSCORE_BIN"

$GEN_SCRIPT --dumpsyms-bin $DUMPSYMS_BIN --build-dir $BUILD_DIR --symbols-dir $SYMBOLS_DIR --mscore-bin $MSCORE_BIN

echo "-----"
ls $SYMBOLS_DIR
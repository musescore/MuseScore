#!/bin/bash

HERE="${BASH_SOURCE%/*}" # path to dir that contains this script

LUPDATE=lupdate
SRC_DIR=$HERE/../../src 
TS_DIR=$HERE/../../share/locale 
ARGS="-recursive -tr-function-alias translate+=trc -tr-function-alias translate+=mtrc -tr-function-alias translate+=qtrc -tr-function-alias qsTranslate+=qsTrc -extensions qml,cpp -no-obsolete"

for f in $TS_DIR/mscore_*.ts
do
  echo "Processing $f ..."
  $LUPDATE $ARGS $SRC_DIR -ts $f
done
#!/bin/bash

ENV_FILE=./../musescore_lupdate_environment.sh
source $ENV_FILE

# Translation routines
# update translation on transifex
# remove obsolete strings
OBSOLETE=-no-obsolete # '-noobsolete' in older QT versions

./build/gen-qt-projectfile . > mscore.pro
lupdate ${OBSOLETE} mscore.pro
./build/gen-instruments-projectfile ./share/instruments > instruments.pro
lupdate ${OBSOLETE} instruments.pro
./build/gen-tours-projectfile ./share/tours > tours.pro
lupdate ${OBSOLETE} tours.pro

rm mscore.pro
rm instruments.pro
rm tours.pro

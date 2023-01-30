#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
echo "MuseScore DTest Compare Datas"

HERE="$(dirname ${BASH_SOURCE[0]})"
CURRENT_DIR="./current_datas"
REFERENCE_DIR="./reference_datas"
OUTPUT_DIR="./comparison"
MSCORE_BIN=build.debug/install/bin/mscore

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--current-dir) CURRENT_DIR="$2"; shift ;;
        -r|--reference-dir) REFERENCE_DIR="$2"; shift ;;
        -o|--output-dir) OUTPUT_DIR="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "::group::Configuration:"
echo "CURRENT_DIR: $CURRENT_DIR"
echo "REFERENCE_DIR: $REFERENCE_DIR"
echo "OUTPUT_DIR: $OUTPUT_DIR"
echo "::endgroup::"

rm -rf $OUTPUT_DIR
mkdir $OUTPUT_DIR

DATAS_REF_LIST=$(ls $REFERENCE_DIR/*.json)
DIFF_NAME_LIST=""
for DATA_REF_FILE in $DATAS_REF_LIST ; do
    data_file_name=$(basename $DATA_REF_FILE)
    FILE_NAME=${data_file_name%.json}
    DATA_CUR_FILE=$CURRENT_DIR/${FILE_NAME}.json
    DATA_DIFF_FILE=$OUTPUT_DIR/${FILE_NAME}.diff.json
    
    if test -f $DATA_CUR_FILE; then
        $MSCORE_BIN --diagnostic-com-drawdata $DATA_REF_FILE $DATA_CUR_FILE --diagnostic-output $DATA_DIFF_FILE #2>/dev/null 1>/dev/null
        code=$?
        echo "diagnostic code: $code"
        if (( $code > 0 )); then
            echo "Different: ref: $DATA_REF_FILE, current: $DATA_CUR_FILE, code: $code"

            if [ "$VTEST_DIFF_FOUND" != "true" ]; then
                export VTEST_DIFF_FOUND=true
                echo "VTEST_DIFF_FOUND=$VTEST_DIFF_FOUND" >> $GITHUB_ENV
            fi

            DIFF_NAME_LIST+=" "$FILE_NAME

            cp $DATA_REF_FILE $OUTPUT_DIR/$FILE_NAME.ref.json
            cp $DATA_CUR_FILE $OUTPUT_DIR

        else
            echo "Equal: ref: $DATA_REF_FILE, current: $DATA_CUR_FILE"
            rm -f $DATA_DIFF_FILE 2>/dev/null
        fi
    fi
done

# Generate pngs
if [ "$VTEST_DIFF_FOUND" == "true" ]; then
    for DIFF_NAME in $DIFF_NAME_LIST ; do
        $MSCORE_BIN --diagnostic-drawdata-to-png $OUTPUT_DIR/$DIFF_NAME.ref.json --diagnostic-output $OUTPUT_DIR/$DIFF_NAME.ref.png
        $MSCORE_BIN --diagnostic-drawdata-to-png $OUTPUT_DIR/$DIFF_NAME.json --diagnostic-output $OUTPUT_DIR/$DIFF_NAME.png
        $MSCORE_BIN --diagnostic-drawdiff-to-png $OUTPUT_DIR/$DIFF_NAME.diff.json $OUTPUT_DIR/$DIFF_NAME.ref.json --diagnostic-output $OUTPUT_DIR/$DIFF_NAME.diff.png
        # generate comparison gif
        convert -delay 80 -loop 0 $OUTPUT_DIR/$DIFF_NAME.png $OUTPUT_DIR/$DIFF_NAME.ref.png $OUTPUT_DIR/$DIFF_NAME.diff.gif
    done
fi


# Generate html report
if [ "$VTEST_DIFF_FOUND" == "true" ]; then

    echo "Generate html report"
    HTML=$OUTPUT_DIR/vtest_compare.html
    rm -f $HTML
    cp $HERE/style.css $OUTPUT_DIR
    echo "<html>" >> $HTML
    echo "  <head>" >> $HTML
    echo "   <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">" >> $HTML
    echo "  </head>" >> $HTML
    echo "  <body>" >> $HTML
    echo "    <div id=\"topmargin\"></div>" >> $HTML

    for DIFF_NAME in $DIFF_NAME_LIST ; do
        echo "    <h2 id=\"$DIFF_NAME\">$DIFF_NAME <a class=\"toc-anchor\" href=\"#$DIFF_NAME\">#</a></h2>" >> $HTML
        echo "    <div>" >> $HTML
        echo "      <div>ref:</div><br/>" >> $HTML
        echo "      <img src=\"$DIFF_NAME.ref.png\"><br/>" >> $HTML
        echo "      <div>current:</div><br/>" >> $HTML
        echo "      <img src=\"$DIFF_NAME.png\"><br/>" >> $HTML
        echo "      <div>diff:</div>" >> $HTML
        echo "      <img src=\"$DIFF_NAME.diff.png\"><br/>" >> $HTML
        echo "      <img src=\"$DIFF_NAME.diff.gif\"><br/>" >> $HTML
        echo "    </div>" >> $HTML
    done

    echo "  </body>" >> $HTML
    echo "</html>" >> $HTML
fi

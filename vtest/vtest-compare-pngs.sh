#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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
echo "MuseScore VTest Compare PNGs"

HERE="$(dirname ${BASH_SOURCE[0]})"
CURRENT_DIR="./current_pngs"
REFERENCE_DIR="./reference_pngs"
OUTPUT_DIR="./comparison"
GEN_GIF=1
CI_MODE=0

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--current-dir) CURRENT_DIR="$2"; shift ;;
        -r|--reference-dir) REFERENCE_DIR="$2"; shift ;;
        -o|--output-dir) OUTPUT_DIR="$2"; shift ;;
        -g|--gen-gif) GEN_GIF=$2; shift ;;
        --ci) CI_MODE=$2; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "::group::Configuration:"
echo "PWD: $PWD"
echo "CURRENT_DIR: $CURRENT_DIR"
echo "REFERENCE_DIR: $REFERENCE_DIR"
echo "OUTPUT_DIR: $OUTPUT_DIR"
echo "::endgroup::"

rm -rf $OUTPUT_DIR
mkdir $OUTPUT_DIR

command -v compare >/dev/null 2>&1
if [[ $? -ne 0 ]]; then
    echo "ERROR: not found compare tool" 
    echo "TO FIX, RUN: sudo apt install imagemagick"
    exit 1
fi

PNG_REF_LIST=$(ls $REFERENCE_DIR/*.png)
DIFF_NAME_LIST=""
DIFF_FOUND=false
for PNG_REF_FILE in $PNG_REF_LIST ; do
    png_file_name=$(basename $PNG_REF_FILE)
    FILE_NAME=${png_file_name%.png}
    PNG_CUR_FILE=$CURRENT_DIR/${FILE_NAME}.png
    PNG_DIFF_FILE=$OUTPUT_DIR/${FILE_NAME}.diff.png
    GIF_DIFF_FILE=$OUTPUT_DIR/${FILE_NAME}.diff.gif
    
    if test -f $PNG_CUR_FILE; then
        code=$(compare -metric AE -fuzz 0.0% $PNG_REF_FILE $PNG_CUR_FILE $PNG_DIFF_FILE 2>&1)
        if (( $code > 0 )); then
            echo "Different: ref: $PNG_REF_FILE, current: $PNG_CUR_FILE, code: $code"
            export DIFF_FOUND=true
            DIFF_NAME_LIST+=" "$FILE_NAME

            cp $PNG_REF_FILE $OUTPUT_DIR/$FILE_NAME.ref.png
            cp $PNG_CUR_FILE $OUTPUT_DIR

            # generate comparison gif
            if [ $GEN_GIF -eq 1 ]; then
                convert -delay 80 -loop 0 $PNG_CUR_FILE $PNG_REF_FILE $GIF_DIFF_FILE
            fi    
        else
            echo "Equal: ref: $PNG_REF_FILE, current: $PNG_CUR_FILE"
            rm -f $PNG_DIFF_FILE 2>/dev/null
        fi
    fi
done


# Generate html report
if [ "$DIFF_FOUND" == "true" ]; then
    export VTEST_DIFF_FOUND=true
    echo "VTEST_DIFF_FOUND=$VTEST_DIFF_FOUND" >> $GITHUB_ENV

    echo "Generate html report"
    HTML=$OUTPUT_DIR/vtest_compare.html
    rm -f $HTML
    cp $HERE/style.css $OUTPUT_DIR
    echo "<html>" >> $HTML
    echo "  <head>" >> $HTML
    echo "   <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">" >> $HTML
    echo "  </head>" >> $HTML
    echo "  <body>" >> $HTML
    echo "    <div id=\"topbar\">" >> $HTML
    echo "      <span>Reference</span>" >> $HTML
    echo "      <span>Current</span>" >> $HTML
    echo "      <span>Diff</span>" >> $HTML
    echo "    </div>" >> $HTML
    echo "    <div id=\"topmargin\"></div>" >> $HTML

    for DIFF_NAME in $DIFF_NAME_LIST ; do
        echo "    <h2 id=\"$DIFF_NAME\">$DIFF_NAME <a class=\"toc-anchor\" href=\"#$DIFF_NAME\">#</a></h2>" >> $HTML
        echo "    <div>" >> $HTML
        echo "      <img src=\"$DIFF_NAME.ref.png\">" >> $HTML
        echo "      <img src=\"$DIFF_NAME.png\">" >> $HTML
        echo "      <img src=\"$DIFF_NAME.diff.png\">" >> $HTML
        echo "      <img src=\"$DIFF_NAME.diff.gif\">" >> $HTML
        echo "    </div>" >> $HTML
    done

    echo "  </body>" >> $HTML
    echo "</html>" >> $HTML

    if [ $CI_MODE -eq 1 ]; then
        exit 0
    else
        exit 1
    fi
fi

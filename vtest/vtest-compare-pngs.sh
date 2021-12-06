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
echo "MuseScore VTest Compare PNGs"

HERE="$(dirname ${BASH_SOURCE[0]})"
CURRENT_DIR="./current_pngs"
REFERENCE_DIR="./reference_pngs"
OUTPUT_DIR="./comparison"

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

PNG_REF_LIST=$(ls $REFERENCE_DIR/*.png)
DIFF_NAME_LIST=""
for PNG_REF_FILE in $PNG_REF_LIST ; do
    png_file_name=$(basename $PNG_REF_FILE)
    FILE_NAME=${png_file_name%.png}
    PNG_CUR_FILE=$CURRENT_DIR/${FILE_NAME}.png
    PNG_DIFF_FILE=$OUTPUT_DIR/${FILE_NAME}.diff.png
    
    if test -f $PNG_CUR_FILE; then
        code=$(compare -metric AE -fuzz 0.0% $PNG_REF_FILE $PNG_CUR_FILE $PNG_DIFF_FILE 2>&1)
        if (( $code > 0 )); then
            echo "Different: ref: $PNG_REF_FILE, current: $PNG_CUR_FILE, code: $code"
            export VTEST_DIFF_FOUND=true
            echo "VTEST_DIFF_FOUND=$VTEST_DIFF_FOUND" >> $GITHUB_ENV
            DIFF_NAME_LIST+=" "$FILE_NAME

            cp $PNG_REF_FILE $OUTPUT_DIR/$FILE_NAME.ref.png
            cp $PNG_CUR_FILE $OUTPUT_DIR
        else
            echo "Equal: ref: $PNG_REF_FILE, current: $PNG_CUR_FILE"
            rm -f $PNG_DIFF_FILE 2>/dev/null
        fi
    fi
done

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
        echo "    </div>" >> $HTML
    done

    echo "  </body>" >> $HTML
    echo "</html>" >> $HTML

else
    rm -rf $OUTPUT_DIR
fi

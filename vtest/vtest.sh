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
echo "MuseScore VTest"

RUN_CMD="compare" # gen_cur, gen_ref
HERE="$(dirname ${BASH_SOURCE[0]})"
SCORES_DIR="$HERE/scores"
ARTIFACTS_DIR=vtest.artifacts
PNG_REF_DIR=$ARTIFACTS_DIR/ref
PNG_CUR_DIR=$ARTIFACTS_DIR/current
COMPARE_DIR=$ARTIFACTS_DIR/compare
MSCORE_BIN=build.debug/install/bin/mscore
DPI=130

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--cmd) RUN_CMD="$2"; shift ;;
        -s|--scores) SCORES_DIR="$2"; shift ;;
        -a|--artifacts) ARTIFACTS_DIR="$2"; shift ;;
        -m|--mscore) MSCORE_BIN="$2"; shift ;;
        --ref-dir) PNG_REF_DIR="$2"; shift ;;
        --cur-dir) PNG_CUR_DIR="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done


echo "RUN_CMD: $RUN_CMD"
echo "SCORES_DIR: $SCORES_DIR"
echo "ARTIFACTS_DIR: $ARTIFACTS_DIR"
echo "MSCORE_BIN: $MSCORE_BIN"
echo "DPI: $DPI"

if [ "$RUN_CMD" == "gen_ref" ]; then RUN_CMD="gen"; IS_REF=1; fi
if [ "$RUN_CMD" == "gen_cur" ]; then RUN_CMD="gen"; IS_REF=0; fi

mkdir -p $ARTIFACTS_DIR

#######################################
# Generation
#######################################

if [ "$RUN_CMD" == "gen" ]; then
    echo "Run png generation";

    if [ $IS_REF -eq 1 ]; then 
        JSON_FILE=$PNG_REF_DIR/vtestjob_ref.json; 
        PNG_DIR=$PNG_REF_DIR
        PNG_PREFIX=".ref.png"
        LOG_FILE=$PNG_REF_DIR/convert_ref.log
    else 
        JSON_FILE=$PNG_CUR_DIR/vtestjob.json; 
        PNG_DIR=$PNG_CUR_DIR
        PNG_PREFIX=".png"
        LOG_FILE=$PNG_CUR_DIR/convert.log
    fi

    rm -rf $PNG_DIR
    mkdir $PNG_DIR

    SCORES_LIST=$(ls -p $SCORES_DIR | grep -v /) 
    
    echo "Generate JSON job file"
    echo "[" >> $JSON_FILE
    for score in $SCORES_LIST ; do
        OUT_FILE=$PNG_DIR/${score%.*}$PNG_PREFIX
        rm -f $OUT_FILE
        echo "{ \"in\" : \"$SCORES_DIR/$score\",         \"out\" : \"$OUT_FILE\" }," >> $JSON_FILE;
    done
    echo "{}]" >> $JSON_FILE
    cat $JSON_FILE        

    echo "Generate PNG files"
    $MSCORE_BIN -j $JSON_FILE -r $DPI 2>&1 | tee $LOG_FILE

    echo "========"
    ls $PNG_DIR
    echo "========"

#######################################
# Compare
#######################################

elif [ "$RUN_CMD" == "compare" ]; then
    echo "Compare PNG files and references"

    # Compare
    rm -rf $COMPARE_DIR
    mkdir $COMPARE_DIR
    PNG_REF_LIST=$(ls $PNG_REF_DIR/*.ref.png)
    DIFF_NAME_LIST=""
    for PNG_REF_FILE in $PNG_REF_LIST ; do
        png_file_name=$(basename $PNG_REF_FILE)
        FILE_NAME=${png_file_name%.ref.png}
        PNG_CUR_FILE=$PNG_CUR_DIR/${FILE_NAME}.png
        PNG_DIFF_FILE=$COMPARE_DIR/${FILE_NAME}.diff.png
        
        if test -f $PNG_CUR_FILE; then
            code=$(compare -metric AE -fuzz 0.0% $PNG_REF_FILE $PNG_CUR_FILE $PNG_DIFF_FILE 2>&1)
            if (( $code > 0 )); then
                echo "Different ref: $PNG_REF_FILE, current: $PNG_CUR_FILE, code: $code"
                export VTEST_DIFF_FOUND=true
                DIFF_NAME_LIST+=$FILE_NAME

                cp $PNG_REF_FILE $COMPARE_DIR
                cp $PNG_CUR_FILE $COMPARE_DIR
            else
                echo "Equal ref: $PNG_REF_FILE, current: $PNG_CUR_FILE"
                rm -f $PNG_DIFF_FILE 2>/dev/null
            fi
        fi
    done

    # Generate html report
    if [ "$VTEST_DIFF_FOUND" == "true" ]; then

        echo "Generate html report"
        HTML=$COMPARE_DIR/vtest_compare.html
        rm -f $HTML
        cp $HERE/style.css $COMPARE_DIR
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
    fi

    if [ "$VTEST_DIFF_FOUND" != "true" ]; then
        rm -rf $COMPARE_DIR
    fi

else 
    echo "Unknown run cmd: $RUN_CMD"; exit 1;
fi


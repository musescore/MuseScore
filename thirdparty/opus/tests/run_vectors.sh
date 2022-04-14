#!/bin/sh

# Copyright (c) 2011-2012 Jean-Marc Valin
#
#  This file is extracted from RFC6716. Please see that RFC for additional
#  information.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  - Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#  - Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#  - Neither the name of Internet Society, IETF or IETF Trust, nor the
#  names of specific contributors, may be used to endorse or promote
#  products derived from this software without specific prior written
#  permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
#  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

rm -f logs_mono.txt logs_mono2.txt
rm -f logs_stereo.txt logs_stereo2.txt

if [ "$#" -ne "3" ]; then
    echo "usage: run_vectors.sh <exec path> <vector path> <rate>"
    exit 1
fi

CMD_PATH=$1
VECTOR_PATH=$2
RATE=$3

: ${OPUS_DEMO:=$CMD_PATH/opus_demo}
: ${OPUS_COMPARE:=$CMD_PATH/opus_compare}

if [ -d "$VECTOR_PATH" ]; then
    echo "Test vectors found in $VECTOR_PATH"
else
    echo "No test vectors found"
    #Don't make the test fail here because the test vectors
    #will be distributed separately
    exit 0
fi

if [ ! -x "$OPUS_COMPARE" ]; then
    echo "ERROR: Compare program not found: $OPUS_COMPARE"
    exit 1
fi

if [ -x "$OPUS_DEMO" ]; then
    echo "Decoding with $OPUS_DEMO"
else
    echo "ERROR: Decoder not found: $OPUS_DEMO"
    exit 1
fi

echo "=============="
echo "Testing mono"
echo "=============="
echo

for file in 01 02 03 04 05 06 07 08 09 10 11 12
do
    if [ -e "$VECTOR_PATH/testvector$file.bit" ]; then
        echo "Testing testvector$file"
    else
        echo "Bitstream file not found: testvector$file.bit"
    fi
    if "$OPUS_DEMO" -d "$RATE" 1 "$VECTOR_PATH/testvector$file.bit" tmp.out >> logs_mono.txt 2>&1; then
        echo "successfully decoded"
    else
        echo "ERROR: decoding failed"
        exit 1
    fi
    "$OPUS_COMPARE" -r "$RATE" "$VECTOR_PATH/testvector${file}.dec" tmp.out >> logs_mono.txt 2>&1
    float_ret=$?
    "$OPUS_COMPARE" -r "$RATE" "$VECTOR_PATH/testvector${file}m.dec" tmp.out >> logs_mono2.txt 2>&1
    float_ret2=$?
    if [ "$float_ret" -eq "0" ] || [ "$float_ret2" -eq "0" ]; then
        echo "output matches reference"
    else
        echo "ERROR: output does not match reference"
        exit 1
    fi
    echo
done

echo "=============="
echo Testing stereo
echo "=============="
echo

for file in 01 02 03 04 05 06 07 08 09 10 11 12
do
    if [ -e "$VECTOR_PATH/testvector$file.bit" ]; then
        echo "Testing testvector$file"
    else
        echo "Bitstream file not found: testvector$file"
    fi
    if "$OPUS_DEMO" -d "$RATE" 2 "$VECTOR_PATH/testvector$file.bit" tmp.out >> logs_stereo.txt 2>&1; then
        echo "successfully decoded"
    else
        echo "ERROR: decoding failed"
        exit 1
    fi
    "$OPUS_COMPARE" -s -r "$RATE" "$VECTOR_PATH/testvector${file}.dec" tmp.out >> logs_stereo.txt 2>&1
    float_ret=$?
    "$OPUS_COMPARE" -s -r "$RATE" "$VECTOR_PATH/testvector${file}m.dec" tmp.out >> logs_stereo2.txt 2>&1
    float_ret2=$?
    if [ "$float_ret" -eq "0" ] || [ "$float_ret2" -eq "0" ]; then
        echo "output matches reference"
    else
        echo "ERROR: output does not match reference"
        exit 1
    fi
    echo
done



echo "All tests have passed successfully"
mono1=`grep quality logs_mono.txt | awk '{sum+=$4}END{if (NR == 12) sum /= 12; else sum = 0; print sum}'`
mono2=`grep quality logs_mono2.txt | awk '{sum+=$4}END{if (NR == 12) sum /= 12; else sum = 0; print sum}'`
echo $mono1 $mono2 | awk '{if ($2 > $1) $1 = $2; print "Average mono quality is", $1, "%"}'

stereo1=`grep quality logs_stereo.txt | awk '{sum+=$4}END{if (NR == 12) sum /= 12; else sum = 0; print sum}'`
stereo2=`grep quality logs_stereo2.txt | awk '{sum+=$4}END{if (NR == 12) sum /= 12; else sum = 0; print sum}'`
echo $stereo1 $stereo2 | awk '{if ($2 > $1) $1 = $2; print "Average stereo quality is", $1, "%"}'

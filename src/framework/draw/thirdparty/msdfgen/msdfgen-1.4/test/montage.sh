#!/bin/sh

# Simple helper so I can compare versions and get a bird's eye of the tests.
# This isn't meant to be run as a unit test.

EXE=../Release/msdfgen

DIR=${1:-.}
echo "Running montage in $DIR"

function runtest()
{
	echo "--[[ Running $1 v$2 ]]--"
	rm -f test-*.$1.png
	rm -f test-*.$1-diff.png
	rm -f test-*.$1-render.png
	python test_msdf.py --svg-dir "$DIR" --legacy $2 --mode $1 --exe "$EXE" --montage
}

function runmode()
{
	runtest $1 2
	runtest $1 0
	compare montage-$1-[02]-diff.png -highlight-color blue montage-$1-2vs0-diff.png
	open montage-$1-2vs0-diff.png
}


runmode msdf
#runmode sdf


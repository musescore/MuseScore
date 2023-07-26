#!/bin/sh -e

#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2002-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
#
#  This file is part the FLAC project.  FLAC is comprised of several
#  components distributed under different licenses.  The codec libraries
#  are distributed under Xiph.Org's BSD-like license (see the file
#  COPYING.Xiph in this distribution).  All other programs, libraries, and
#  plugins are distributed under the GPL (see COPYING.GPL).  The documentation
#  is distributed under the Gnu FDL (see COPYING.FDL).  Each file in the
#  FLAC distribution contains at the top the terms under which it may be
#  distributed.
#
#  Since this particular file is relevant to all components of FLAC,
#  it may be distributed under the Xiph.Org license, which is the least
#  restrictive of those mentioned above.  See the file COPYING.Xiph in this
#  distribution.

. ./common.sh

PATH="$(pwd)/../src/flac:$PATH"
PATH="$(pwd)/../src/metaflac:$PATH"
PATH="$(pwd)/../objs/$BUILD/bin:$PATH"

if echo a | (grep -E '(a|b)') >/dev/null 2>&1
	then EGREP='grep -E'
	else EGREP='egrep'
fi

testdir="metaflac-test-files"
flacfile="metaflac1.flac"

flac${EXE} --help 1>/dev/null 2>/dev/null || die "ERROR can't find flac executable"
metaflac${EXE} --help 1>/dev/null 2>/dev/null || die "ERROR can't find metaflac executable"

run_flac ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 flac $*" >>test_metaflac.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 flac${EXE} ${TOTALLY_SILENT} --no-error-on-compression-fail $* 4>>test_metaflac.valgrind.log
	else
		flac${EXE} ${TOTALLY_SILENT} --no-error-on-compression-fail $*
	fi
}

run_metaflac ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 metaflac $*" >>test_metaflac.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 metaflac${EXE} $* 4>>test_metaflac.valgrind.log
	else
		metaflac${EXE} $*
	fi
}

run_metaflac_silent ()
{
	if [ -z "$SILENT" ] ; then
		run_metaflac $*
	else
		if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
			echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 metaflac $*" >>test_metaflac.valgrind.log
			valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 metaflac${EXE} $* 2>/dev/null 4>>test_metaflac.valgrind.log
		else
			metaflac${EXE} $* 2>/dev/null
		fi
	fi
}

run_metaflac_to_metaflac_silent ()
{
	if [ "$FLAC__TEST_WITH_VALGRIND" = yes ] ; then
		echo "valgrind --leak-check=yes --show-reachable=yes --num-callers=50 metaflac $*" >>test_metaflac.valgrind.log
		valgrind --leak-check=yes --show-reachable=yes --num-callers=50 --log-fd=4 metaflac${EXE} $* 2>/dev/null 4>>test_metaflac.valgrind.log
	else
		metaflac${EXE} $1 | metaflac${EXE} $2 2>/dev/null
	fi
}


check_flac ()
{
	run_flac --silent --test $flacfile || die "ERROR in $flacfile" 1>&2
}

echo "Generating stream..."
bytes=80000
if dd if=/dev/zero ibs=1 count=$bytes 2>/dev/null | flac${EXE} ${TOTALLY_SILENT} --force --verify -0 --input-size=$bytes --output-name=$flacfile --force-raw-format --endian=big --sign=signed --channels=1 --bps=8 --sample-rate=8000 - ; then
	chmod +w $flacfile
else
	die "ERROR during generation"
fi

check_flac

testdatadir=${top_srcdir}/test/metaflac-test-files

filter ()
{
	# minor danger, changing vendor strings will change the length of the
	# VORBIS_COMMENT block, but if we add "^  length: " to the patterns,
	# we lose info about PADDING size that we need
	# grep pattern 1: remove vendor string
	# grep pattern 2: remove minimum/maximum frame and block size from STREAMINFO
	# grep pattern 3: remove hexdump data from PICTURE metadata blocks
	# sed pattern 1: remove stream offset values from SEEKTABLE points
	$EGREP -v '^  vendor string: |^  m..imum .....size: |^    0000[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]: ' \
		| sed -e 's/, stream_offset.*//'
}
metaflac_test ()
{
	case="$testdatadir/$1"
	desc="$2"
	args="$3"
	expect="$case-expect.meta"
	echo $ECHO_N "test $1: $desc... " $ECHO_C
	run_metaflac $args $flacfile | filter > $testdir/out1.meta || die "ERROR running metaflac"
	# Ignore lengths which can be affected by the version string.
	sed "s/length:.*/length: XXX/" $testdir/out1.meta > $testdir/out.meta
	diff -w $expect $testdir/out.meta > /dev/null 2>&1 || die "ERROR: metadata does not match expected $expect"
	# To blindly accept (and check later): cp -f $testdir/out.meta $expect
	echo OK
}

metaflac_test_nofilter ()
{
	case="$testdatadir/$1"
	desc="$2"
	args="$3"
	expect="$case-expect.meta"
	echo $ECHO_N "test $1: $desc... " $ECHO_C
	run_metaflac $args $flacfile > $testdir/out.meta || die "ERROR running metaflac"
	diff -w $expect $testdir/out.meta || die "ERROR: metadata does not match expected $expect"
	echo OK
}

metaflac_test_binary ()
{
	case="$testdatadir/$1"
	desc="$2"
	args="$3"
	expect="$case-expect.meta"
	echo $ECHO_N "test $1: $desc... " $ECHO_C
	run_metaflac $args $flacfile > $testdir/out.meta || die "ERROR running metaflac"
	cmp $expect $testdir/out.meta || die "ERROR: metadata does not match expected $expect"
	echo OK
}

metaflac_test case00 "--list" "--list"

metaflac_test case01 "STREAMINFO --show-* shortcuts" "
	--show-md5sum
	--show-min-blocksize
	--show-max-blocksize
	--show-min-framesize
	--show-max-framesize
	--show-sample-rate
	--show-channels
	--show-bps
	--show-total-samples"

run_metaflac --preserve-modtime --add-padding=12345 $flacfile
check_flac
metaflac_test case02 "--add-padding" "--list"

# some flavors of /bin/sh (e.g. Darwin's) won't even handle quoted spaces, so we underscore:
run_metaflac --set-tag="ARTIST=The_artist_formerly_known_as_the_artist..." $flacfile
check_flac
metaflac_test case03 "--set-tag=ARTIST" "--list"

run_metaflac --set-tag="ARTIST=Chuck_Woolery" $flacfile
check_flac
metaflac_test case04 "--set-tag=ARTIST" "--list"

run_metaflac --set-tag="ARTIST=Vern" $flacfile
check_flac
metaflac_test case05 "--set-tag=ARTIST" "--list"

run_metaflac --set-tag="TITLE=He_who_smelt_it_dealt_it" $flacfile
check_flac
metaflac_test case06 "--set-tag=TITLE" "--list"

if [ ! $git_commit_version_hash ] ; then
	metaflac_test case07 "--show-vendor-tag --show-tag=ARTIST" "--show-vendor-tag --show-tag=ARTIST"
else
	echo "test case07 is skipped because version is taken from git"
fi

run_metaflac --remove-first-tag=ARTIST $flacfile
check_flac
metaflac_test case08 "--remove-first-tag=ARTIST" "--list"

run_metaflac --remove-tag=ARTIST $flacfile
check_flac
metaflac_test case09 "--remove-tag=ARTIST" "--list"

metaflac_test case10 "--list --block-type=VORBIS_COMMENT" "--list --block-type=VORBIS_COMMENT"
metaflac_test case11 "--list --block-number=0" "--list --block-number=0"
metaflac_test case12 "--list --block-number=1,2,999" "--list --block-number=1,2,999"
metaflac_test case13 "--list --block-type=VORBIS_COMMENT,PADDING" "--list --block-type=VORBIS_COMMENT,PADDING"
metaflac_test case14 "--list --except-block-type=SEEKTABLE,VORBIS_COMMENT" "--list --except-block-type=SEEKTABLE,VORBIS_COMMENT"
metaflac_test case15 "--list --except-block-type=STREAMINFO" "--list --except-block-type=STREAMINFO"

run_metaflac --add-padding=4321 $flacfile $flacfile
check_flac
metaflac_test case16 "--add-padding=4321 * 2" "--list"

run_metaflac --merge-padding $flacfile
check_flac
metaflac_test case17 "--merge-padding" "--list"

run_metaflac --add-padding=0 $flacfile
check_flac
metaflac_test case18 "--add-padding=0" "--list"

run_metaflac --sort-padding $flacfile
check_flac
metaflac_test case19 "--sort-padding" "--list"

run_metaflac --add-padding=0 $flacfile
check_flac
metaflac_test case20 "--add-padding=0" "--list"

run_metaflac --remove-all-tags $flacfile
check_flac
metaflac_test case21 "--remove-all-tags" "--list"

run_metaflac --remove --block-number=1,99 --dont-use-padding $flacfile
check_flac
metaflac_test case22 "--remove --block-number=1,99 --dont-use-padding" "--list"

run_metaflac --remove --block-number=99 --dont-use-padding $flacfile
check_flac
metaflac_test case23 "--remove --block-number=99 --dont-use-padding" "--list"

run_metaflac --remove --block-type=PADDING $flacfile
check_flac
metaflac_test case24 "--remove --block-type=PADDING" "--list"

run_metaflac --remove --block-type=PADDING --dont-use-padding $flacfile
check_flac
metaflac_test case25 "--remove --block-type=PADDING --dont-use-padding" "--list"

run_metaflac --add-padding=0 $flacfile $flacfile
check_flac
metaflac_test case26 "--add-padding=0 * 2" "--list"

run_metaflac --remove --except-block-type=PADDING $flacfile
check_flac
metaflac_test case27 "--remove --except-block-type=PADDING" "--list"

run_metaflac --remove-all $flacfile
check_flac
metaflac_test case28 "--remove-all" "--list"

run_metaflac --remove-all --dont-use-padding $flacfile
check_flac
metaflac_test case29 "--remove-all --dont-use-padding" "--list"

run_metaflac --remove-all --dont-use-padding $flacfile
check_flac
metaflac_test case30 "--remove-all --dont-use-padding" "--list"

run_metaflac --set-tag="f=0123456789abcdefghij" $flacfile
check_flac
metaflac_test case31 "--set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0123456789abcdefghi" $flacfile
check_flac
metaflac_test case32 "--remove-all-tags --set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0123456789abcde" $flacfile
check_flac
metaflac_test case33 "--remove-all-tags --set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0" $flacfile
check_flac
metaflac_test case34 "--remove-all-tags --set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0123456789" $flacfile
check_flac
metaflac_test case35 "--remove-all-tags --set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0123456789abcdefghi" $flacfile
check_flac
metaflac_test case36 "--remove-all-tags --set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0123456789" $flacfile
check_flac
metaflac_test case37 "--remove-all-tags --set-tag=..." "--list"

run_metaflac --remove-all-tags --set-tag="f=0123456789abcdefghij" $flacfile
check_flac
metaflac_test case38 "--remove-all-tags --set-tag=..." "--list"

echo "TITLE=Tittle" | run_metaflac --import-tags-from=- $flacfile
check_flac
metaflac_test case39 "--import-tags-from=-" "--list"

cat > vc.txt << EOF
artist=Fartist
artist=artits
EOF
run_metaflac --import-tags-from=vc.txt $flacfile
check_flac
metaflac_test case40 "--import-tags-from=[FILE]" "--list"

rm vc.txt

run_metaflac --add-replay-gain $flacfile
check_flac
metaflac_test case41 "--add-replay-gain" "--list"

run_metaflac --remove-replay-gain $flacfile
check_flac
metaflac_test case42 "--remove-replay-gain" "--list"

run_metaflac --scan-replay-gain $flacfile
check_flac
metaflac_test case42 "--scan-replay-gain" "--list"

# CUESHEET blocks
cs_in=${top_srcdir}/test/cuesheets/good.000.cue
cs_out=metaflac.cue
cs_out2=metaflac2.cue
run_metaflac --import-cuesheet-from="$cs_in" $flacfile
check_flac
metaflac_test case43 "--import-cuesheet-from" "--list"
run_metaflac --export-cuesheet-to=$cs_out $flacfile
run_metaflac --remove --block-type=CUESHEET $flacfile
check_flac
metaflac_test case44 "--remove --block-type=CUESHEET" "--list"
run_metaflac --import-cuesheet-from=$cs_out $flacfile
check_flac
metaflac_test case45 "--import-cuesheet-from" "--list"
run_metaflac --export-cuesheet-to=$cs_out2 $flacfile
echo "comparing cuesheets:"
diff $cs_out $cs_out2 || die "ERROR, cuesheets should be identical"
echo identical

rm -f $cs_out $cs_out2

# PICTURE blocks
ncase=46
for f in \
	0.gif \
	1.gif \
	2.gif \
; do
	run_metaflac --import-picture-from="|image/gif|$f||${top_srcdir}/test/pictures/$f" $flacfile
	check_flac
	metaflac_test "case$ncase" "--import-picture-from" "--list"
	ncase=$((ncase + 1))
done
for f in \
	0.jpg \
	4.jpg \
; do
	run_metaflac --import-picture-from="4|image/jpeg|$f||${top_srcdir}/test/pictures/$f" $flacfile
	check_flac
	metaflac_test "case$ncase" "--import-picture-from" "--list"
	ncase=$((ncase + 1))
done
for f in \
	0.png \
	1.png \
	2.png \
	3.png \
	4.png \
	5.png \
	6.png \
	7.png \
	8.png \
; do
	run_metaflac --import-picture-from="5|image/png|$f||${top_srcdir}/test/pictures/$f" $flacfile
	check_flac
	metaflac_test "case$ncase" "--import-picture-from" "--list"
	ncase=$((ncase + 1))
done
[ $ncase = 60 ] || die "expected case# to be 60"

fn=export-picture-check
echo $ECHO_N "Testing --export-picture-to... " $ECHO_C
run_metaflac --export-picture-to=$fn $flacfile
check_flac
cmp $fn ${top_srcdir}/test/pictures/0.gif || die "ERROR, exported picture file and original differ"
echo OK
rm -f $fn
echo $ECHO_N "Testing --block-number --export-picture-to... " $ECHO_C
run_metaflac --block-number=9 --export-picture-to=$fn $flacfile
check_flac
cmp $fn ${top_srcdir}/test/pictures/0.png || die "ERROR, exported picture file and original differ"
echo OK
rm -f $fn

run_metaflac --remove --block-type=PICTURE $flacfile
check_flac
metaflac_test case60 "--remove --block-type=PICTURE" "--list"
run_metaflac --import-picture-from="1|image/png|standard_icon|32x32x24|${top_srcdir}/test/pictures/0.png" $flacfile
check_flac
metaflac_test case61 "--import-picture-from" "--list"
run_metaflac --import-picture-from="2|image/png|icon|64x64x24|${top_srcdir}/test/pictures/1.png" $flacfile
check_flac
metaflac_test case62 "--import-picture-from" "--list"
run_metaflac --remove-all-tags-except=artist=title $flacfile
check_flac
metaflac_test case63 "--remove-all-tags-except=artist=title" "--list"
metaflac_test case64 "--export-tags-to=-" "--export-tags-to=-"
metaflac_test case64 "--show-all-tags" "--show-all-tags"

run_flac ${top_srcdir}/test/foreign-metadata-test-files/AIFF-ID3.aiff --keep-foreign-metadata -f -o $flacfile
metaflac_test_binary case65 "--data-format=binary" "--list --data-format=binary-headerless --block-type=APPLICATION:aiff"

# UNKNOWN blocks
flacfile=metaflac2.flac
echo $ECHO_N "Testing FLAC file with unknown metadata... " $ECHO_C
cp -p ${top_srcdir}/test/metaflac.flac.in $flacfile
# remove the VORBIS_COMMENT block so vendor string changes don't interfere with the comparison:
run_metaflac --remove --block-type=VORBIS_COMMENT --dont-use-padding $flacfile
cmp $flacfile ${top_srcdir}/test/metaflac.flac.ok || die "ERROR, $flacfile and metaflac.flac.ok differ"
echo OK

flacfile=metaflac3.flac
cp -p ${top_srcdir}/test/metaflac.flac.in $flacfile

flacfile2=metaflac4.flac
cp $flacfile $flacfile2
run_metaflac --remove-all --dont-use-padding $flacfile

echo $ECHO_N "Appending a streaminfo metadata block... " $ECHO_C
if run_metaflac_to_metaflac_silent "--list --data-format=binary $flacfile2" "--append $flacfile" ; then
        die "ERROR: it should have failed but didn't"
else
        echo "OK, it failed as it should"
fi

echo $ECHO_N "Appending a seektable metadata block... " $ECHO_C
if run_metaflac_to_metaflac_silent "--list --data-format=binary --except-block-type=STREAMINFO $flacfile2" "--append $flacfile" ; then
        die "ERROR: it should have failed but didn't"
else
        echo "OK, it failed as it should"
fi

run_metaflac --add-seekpoint=0 $flacfile

echo $ECHO_N "Appending a vorbis comment metadata block... " $ECHO_C
if run_metaflac_to_metaflac_silent "--list --data-format=binary --block-type=VORBIS_COMMENT $flacfile2" "--append $flacfile" ; then
        echo "OK"
else
        die "ERROR, couldn't add vorbis comment metadata block"
fi

echo $ECHO_N "Appending another vorbis comment metadata block... " $ECHO_C
if run_metaflac_to_metaflac_silent "--list --data-format=binary --block-type=VORBIS_COMMENT $flacfile2" "--append $flacfile" ; then
        die "ERROR: it should have failed but didn't"
else
        echo "OK, it failed as it should"
fi

if run_metaflac_to_metaflac_silent "--list --data-format=binary --except-block-type=STREAMINFO,SEEKTABLE,VORBIS_COMMENT $flacfile2" "--append $flacfile" ; then
		:
else
        die "ERROR, couldn't add vorbis comment metadata block"
fi

metaflac_test_nofilter case66 "--append" "--list"

if run_metaflac_to_metaflac_silent "--list --data-format=binary --except-block-type=STREAMINFO,SEEKTABLE,VORBIS_COMMENT $flacfile2" "--append --block-number=0 $flacfile" ; then
		:
else
        die "ERROR, couldn't add vorbis comment metadata block"
fi

metaflac_test_nofilter case67 "--append --block-number=0" "--list"

rm -f metaflac-test-files/out.meta  metaflac-test-files/out1.meta

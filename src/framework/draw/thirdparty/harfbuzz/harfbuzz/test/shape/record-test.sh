#!/bin/bash

dir=`mktemp -d`

if which sha1sum 2>/dev/null >/dev/null; then
	SHA1SUM=sha1sum
elif which shasum 2>/dev/null >/dev/null; then
	SHA1SUM='shasum -a 1'
elif which digest 2>/dev/null >/dev/null; then
	SHA1SUM='digest -a sha1'
else
	echo "'sha1sum' not found"
	exit 2
fi

out=/dev/stdout
if test "x$1" == 'x-o'; then
	shift
	out=$1
	shift
fi
hb_subset=$1
shift
hb_shape=$1
shift
fontfile=$1
if test "x${fontfile:0:1}" == 'x-'; then
	echo "Specify font file before other options." >&2
	exit 1
fi
shift
if ! echo "$hb_subset" | grep -q 'subset'; then
	echo "Specify hb-subset (or \"fonttools subset\"): got '$hb_subset'." >&2
	exit 1
fi
if ! echo "$hb_shape" | grep -q 'hb-shape'; then
	echo "Specify hb-shape (not hb-view, etc): got '$hb_shape'." >&2
	exit 1
fi
options=
have_text=false
for arg in "$@"; do
	if test "x${arg:0:1}" == 'x-'; then
		if echo "$arg" | grep -q ' '; then
			echo "Space in argument is not supported: '$arg'." >&2
			exit 1
		fi
		options="$options${options:+ }$arg"
		continue
	fi
	if $have_text; then
		echo "Too many arguments found...  Use '=' notation for options: '$arg'" >&2
		exit 1;
	fi
	text="$arg"
	have_text=true
done
if ! $have_text; then
	text=`cat`
fi
unicodes=`echo "$text" | ./hb-unicode-decode`
glyphs=`echo "$text" | $hb_shape $options "$fontfile"`
if test $? != 0; then
	echo "hb-shape failed." >&2
	exit 2
fi
glyph_ids=`echo "$text" | $hb_shape $options --no-glyph-names --no-clusters --no-positions "$fontfile" | sed 's/[][]//g; s/|/,/g'`

cp "$fontfile" "$dir/font.ttf"
echo $hb_subset \
	--glyph-names \
	--no-hinting \
	--layout-features='*' \
	--gids="$glyph_ids" \
	--text="$text" \
	--output-file="$dir/font.subset.ttf" \
	"$dir/font.ttf"
$hb_subset \
	--glyph-names \
	--no-hinting \
	--layout-features='*' \
	--gids="$glyph_ids" \
	--text="$text" \
	--output-file="$dir/font.subset.ttf" \
	"$dir/font.ttf"
if ! test -s "$dir/font.subset.ttf"; then
	echo "Subsetter didn't produce nonempty subset font in $dir/font.subset.ttf" >&2
	exit 2
fi

# Verify that subset font produces same glyphs!
glyphs_subset=`echo "$text" | $hb_shape $options "$dir/font.subset.ttf"`

if ! test "x$glyphs" = "x$glyphs_subset"; then
	echo "Subset font produced different glyphs!" >&2
	echo "Perhaps font doesn't have glyph names; checking visually..." >&2
	hb_view=${hb_shape/shape/view}
	echo "$text" | $hb_view $options "$dir/font.ttf" --output-format=png --output-file="$dir/orig.png"
	echo "$text" | $hb_view $options "$dir/font.subset.ttf" --output-format=png --output-file="$dir/subset.png"
	if ! cmp "$dir/orig.png" "$dir/subset.png"; then
		echo "Images differ.  Please inspect $dir/*.png." >&2
		echo "$glyphs" >> "$out"
		echo "$glyphs_subset" >> "$out"
		exit 2
	fi
	echo "Yep; all good." >&2
	rm -f "$dir/orig.png"
	rm -f "$dir/subset.png"
	glyphs=$glyphs_subset
fi

sha1sum=`$SHA1SUM "$dir/font.subset.ttf" | cut -d' ' -f1`
subset="data/in-house/fonts/$sha1sum.ttf"
mv "$dir/font.subset.ttf" "$subset"

# There ought to be an easier way to do this, but it escapes me...
unicodes_file=`mktemp`
glyphs_file=`mktemp`
echo "$unicodes" > "$unicodes_file"
echo "$glyphs" > "$glyphs_file"
# Open the "file"s
exec 3<"$unicodes_file"
exec 4<"$glyphs_file"
relative_subset="$subset"
if test "$out" != "/dev/stdout"; then
	relative_subset="$(/usr/bin/env python3 -c 'import os, sys; print (os.path.relpath (sys.argv[1], sys.argv[2]))' "$subset" "$(dirname "$out")")"
fi
while read uline <&3 && read gline <&4; do
	echo "$relative_subset;$options;$uline;$gline" >> "$out"
done


rm -f "$dir/font.ttf"
rmdir "$dir"

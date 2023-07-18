Braille tables for use with [liblouis]
======================================

[liblouis]: ../thirdparty/liblouis

These custom tables provide new capabilities to [liblouis] such as special
handling for text that appears in music.

File                    | Defines Braille dots for...
:-----------------------|:-----------------------------------------------------
ascii-to-unicode.dis    | Conversion from ASCII Braille to Unicode Braille.
ascii-us-patterns.cti   | All possible 8-dot Braille letters.
en-us-symbols.mus       | Musical symbols needed for 6-key Braille input.
fr.mus                  | French letters used in common musical terms.
it.mus                  | Italian letters used in common musical terms.
smufl-symbols.mus       | Symbols in [SMuFL](https://www.smufl.org/).
unicode-to-ascii.dis    | Conversion from Unicode Braille to ASCII Braille.

Names of files in this directory must not match the names of any of
[liblouis' own braille tables](../thirdparty/liblouis/tables) otherwise
they will collide at install time when they get copied into the same
directory.

# Modified Stave Notation (MSN)

MSN style files created with help from the Royal National Institute of Blind People (RNIB).

## Rules for MSN styles

The following rules are useful for any group of related styles, in particular these MSN styles:

1.  XML tags **must** be specified in the order that MuseScore Studio writes them when saving a
    style. Each tag must appear zero or one times per file. No duplicate tags are allowed.

2.  All files in the group **must** contain the same tags. If a tag appears in one file then it
    must appear in all files. Values within tags do not have to be the be the same in each file.

3.  If a particular value is the same in all files, and it also matches MuseScore Studio's default
    value for this tag, then the tag **should** be omitted from all files.

4.  Values that are considered intrinsic to the style (in this case to MSN) **should** be retained
    in the style files even if they do happen to match MuseScore Studio's (current) default values.

These rules enable MSN styles to benefit from future improvements to MuseScore Studio's default
style values, while also ensuring that users can quickly switch between different MSN styles
without leaving any values in place from the previous style.

## Creating or modifying styles

See [Wiki: Working with style files](https://github.com/musescore/MuseScore/wiki/Working-with-style-files).

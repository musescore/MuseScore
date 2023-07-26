% metaflac(1) Version 1.4.3 | Free Lossless Audio Codec metadata tool

# NAME

metaflac - program to list, add, remove, or edit metadata in one or more
FLAC files.

# SYNOPSIS

**metaflac** \[ *options* \] \[ *operations* \] *FLACfile ...*

# DESCRIPTION

Use **metaflac** to list, add, remove, or edit metadata in one or more
FLAC files. You may perform one major operation, or many shorthand
operations at a time.

# GENERAL USAGE

metaflac is the command-line .flac file metadata editor. You can use it
to list the contents of metadata blocks, edit, delete or insert blocks,
and manage padding.

metaflac takes a set of "options" (though some are not optional) and a
set of FLAC files to operate on. There are three kinds of "options":

- Major operations, which specify a mode of operation like listing
  blocks, removing blocks, etc. These will have sub-operations describing
  exactly what is to be done.

- Shorthand operations, which are convenient synonyms for major
  operations. For example, there is a shorthand operation
  --show-sample-rate that shows just the sample rate field from the
  STREAMINFO metadata block.

- Global options, which affect all the operations.

All of these are described in the tables below. At least one shorthand
or major operation must be supplied. You can use multiple shorthand
operations to do more than one thing to a file or set of files. Most of
the common things to do to metadata have shorthand operations. As an
example, here is how to show the MD5 signatures for a set of three FLAC
files:

`metaflac --show-md5sum file1.flac file2.flac file3.flac`

Another example; this removes all DESCRIPTION and COMMENT tags in a set
of FLAC files, and uses the --preserve-modtime global option to keep the
FLAC file modification times the same (usually when files are edited the
modification time is set to the current time):

`metaflac --preserve-modtime --remove-tag=DESCRIPTION --remove-tag=COMMENT file1.flac file2.flac file3.flac`

# OPTIONS

**\--preserve-modtime**  
:	Preserve the original modification time in spite of edits.

**\--with-filename**  
:	Prefix each output line with the FLAC file name (the default if more
	than one FLAC file is specified). This option has no effect for
	options exporting to a file, like --export-tags-to.

**\--no-filename**  
:	Do not prefix each output line with the FLAC file name (the default
	if only one FLAC file is specified).

**\--no-utf8-convert**  
:	Do not convert tags from UTF-8 to local charset, or vice versa. This
	is useful for scripts, and setting tags in situations where the
	locale is wrong.

**\--dont-use-padding**  
:	By default metaflac tries to use padding where possible to avoid
	rewriting the entire file if the metadata size changes. Use this
	option to tell metaflac to not take advantage of padding this way.

# SHORTHAND OPERATIONS

**\--show-md5sum**  
:	Show the MD5 signature from the STREAMINFO block.

**\--show-min-blocksize**  
:	Show the minimum block size from the STREAMINFO block.

**\--show-max-blocksize**  
:	Show the maximum block size from the STREAMINFO block.

**\--show-min-framesize**  
:	Show the minimum frame size from the STREAMINFO block.

**\--show-max-framesize**  
:	Show the maximum frame size from the STREAMINFO block.

**\--show-sample-rate**  
:	Show the sample rate from the STREAMINFO block.

**\--show-channels**  
:	Show the number of channels from the STREAMINFO block.

**\--show-bps**  
:	Show the \# of bits per sample from the STREAMINFO block.

**\--show-total-samples**  
:	Show the total \# of samples from the STREAMINFO block.

**\--show-vendor-tag**  
:	Show the vendor string from the VORBIS_COMMENT block.

**\--show-tag=name**  
:	Show all tags where the field name matches 'name'.

**\--show-all-tags**  
:	Show all tags. This is an alias for --export-tags-to=-.

**\--remove-tag=name**  
:	Remove all tags whose field name is 'name'.

**\--remove-first-tag=name**  
:	Remove first tag whose field name is 'name'.

**\--remove-all-tags**  
:	Remove all tags, leaving only the vendor string.

**\--remove-all-tags-except=NAME1\[=NAME2\[=...\]\]**  
:   Remove all tags, except the vendor string and the tag names
    specified. Tag names must be separated by an = character.

**\--set-tag=field**  
:	Add a tag. The field must comply with the Vorbis comment spec, of the
	form "NAME=VALUE". If there is currently no tag block, one will be
	created.

**\--set-tag-from-file=field**  
:	Like \--set-tag, except the VALUE is a filename whose contents will
	be read verbatim to set the tag value. Unless \--no-utf8-convert is
	specified, the contents will be converted to UTF-8 from the local
	charset. This can be used to store a cuesheet in a tag (e.g.
	\--set-tag-from-file="CUESHEET=image.cue"). Do not try to store
	binary data in tag fields! Use APPLICATION blocks for that.

**\--import-tags-from=file**  
:	Import tags from a file. Use '-' for stdin. Each line should be of
	the form NAME=VALUE. Multi-line comments are currently not supported.
	Specify \--remove-all-tags and/or \--no-utf8-convert before
	\--import-tags-from if necessary. If FILE is '-' (stdin), only one
	FLAC file may be specified.

**\--export-tags-to=file**  
:	Export tags to a file. Use '-' for stdout. Each line will be of the
	form NAME=VALUE. Specify \--no-utf8-convert if necessary.

**\--import-cuesheet-from=file**  
:	Import a cuesheet from a file. Use '-' for stdin. Only one FLAC file
	may be specified. A seekpoint will be added for each index point in
	the cuesheet to the SEEKTABLE unless \--no-cued-seekpoints is
	specified.

**\--export-cuesheet-to=file**  
:	Export CUESHEET block to a cuesheet file, suitable for use by CD
	authoring software. Use '-' for stdout. Only one FLAC file may be
	specified on the command line.

**\--import-picture-from={***FILENAME***\|***SPECIFICATION***}**  
:	Import a picture and store it in a PICTURE metadata block. More than
	one \--import-picture-from command can be specified. Either a filename
	for the picture file or a more complete specification form can be
	used. The SPECIFICATION is a string whose parts are separated by \|
	(pipe) characters. Some parts may be left empty to invoke default
	values. FILENAME is just shorthand for "\|\|\|\|FILENAME". For
	details on the specification, see the section **Picture
	specification** in the **flac(1)** man page.

**\--export-picture-to=file**  
:	Export PICTURE block to a file. Use '-' for stdout. Only one FLAC
	file may be specified on the command line. The first PICTURE block
	will be exported unless \--export-picture-to is preceded by a
	\--block-number=# option to specify the exact metadata block to
	extract. Note that the block number is the one shown by \--list.

**\--add-replay-gain**  
:	Calculates the title and album gains/peaks of the given FLAC files as
	if all the files were part of one album, then stores them as FLAC
	tags. The tags are the same as those used by vorbisgain. Existing
	ReplayGain tags will be replaced. If only one FLAC file is given,
	the album and title gains will be the same. Since this operation
	requires two passes, it is always executed last, after all other
	operations have been completed and written to disk. All FLAC files
	specified must have the same resolution, sample rate, and number of
	channels. Only mono and stereo files are allowed, and the sample
	rate must be 8, 11.025, 12, 16, 18.9, 22.05, 24, 28, 32, 36, 37.8,
	44.1, 48, 56, 64, 72, 75.6, 88.2, 96, 112, 128, 144, 151.2, 176.4,
	192, 224, 256, 288, 302.4, 352.8, 384, 448, 512, 576, or 604.8 kHz.

**\--scan-replay-gain**  
:	Like \--add-replay-gain, but only analyzes the files rather than
	writing them to the tags.

**\--remove-replay-gain**  
:	Removes the ReplayGain tags.

**\--add-seekpoint={***\#***\|***X***\|***\#x***\|***\#s***}**  
:	Add seek points to a SEEKTABLE block. Using \#, a seek point at that
	sample number is added. Using X, a placeholder point is added at the
	end of a the table. Using \#x, \# evenly spaced seek points will be
	added, the first being at sample 0. Using \#s, a seekpoint will be
	added every \# seconds (# does not have to be a whole number; it can
	be, for example, 9.5, meaning a seekpoint every 9.5 seconds). If no
	SEEKTABLE block exists, one will be created. If one already exists,
	points will be added to the existing table, and any duplicates will
	be turned into placeholder points. You may use many \--add-seekpoint
	options; the resulting SEEKTABLE will be the unique-ified union of
	all such values. Example: \--add-seekpoint=100x \--add-seekpoint=3.5s
	will add 100 evenly spaced seekpoints and a seekpoint every 3.5
	seconds.

**\--add-padding=length**  
:	Add a padding block of the given length (in bytes). The overall
	length of the new block will be 4 + length; the extra 4 bytes is for
	the metadata block header.

# MAJOR OPERATIONS

**\--list**  
:	List the contents of one or more metadata blocks to stdout. By
	default, all metadata blocks are listed in text format. Use the
	options **\--block-number**, **\--block-type** or
	**\--except-block-type** to change this behavior.
	
**\--remove**  
:	Remove one or more metadata blocks from the metadata. Use the options
	**\--block-number**, **\--block-type** or **\--except-block-type** 
	to specify which blocks should be removed. Note that if both
	\--block-number and \--[except-]block-type are specified, the result
	is the logical AND of both arguments. Unless \--dont-use-padding
	is specified, the blocks will be replaced with padding. You may not 
	remove the STREAMINFO block. 

**\--block-number=#\[,#\[...\]\]**  
:	An optional comma-separated list of block numbers to display. The
	first block, the STREAMINFO block, is block 0.

**\--block-type=type\[,type\[...\]\]**

**\--except-block-type=type\[,type\[...\]\]**
:	An optional comma-separated list of block types to be included or
	ignored with this option. Use only one of \--block-type or
	\--except-block-type. The valid block types are: STREAMINFO, PADDING,
	APPLICATION, SEEKTABLE, VORBIS_COMMENT, PICTURE. You may narrow down
	the types of APPLICATION blocks selected by appending APPLICATION 
	with a colon and the ID of the APPLICATION block in either ASCII
	or hexadecimal representation. E.g.	APPLICATION:abcd for the
	APPLICATION block(s) whose textual representation of the 4-byte ID
	is "abcd" or APPLICATION:0xXXXXXXXX for the APPLICATION block(s)
	whose hexadecimal big- endian representation of the 4-byte ID
	is "0xXXXXXXXX". For the example "abcd" above the hexadecimal
	equivalalent is 0x61626364

**\--application-data-format=hexdump\|text**  
:	If the application block you are displaying contains binary data but
	your \--data-format=text, you can display a hex dump of the
	application data contents instead using
	\--application-data-format=hexdump.

**\--data-format=binary\|binary-headerless\|text**  
:	For use with --list. By default a human-readable text
	representation of the data is isplayed. You may specify
	--data-format=binary to dump the raw binary form of each metadata
	block. Specify --data-format=binary-headerless to omit output of
	metadata block headers, including the id of APPLICATION metadata
	blocks.

**\--append**  
:	Insert a metadata block from a file. This must be a binary block as
	exported with --list --data-format=binary. The insertion point is
	defined with --block-number=#.  The new block will be added after the
	given block number.  This prevents the illegal insertion of a block
	before the first STREAMINFO block.  You may not --append another
	STREAMINFO block. It is possible to copy a metadata block from one
	file to another with this option. For example use
	`metaflac --list --data-format=binary --block-number=6 file.flac > block`
	to export the block, and then import it with
	`metaflac --append anotherfile.flac < block`

**\--remove-all**  
:	Remove all metadata blocks (except the STREAMINFO block) from the
	metadata. Unless \--dont-use-padding is specified, the blocks will be
	replaced with padding.

**\--merge-padding**  
:	Merge adjacent PADDING blocks into single blocks.

**\--sort-padding**  
:	Move all PADDING blocks to the end of the metadata and merge them
	into a single block.

# SEE ALSO

**flac(1)**

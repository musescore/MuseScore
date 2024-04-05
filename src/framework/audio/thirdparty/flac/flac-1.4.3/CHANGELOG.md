# Changelog

This changelog is not exhaustive, review [the git commit log](https://github.com/xiph/flac/commits) for an exhaustive list of changes.

## FLAC 1.4.3 (23-Jun-2023)

As there have been additions to the libFLAC interfaces, the libFLAC version number is incremented to 13. The libFLAC++ version number stays at 10.

* General
	* All PowerPC-specific code has been removed, as it turned out those improvements didn't actually improve anything
	* Large improvements in encoder speed for all presets. The largest change is for the fastest presets and for 24-bit and 32-bit inputs.
	* Small improvement in decoder speed for BMI2-capable CPUs
	* Various documentation fixes and cleanups (Mark Grassi, Jake Schmidt)
	* Various fixes (Ozkan Sezer, Zhipeng Xue, orbea, Sam James, Harish Mahendrakar)
	* Fix building on Universal Windows Platform (Dmitry Kostjučenko)
* flac
	* A lot of small fixes for bugs found by fuzzing
	* Various improvements to the --keep-foreign-metadata and --keep-foreign-metadata-if-present options on decoding
		* The output format (WAV/AIFF/RF64 etc.) is now automatically selected based on what kind of foreign metadata is stored
		* Decoded file is checked afterwards, to see whether stored foreign format data agrees with FLAC audio properties
		* AIFF-C sowt data can now be restored
	* Add --force-legacy-wave-format option, to decode to WAV with WAVEFORMATPCM where WAVE_FORMAT_EXTENSIBLE would be more appropriate
	* Add --force-aiff-c-none-format and --force-aiff-c-sowt-format to decode to AIFF-C
	* The storage of WAVEFORMATEXTENSIBLE_CHANNEL_MASK is no longer restricted to known channel orderings
	* Throw an error when WAV or AIFF files are over 4GiB in length and the --ignore-chunk-sizes option is not set
	* Warn on testing files when ID3v2 tags are found
	* Warn when data trails the audio data of a WAV/AIFF/RF64/W64 file
	* Fix output file not being deleted after error on Windows
	* Removal of the --sector--align option
* metaflac
	* A lot of small fixes for bugs found by fuzzing
	* Added options --append and --data-format, which makes it possible to copy metadata blocks from one FLAC file to another
	* Added option --remove-all-tags-except
	* Added option --show-all-tags (harridu, Martijn van Beurden)
* libFLAC
	* No longer write seektables to Ogg, even when specifically asked for. Seektables in Ogg are not defined
	* Add functions FLAC__metadata_object_set_raw and FLAC__metadata_object_get_raw to convert between blob and FLAC__StreamMetadata
* Build system
	* Autoconf (configure)
		* The option --enable-64-bit-words is now on by default
	* CMake
		* The option ENABLE_64_BIT_WORDS is now on by default
* Testing/validation
	* Fuzzers were added for the flac and metaflac command line tools
	* Fuzzer coverage was improved

## FLAC 1.4.2 (22-Oct-2022)

Once again, this release only has a few changes. A problem with FLAC playback in GStreamer (and possibly other libFLAC users) was the reason for the short time since the last release

* General
    * Remove xmms plugin (Martijn van Beurden, TokyoBlackHole)
    * Remove all pure assembler, removing build dependency on nasm
    * Made console output more uniform across different platforms and CPUs
    * Improve ability to tune compile for a certain system (for example with -march=native) when combining with --disable-asm-optimizations: plain C functions can now be better optimized
* Build system
    * Default CFLAGS are now prepended instead of dropped when user CFLAGS are set
    * -msse2 is no longer added by default (was only applicable to x86)
    * Fix cross-compiling and out-of-tree building when pandoc and doxygen are not available
    * Fix issue with Clang not compiling functions with intrinsics
    * Fix detection of bswap intrinsics (Ozkan Sezer)
    * Improve search for libssp on MinGW (Ozkan Sezer, Martijn van Beurden)
* libFLAC
    * Fix issue when the libFLAC user seeks in a file instead of libFLAC itself

## FLAC 1.4.1 (22-Sep-2022)

This release only has a few changes. It was triggered by a problem in the 1.4.0 tarball: man pages were empty and api documentation missing

* CMake fixes (Tomasz Kłoczko)
* Add checks that man pages and api docs end up in tarball
* Enable installation of prebuilt man pages and api docs
* Fix compiler warnings (Johannes Kauffmann, Ozkan Sezer)
* Fix format specifier (manxorist)
* Enable building on Universal Windows Platform (Steve Lhomme)
* Fix versioning from git

## FLAC 1.4.0 (09-Sep-2022)

As there have been changes to the library interfaces, the libFLAC version number is incremented to 12, the libFLAC++ version number is incremented to 10. As some changes were breaking, the version age numbers (see [libtool versioning](https://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning)) have been reset to 0. For more details on the changes to the API, see the [porting guide](https://xiph.org/flac/api/group__porting__1__3__4__to__1__4__0.html).

The XMMS plugin and 'common' plugin code (used only by the XMMS plugin) are deprecated, they will be removed in a future release.

* General:
    * It is now possible to limit the minimum bitrate of a FLAC file generated by libFLAC and with the `flac` tool to 1 bit/sample. This function can be used to aid live streaming, for example for internet radio
    * Encoding files with sample rates up to 1'048'575Hz is now possible. (Con Kolivas)
    * Compression of preset -3 through -8 was slightly improved at the cost of a small decrease in encoding speed by increasing the precision with which autocorrelation was calculated (Martijn van Beurden)
    * Encoding speed of preset -0, -1 and -2 was slightly improved
    * Compression of presets -1 and -4 was slighly improved on certain material by changing the adaptive mid-side heuristics
    * Speedups specifically targeting 64-bit ARMv8 devices using NEON were integrated (Ronen Gvili, Martijn van Beurden)
    * Speedups for x86_64 CPUs having the FMA instruction set extention are added
    * Encoding and decoding of 32-bit PCM is now possible
* (Ogg) FLAC format:
    * The FLAC format document is being rewritten by the IETF CELLAR working group. The latest draft can be found on [https://datatracker.ietf.org/doc/draft-ietf-cellar-flac/](https://datatracker.ietf.org/doc/draft-ietf-cellar-flac/)
    * The FLAC format document specifies no bounds for the residual. In other to match current decoder implementations, it is proposed to bound the residual to the range provided by a 32-bit int signed two's complement. This limit must be checked by FLAC encoders as to keep FLAC decoders free from the complexity of being to decode a residual exceeding a 32-bit int.
    * There is now a set of files available to test whether a FLAC decoder implements the format correctly. This FLAC decoder testbench can be found at [https://github.com/ietf-wg-cellar/flac-test-files](https://github.com/ietf-wg-cellar/flac-test-files). Also, results of testing hard- and software can be found here at [https://wiki.hydrogenaud.io/index.php?title=FLAC_decoder_testbench](https://wiki.hydrogenaud.io/index.php?title=FLAC_decoder_testbench).
* flac:
    * The option --limit-min-bitrate was added to aid streaming, see [github #264](https://github.com/xiph/flac/pull/264)
    * The option --keep-foreign-metadata-if-present is added. This option works the same as --keep-foreign-metadata, but does return a warning instead of an error if no foreign metadata was found to store or restore
    * The warning returned by the foreign metadata handling is now clearer in case a user tries to restore foreign metadata of the wrong type, for example decoding a FLAC file containing AIFF foreign metadata to a WAV file
    * A problem when using the analyse function causing the first frame to have a wrong size and offset was fixed
    * Fix bug where channel mask of a file is unintentionally reused when several files are processed with one command
    * The order of compression-related commands is no longer important, i.e. -8ep gives the same result as -ep8. Previously, a compression level (like -8) would override a more specific setting (like -e or -p). This is no longer the case
    * flac now checks the block-align property of WAV files to ensure non-standard WAV files (for which flac has no handling) are not mangled
* metaflac:
    * (none)
* build system:
    * MSVC and Makefile.lite build system files have been removed. Building with MSVC (Visual Studio) can be done by using CMake
    * Various CMake improvements, especially for creating MSVC build files (Martijn van Beurden, martinRenou, CookiePLMonster, David Callu, Tyler Dunn, Cameron Cawley)
    * Various fixes for MinGW (Martijn van Beurden, Cameron Cawley)
    * Removed obsolete autotools macro's to silence warnings
    * Fixes for FreeBSD PowerPC (pkubaj)
    * Fixed some compiler warnings (Martijn van Beurden, Tyler Dunn)
    * Fix building with uclibc (Fabrice Fontaine)
* testing/validation:
    * Addition of new encoder fuzzer, adding fuzzing for 8, 24 and 32-bit inputs
    * Addition of new decoder fuzzer, adding coverage of seeking code
    * Addition of metadata fuzzer, adding coverage of metadata APIs
    * Various improvements to fuzzers to improve code coverage, fuzzing speed and stability
    * Many changes to test suite to improve cross-platform compatibility (Rosen Penev)
    * Windows CI now also builds the whole test suite
    * Clang-format file added (Rosen Penev)
    * Add warning on using v141_xp platform toolset with /MT (Martijn van Beurden, Paul Sanders)
* libraries:
    * Various seeking fixes (Martijn van Beurden, Robert Kausch)
    * Various bugs fixed found by fuzzing
    * On decoding, it is now checked whether residuals can be contained by a 32-bit int, preventing integer overflow
    * Add check that samples supplied to libFLAC actually fall within the bps set
    * Add checks when parsing metadata blocks to not allocate excessive amounts of memory and not overread
    * Undocumented Windows-only utf8 functions are no longer exported to the DLL interface
    * Removed all assembler and intrinsics code from the decoder to improve fuzzing, as they provided only a small speed benefit 
    * The bitwriter buffer is limited in size to 2^24 bytes, so it cannot write excessively large files. This is a backup in case another bug in this area creeps (back) in.
    * The metadata iterations should now never return a vorbiscomment entry with NULL as an entry, now always at least an empty string is returned
* documentation:
    * Removed html documentation and generate man pages from markdown
* Interface changes:
    * libFLAC:
        * Addition of FLAC__stream_encoder_set_limit_min_bitrate() and FLAC__stream_encoder_get_limit_min_bitrate(), see [github #264](https://github.com/xiph/flac/pull/264)
        * get_client_data_from_decoder is renamed FLAC__get_decoder_client_data(), see [github #124](https://github.com/xiph/flac/pull/124)
        * All API functions taking a filename as an argument now take UTF-8 filenames on Windows, and no longer accept filenames using the current codepage
        * FLAC__Frame struct has changed: warmup samples are now stored in FLAC__int64 instead of FLAC__int32 types, and verbatim samples can now be stored in either FLAC__int32 or FLAC__int64 depending on whether samples fix the former or latter
        * The FLAC__StreamMetadata struct now has a tag, so it can be forward declared
    * libFLAC++:
        * Addition of ::set_limit_min_bitrate() and ::get_limit_min_bitrate(), see [github #264](https://github.com/xiph/flac/pull/264)
        * All API functions taking a filename as an argument now take UTF-8 filenames on Windows, and no longer accept filenames using the current codepage
        * The ::FLAC__Frame struct has changed, see the libFLAC interface change.

## FLAC 1.3.4 (20-Feb-2022)

This release mostly fixes (security related) bugs. When building with MSVC, using CMake is preferred, see the README under "Building with CMake" for more information. Building with MSVC using solution files is deprecated and these files will be removed in the future. As there have been no changes to the library interfaces, the libFLAC version number remains 11, and libFLAC++ version number remains 9.

* General:
    * Fix 12 decoder bugs found by oss-fuzz, including CVE-2020-0499 (erikd, Martijn van Beurden)
    * Fix encoder bug CVE-2021-0561 (NeelkamalSemwal)
    * Integrate oss-fuzzers (erikd, Guido Vranken)
    * Seeking fixes (NeelkamalSemwal, Robert Kausch)
    * Various fixes and improvements (Andrei Astafev, Rosen Penev, Håkan Kvist, oreo639, erikd, Tamás Zahola, Ulrik Mikaelsson, Tyler Dunn, tmkk)
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * Various fixes and improvements (Andrei Astafev, Martijn van Beurden)
* metaflac:
    * (none)
* build system:
    * CMake improvements (evpobr, Vitaliy Kirsanov, erikd, Ozkan Sezer, Tyler Dunn, tg-m DeadSix27, ericLemanissier, Chocobo1).
    * Fixes for MinGW and MSVC (Ozkan Sezer).
    * Fix for clang (Ozkan Sezer)
    * Fix for PowerPC (Peter Seiderer, Thomas BERNARD)
    * Fix for FreeBSD PowerPC (pkubaj).
* testing/validation:
    * Add Windows target to CI, improve logging (Ralph Giles)
    * CI improvements (Ralph Giles, Ewout ter Hoeven)
* documentation:
    * Doxygen fixes (Tyler Dunn)
    * Fix typos (Tim Gates, maxz)
* Interface changes:
    * libFLAC:
        * (none)
    * libFLAC++:
        * (none)

## FLAC 1.3.3 (4-Augs-2019)  

* General:
    * Fix CPU detection (Janne Hyvärinen).
    * Switch from unsigned types to uint32_t (erikd).
    * CppCheck fixes (erikd).
    * Improve SIMD decoding of 24 bit files (lvqcl).
    * POWER* amnd POWER9 improvements (Anton Blanchard).
    * More tests.
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * When converting to WAV, use WAVEFORMATEXTENSIBLE when bits per second is not 8 or 16 (erikd).
    * Fix --output-prefix with input-files in sub-directories (orbea).
* metaflac:
    * (none)
* plugins:
    * (none)
* build system:
    * Cmake support (Vitaliy Kirsanov, evpobr).
    * Visual Studio updates (Janne Hyvärinen).
    * Fix for MSVC when UNICODE is enabled (lvqcl).
    * Fix for OpenBSD/i386 (Christian Weisgerber).
* documentation:
    * (none)
* libraries:
    * (none).
* Interface changes:
    * libFLAC:
        * (none)
    * libFLAC++:
        * (none)

## FLAC 1.3.2 (01-Jan-2017)  

* General:
    * Fix undefined behaviour using GCC/Clang UBSAN (erikd).
    * General hardening via fuzz testing with AFL (erikd and others).
    * General code improvements (lvqcl, erikd and others).
    * Add FLAC in MP4 specification docs (Ralph Giles).
    * MSVS build cleanups (lvqcl).
    * Fix some cppcheck warnings (erikd).
    * Assume all currently used OSes support SSE2.
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * Fix potential infinite loop on flac-to-flac conversion (erikd).
    * Add WAVEFORMATEXTENSIBLE to WAV (as needed) when decoding (lvqcl).
    * Only write vorbis-comments if they are non-empty.
    * Error out if decoding RAW with bits != (8|16|24).
* metaflac:
    * Add --scan-replay-gain option.
* plugins:
    * (none)
* build system:
    * Fixes for MSVC and Makefile.lite build systems.
* documentation:
    * (none)
* libraries:
    * CPU detection cleanup and fixes (Julian Calaby, erikd and lvqcl).
    * Fix two stream decoder bugs (Max Kellermann).
    * Fix a NULL dereference bug (on a malformed file).
    * Changed the LPC order guess for a slight compression improvement, particularly for classical music (Martijn van Beurden).
    * Improved encoding speed on older Intel CPUs.
    * Fixed a seeking bug when decoding certain files (Miroslav Lichvar).
    * Put an upper bound (32768) on the number of seek points.
    * Fix potential memory leaks.
    * Support 64bit brword/bwword allowing FLAC__BYTES_PER_WORD to be set to 8 (disabled by default).
    * Fix an out-of-bounds heap read.
    * Win32: Only use large buffers when writing to disk.
* Interface changes:
    * libFLAC:
        * (none)
    * libFLAC++:
        * (none)

## FLAC 1.3.1 (25-Nov-2014)  

* General:
    * Improved decoding efficiency of all bit depths but especially so for 24 bits for IA32 architecture (lvqcl and Miroslav Lichvar).
    * Faster encoding using SSE and AVX (lvqcl).
    * Fixed bartlett, bartlett_hann and triangle functions.
    * New apodization functions partial_tukey and punchout_tukey for improved compression (Martijn van Beurden).
    * Retuned compression presets to incorporate new apodization functions (Martijn van Beurden).
    * Fix -Wcast-align warnings on armhf architecture (Erik de Castro Lopo).
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * Help output documentation improvements.
    * I/O buffering improvements on Windows to reduce disk fragmentation when writing files.
    * Only write vorbis-comments if they are non-empty.
* metaflac:
    * (none)
* plugins:
    * Fix symbol visibility in XMMS plugin.
* build system:
    * Many fixes and improvements across all the build systems.
* documentation:
    * Document new [apodization windows](https://xiph.org/flac/documentation_tools_flac.html#flac_options_apodization).
* libraries:
    * Fix CVE-2014-9028 (heap write overflow) and CVE-2014-8962 (heap read overflow) (Erik de Castro Lopo).
* Interface changes:
    * libFLAC:
        * (none)
    * libFLAC++:
        * (none)

## FLAC 1.3.0 (26-May-2013)  

* General:
    * Move development to Xiph.org git repository.
    * The <span class="argument">[--sector-align](https://xiph.org/flac/documentation_tools_flac.html#flac_options_sector_align)</span> option of <span class="commandname">flac</span> has been deprecated and may not exist in future versions. [shntool](http://www.etree.org/shnutils/shntool/) provides similar functionality.
    * Support for the RF64 and Wave64 formats in <span class="commandname">flac</span> (see below).
    * Better handling of cuesheets with non-CD-DA sample rates.
    * The <span class="argument">[--ignore-chunk-sizes](https://xiph.org/flac/documentation_tools_flac.html#flac_options_ignore_chunk_sizes)</span> option has been added to the <span class="commandname">flac</span> command line tool.
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * Added support for encoding from and decoding to the RF64 format, and a new corresponding option <span class="argument">[--force-rf64-format](https://xiph.org/flac/documentation_tools_flac.html#flac_options_force_rf64_format)</span>. ([SF #1762502](http://sourceforge.net/p/flac/feature-requests/78/)). <span class="argument">[--keep-foreign-metadata](https://xiph.org/flac/documentation_tools_flac.html#flac_options_keep_foreign_metadata)</span> is also supported.
    * Added support for encoding from and decoding to the Sony Wave64 format, and a new corresponding option <span class="argument">[--force-wave64-format](https://xiph.org/flac/documentation_tools_flac.html#flac_options_force_wave64_format)</span>. ([SF #1769582](http://sourceforge.net/p/flac/feature-requests/79/)). <span class="argument">[--keep-foreign-metadata](https://xiph.org/flac/documentation_tools_flac.html#flac_options_keep_foreign_metadata)</span> is also supported.
    * Added new options <span class="argument">[--preserve-modtime](https://xiph.org/flac/documentation_tools_flac.html#flac_options_preserve_modtime)</span> and <span class="argument">[--no-preserve-modtime](https://xiph.org/flac/documentation_tools_flac.html#negative_options)</span> to specify whether or not output files should copy the timestamp and permissions from their input files. The default is <span class="argument">[--preserve-modtime](https://xiph.org/flac/documentation_tools_flac.html#flac_options_preserve_modtime)</span> as in previous versions. ([SF #1805428](http://sourceforge.net/p/flac/feature-requests/85/)).
    * Allow MM:SS:FF and MM:SS.SS time formats in non-CD-DA cuesheets. ([SF #1947353](http://sourceforge.net/p/flac/feature-requests/95/), [SF #2182432](http://sourceforge.net/p/flac/bugs/338/))
    * The <span class="argument">[--sector-align](https://xiph.org/flac/documentation_tools_flac.html#flac_options_sector_align)</span> option of <span class="commandname">flac</span> has been deprecated and may not exist in future versions. [shntool](http://www.etree.org/shnutils/shntool/) provides similar functionality. ([SF #1805946](http://sourceforge.net/p/flac/feature-requests/86/))
    * Improved error message when user attempts to decode a non-FLAC file ([SF #2222789](http://sourceforge.net/p/flac/bugs/341/)).
    * Fix bug where <span class="commandname">flac</span> was disallowing use of <span class="argument">--replay-gain</span> when encoding from stdin ([SF #1840124](http://sourceforge.net/p/flac/bugs/313/)).
    * Fix bug with fractional seconds on some locales ([SF #1815517](http://sourceforge.net/p/flac/bugs/309/), [SF #1858012](http://sourceforge.net/p/flac/bugs/321/)).
    * Read and write appropriate channel masks for 6.1 and 7.1 surround input WAV files. Documentation was also updated.
    * Correct Wave64 GUIDs.
    * Support 56kHz to 192kHz gain analysis (patch from Earl Chew)
    * Add ability to handle unicode filenames on Windows (large set of patches from Janne Hyvärinen)
* metaflac:
    * Allow MM:SS:FF and MM:SS.SS time formats in non-CD-DA cuesheets. ([SF #1947353](http://sourceforge.net/p/flac/feature-requests/95/), [SF #2182432](http://sourceforge.net/p/flac/bugs/338/))
* plugins:
    * Minor updates for XMMS plugin.
    * Winamp2 plugin was dropped because Nullsoft has provided native FLAC support since 2006.
* build system:
    * Fixes for autotools (including [SF #1859664](http://sourceforge.net/p/flac/patches/28/)).
    * Fixes for MinGW (including [SF #2000973](http://sourceforge.net/p/flac/bugs/), [SF #2209829](http://sourceforge.net/p/flac/bugs/)).
    * Fixes for gcc (including [SF #1834168](http://sourceforge.net/p/flac/bugs/), [SF #2002481](http://sourceforge.net/p/flac/bugs/334/)).
    * Fixes for Sun Studio/Forte ([SF #1701960](http://sourceforge.net/p/flac/patches/22/)).
    * Fixes for windows builds (including [SF #1676822](http://sourceforge.net/p/flac/bugs/257/), [SF #1756624](http://sourceforge.net/p/flac/feature-requests/73/), [SF #1809863](http://sourceforge.net/p/flac/bugs/307/), [SF #1911149](http://sourceforge.net/p/flac/feature-requests/)).
    * Fixes for FreeBSD and OpenBSD.
    * Compile with GNU gcc _FORTIFY_SOURCE=2 and stack protection where those features are detected.
    * Enable a bunch of GCC compiler warnings and fix code that generates warnings.
* documentation:
    * Document <span class="argument">[--ignore-chunk-sizes](https://xiph.org/flac/documentation_tools_flac.html#flac_options_ignore_chunk_sizes)</span> and <span class="argument">[--apply-replaygain-which-is-not-lossless](https://xiph.org/flac/documentation_tools_flac.html#flac_options_apply_replaygain_which_is_not_lossless)</span> option for <span class="commandname">flac</span>.
* libraries:
    * libFLAC encoder was defaulting to level 0 compression instead of 5 ([SF #1816825](http://sourceforge.net/p/flac/bugs/310/)).
    * Fix bug in bitreader handling of read callback returning a short count ([SF #2490454](http://sourceforge.net/p/flac/bugs/345/)).
    * Improve decoder's ability to distinguish between a FLAC sync code and an MPEG one ([SF #2491433](http://sourceforge.net/p/flac/bugs/346/)).
* Interface changes:
    * libFLAC:
        * **Added** FLAC__format_blocksize_is_subset()
    * libFLAC++:
        * Add a number of convenience methods.

## FLAC 1.2.1 (17-Sep-2007)  

* General:
    * With the new <span class="argument">[--keep-foreign-metadata](https://xiph.org/flac/documentation_tools_flac.html#flac_options_keep_foreign_metadata)</span> in <span class="commandname">flac</span>, non-audio RIFF and AIFF chunks can be stored in FLAC files and recreated when decoding. This allows, among other, things support for archiving BWF files and other WAVE files from editing tools that preserves all the metadata.
* FLAC format:
    * Specified 2 new APPLICATION metadata blocks for storing WAVE and AIFF chunks (for use with [--keep-foreign-metadata](https://xiph.org/flac/documentation_tools_flac.html#flac_options_keep_foreign_metadata) in <span class="commandname">flac</span>).
    * The lead-out track number for non-CDDA cuesheets now must be 255.
* Ogg FLAC format:
    * This is not a format change, but changed default extension for Ogg FLAC from .ogg to .oga, according to new Xiph [specification](http://wiki.xiph.org/index.php/MIME_Types_and_File_Extensions) ([SF #1762492](http://sourceforge.net/p/flac/bugs/283/)).
* flac:
    * Added a new option <span class="argument">[--no-utf8-convert](https://xiph.org/flac/documentation_tools_flac.html#flac_options_no_utf8_convert)</span> which works like it does in <span class="commandname">metaflac</span> ([SF #973740](http://sourceforge.net/p/flac/feature-requests/35/)).
    * Added a new option <span class="argument">[--keep-foreign-metadata](https://xiph.org/flac/documentation_tools_flac.html#flac_options_keep_foreign_metadata)</span> which can save/restore RIFF and AIFF chunks to/from FLAC files ([SF #363478](http://sourceforge.net/p/flac/feature-requests/9/)).
    * Changed default extension for Ogg FLAC from .ogg to .oga, according to new Xiph [specification](http://wiki.xiph.org/index.php/MIME_Types_and_File_Extensions) ([SF #1762492](http://sourceforge.net/p/flac/bugs/283/)).
    * Fixed bug where using <span class="argument">--replay-gain</span> without any padding option caused only a small PADDING block to be created ([SF #1760790](http://sourceforge.net/p/flac/bugs/282/)).
    * Fixed bug where encoding from stdin on Windows could fail if WAVE/AIFF contained unknown chunks ([SF #1776803](http://sourceforge.net/p/flac/bugs/290/)).
    * Fixed bug where importing non-CDDA cuesheets would cause an invalid lead-out track number ([SF #1764105](http://sourceforge.net/p/flac/bugs/286/)).
* metaflac:
    * Changed default extension for Ogg FLAC from .ogg to .oga, according to new Xiph [specification](http://wiki.xiph.org/index.php/MIME_Types_and_File_Extensions) ([SF #1762492](http://sourceforge.net/p/flac/bugs/283/)).
    * Fixed bug where importing non-CDDA cuesheets would cause an invalid lead-out track number ([SF #1764105](http://sourceforge.net/p/flac/bugs/286/)).
* plugins:
    * (none)
* build system:
    * New configure option <span class="argument">--disable-cpplibs</span> to prevent building libFLAC++ ([SF #1723295](http://sourceforge.net/p/flac/patches/23/)).
    * Fixed bug compiling <span class="commandname">flac</span> without Ogg support ([SF #1760786](http://sourceforge.net/p/flac/bugs/281/)).
    * Fixed bug where sometimes an existing installation of flac could interfere with the build process ([SF #1763690](http://sourceforge.net/p/flac/bugs/285/)).
    * OS X fixes ([SF #1786225](http://sourceforge.net/p/flac/patches/25/)).
    * MinGW fixes ([SF #1684879](http://sourceforge.net/p/flac/bugs/264/)).
    * Solaris 10 fixes ([SF #1783225](http://sourceforge.net/p/flac/bugs/294/) [SF #1783630](http://sourceforge.net/p/flac/bugs/295/)).
    * OS/2 fixes ([SF #1771378](http://sourceforge.net/p/flac/bugs/287/) [SF #1229495](http://sourceforge.net/p/flac/bugs/174/)).
    * automake-1.10 fixes ([SF #1791361](http://sourceforge.net/p/flac/bugs/300/) [SF #1792179](http://sourceforge.net/p/flac/bugs/302/)).
* documentation:
    * Added new [tutorial](https://xiph.org/flac/documentation_tools_flac.html#tutorial) section for <span class="commandname">flac</span>.
    * Added [example code](https://xiph.org/flac/documentation_example_code.html) section for using libFLAC/libFLAC++.
* libraries:
    * libFLAC: Fixed very rare seek bug ([SF #1684049](http://sourceforge.net/p/flac/bugs/263/)).
    * libFLAC: Fixed seek bug with Ogg FLAC and small streams ([SF #1792172](http://sourceforge.net/p/flac/bugs/301/)).
    * libFLAC: 64-bit fixes ([SF #1790872](http://sourceforge.net/p/flac/bugs/299/)).
    * libFLAC: Fix assembler code to be position independent.
    * libFLAC: Optimization of a number of inner loop functions.
    * Added support for encoding the residual coding method introduced in libFLAC 1.2.0 (RESIDUAL_CODING_METHOD_PARTITIONED_RICE2) which will encode 24-bit files more efficiently.
* Interface changes:
    * libFLAC:
        * **Added** FLAC__metadata_simple_iterator_is_last()
        * **Added** FLAC__metadata_simple_iterator_get_block_offset()
        * **Added** FLAC__metadata_simple_iterator_get_block_length()
        * **Added** FLAC__metadata_simple_iterator_get_application_id()
    * libFLAC++:
        * **Added** FLAC::Metadata::SimpleIterator::is_last()
        * **Added** FLAC::Metadata::SimpleIterator::get_block_offset()
        * **Added** FLAC::Metadata::SimpleIterator::get_block_length()
        * **Added** FLAC::Metadata::SimpleIterator::get_application_id()

## FLAC 1.2.0 (23-Jul-2007)  

* General:
    * Small encoding speedups for all modes.
* FLAC format:
    * One of the reserved bits in the FLAC frame header has been assigned for future use; make sure to refer to the [porting guide](https://xiph.org/flac/api/group__porting__1__1__4__to__1__2__0.html) if you parse FLAC streams manually.
* Ogg FLAC format:
    * (none)
* flac:
    * Added runtime detection of SSE OS support for most operating systems.
    * Added a new undocumented option <span class="argument">--ignore-chunk-sizes</span> for ignoring the size of the 'data' chunk (WAVE) or 'SSND' chunk (AIFF). Can be used to encode files with bogus data sizes (e.g. with WAV files piped from foobar2000 to flac.exe as an external encoder). **Use with caution**: all subsequent data is treated as audio, so the data/SSND chunk must be the last or the following data/tags will be treated as audio and encoded.
* metaflac:
    * (none)
* plugins:
    * (none)
* build system:
    * Added solution and project files for building with VC++ 2005.
* libraries:
    * Added runtime detection of SSE OS support for most operating systems.
    * Fixed bug where invalid seek tables could cause some seeks to fail.
    * Added support for decoding the new residual coding method (RESIDUAL_CODING_METHOD_PARTITIONED_RICE2).
* Interface changes (see also the [porting guide](https://xiph.org/flac/api/group__porting__1__1__4__to__1__2__0.html) for specific instructions on porting to FLAC 1.2.0):
    * libFLAC:
        * **Added** FLAC__format_sample_rate_is_subset()
    * libFLAC++:
        * **Added** FLAC::Decoder::Stream::get_decode_position()

## FLAC 1.1.4 (13-Feb-2007)  

* General:
    * Improved compression with no change to format or decrease in speed.
    * Encoding and decoding speedups for all modes. Encoding at -8 is twice as fast.
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * Improved compression with no change to format or decrease in speed.
    * Encoding and decoding speedups for all modes. Encoding at -8 is twice as fast.
    * Added a new option <span class="argument">[-w,--warnings-as-errors](https://xiph.org/flac/documentation_tools_flac.html#flac_options_warnings_as_errors)</span> for treating all warnings as errors.
    * Allow <span class="argument">[--picture](https://xiph.org/flac/documentation_tools_flac.html#flac_options_picture)</span> option to take only a filename, and have all other attributes extracted from the file itself.
    * Fixed a bug that caused suboptimal default compression settings in some locales ([SF #1608883](http://sourceforge.net/p/flac/bugs/237/)).
    * Fixed a bug where FLAC-to-FLAC transcoding of a corrupted FLAC file would truncate the transcoded file at the first error ([SF #1615019](http://sourceforge.net/p/flac/bugs/241/)).
    * Fixed a bug where using <span class="argument">[-F](https://xiph.org/flac/documentation_tools_flac.html#flac_options_decode_through_errors)</span> with FLAC-to-FLAC transcoding of a corrupted FLAC would have no effect ([SF #1615391](http://sourceforge.net/p/flac/bugs/242/)).
    * Fixed a bug where new PICTURE metadata blocks specified with <span class="argument">[--picture](https://xiph.org/flac/documentation_tools_flac.html#flac_options_picture)</span> would not be transferred during FLAC-to-FLAC transcoding ([SF #1627993](http://sourceforge.net/p/flac/bugs/246/)).
* metaflac:
    * Allow <span class="argument">[--import-picture-from](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_import_picture_from)</span> option to take only a filename, and have all other attributes extracted from the file itself.
* plugins:
    * Fixed a bug in the XMMS plugin where Ctrl-3 (file info) would cause a crash if the file did not exist ([SF #1634941](http://sourceforge.net/p/flac/patches/20/)).
* build system:
    * Fixed a makefile linkage bug with libogg ([SF #1611414](http://sourceforge.net/p/flac/bugs/239/)).
    * Added pkg-config files for libFLAC and libFLAC++ ([SF #1647881](http://sourceforge.net/p/flac/patches/21/)).
    * Added <span class="argument">--disable-ogg</span> option for building without Ogg support even if libogg is installed ([SF #1196996](http://sourceforge.net/p/flac/bugs/165/)).
* libraries:
    * Completely rewritten bitbuffer which uses native machine word size instead of bytes for dramatic speed improvements. The speedup should be most dramatic on CPUs with slower byte manipulation capability and big-endian machines.
    * Much faster Rice partition size estimation which greatly speeds encoding in higher modes.
    * Increased compression for all modes.
    * Reduced memory requirements for encoder and decoder.
    * Fixed a bug with default apodization settings that were erroneous in some locales ([SF #1608883](http://sourceforge.net/p/flac/bugs/237/)).
* Interface changes:
    * libFLAC:
        * (behavior only) FLAC__stream_encoder_set_metadata() now makes a copy of the "metadata" array of pointers (but still not copies of the objects themselves) so the client does not need to maintain its copy of the array after the call.
    * libFLAC++:
        * (none)

## FLAC 1.1.3 (27-Nov-2006)  

* General:
    * Improved compression with no impact on format or decoding speed.
    * Much better recovery for corrupted files
    * Better multichannel support
    * Large file (>2GB) support everywhere
    * <span class="commandname">flac</span> now supports FLAC and Ogg FLAC as input to the encoder (e.g. can re-encode FLAC to FLAC) and preserve all the metadata like tags, etc.
    * New <span class="code">[PICTURE](https://xiph.org/flac/format.html#def_PICTURE)</span> metadata block for storing things like cover art, new <span class="argument">[--picture](https://xiph.org/flac/documentation_tools_flac.html#flac_options_picture)</span> option to <span class="commandname">flac</span> and <span class="argument">[--import-picture-from](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_import_picture_from)</span> option to <span class="commandname">metaflac</span> for importing pictures, new <span class="argument">[--export-picture-to](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_export_picture_to)</span> option to <span class="commandname">metaflac</span> for exporting pictures, and metadata API [additions](https://xiph.org/flac/api/group__flac__metadata__level0.html#ga3) for searching for suitable pictures based on type, size and color constraints.
    * Support for new <tt>REPLAYGAIN_REFERENCE_LOUDNESS</tt> tag.
    * Fixed a bug in Ogg FLAC encoding where metadata was not being updated properly. Existing Ogg FLAC files should be recoded to fix up the metadata, e.g. <span class="command">flac -Vf -S 10s --ogg file.ogg</span>
    * In the developer libraries, the interface has been simplfied by merging the three decoding layers into a single class; ditto for the encoders. Also, libOggFLAC has been merged into libFLAC and libOggFLAC++ has been merged into libFLAC++ so there is a single API supporting both native FLAC and Ogg FLAC.
* FLAC format:
    * New <span class="code">[PICTURE](https://xiph.org/flac/format.html#def_PICTURE)</span> metadata block for storing things like cover art.
    * Speaker assignments and channel orders for 3-6 channels (see [frame header](https://xiph.org/flac/format.html#frame_header)).
    * Further restrictions on the [FLAC subset](https://xiph.org/flac/format.html#subset) when the sample rate is <=48kHz; in this case the maximum LPC order is now 12 and maximum blocksize is 4608\. This is to further limit the processing and memory requirements for hardware implementations while not measurably affecting compression.
* Ogg FLAC format:
    * (none)
* flac:
    * Improved the <span class="argument">[-F](https://xiph.org/flac/documentation_tools_flac.html#flac_options_decode_through_errors)</span> option to allow decoding of FLAC files whose metadata is corrupted, and other kinds of severe corruption.
    * Encoder can now take FLAC and Ogg FLAC as input. The output FLAC file will have all the same metadata as the original unless overridden with options on the command line.
    * Encoder can now take WAVEFORMATEXTENSIBLE WAVE files as input; decoder will output WAVEFORMATEXTENSIBLE WAVE files when necessary to conform to the latest Microsoft specifications.
    * Now properly supports AIFF and WAVEFORMATEXTENSIBLE multichannel input, performing necessary channel reordering both for encoding and decoding. WAVEFORMATEXTENSIBLE channel mask is also saved to a tag on encoding and restored on decoding for situations when there is no natural mapping to FLAC channel assignments.
    * Expanded support for "odd" sample resolutions to WAVE and AIFF input; all resolutions from 4 to 24 bits-per-sample now supported for all input types.
    * Added a new option <span class="argument">[--tag-from-file](https://xiph.org/flac/documentation_tools_flac.html#flac_options_tag_from_file)</span> for setting a tag from file (e.g. for importing a cuesheet as a tag).
    * Added a new option <span class="argument">[--picture](https://xiph.org/flac/documentation_tools_flac.html#flac_options_picture)</span> for adding pictures.
    * Added a new option <span class="argument">[--apodization](https://xiph.org/flac/documentation_tools_flac.html#flac_options_apodization)</span> for specifying the window function(s) to be used in LPC analysis.
    * Added support for encoding from non-compressed AIFF-C ([SF #1090933](http://sourceforge.net/p/flac/bugs/143/)).
    * Importing of non-CDDA-compliant cuesheets now only issues a warning, not an error (see [here](http://www.hydrogenaud.io/forums/index.php?showtopic=31282)).
    * MD5 comparison failures on decoding are now an error instead of a warning and will also return a non-zero exit code ([SF #1493725](http://sourceforge.net/p/flac/bugs/221/)).
    * The default padding size is now 8K, or 64K if the input audio stream is more than 20 minutes long.
    * Fixed a bug in cuesheet parsing where it would return an error if the last line of the cuesheet did not end with a newline.
    * Fixed a bug that caused a crash when <span class="argument">-a</span> and <span class="argument">-t</span> were used together ([SF #1229481](http://sourceforge.net/p/flac/bugs/173/)).
    * Fixed a bug with --sector-align where appended samples were not always totally silent ([SF #1237707](http://sourceforge.net/p/flac/bugs/179/)).
    * Fixed bugs with --sector-align and raw input files.
    * Fixed a bug printing out unknown AIFF subchunk names ([SF #1267476](http://sourceforge.net/p/flac/bugs/186/)).
    * Fixed a bug where WAVE files with "data" subchunks of size 0 where accepted ([SF #1293830](http://sourceforge.net/p/flac/bugs/190/)).
    * Fixed a bug where sync error at end-of-stream of truncated files was not being caught ([SF #1244071](http://sourceforge.net/p/flac/bugs/183/)).
    * Fixed a problem with filename parsing if file does not have extension but also has a . in the path ([SF #1161916](http://sourceforge.net/p/flac/bugs/159/)).
    * Fixed a problem with fractional-second parsing for <span class="argument">--skip</span>/<span class="argument">--until</span> in some locales ([SF #1031043](http://sourceforge.net/p/flac/bugs/125/)).
    * Increase progress report rate when -p and -e are used together ([SF #1580122](http://sourceforge.net/p/flac/bugs/229/)).
* metaflac:
    * Added support for read-only operations on Ogg FLAC files.
    * Added a new option <span class="argument">[--set-tag-from-file](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_set_tag_from_file)</span> for setting a tag from file (e.g. for importing a cuesheet as a tag).
    * Added a new option <span class="argument">[--import-picture-from](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_import_picture_from)</span> for importing pictures.
    * Added a new option <span class="argument">[--export-picture-to](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_export_picture_to)</span> for exporting pictures.
    * Added shorthand operation <span class="argument">[--remove-replay-gain](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_remove_replay_gain)</span> for removing ReplayGain tags.
    * <span class="argument">[--export-cuesheet-to](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac_shorthand_export_cuesheet_to)</span> now properly specifies the FLAC file name ([SF #1272825](http://sourceforge.net/p/flac/feature-requests/46/)).
    * Importing of non-CDDA-compliant cuesheets now issues a warning.
    * Removed the following deprecated tag editing options; you should use the new option names shown instead:
        * Removed <span class="argument">--show-vc-vendor</span>; use <span class="argument">--show-vendor-tag</span>
        * Removed <span class="argument">--show-vc-field</span>; use <span class="argument">--show-tag</span>
        * Removed <span class="argument">--remove-vc-all</span>; use <span class="argument">--remove-all-tags</span>
        * Removed <span class="argument">--remove-vc-field</span>; use <span class="argument">--remove-tag</span>
        * Removed <span class="argument">--remove-vc-firstfield</span>; use <span class="argument">--remove-first-tag</span>
        * Removed <span class="argument">--set-vc-field</span>; use <span class="argument">--set-tag</span>
        * Removed <span class="argument">--import-vc-from</span>; use <span class="argument">--import-tags-from</span>
        * Removed <span class="argument">--export-vc-to</span>; use <span class="argument">--export-tags-to</span>
    * Disallow multiple input FLAC files when --import-tags-from=- is used ([SF #1082577](http://sourceforge.net/p/flac/bugs/141/)).
* plugins:
    * When ReplayGain is on, if tags for the preferred kind of gain (album/track) are not in a stream, the other kind will be used.
    * Added ReplayGain info to file info box in XMMS plugin
    * Fixed UTF-8 decoder to disallow non-shortest-form and surrogate sequences (see [here](http://www.unicode.org/versions/corrigendum1.html)).
* build system:
    * Added support for building on OS/2 with EMX ([SF #1229495](http://sourceforge.net/p/flac/bugs/174/))
    * Added support for building with Borland C++ ([SF #1599018](http://sourceforge.net/p/flac/patches/17/))
    * Added a <span class="argument">--disable-xmms-plugin</span> option to <span class="command">configure</span> to prevent building the XMMS plugin ([SF #930494](http://sourceforge.net/p/flac/feature-requests/33/)).
    * Added a <span class="argument">--disable-doxygen-docs</span> option to <span class="command">configure</span> for disabling Doxygen-based API doc generation ([SF #1365935](http://sourceforge.net/p/flac/patches/12/)).
    * Added a <span class="argument">--disable-thorough-tests</span> option to <span class="command">configure</span> to do the basic library, stream, and tool tests in a reasonable time ([SF #1077948](http://sourceforge.net/p/flac/feature-requests/40/)).
    * Added large file support with <span class="argument">AC_SYS_LARGEFILE</span>; use <span class="argument">--disable-largefile</span> with <span class="command">configure</span> to disable.
* libraries:
    * Merged libOggFLAC into libFLAC; both formats are now supported through the same API.
    * Merged libOggFLAC++ into libFLAC++; both formats are now supported through the same API.
    * libFLAC and libFLAC++: Simplified encoder setup with new <span class="argument">FLAC__stream_encoder_set_compression_level()</span> function.
    * libFLAC: Improved compression with no impact on FLAC format or decoding time by adding a windowing stage before LPC analysis.
    * libFLAC: Fixed a bug where missing STREAMINFO fields (min/max framesize, total samples, MD5 sum) and seek point offsets were not getting rewritten back to Ogg FLAC file ([SF #1338969](http://sourceforge.net/p/flac/bugs/197/)).
    * libFLAC: Fixed a bug in cuesheet parsing where it would return an error if the last line of the cuesheet did not end with a newline.
    * libFLAC: Fixed UTF-8 decoder to disallow non-shortest-form and surrogate sequences (see [here](http://www.unicode.org/versions/corrigendum1.html)).
    * libFLAC: Fixed a bug in the return value for <span class="argument">FLAC__stream_decoder_set_metadata_respond_application()</span> and <span class="argument">FLAC__stream_decoder_set_metadata_ignore_application()</span> when there was a memory allocation error ([SF #1235005](http://sourceforge.net/p/flac/bugs/176/)).
* Interface changes (see also the [porting guide](https://xiph.org/flac/api/group__porting__1__1__2__to__1__1__3.html) for specific instructions on porting to FLAC 1.1.3):
    * all libraries;
        * Merged libOggFLAC into libFLAC; both formats are now supported through the same API.
        * Merged libOggFLAC++ into libFLAC++; both formats are now supported through the same API.
        * Merged seekable stream decoder and file decoder into the stream decoder.
        * Merged seekable stream encoder and file encoder into the stream encoder.
        * Added #defines for the API version number to make porting easier; see <tt>include/lib*FLAC*/export.h</tt>.
    * libFLAC:
        * **Added** FLAC__stream_encoder_set_apodization()
        * **Added** FLAC__stream_encoder_set_compression_level()
        * **Added** FLAC__metadata_object_cuesheet_calculate_cddb_id()
        * **Added** FLAC__metadata_get_cuesheet()
        * **Added** FLAC__metadata_get_picture()
        * **Added** FLAC__metadata_chain_read_ogg() and FLAC__metadata_chain_read_ogg_with_callbacks()
        * **Changed** FLAC__stream_encoder_finish() now returns a FLAC__bool to signal a verify failure, or error processing last frame or updating metadata.
        * **Changed** FLAC__StreamDecoderState: removed state FLAC__STREAM_DECODER_UNPARSEABLE_STREAM
        * **Changed** FLAC__StreamDecoderErrorStatus: new error code FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM
        * The above two changes mean that when the decoder encounters what it thinks are unparseable frames from a future decoder, instead of returning a fatal error with the FLAC__STREAM_DECODER_UNPARSEABLE_STREAM state, it just calls the error callback with FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM and leaves the behavior up to the application.
    * libFLAC++:
        * **Added** FLAC::Metadata::Picture
        * **Added** FLAC::Encoder::Stream::set_apodization()
        * **Added** FLAC::Encoder::Stream::set_compression_level()
        * **Added** FLAC::Metadata::CueSheet::calculate_cddb_id()
        * **Added** FLAC::Metadata::get_cuesheet()
        * **Added** FLAC::Metadata::get_picture()
        * **Changed** FLAC::Metadata::Chain::read() to accept a flag denoting Ogg FLAC input
        * **Changed** FLAC::Decoder::Stream::finish() now returns a bool to signal an MD5 failure like FLAC__stream_decoder_finish() does.
        * **Changed** FLAC::Encoder::Stream::finish() now returns a bool to signal a verify failure, or error processing last frame or updating metadata.
    * libOggFLAC:
        * Merged into libFLAC.
    * libOggFLAC++:
        * Merged into libFLAC++.

## FLAC 1.1.2 (05-Feb-2005)  

* General:
    * Sped up decoding by a few percent overall.
    * Sped up encoding when not using LPC (i.e. when using <span class="commandname">flac</span> options <span class="argument">-0</span>, <span class="argument">-1</span>, <span class="argument">-2</span>, or <span class="argument">-l 0</span>).
    * Fixed a decoding bug that could cause sync errors with some ID3v1-tagged FLAC files.
    * Added [HTML documentation for metaflac](https://xiph.org/flac/documentation_tools_metaflac.html#metaflac).
* FLAC format:
    * (none)
* Ogg FLAC format:
    * (none)
* flac:
    * New option <span class="argument">[--input-size](https://xiph.org/flac/documentation_tools_flac.html#flac_options_input_size)</span> to manually specify the input size when encoding raw samples from stdin.
* metaflac:
    * (none)
* plugins:
    * Added support for HTTP streaming in XMMS plugin. **NOTE**: there is a bug in the XMMS mpg123 plugin that hijacks FLAC streams; to fix it you will need to add the '.flac' extension to the list of exceptions in <span class="code">Input/mpg123/mpg123.c:is_our_file()</span> in the XMMS sources and recompile.
* build system:
    * (none)
* libraries:
    * libFLAC: Sped up Rice block decoding in the bitbuffer, resulting in decoding speed gains of a few percent.
    * libFLAC: Sped up encoding when not using LPC (i.e. <span class="code">max_lpc_order == 0</span>).
    * libFLAC: Trailing NUL characters maintained on Vorbis comment entries so they can be treated like C strings.
    * libFLAC: More FLAC tag (i.e. Vorbis comment) validation.
    * libFLAC: Fixed a bug in the logic that determines the frame or sample number in a frame header; the bug could cause sync errors with some ID3v1-tagged FLAC files.
    * libFLAC, libOggFLAC: Can now be compiled to use only integer instructions, including encoding. The decoder is almost completely integer anyway but there were a couple places that needed a fixed-point replacement. There is no fixed-point version of LPC analysis yet, so if libFLAC is compiled integer-only, the encoder will behave as if the max LPC order is 0 (i.e. used fixed predictors only). LPC decoding is supported in all cases as it always was integer-only.
* Interface changes:
    * libFLAC:
        * **Changed:** Metadata object interface now maintains a trailing NUL on Vorbis comment entries for convenience.
        * **Changed:** Metadata object interface now validates all Vorbis comment entries on input and returns false if an entry does not conform to the Vorbis comment spec.
        * **Added** FLAC__format_vorbiscomment_entry_name_is_legal()
        * **Added** FLAC__format_vorbiscomment_entry_value_is_legal()
        * **Added** FLAC__format_vorbiscomment_entry_is_legal()
        * **Added** FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair()
        * **Added** FLAC__metadata_object_vorbiscomment_entry_to_name_value_pair()
        * **Changed** the signature of FLAC__metadata_object_vorbiscomment_entry_matches(): the first argument is now <span class="code">FLAC__StreamMetadata_VorbisComment_Entry entry</span> (was <span class="code">const FLAC__StreamMetadata_VorbisComment_Entry \*entry</span>), i.e. <span class="code">entry</span> is now pass-by-value.
    * libFLAC++:
        * **Changed:** Metadata object interface now maintains a trailing NUL on Vorbis comment values for convenience.
        * **Changed:** Metadata object interface now validates all Vorbis comment entries on input and returns false if an entry does not conform to the Vorbis comment spec.
        * **Changed:** All Metadata objects' operator=() methods now return a reference to themselves.
        * **Added** methods to FLAC::Metadata::VorbisComment::Entry for setting comment values from null-terminated strings:
            * Entry(const char \*field)
            * Entry(const char \*field_name, const char \*field_value)
            * bool set_field(const char \*field)
            * bool set_field_value(const char \*field_value)
        * **Changed** the signature of FLAC::Metadata::VorbisComment::get_vendor_string() and FLAC::Metadata::VorbisComment::set_vendor_string() to use a UTF-8, NUL-terminated string <span class="code">const FLAC__byte *</span> for the vendor string instead of <span class="code">FLAC::Metadata::VorbisComment::Entry</span>.
        * **Added** FLAC::Metadata::*::assign() to all Metadata objects.
        * **Added** bool FLAC::Metadata::get_tags(const char \*filename, VorbisComment &tags)
    * libOggFLAC:
        * (none)
    * libOggFLAC++:
        * (none)

## FLAC 1.1.1 (01-Oct-2004)  

* General:
    * Ogg FLAC seeking now works
    * New optimizations almost double the decoding speed on PowerPC (e.g. Mac G4/G5)
    * A native OS X release thanks to updated Project Builder and autotools files
* FLAC format:
    * Made invalid the metadata block type 127 so that audio frames can always be distinguished from metadata by seeing 0xff as the first byte. (This was also required for the Ogg FLAC mapping.)
* Ogg FLAC format:
    * First official FLAC->Ogg bitstream mapping standardized (see new [FLAC-to-Ogg mapping specification](ogg_mapping.html)). See the documentation for the <span class="argument">[--ogg](https://xiph.org/flac/documentation_tools_flac.html#flac_options_ogg)</span> switch about having to re-encode older Ogg FLAC files.
* flac:
    * Print an error when output file already exists instead of automatically overwriting.
    * New option <span class="argument">[-f](https://xiph.org/flac/documentation_tools_flac.html#flac_options_force)</span> (<span class="argument">[--force](https://xiph.org/flac/documentation_tools_flac.html#flac_options_force)</span>) to force overwriting if the output file already exists.
    * New option <span class="argument">[--cue](https://xiph.org/flac/documentation_tools_flac.html#flac_options_cue)</span> to select a specific section to decode using cuesheet track/index points.
    * New option <span class="argument">[--totally-silent](https://xiph.org/flac/documentation_tools_flac.html#flac_options_totally_silent)</span> to suppress all output.
    * New (but undocumented) option <span class="argument">--apply-replaygain-which-is-not-lossless</span> which applies ReplayGain to the decoded output. See [this thread](http://www.hydrogenaud.io/forums/index.php?showtopic=17293&st=11) for usage and caveats.
    * When encoding to Ogg FLAC, use a random serial number (instead of 0 as was done before) when a serial number is not specified.
    * When encoding multiple Ogg FLAC streams, <span class="argument">--serial-number</span> or random serial number sets the first number, which is then incremented for subsequent streams (before, the same serial number was used for all streams).
    * Decoder no longer exits with an error when writing to stdout and the pipe is broken.
    * Better explanation of common error messages.
    * Default extension when writing AIFF files is .aif (before, it was .aiff).
    * Write more common representation of SANE numbers in AIFF files.
    * Bug fix: calculating ReplayGain on 48kHz streams.
    * Bug fix: check for supported block alignments in WAVE files.
    * Bug fix: "offset" field in AIFF SSND chunk properly handled.
    * Bug fix: [#679166](http://sourceforge.net/p/flac/bugs/77/): flac doesn't respect RIFF subchunk padding byte.
    * Bug fix: [#828391](http://sourceforge.net/p/flac/bugs/90/): --add-replay-gain segfaults.
    * Bug fix: [#851155](http://sourceforge.net/p/flac/bugs/96/): Can't seek to position in flac file.
    * Bug fix: [#851756](http://sourceforge.net/p/flac/bugs/97/): flac --skip --until reads entire file.
    * Bug fix: [#877122](http://sourceforge.net/p/flac/bugs/100/): problem parsing cuesheet with CATALOG entry.
    * Bug fix: [#896057](http://sourceforge.net/p/flac/bugs/104/): parsing ISRC number from cuesheet.
* metaflac:
    * Renamed the tag editing options as follows (the <span class="argument">...-vc-...</span> options still work but are deprecated):
        * <span class="argument">--show-vc-vendor</span> becomes <span class="argument">--show-vendor-tag</span>
        * <span class="argument">--show-vc-field</span> becomes <span class="argument">--show-tag</span>
        * <span class="argument">--remove-vc-all</span> becomes <span class="argument">--remove-all-tags</span>
        * <span class="argument">--remove-vc-field</span> becomes <span class="argument">--remove-tag</span>
        * <span class="argument">--remove-vc-firstfield</span> becomes <span class="argument">--remove-first-tag</span>
        * <span class="argument">--set-vc-field</span> becomes <span class="argument">--set-tag</span>
        * <span class="argument">--import-vc-from</span> becomes <span class="argument">--import-tags-from</span>
        * <span class="argument">--export-vc-to</span> becomes <span class="argument">--export-tags-to</span>
    * Better explanation of common error messages.
    * Bug fix: calculating ReplayGain on 48kHz streams.
    * Bug fix: incorrect numbers when printing seek points.
* plugins:
    * Speed optimization in ReplayGain synthesis.
    * Speed optimization in XMMS playback.
    * Support for big-endian architectures in XMMS plugin.
    * Removed support for ID3 tags.
    * Bug fix: make hard limiter default to off in XMMS plugin.
    * Bug fix: stream length calculation bug in XMMS plugin, debian bug #200435
    * Bug fix: small memory leak in XMMS plugin.
* build system:
    * <span class="code">ordinals.h</span> is now static, not a build-generated file anymore.
* libraries:
    * libFLAC: PPC+Altivec optimizations of some decoder routines.
    * libFLAC: Make stream encoder encode the blocksize and sample rate in the frame header if at all possible (not in STREAMINFO), even if subset encoding was not requested.
    * libFLAC: Bug fix: fixed seek routine where infinite loop could happen when seeking past end of stream.
    * libFLAC, libFLAC++: added methods to skip single frames, useful for quickly finding frame boundaries (see interface changes below).
    * libOggFLAC, libOggFLAC++: New seekable-stream and file encoder and decoder APIs to match native FLAC APIs (see interface changes below).
* Interface changes:
    * libFLAC:
        * **Added** FLAC__metadata_get_tags()
        * **Added** callback-based versions of metadata editing functions:
            * FLAC__metadata_chain_read_with_callbacks()
            * FLAC__metadata_chain_write_with_callbacks()
            * FLAC__metadata_chain_write_with_callbacks_and_tempfile()
            * FLAC__metadata_chain_check_if_tempfile_needed()
        * **Added** decoder functions for skipping single frames, also useful for quickly finding frame boundaries:
            * FLAC__stream_decoder_skip_single_frame()
            * FLAC__seekable_stream_decoder_skip_single_frame()
            * FLAC__file_decoder_skip_single_frame()
        * **Added** new required tell callback on seekable stream encoder:
            * FLAC__SeekableStreamEncoderTellStatus and FLAC__SeekableStreamEncoderTellStatusString\[\]
            * FLAC__SeekableStreamEncoderTellCallback
            * FLAC__seekable_stream_encoder_set_tell_callback()
        * **Changed** FLAC__SeekableStreamEncoderState by adding FLAC__SEEKABLE_STREAM_ENCODER_TELL_ERROR
        * **Changed** Tell callback is now required to initialize seekable stream encoder
        * **Deleted** erroneous and unimplemented FLAC__file_decoder_process_remaining_frames()
    * libFLAC++:
        * **Added** FLAC::Metadata::get_tags()
        * **Added** decoder functions for skipping single frames, also useful for quickly finding frame boundaries:
            * FLAC::Decoder::Stream::skip_single_frame()
            * FLAC::Decoder::SeekableStream::skip_single_frame()
            * FLAC::Decoder::File::skip_single_frame()
        * **Added** encoder functions for setting metadata:
            * FLAC::Encoder::Stream::set_metadata(FLAC::Metadata::Prototype **metadata, unsigned num_blocks)
            * FLAC::Encoder::SeekableStream::set_metadata(FLAC::Metadata::Prototype **metadata, unsigned num_blocks)
            * FLAC::Encoder::File::set_metadata(FLAC::Metadata::Prototype **metadata, unsigned num_blocks)
        * **Added** new required tell callback on seekable stream encoder:
            * pure virtual FLAC::Encoder::SeekableStream::tell_callback()
        * **Changed** Tell callback is now required to initialize seekable stream encoder
        * **Deleted** the following methods:
            * FLAC::Decoder::Stream::State::resolved_as_cstring()
            * FLAC::Encoder::Stream::State::resolved_as_cstring()
    * libOggFLAC:
        * **Added** OggFLAC__SeekableStreamDecoder interface
        * **Added** OggFLAC__FileDecoder interface
        * **Added** OggFLAC__SeekableStreamEncoder interface
        * **Added** OggFLAC__FileEncoder interface
        * **Added** OggFLAC__stream_decoder_get_resolved_state_string()
        * **Added** OggFLAC__stream_encoder_get_resolved_state_string()
        * **Added** OggFLAC__stream_encoder_set_metadata_callback()
        * **Changed** OggFLAC__StreamDecoderState by adding OggFLAC__STREAM_DECODER_END_OF_STREAM
    * libOggFLAC++:
        * **Added** OggFLAC::Decoder::SeekableStream interface
        * **Added** OggFLAC::Decoder::File interface
        * **Added** OggFLAC::Encoder::SeekableStream interface
        * **Added** OggFLAC::Encoder::File interface
        * **Added** OggFLAC::Decoder::Stream::get_resolved_state_string()
        * **Added** OggFLAC::Encoder::Stream::get_resolved_state_string()
        * **Added** pure virtual OggFLAC::Encoder::Stream::metadata_callback()

## FLAC 1.1.0 (26-Jan-2003)  

General:

* All code is now [Valgrind](http://valgrind.org/)-clean!
* New [CUESHEET](https://xiph.org/flac/format.html#def_CUESHEET) metadata block for storing CD TOC and index point information. Now a CD can be completely backed up to a single FLAC file for archival.
* [ReplayGain](http://www.replaygain.org/) support.
* Better compression of 24-bit files.
* More complete AIFF support.
* 3DNow! optimizations enabled by default.
* Complete MSVC build system with .dsp projects for everything, which can build both static libs and DLLs, and in debug or release mode, all in the same source tree.
    
<span class="commandname">flac</span>:

* Can now decode FLAC to AIFF; new <span class="argument">--force-aiff-format</span> option.
* New <span class="argument">--cuesheet</span> option for reading and storing a cuesheet when encoding a whole CD. Automatically creates seek points for track and index points unless <span class="argument">--no-cued-seekpoints</span> is used.
* New <span class="argument">--replay-gain</span> option for calculating ReplayGain values and storing them as tags.
* New <span class="argument">--until</span> option complements <span class="argument">--skip</span> to stop decoding at a specified point in the stream.
* <span class="argument">--skip</span> and <span class="argument">--until</span> now also accept mm:ss.ss format.
* New <span class="argument">-S #s</span> flavor to specify seekpoints every '#' number of seconds.
* <span class="commandname">flac</span> now defaults to <span class="argument">-S 10s</span> instead of <span class="argument">-S 100x</span> for the seek table.
* <span class="commandname">flac</span> now adds a 4k PADDING block by default (turn off with <span class="argument">--no-padding</span>).
* Fixed a bug with --skip and AIFF-to-FLAC encoding.
* Fixed a bug where decoding a FLAC file whose total_samples==0 in the STREAMINFO would corrupt the WAVE header.

<span class="commandname">metaflac</span>:

* New <span class="argument">--import-cuesheet-from</span> option for reading and storing a cuesheet to a FLAC-encoded CD. Automatically creates seek points for track and index points unless <span class="argument">--no-cued-seekpoints</span> is used.
* New <span class="argument">--export-cuesheet-to</span> option for writing a cuesheet from a FLAC file for use with CD authoring software.
* New <span class="argument">--add-replay-gain</span> option for calculating ReplayGain values and storing them as tags.
* New <span class="argument">--add-seekpoint</span> option to add seekpoints to an existing FLAC file. Includes new <span class="argument">--add-seekpoint=#s</span> flavor to add seekpoints every '#' number of seconds.

XMMS plugin:

* Configurable sample resolution conversion with dither.
* ReplayGain support with customizable noise shaping, pre-amp, and optional hard limiter.
* New Vorbis comment editor.
* File info now works.
* Bitrate now shows the smoothed instantaneous bitrate.
* Uses the ARTIST tag if there is no PERFORMER tag.

Winamp2 plugin:

* Configurable sample resolution conversion with dither.
* ReplayGain support with customizable noise shaping, pre-amp, and optional hard limiter.
* File info now works.
* Uses the ARTIST tag if there is no PERFORMER tag.

Libraries (developers take note!):

* All code and tests are instrumented for Valgrind. All tests run Valgrind-clean, meaning no memory leaks or buffer over/under-runs.
* Separate 64-bit datapath through the filter in <span class="commandname">libFLAC</span> for better compression of >16 bps files.
* <span class="code">FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)</span> now sets the vendor string.
* The documentation on the usage of <span class="code">FLAC::Iterator::get_block()</span> in <span class="commandname">libFLAC++</span> has an important correction. If you use this class make sure to read [this](https://xiph.org/flac/api/group__flacpp__metadata__level2.html).

## FLAC 1.0.4 (24-Sep-2002)  

Plugins:

* Support for Vorbis comments, ID3 v1 and v2 tags.
* Configurable title formatting and charset conversion in XMMS plugin.
* Support for 8- and 24-bit FLAC files. There is a compile-time option for raw 24-bit output or 24bps-to-16bps linear dithering (the default).

<span class="commandname">flac</span>:

* Improved option parser (now uses getopt).
* AIFF input support (thanks to Brady Patterson).
* Small decoder speedup.
* <span class="argument">--sector-align</span> now supported for raw input files.
* New -T, --tag options for adding Vorbis comments while encoding.
* New --serial-number option for use with --ogg.
* Automatically writes vendor string in Vorbis comments.
* Drastically reduced memory requirements.
* Fixed bug where extra fmt/data chunks that were supposed to be skipped were not getting skipped.
* Fixed bug in granulepos setting for Ogg FLAC streams.
* Fixed memory leak when encoding multiple files with -V.

<span class="commandname">metaflac</span>:

* UTF-8 support in Vorbis comments.
* New --import-vc-from and --export-vc-to commands for importing/exporting Vorbis comments from/to a file. For example, the following can be used to copy tags back and forth:  
<span class="code">metaflac --export-vc-to=- --no-utf8-convert file.flac | vorbiscomment --raw -w file.ogg  
vorbiscomment --raw -l file.ogg | metaflac --import-vc-from=- --no-utf8-convert file.flac  
</span>
* Fixed [bug #606796](http://sourceforge.net/p/flac/bugs/54/) where <span class="commandname">metaflac</span> was failing on read-only files.

Libraries:

* All APIs now meticulously documented via Doxygen. [See here](https://xiph.org/flac/api/index.html).
* New <span class="commandname">libOggFLAC</span> and <span class="commandname">libOggFLAC++</span> libraries. These wrap around <span class="commandname">libFLAC</span> to provide encoding and decoding of Ogg FLAC streams, providing interfaces similar to the ones of the native FLAC libraries. These are also documented via Doxygen.
* New FLAC__SeekableStreamEncoder and FLAC__FileEncoder in <span class="commandname">libFLAC</span> simplify common encoding tasks.
* New verify mode in all encoders.
* FLAC__stream_encoder_finish() now resets the defaults just like the stream decoders.
* Drastically reduced memory requirements of encoders and decoders.
* Encoder now automatically writes vendor string in VORBIS_COMMENT block.
* Encoding speedup of fixed predictors and MD5 speedup for 16bps mono/stereo signals on x86 (thanks to Miroslav Lichvar).
* Fixed bug in metadata interface where a bps in STREAMINFO > 16 was incorrectly parsed.
* Fixed bug where aborting stream decoder could cause infinite loop.
* Behavior change: simplified decoder \*_process() commands.
* Behavior change: calling FLAC__stream_encoder_init() calls write callback once for "fLaC" signature and once for each metadata block.
    * Behavior change: deprecated do_escape_coding and rice_parameter_search_distance in encoder.

## FLAC 1.0.3 (03-Jul-2002)  

New features:

* 24-bit input support restored in <span class="commandname">flac</span>.
* Decoder speedup in <span class="commandname">libFLAC</span>, which is directly passed on to the command-line decoder and plugins.
* New <span class="argument">-F</span> option to <span class="commandname">flac</span> to continue decoding in spite of errors.
* Correctly set granulepos in Ogg packets so seeking Ogg FLAC streams will be easier.
* New [VORBIS_COMMENT](https://xiph.org/flac/format.html#metadata_block_vorbis_comment) metadata block for tagging with Vorbis-style comments.
* Vastly improved <span class="commandname">metaflac</span>, now with many editing and tagging options.
* Partial id3v1 support in Winamp plugins.
* Updated Winamp 3 plugin.
* Note: new semantics for -P option in <span class="commandname">flac</span>.
* Note: removed -R option in <span class="commandname">flac</span>.

New library features:

* Previously mentioned decoder speedup in <span class="commandname">libFLAC</span>.
* New metadata interface to <span class="commandname">libFLAC</span> for manipulating metadata in FLAC files.
* New <span class="commandname">libFLAC++</span> API, an object wrapper around <span class="commandname">libFLAC</span>.
* New [VORBIS_COMMENT](https://xiph.org/flac/format.html#metadata_block_vorbis_comment) metadata block for tagging with Vorbis-style comments.
* Customizable metadata filtering by type in decoders.
* Stream encoder can take an arbitrary list of metadata blocks, instead of just one SEEKTABLE and/or PADDING block.

Bugs fixed:

* Fixed bug with using pipes under Windows.
* Fixed several bugs in the plugins and made them more robust in general.
* Fixed bug in <span class="commandname">flac</span> where decoding to WAVE of a FLAC file with 0 for total_samples in the STREAMINFO block yielded a WAVE chunk of 0 size.
* Fixed bug in Ogg packet numbering.

## FLAC 1.0.2 (03-Dec-2001)  

* This release is only to fix a bug that was causing some of the plugins to crash sporadically. It can also affect <span class="commandname">libFLAC</span> users that reuse one file decoder instance for multiple files

## FLAC 1.0.1 (14-Nov-2001)  

New features for users:

* Support for Ogg-FLAC, i.e. <span class="commandname">flac</span> can now read and write FLAC streams using Ogg as the transport layer.
* New Winamp 3 plugin based on the Wasabi Beta 1 SDK.
* New utilities for adding FLAC support to the Monkey's Audio GUI (see [how](https://xiph.org/flac/documentation_tasks.html#monkey)).
* Mac OS X support. The download area now contains an OS X binary release.
* Mingw32 support.
* Better handling of MS-specific 'fmt' chunks in WAVE files.

New features for developers:

* Added a SeekableStreamDecoder layer between StreamDecoder and FileDecoder. This makes it easier to use libFLAC in situations where files have been abstracted away. See the latest [documentation](https://xiph.org/flac/api/index.html) for more. The interface for the StreamDecoder and FileDecoder remain the same and are still binary-compatible with libFLAC 1.0.
* Drastically reduced the stack requirements of the encoder.

Bug fixes:

* Fixed a serious bug with <span class="commandname">flac</span> and raw input where the encoder was trying to rewind when it shouldn't, which would add 12 junk samples to the encoded file. This was not present in WAVE encoding.
* Fixed a minor bug in <span class="commandname">libFLAC</span> with setting the file name to stdin on a file decoder.
* Fixed a minor bug in <span class="commandname">libFLAC</span> where multiple calls to setting the file name on a file decoder caused leaked memory.
* Fixed a minor bug in <span class="commandname">metaflac</span>, now correctly skips an id3v2 tag if present.
* Fixed a minor bug in <span class="commandname">metaflac</span>, now correctly skips long metadata blocks.

## FLAC 1.0 (20-Jul-2001)  

It's finally here. There are a few new features but mostly it is minor bug fixes since 0.10:

* New '--sector-align' option to <span class="commandname">flac</span> which aligns a group of encoded files on CD audio sector boundaries.
* New '--output-prefix' option to <span class="commandname">flac</span> to allow the user to prepend a prefix to all output filenames (useful, for example, for encoding/decoding to a different directory).
* Better WAVE autodetection (doesn't rely on ungetc() anymore).
* Cleaner one-line encoding/decoding stats.
* Changes to the libFLAC interface and type names to make binary compatibility easier to maintain in the future.
* New '--sse-os' option to 'configure' to enable faster SSE-based routines.
* Another (hopefully last) fix to the Winamp 2 plugin.
* Slightly improved Rice parameter estimation.
* Bug fixes for some very rare corner cases when encoding.

## FLAC 0.10 (07-Jun-2001)  

This is probably the final beta. There have been many improvements in the last two months:

* Both the encoder and decoder have been significantly sped up. Aside from C improvements, the code base now has an assembly infrastructure that allows assembly routines for different architectures to be easily integrated. Many key routines have now have faster IA-32 implementations (thanks to Miroslav).
* A new metadata block [SEEKTABLE](https://xiph.org/flac/format.html#def_SEEKTABLE) has been defined to hold an arbitrary number of seek points, which speeds up seeking within a stream.
* <span class="commandname">flac</span> now has a command-line usage similar to 'gzip'; make sure to see the latest [documentation](https://xiph.org/flac/documentation.html) for the new usage. It also attempts to preserve the input file's timestamp and permissions.
* The -# options in <span class="commandname">flac</span> have been tweaked to yield the best compression-to-encode-time ratios. The new default is -5.
* <span class="commandname">flac</span> can now usually autodetect WAVE files when encoding so that -fw is usually not needed when encoding from stdin.
* The WAVE reader in <span class="commandname">flac</span> now just ignores (with a warning) unsupported sub-chunks instead of aborting with an error.
* Added an option '--delete-input-file' to <span class="commandname">flac</span> which automatically deletes the input after a successful encode/decode.
* Added an option '-o' to <span class="commandname">flac</span> to force the output file name (the old usage of "flac - outputfilename" is no longer supported).
* Changed the XMMS plugin to send smaller chunks of samples (now 512) so that visualization is not slow.
* Fixed a bug in the stream decoder where the decoded samples counter got corrupted after a seek.

## FLAC 0.9 (31-Mar-2001)  

Bug fixes and some new features:

* FLAC's sync code has been lengthened to 14 bits from 9 bits. This should enable a faster and more robust synchronization mechanism.
* Two reserved bits were added to the frame header.
* A CRC-16 was added to the FLAC frame footer, and the decoder now does frame integrity checking based on the CRC.
* The format now includes a new subframe field to indicate when a subblock has one or more 0 LSBs for all samples. This increases compression on some kinds of data.
* Added two options to the analysis mode, one for including the residual signal in the analysis file, and one for generating gnuplot files of each subframe's residual distribution with some statistics. See the latest [documentation](https://xiph.org/flac/documentation.html#analysis_options).
* XMMS plugin now supports 8-bit files.
* Fixed a bug in the Winamp2 plugin where the audio sounded garbled.
* Fixed a bug in the Winamp2 plugin where Winamp would hang sporadically at the end of a track (c.f. [bug #231197](http://sourceforge.net/projects/flac/&atid=113478)).

## FLAC 0.8 (05-Mar-2001)  

Changes since 0.7:

* Created a new utility called <span class="commandname">metaflac</span>. It is a metadata editor for .flac files. Right now it just lists the contents of the metadata blocks but eventually it will allow update/insertion/deletion.
* Added two new metadata blocks: PADDING which has an obvious function, and APPLICATION, which is meant to be open to third party applications. See the [latest format docs](https://xiph.org/flac/format.html#def_APPLICATION) for more info, or the new [id registration page](https://xiph.org/flac/id.html).
* Added a <span class="argument">-P</span> option to <span class="commandname">flac</span> to reserve a PADDING block when encoding.
* Added support for 24-bit files to <span class="commandname">flac</span> (the FLAC format always supported it).
* Started the Winamp3 plugin.
* Greatly expanded the test suite, adding more streams (24-bit streams, noise streams, non-audio streams, more patterns) and more option combinations to the encoder. The test suite runs about 30 streams and over 5000 encodings now.
* Fixed a bug in <span class="commandname">libFLAC</span> that happened when using an exhaustive LPC coefficient quantization search with 8 bps input.
* Fixed a bug in <span class="commandname">libFLAC</span> where the error estimation in the fixed predictor could overflow.
* Fixed a bug in <span class="commandname">libFLAC</span> where LPC was attempted even when the autocorrelation coefficients implied it wouldn't help.
* Reworked the LPC coefficient quantizer, which also fixed another bug that might occur in rare cases.
* Really fixed the '-V overflow' bug (c.f. [bug #231976](http://sourceforge.net/p/flac/bugs/5/)).
* Fixed a bug in <span class="commandname">flac</span> related to the decode buffer sizing.FLAC is very close to being ready for an official release. The only known problems left are with the Winamp plugins, which should be fixed soon, and pipes with MSVC.

## FLAC 0.7 (12-Feb-2001)  

Changes:
 
* Fixed a bug that happened when both -fr and --seek were used at the same time.
* Fixed a bug with -p (c.f. [bug #230992](http://sourceforge.net/p/flac/bugs/1/)).
* Fixed a bug that happened when using large (>32K) blocksizes and -V (c.f. [bug #231976](http://sourceforge.net/p/flac/bugs/5/)).
* Fixed a bug where encoder was double-closing a file.
* Expanded the test suite.
* Added more optimization flags for gcc, which should speed up flac.

## FLAC 0.6 (28-Jan-2001)  

The encoder is now much faster. The -m option has been sped up by 4x and -r improved, meaning that in the default compression mode (-6), encoding should be at least 3 times faster. Other changes:

* Some bugs related to <span class="commandname">flac</span> and pipes were fixed
* A "loose mid-side" (<span class="argument">-M</span>) option to the encoder has been added, which adaptively switches between independent and mid-side coding, instead of the exhaustive search that <span class="argument">-m</span> does.
* An analyze mode (<span class="argument">-a</span>) has been added to <span class="commandname">flac</span>. This is useful mainly for developers; currently it will dump info about each frame and subframe to a file. It's a text file in a format that can be easily processed by scripts; a separate analysis program is in the works.
* The source now has an autoconf/libtool-based build system. This should allow the source to build "out-of-the-box" on many more platforms.

## FLAC 0.5 (15-Jan-2001)  

This is the first beta version of FLAC. Being beta, there will be no changes to the format that will break older streams, unless a serious bug involving the format is found. What this means is that, barring such a bug, streams created with 0.5 will be decodable by future versions. This version also includes some new features:

* An [MD5 signature](http://userpages.umbc.edu/~mabzug1/cs/md5/md5.html) of the unencoded audio is computed during encoding, and stored in the Encoding metadata block in the stream header. When decoding, <span class="commandname">flac</span> will now compute the MD5 signature of the decoded data and compare it against the signature in the stream header.
* A test mode (<span class="argument">-t</span>) has been added to <span class="commandname">flac</span>. It works like decode mode but doesn't write an output file.

## FLAC 0.4 (23-Dec-2000)  

This version fixes a bug in the constant subframe detection. More importantly, a verify option (-V) has been added to <span class="commandname">flac</span> that verifies the encoding process. With this option turned on, <span class="commandname">flac</span> will create a parallel decoder while encoding to make sure that the encoded output decodes to exactly match the original input. In this way, any unknown bug in the encoder will be caught and <span class="commandname">flac</span> will abort with an error message.

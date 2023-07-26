#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2019-2023  Xiph.Org Foundation
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

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/oss-fuzz

AM_CFLAGS = ${LIB_FUZZING_ENGINE}
AM_CXXFLAGS = -std=c++11 $(LIB_FUZZING_ENGINE)
LDADD = $(flac_libs)


EXTRA_DIST = \
	fuzzer_encoder.dict \
	common.h \
	Readme.md \
	fuzzing/datasource/datasource.hpp \
	fuzzing/datasource/id.hpp \
	fuzzing/exception.hpp \
	fuzzing/memory.hpp \
	fuzzing/types.hpp

noinst_PROGRAMS =

if USE_OSSFUZZERS
noinst_PROGRAMS += fuzzer_encoder fuzzer_encoder_v2 fuzzer_decoder fuzzer_seek fuzzer_metadata fuzzer_reencoder fuzzer_tool_flac fuzzer_tool_metaflac
endif

fuzzer_encoder_SOURCES = encoder.cc
fuzzer_encoder_v2_SOURCES = encoder_v2.cc
fuzzer_decoder_SOURCES = decoder.cc
fuzzer_seek_SOURCES = seek.cc
fuzzer_metadata_SOURCES = metadata.cc
fuzzer_reencoder_SOURCES = reencoder.cc
fuzzer_tool_flac_SOURCES = ${flac_SOURCES} empty.cc tool_flac.c # empty.cc is to force use of C++ linker, which is mandated by oss-fuzz
fuzzer_tool_flac_LDADD =  \
        $(top_builddir)/src/share/utf8/libutf8.la \
        $(top_builddir)/src/share/grabbag/libgrabbag.la \
        $(top_builddir)/src/share/getopt/libgetopt.la \
        $(top_builddir)/src/share/replaygain_analysis/libreplaygain_analysis.la \
        $(top_builddir)/src/share/replaygain_synthesis/libreplaygain_synthesis.la \
        $(top_builddir)/src/libFLAC/libFLAC.la \
        @LTLIBICONV@ \
        -lm
fuzzer_tool_metaflac_SOURCES = ${metaflac_SOURCES} empty.cc tool_metaflac.c # empty.cc is to force use of C++ linker, which is mandated by oss-fuzz
fuzzer_tool_metaflac_LDADD =  \
        $(top_builddir)/src/share/utf8/libutf8.la \
        $(top_builddir)/src/share/grabbag/libgrabbag.la \
        $(top_builddir)/src/share/getopt/libgetopt.la \
        $(top_builddir)/src/share/replaygain_analysis/libreplaygain_analysis.la \
        $(top_builddir)/src/share/replaygain_synthesis/libreplaygain_synthesis.la \
        $(top_builddir)/src/libFLAC/libFLAC.la \
        @LTLIBICONV@ \
        -lm

flac_libs = \
	$(top_builddir)/src/libFLAC/libFLAC-static.la \
	$(top_builddir)/src/libFLAC++/libFLAC++-static.la \
	@OGG_LIBS@ \
	-lm

flac_SOURCES = \
        ${top_builddir}/src/flac/analyze.c \
        ${top_builddir}/src/flac/decode.c \
        ${top_builddir}/src/flac/encode.c \
        ${top_builddir}/src/flac/foreign_metadata.c \
        ${top_builddir}/src/flac/local_string_utils.c \
        ${top_builddir}/src/flac/utils.c \
        ${top_builddir}/src/flac/vorbiscomment.c \
        ${top_builddir}/src/flac/analyze.h \
        ${top_builddir}/src/flac/decode.h \
        ${top_builddir}/src/flac/encode.h \
        ${top_builddir}/src/flac/foreign_metadata.h \
        ${top_builddir}/src/flac/local_string_utils.h \
        ${top_builddir}/src/flac/utils.h \
        ${top_builddir}/src/flac/vorbiscomment.h

metaflac_SOURCES = \
        ${top_builddir}/src/metaflac/operations.c \
        ${top_builddir}/src/metaflac/operations_shorthand_cuesheet.c \
        ${top_builddir}/src/metaflac/operations_shorthand_picture.c \
        ${top_builddir}/src/metaflac/operations_shorthand_seektable.c \
        ${top_builddir}/src/metaflac/operations_shorthand_streaminfo.c \
        ${top_builddir}/src/metaflac/operations_shorthand_vorbiscomment.c \
        ${top_builddir}/src/metaflac/options.c \
        ${top_builddir}/src/metaflac/usage.c \
        ${top_builddir}/src/metaflac/utils.c \
        ${top_builddir}/src/metaflac/operations.h \
        ${top_builddir}/src/metaflac/operations_shorthand.h \
        ${top_builddir}/src/metaflac/options.h \
        ${top_builddir}/src/metaflac/usage.h \
        ${top_builddir}/src/metaflac/utils.h

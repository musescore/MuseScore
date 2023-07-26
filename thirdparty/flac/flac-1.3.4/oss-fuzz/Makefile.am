#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2019  Xiph.Org Foundation
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

AM_CXXFLAGS = -std=c++11

EXTRA_DIST = \
	fuzz-encoder.dict \
	fuzzing/Readme.md \
	fuzzing/datasource/datasource.hpp \
	fuzzing/datasource/id.hpp \
	fuzzing/exception.hpp \
	fuzzing/memory.hpp \
	fuzzing/types.hpp

if USE_OSSFUZZ_FLAG
FUZZ_FLAG = $(LIB_FUZZING_ENGINE)
FUZZ_LDADD = -lFuzzer
else
if USE_OSSFUZZ_STATIC
FUZZ_LDADD = $(LIB_FUZZING_ENGINE)
FUZZ_FLAG = -lFuzzer
endif
endif

noinst_PROGRAMS =

if USE_OSSFUZZERS
noinst_PROGRAMS += fuzz-decoder fuzz-encoder
endif

fuzz_decoder_SOURCES = fuzz-decoder.cc
fuzz_decoder_CXXFLAGS = $(AM_CXXFLAGS) $(FUZZ_FLAG)
fuzz_decoder_LDFLAGS = $(AM_LDFLAGS) -static
fuzz_decoder_LDADD = $(flac_libs) $(FUZZ_LDADD)

fuzz_encoder_SOURCES = fuzz-encoder.cc
fuzz_encoder_CXXFLAGS = $(AM_CXXFLAGS) $(FUZZ_FLAG)
fuzz_encoder_LDFLAGS = $(AM_LDFLAGS) -static
fuzz_encoder_LDADD = $(flac_libs) $(FUZZ_LDADD)

flac_libs = \
	$(top_builddir)/src/libFLAC/libFLAC-static.la \
	$(top_builddir)/src/libFLAC++/libFLAC++-static.la \
	@OGG_LIBS@ \
	-lm


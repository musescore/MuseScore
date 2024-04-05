#  libFLAC - Free Lossless Audio Codec library
#  Copyright (C) 2001-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
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
#  - Neither the name of the Xiph.org Foundation nor the names of its
#  contributors may be used to endorse or promote products derived from
#  this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AM_CPPFLAGS = -I$(top_builddir) -I$(srcdir)/include -I$(top_srcdir)/include
lib_LTLIBRARIES = libFLAC.la
noinst_LTLIBRARIES = libFLAC-static.la
if DEBUG
DEBUGCFLAGS = -DFLAC__OVERFLOW_DETECT
else
if ASSOC_MATH_AVAILABLE
ASSOCMATHCFLAGS = -fassociative-math -fno-signed-zeros -fno-trapping-math -freciprocal-math
endif
endif

AM_CFLAGS = $(DEBUGCFLAGS) ${ASSOCMATHCFLAGS} @OGG_CFLAGS@

libFLAC_la_LIBADD = @OGG_LIBS@ -lm

SUBDIRS = include .

m4datadir = $(datadir)/aclocal
m4data_DATA = libFLAC.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flac.pc

EXTRA_DIST = \
	CMakeLists.txt \
	flac.pc.in \
	libFLAC.m4 \
	version.rc \
	deduplication/bitreader_read_rice_signed_block.c \
	deduplication/lpc_compute_autocorrelation_intrin.c \
	deduplication/lpc_compute_autocorrelation_intrin_sse2.c \
	deduplication/lpc_compute_autocorrelation_intrin_neon.c

if OS_IS_WINDOWS
windows_unicode_compat = ../share/win_utf8_io/win_utf8_io.c
if HAVE_WINDRES
libFLAC_la_DEPENDENCIES = version.o
windows_resource_link = -Wl,version.o
endif
endif

if FLaC__HAS_OGG
extra_ogg_sources = \
	ogg_decoder_aspect.c \
	ogg_encoder_aspect.c \
	ogg_helper.c \
	ogg_mapping.c
endif

# see 'http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning' for numbering convention
libFLAC_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined -version-info 13:0:1 $(windows_resource_link)

libFLAC_sources = \
	bitmath.c \
	bitreader.c \
	bitwriter.c \
	cpu.c \
	crc.c \
	fixed.c \
	fixed_intrin_sse2.c \
	fixed_intrin_ssse3.c \
	fixed_intrin_sse42.c \
	fixed_intrin_avx2.c \
	float.c \
	format.c \
	lpc.c \
	lpc_intrin_sse2.c \
	lpc_intrin_sse41.c \
	lpc_intrin_avx2.c \
	lpc_intrin_fma.c \
	lpc_intrin_neon.c \
	md5.c \
	memory.c \
	metadata_iterators.c \
	metadata_object.c \
	stream_decoder.c \
	stream_encoder.c \
	stream_encoder_intrin_sse2.c \
	stream_encoder_intrin_ssse3.c \
	stream_encoder_intrin_avx2.c \
	stream_encoder_framing.c \
	window.c \
	$(windows_unicode_compat) \
	$(extra_ogg_sources)

libFLAC_la_SOURCES = $(libFLAC_sources)

# needed for test_libFLAC
libFLAC_static_la_SOURCES = $(libFLAC_sources)

.rc.o:
	$(RC) $(AM_CPPFLAGS) $< $@

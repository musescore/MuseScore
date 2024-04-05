#  libFLAC++ - Free Lossless Audio Codec library
#  Copyright (C) 2002-2009  Josh Coalson
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

lib_LTLIBRARIES = libFLAC++.la
noinst_LTLIBRARIES = libFLAC++-static.la

m4datadir = $(datadir)/aclocal
m4data_DATA = libFLAC++.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flac++.pc
AM_CPPFLAGS = -I$(top_builddir) -I$(srcdir)/include -I$(top_srcdir)/include
EXTRA_DIST = \
	CMakeLists.txt \
	flac++.pc.in \
	libFLAC++.m4 \
	version.rc

libFLAC___sources = \
	metadata.cpp \
	stream_decoder.cpp \
	stream_encoder.cpp

if OS_IS_WINDOWS
if HAVE_WINDRES
libFLAC___la_DEPENDENCIES = version.o
windows_resource_link = -Wl,version.o
endif
endif

# see 'http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning' for numbering convention
libFLAC___la_LDFLAGS = $(AM_LDFLAGS) -no-undefined -version-info 10:1:0 $(windows_resource_link)
libFLAC___la_LIBADD = ../libFLAC/libFLAC.la
libFLAC___la_SOURCES = $(libFLAC___sources)

libFLAC___static_la_SOURCES = $(libFLAC___sources)
libFLAC___static_la_LIBADD = ../libFLAC/libFLAC-static.la

.rc.o:
	$(RC) $(AM_CPPFLAGS) $< $@

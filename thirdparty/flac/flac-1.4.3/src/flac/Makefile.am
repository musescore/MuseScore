#  flac - Command-line FLAC encoder/decoder
#  Copyright (C) 2000-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

if OS_IS_WINDOWS
win_utf8_lib = $(top_builddir)/src/share/win_utf8_io/libwin_utf8_io.la
if HAVE_WINDRES
flac_DEPENDENCIES = version.o
windows_resource_link = -Wl,version.o
endif
endif

bin_PROGRAMS = flac

AM_CFLAGS = @OGG_CFLAGS@
AM_CPPFLAGS = -I$(top_builddir) -I$(srcdir)/include -I$(top_srcdir)/include
EXTRA_DIST = \
	CMakeLists.txt \
	iffscan.c \
	version.rc

flac_SOURCES = \
	analyze.c \
	decode.c \
	encode.c \
	foreign_metadata.c \
	main.c \
	local_string_utils.c \
	utils.c \
	vorbiscomment.c \
	analyze.h \
	decode.h \
	encode.h \
	foreign_metadata.h \
	local_string_utils.h \
	utils.h \
	vorbiscomment.h

flac_LDADD = \
	$(top_builddir)/src/share/utf8/libutf8.la \
	$(top_builddir)/src/share/grabbag/libgrabbag.la \
	$(top_builddir)/src/share/getopt/libgetopt.la \
	$(top_builddir)/src/share/replaygain_analysis/libreplaygain_analysis.la \
	$(top_builddir)/src/share/replaygain_synthesis/libreplaygain_synthesis.la \
	$(top_builddir)/src/libFLAC/libFLAC.la \
	$(win_utf8_lib) \
	@LTLIBICONV@ \
	-lm

flac_LDFLAGS = $(AM_LDFLAGS) $(windows_resource_link)

CLEANFILES = flac.exe

.rc.o:
	$(RC) $(AM_CPPFLAGS) $< $@

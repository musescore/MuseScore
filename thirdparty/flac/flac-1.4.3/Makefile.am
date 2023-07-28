#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2001-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
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

#
# automake provides the following useful targets:
#
# all: build all programs and libraries using the current
# configuration (set by configure)
#
# check: build and run all self-tests
#
# clean: remove everything except what's required to build everything
#
# distclean: remove everything except what goes in the distribution
#

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = doc include m4 man src test microbench oss-fuzz

if EXAMPLES
SUBDIRS += examples
endif

EXTRA_DIST = \
	CMakeLists.txt \
	config.cmake.h.in \
	flac-config.cmake.in \
	cmake/CheckA64NEON.c.in \
	cmake/CheckA64NEON.cmake \
	cmake/CheckCPUArch.c.in \
	cmake/CheckCPUArch.cmake \
	cmake/FindOgg.cmake \
	cmake/UseSystemExtensions.cmake \
	CHANGELOG.md \
	COPYING.FDL \
	COPYING.GPL \
	COPYING.LGPL \
	COPYING.Xiph \
	README.md \
	autogen.sh \
	config.rpath \
	depcomp \
	ltmain.sh

CLEANFILES = *~

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

TESTS_ENVIRONMENT = FLAC__TEST_LEVEL=@FLAC__TEST_LEVEL@ FLAC__TEST_WITH_VALGRIND=@FLAC__TEST_WITH_VALGRIND@ ECHO_N="@ECHO_N@" ECHO_C="@ECHO_C@"

SUBDIRS = cuesheets foreign-metadata-test-files flac-to-flac-metadata-test-files metaflac-test-files pictures

check_SCRIPTS = \
	test_libFLAC.sh \
	test_libFLAC++.sh \
	test_grabbag.sh \
	test_flac.sh \
	test_metaflac.sh \
	test_replaygain.sh \
	test_seeking.sh \
	test_streams.sh \
	test_compression.sh

# This one should pass when building out-of-tree (e.g. 'make distcheck').
check: $(check_SCRIPTS)
	$(TESTS_ENVIRONMENT) $(srcdir)/test_libFLAC.sh
if FLaC__WITH_CPPLIBS
	$(TESTS_ENVIRONMENT) $(srcdir)/test_libFLAC++.sh
endif
	$(TESTS_ENVIRONMENT) $(srcdir)/test_grabbag.sh
	$(TESTS_ENVIRONMENT) $(srcdir)/test_flac.sh
	$(TESTS_ENVIRONMENT) $(srcdir)/test_metaflac.sh
	$(TESTS_ENVIRONMENT) $(srcdir)/test_replaygain.sh
	$(TESTS_ENVIRONMENT) $(srcdir)/test_seeking.sh
	$(TESTS_ENVIRONMENT) $(srcdir)/test_streams.sh
	$(TESTS_ENVIRONMENT) $(srcdir)/test_compression.sh
	@echo "----------------"
	@echo "All tests passed"
	@echo "----------------"

EXTRA_DIST = \
	CMakeLists.txt \
	cuesheet.ok \
	generate_streams.sh \
	metaflac.flac.in \
	metaflac.flac.ok \
	picture.ok \
	$(check_SCRIPTS) \
	write_iff.pl

clean-local:
	-rm -f *.raw *.flac *.oga *.ogg *.cmp *.aiff *.wav *.w64 *.rf64 *.diff *.log *.cue core

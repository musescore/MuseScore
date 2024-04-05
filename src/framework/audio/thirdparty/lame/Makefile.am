## $Id: Makefile.am,v 1.39 2012/06/19 00:00:55 rbrito Exp $

include $(top_srcdir)/Makefile.am.global

.PHONY: test

SUBDIRS = mpglib libmp3lame frontend Dll doc include misc dshow ACM \
	mac macosx vc_solution

CLEANFILES = testcase.new.mp3

EXTRA_DIST = \
	API \
	DEFINES \
	HACKING \
	INSTALL.configure \
	LICENSE \
	Makefile.MSVC \
	Makefile.unix \
	Makefile.am.global \
	README.WINGTK \
	STYLEGUIDE \
	USAGE \
	configMS.h \
	debian \
	lame.bat \
	lame.spec.in \
	lame.spec \
	testcase.mp3 \
	testcase.wav

#
# The differences depend on the used processor architecture, the used
# compiler and the used options for the compiler, so make test may
# show some differences. You should only be concerned if you are a
# LAME developer and the number of differences change after you
# modified the source.
#
testcase.new.mp3: testcase.wav frontend/lame$(EXEEXT)
	time frontend/lame$(EXEEXT) --nores $(top_srcdir)/testcase.wav testcase.new.mp3 || $(RM_F) testcase.new.mp3

test: testcase.mp3 testcase.new.mp3
	@echo
	@echo "The following output has value only for a LAME-developer, do not make _any_"
	@echo "assumptions about what this number means. You do not need to care about it."
	@cmp -l testcase.new.mp3 $(top_srcdir)/testcase.mp3 | wc -l

testupdate: testcase.mp3 testcase.new.mp3
	cp testcase.new.mp3 $(top_srcdir)/testcase.mp3

testg: frontend/mp3x$(EXEEXT) $(top_srcdir)/../test/castanets.wav
	frontend/mp3x$(EXEEXT) -h $(top_srcdir)/../test/castanets.wav

update:
	cd $(top_srcdir) && CVS_RSH=ssh cvs -z3 -q update -dAP || true

diff:
	cd $(top_srcdir) && CVS_RSH=ssh cvs -z3 diff -u || true

frontend/lame$(EXEEXT):
	$(MAKE) $(MAKEFLAGS)

frontend/mp3x$(EXEEXT): frontend/lame$(EXEEXT)


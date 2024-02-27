# Process this file with automake to produce Makefile.in

NULL =

# Convenience targets:
lib:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/src lib

update:
	(cd $(srcdir) && ./update.sh)

EXTRA_DIST = \
	README \
	COPYING \
	DISABLED \
	update.py \
	meson.build \
	fonts \
	$(TESTS) \
	$(NULL)

TEST_EXTENSIONS = .tests
if HAVE_FREETYPE
TESTS_ENVIRONMENT = HAVE_FREETYPE=1
else
TESTS_ENVIRONMENT = HAVE_FREETYPE=0
endif
TESTS_LOG_COMPILER = $(srcdir)/../../run-tests.py $(top_builddir)/util/hb-shape$(EXEEXT)

include Makefile.sources

-include $(top_srcdir)/git.mk

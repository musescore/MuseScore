# Process this file with automake to produce Makefile.in

NULL =

# Convenience targets:
lib:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/src lib

EXTRA_DIST = \
	fonts \
	meson.build \
	$(TESTS) \
	$(NULL)

TEST_EXTENSIONS = .tests
TESTS_ENVIRONMENT = 
if HAVE_FREETYPE
TESTS_ENVIRONMENT += HAVE_FREETYPE=1
endif
if HAVE_CORETEXT
TESTS_ENVIRONMENT += HAVE_CORETEXT=1
endif
if HAVE_DIRECTWRITE
TESTS_ENVIRONMENT += HAVE_DIRECTWRITE=1
endif
if HAVE_UNISCRIBE
TESTS_ENVIRONMENT += HAVE_UNISCRIBE=1
endif
TESTS_LOG_COMPILER = $(srcdir)/../../run-tests.py $(top_builddir)/util/hb-shape$(EXEEXT)

include Makefile.sources

-include $(top_srcdir)/git.mk

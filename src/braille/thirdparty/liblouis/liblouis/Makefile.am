liblouisincludedir = $(includedir)/liblouis

# Don't include liblouis.h in dist, this will break subdir builds
# when dist is configured with a different ucs-option than the build,
# i.e. ucs2 dist and ucs4 build and vice versa.
nodist_liblouisinclude_HEADERS = \
	liblouis.h

lib_LTLIBRARIES = liblouis.la

AM_CPPFLAGS = 						\
	$(WARN_CFLAGS)					\
	-DTABLESDIR=\""$(datadir)"/liblouis/tables\"	\
	-I$(top_srcdir)/gnulib				\
	-I$(top_builddir)/gnulib

if USE_COVERAGE
AM_CPPFLAGS += -fprofile-instr-generate -fcoverage-mapping
endif

liblouis_la_LIBADD = $(top_builddir)/gnulib/libgnu.la

liblouis_la_LDFLAGS =	\
	-version-info $(LIBLOUIS_CURRENT):$(LIBLOUIS_REVISION):$(LIBLOUIS_AGE) -no-undefined \
	$(LTLIBINTL)

if HAVE_LD_OUTPUT_DEF
liblouis_la_LDFLAGS += -Wl,--output-def,liblouis-$(DLL_VERSION).def
defexecdir = $(bindir)
defexec_DATA = liblouis-$(DLL_VERSION).def
DISTCLEANFILES = $(defexec_DATA)
endif

liblouis_la_SOURCES =				\
	internal.h					\
	compileTranslationTable.c		\
	lou_translateString.c			\
	lou_backTranslateString.c		\
	commonTranslationFunctions.c		\
	metadata.c 				\
	pattern.c				\
	logging.c				\
	utils.c				\
	maketable.c

# Don't include liblouis.h in dist, this will break subdir builds
# when dist is configured with a different ucs-option than the build,
# i.e. ucs2 dist and ucs4 build and vice versa.
nodist_liblouis_la_SOURCES = \
	liblouis.h

if HAVE_SOURCE_FORMATTER
format-sources-local:
	for file in $(SOURCES); do $(SOURCE_FORMATTER) -i $$file; done
	$(SOURCE_FORMATTER) -i liblouis.h.in
endif

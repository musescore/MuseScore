ACLOCAL_AMFLAGS = -I m4

DEFS += -DRANDOM_PREFIX=libopusenc -DOUTSIDE_SPEEX

AM_CFLAGS = -I$(top_srcdir)/include $(DEPS_CFLAGS)

dist_doc_DATA = COPYING AUTHORS README.md

opusincludedir = ${includedir}/opus
opusinclude_HEADERS = include/opusenc.h

lib_LTLIBRARIES = libopusenc.la
noinst_HEADERS = src/arch.h \
		 src/ogg_packer.h \
		 src/opus_header.h \
		 src/picture.h \
		 src/resample_sse.h \
		 src/speex_resampler.h \
		 src/unicode_support.h

libopusenc_la_SOURCES = \
	src/ogg_packer.c \
	src/opus_header.c \
	src/opusenc.c \
	src/picture.c \
	src/resample.c \
	src/unicode_support.c
libopusenc_la_LIBADD = $(DEPS_LIBS) $(lrintf_lib)
libopusenc_la_LDFLAGS = -no-undefined \
 -version-info @OP_LT_CURRENT@:@OP_LT_REVISION@:@OP_LT_AGE@

if OP_ENABLE_EXAMPLES
noinst_PROGRAMS = examples/opusenc_example
endif

examples_opusenc_example_SOURCES = examples/opusenc_example.c
examples_opusenc_example_LDADD = libopusenc.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libopusenc.pc

debug:
	$(MAKE) CFLAGS="${CFLAGS} -O0 -ggdb -DOP_ENABLE_ASSERTIONS" all

EXTRA_DIST = \
 libopusenc.pc.in \
 libopusenc-uninstalled.pc.in \
 doc/Doxyfile.in \
 doc/opus_logo.svg \
 doc/Makefile \
 win32/VS2015/opusenc.sln \
 win32/VS2015/opusenc.vcxproj \
 win32/VS2015/opusenc.vcxproj.filters \
 win32/VS2015/common.props \
 win32/genversion.bat \
 win32/config.h

# Targets to build and install just the library without the docs
libopusenc install-libopusenc: NO_DOXYGEN = 1

libopusenc: all
install-libopusenc: install

# Or just the docs
docs: doc/doxygen-build.stamp

install-docs:
	@if [ -z "$(NO_DOXYGEN)" ]; then \
	  ( cd doc && \
	    echo "Installing documentation in $(DESTDIR)$(docdir)"; \
	    $(INSTALL) -d $(DESTDIR)$(docdir)/html/search; \
	    for f in `find html -type f \! -name "installdox"` ; do \
	      $(INSTALL_DATA) $$f $(DESTDIR)$(docdir)/$$f;     \
	    done ) \
	fi

doc/doxygen-build.stamp: doc/Doxyfile $(top_srcdir)/doc/opus_logo.svg \
                         $(top_srcdir)/include/*.h
	@[ -n "$(NO_DOXYGEN)" ] || ( cd doc && doxygen && touch $(@F) )


if HAVE_DOXYGEN

# Or everything (by default)
all-local: docs

install-data-local: install-docs

clean-local:
	$(RM) -r doc/html
	$(RM) -r doc/latex
	$(RM) doc/doxygen-build.stamp

uninstall-local:
	$(RM) -r $(DESTDIR)$(docdir)/html

endif

# We check this every time make is run, with configure.ac being touched to
# trigger an update of the build system files if update_version changes the
# current PACKAGE_VERSION (or if package_version was modified manually by a
# user with either AUTO_UPDATE=no or no update_version script present - the
# latter being the normal case for tarball releases).
#
# We can't just add the package_version file to CONFIGURE_DEPENDENCIES since
# simply running autoconf will not actually regenerate configure for us when
# the content of that file changes (due to autoconf dependency checking not
# knowing about that without us creating yet another file for it to include).
#
# The MAKECMDGOALS check is a gnu-make'ism, but will degrade 'gracefully' for
# makes that don't support it.  The only loss of functionality is not forcing
# an update of package_version for `make dist` if AUTO_UPDATE=no, but that is
# unlikely to be a real problem for any real user.
$(top_srcdir)/configure.ac: force
	@case "$(MAKECMDGOALS)" in \
	    dist-hook)                             exit 0       ;; \
	    dist-* | dist | distcheck | distclean) _arg=release ;; \
	esac; \
	if ! $(top_srcdir)/update_version $$_arg 2> /dev/null; then \
	    if [ ! -e $(top_srcdir)/package_version ]; then \
		echo 'PACKAGE_VERSION="unknown"' > $(top_srcdir)/package_version; \
	    fi; \
	    . $(top_srcdir)/package_version || exit 1; \
	    [ "$(PACKAGE_VERSION)" != "$$PACKAGE_VERSION" ] || exit 0; \
	fi; \
	touch $@

force:

# Create a minimal package_version file when make dist is run.
dist-hook:
	echo 'PACKAGE_VERSION="$(PACKAGE_VERSION)"' > $(top_distdir)/package_version


.PHONY: libopusenc install-libopusenc docs install-docs

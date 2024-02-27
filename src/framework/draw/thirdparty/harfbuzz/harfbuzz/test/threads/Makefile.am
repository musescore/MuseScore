# Process this file with automake to produce Makefile.in

NULL =
EXTRA_DIST =
SUBDIRS =

EXTRA_DIST += \
	meson.build \
	hb-shape-threads.cc \
	hb-subset-threads.cc \
	$(NULL)

# Convenience targets:
lib:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/src lib

-include $(top_srcdir)/git.mk

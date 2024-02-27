# Process this file with automake to produce Makefile.in

NULL =
EXTRA_DIST =
CLEANFILES =
SUBDIRS = data

# Convenience targets:
lib: libs # Always build subsetter lib in this subdir
libs:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/src libs

EXTRA_DIST += \
	meson.build \
	run-tests.py \
	run-repack-tests.py \
	subset_test_suite.py \
	repack_test.py \
	$(NULL)

CLEANFILES += \
	subset_test_suite.py[co] \
	$(NULL)

-include $(top_srcdir)/git.mk

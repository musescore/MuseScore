# Process this file with automake to produce Makefile.in

NULL =
EXTRA_DIST =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

EXTRA_DIST += meson.build

include Makefile.sources

# Convenience targets:
lib:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/src lib
libs:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/src libs

bin_PROGRAMS =

AM_CPPFLAGS = \
	-DHB_DISABLE_DEPRECATED \
	-I$(top_srcdir)/src/ \
	-I$(top_builddir)/src/ \
	$(GLIB_CFLAGS) \
	$(FREETYPE_CFLAGS) \
	$(CAIRO_FT_CFLAGS) \
	$(CHAFA_CFLAGS) \
	$(NULL)
LDADD = \
	$(top_builddir)/src/libharfbuzz.la \
	-lm \
	$(GLIB_LIBS) \
	$(FREETYPE_LIBS) \
	$(NULL)

if HAVE_GLIB

if HAVE_CAIRO
hb_view_SOURCES = $(HB_VIEW_sources)
hb_view_LDADD = \
	$(top_builddir)/src/libharfbuzz-cairo.la \
	$(LDADD) \
	$(CAIRO_LIBS) \
	$(CAIRO_FT_LIBS) \
	$(CHAFA_LIBS) \
	$(NULL)
bin_PROGRAMS += hb-view
endif # HAVE_CAIRO

hb_shape_SOURCES = $(HB_SHAPE_sources)
bin_PROGRAMS += hb-shape

hb_info_SOURCES = $(HB_INFO_sources)
hb_info_LDADD = \
	$(LDADD) \
	$(NULL)
if HAVE_GOBJECT
hb_info_LDADD += \
	$(top_builddir)/src/libharfbuzz-gobject.la \
	$(GOBJECT_LIBS) \
	$(NULL)
endif # HAVE_GOBJECT
if HAVE_CHAFA
hb_info_LDADD += $(CHAFA_LIBS)
endif # HAVE_CHAFA
bin_PROGRAMS += hb-info

hb_subset_SOURCES = $(HB_SUBSET_CLI_sources)
hb_subset_LDADD = \
	$(top_builddir)/src/libharfbuzz-subset.la \
	$(LDADD)
bin_PROGRAMS += hb-subset

hb_ot_shape_closure_SOURCES = $(HB_OT_SHAPE_CLOSURE_sources)
bin_PROGRAMS += hb-ot-shape-closure

endif # HAVE_GLIB

#if HAVE_FONTCONFIG
#hb_fc_list_SOURCES = \
#	hb-fc.cc \
#	hb-fc.h \
#	hb-fc-list.c \
#	$(NULL)
#hb_fc_list_LDADD = \
#	$(LDADD) \
#	$(FONTCONFIG_LIBS) \
#	$(NULL)
#bin_PROGRAMS += hb-fc-list
#endif # HAVE_FONTCONFIG

-include $(top_srcdir)/git.mk

#!/usr/bin/make -f

# If this is set to true (via the environment) then CRC checking will be
# disabled in libogg giving fuzzers a better chance at finding something.
disable_ogg_crc ?= false

# Build libsndfile as a dynamic/shared library, but statically link to
# libFLAC, libogg, libopus and libvorbis

ogg_version = libogg-1.3.4
ogg_sha256sum = c163bc12bc300c401b6aa35907ac682671ea376f13ae0969a220f7ddf71893fe

vorbis_version = libvorbis-1.3.7
vorbis_sha256sum = b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b

flac_version = flac-1.3.3
flac_sha256sum = 213e82bd716c9de6db2f98bcadbc4c24c7e2efe8c75939a1a84e28539c4e1748

opus_version = opus-1.3.1
opus_sha256sum = 65b58e1e25b2a114157014736a3d9dfeaad8d41be1c8179866f144a2fb44ff9d

#-------------------------------------------------------------------------------
# Code follows.

ogg_tarball = $(ogg_version).tar.xz
vorbis_tarball = $(vorbis_version).tar.xz
flac_tarball = $(flac_version).tar.xz
opus_tarball = $(opus_version).tar.gz

download_url = http://downloads.xiph.org/releases/
tarball_dir = Build/Tarballs
stamp_dir = Build/Stamp

build_dir = $(shell pwd)/Build
config_options = --prefix=$(build_dir) --disable-shared --enable-option-checking

pwd = $(shell pwd)

help :
	@echo
	@echo "This script will build libsndfile as a dynamic/shared library but statically linked"
	@echo "to libFLAC, libogg and libvorbis. It should work on Linux and Mac OS X. It might"
	@echo "work on Windows with a correctly set up MinGW."
	@echo
	@echo "It requires all the normal build tools require to build libsndfile plus wget."
	@echo

config : Build/Stamp/configure

build : Build/Stamp/build

clean :
	rm -rf Build/flac-* Build/libogg-* Build/libvorbis-* Build/opus-*
	rm -rf Build/bin Build/include Build/lib Build/share
	rm -f Build/Stamp/install Build/Stamp/extract Build/Stamp/sha256sum Build/Stamp/build-ogg

Build/Stamp/init :
	mkdir -p $(stamp_dir) $(tarball_dir)
	touch $@

Build/Tarballs/$(flac_tarball) : Build/Stamp/init
	(cd $(tarball_dir) && wget $(download_url)flac/$(flac_tarball) -O $(flac_tarball))
	touch $@

Build/Tarballs/$(ogg_tarball) : Build/Stamp/init
	(cd $(tarball_dir) && wget $(download_url)ogg/$(ogg_tarball) -O $(ogg_tarball))
	touch $@

Build/Tarballs/$(vorbis_tarball) : Build/Stamp/init
	(cd $(tarball_dir) && wget $(download_url)vorbis/$(vorbis_tarball) -O $(vorbis_tarball))
	touch $@

Build/Tarballs/$(opus_tarball) : Build/Stamp/init
	(cd $(tarball_dir) && wget https://archive.mozilla.org/pub/opus/$(opus_tarball) -O $(opus_tarball))
	touch $@

Build/Stamp/tarballs : Build/Tarballs/$(flac_tarball) Build/Tarballs/$(ogg_tarball) Build/Tarballs/$(vorbis_tarball) Build/Tarballs/$(opus_tarball)
	touch $@

Build/Stamp/sha256sum : Build/Stamp/tarballs
	test `sha256sum $(tarball_dir)/$(ogg_tarball) | sed "s/ .*//"` = $(ogg_sha256sum)
	test `sha256sum $(tarball_dir)/$(vorbis_tarball) | sed "s/ .*//"` = $(vorbis_sha256sum)
	test `sha256sum $(tarball_dir)/$(flac_tarball) | sed "s/ .*//"` = $(flac_sha256sum)
	test `sha256sum $(tarball_dir)/$(opus_tarball) | sed "s/ .*//"` = $(opus_sha256sum)
	touch $@

Build/Stamp/extract : Build/Stamp/sha256sum
	# (cd Build && tar xf Tarballs/$(ogg_tarball))
	(cd Build && tar xf Tarballs/$(flac_tarball))
	(cd Build && tar xf Tarballs/$(vorbis_tarball))
	(cd Build && tar xf Tarballs/$(opus_tarball))
	touch $@

Build/Stamp/build-ogg : Build/Stamp/sha256sum
ifeq ($(disable_ogg_crc), true)
	echo "Ogg/CRC enabled"
	(cd Build && git clone https://github.com/xiph/ogg $(ogg_version))
	(cd Build/$(ogg_version) && ./autogen.sh && CFLAGS=-fPIC ./configure $(config_options) --disable-crc && make all install)
else
	echo "Ogg/CRC disabled"
	(cd Build && tar xf Tarballs/$(ogg_tarball))
	(cd Build/$(ogg_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
endif
	touch $@

Build/Stamp/install-libs : Build/Stamp/extract Build/Stamp/build-ogg
	(cd Build/$(vorbis_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
	(cd Build/$(flac_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
	(cd Build/$(opus_version) && CFLAGS=-fPIC ./configure $(config_options) && make all install)
	touch $@

configure : configure.ac
	./autogen.sh

Build/Stamp/configure : Build/Stamp/install-libs configure
	PKG_CONFIG_LIBDIR=Build/lib/pkgconfig ./configure
	sed -i 's#^EXTERNAL_XIPH_CFLAGS.*#EXTERNAL_XIPH_CFLAGS = -I$(pwd)/Build/include#' Makefile
	sed -i 's#^EXTERNAL_XIPH_LIBS.*#EXTERNAL_XIPH_LIBS = -static $(pwd)/Build/lib/libFLAC.la $(pwd)/Build/lib/libvorbis.la $(pwd)/Build/lib/libvorbisenc.la $(pwd)/Build/lib/libopus.la $(pwd)/Build/lib/libogg.la -dynamic #' Makefile
	make clean
	touch $@

Build/Stamp/build : Build/Stamp/configure
	make all check
	touch $@


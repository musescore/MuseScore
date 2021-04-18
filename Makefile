#=============================================================================
#  Mscore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2012 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

$(warning DEPRECATION NOTICE. This method of compiling will soon be removed.)
$(warning )
$(warning Please use 'build.cmake' instead:)
$(warning )
$(warning $    $$ ./build.cmake)
$(warning )
$(warning See https://github.com/musescore/MuseScore/pull/7531 for details.)
$(warning )

CPUS      := $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || getconf NPROCESSORS_ONLN 2>/dev/null || echo 1)

PREFIX    = "/usr/local"
VERSION   := $(shell cmake -P config.cmake | sed -n -e "s/^.*VERSION  *//p")

MUSESCORE_BUILD_CONFIG="dev"
MUSESCORE_REVISION=""
BUILD_NUMBER=""
TELEMETRY_TRACK_ID=""
CRASH_REPORT_URL=""

# Override SUFFIX and LABEL when multiple versions are installed to avoid conflicts.
SUFFIX=""# E.g.: SUFFIX="dev" --> "mscore" becomes "mscoredev"
LABEL=""# E.g.: LABEL="Development Build" --> "MuseScore 2" becomes "MuseScore 2 Development Build"

BUILD_JACK="OFF"      # Override with "ON" to enable.
BUILD_WEBENGINE="ON"  # Override with "OFF" to disable.
USE_SYSTEM_FREETYPE="OFF" # Override with "ON" to enable. Requires freetype >= 2.5.2.
COVERAGE="OFF"        # Override with "ON" to enable.
DOWNLOAD_SOUNDFONT="ON"   # Override with "OFF" to disable latest soundfont download.

UPDATE_CACHE="TRUE"# Override if building a DEB or RPM, or when installing to a non-standard location.
NO_RPATH="FALSE"# Package maintainers may want to override this (e.g. Debian)

BUILD_UNIT_TESTS="OFF"
BUILD_VST="OFF"
VST3_SDK_PATH=""

#
# change path to include your Qt5 installation
#
BINPATH      = "${PATH}"

release:
	if test ! -d build.release; then mkdir build.release; fi; \
      cd build.release;                          \
      export PATH=${BINPATH};                    \
      cmake -DCMAKE_BUILD_TYPE=RELEASE	       \
  	  -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}"       \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}"       \
  	  -DMSCORE_INSTALL_SUFFIX="${SUFFIX}"      \
  	  -DMUSESCORE_LABEL="${LABEL}"             \
	  -DMUSESCORE_BUILD_CONFIG="${MUSESCORE_BUILD_CONFIG}" \
	  -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
  	  -DCMAKE_BUILD_NUMBER="${BUILD_NUMBER}"   \
  	  -DTELEMETRY_TRACK_ID="${TELEMETRY_TRACK_ID}" \
  	  -DCRASH_REPORT_URL="${CRASH_REPORT_URL}" \
	  -DBUILD_JACK="${BUILD_JACK}"             \
   	  -DBUILD_WEBENGINE="${BUILD_WEBENGINE}"   \
   	  -DUSE_SYSTEM_FREETYPE="${USE_SYSTEM_FREETYPE}" \
   	  -DDOWNLOAD_SOUNDFONT="${DOWNLOAD_SOUNDFONT}"   \
	  -DBUILD_VST="${BUILD_VST}"         		\
	  -DVST3_SDK_PATH="${VST3_SDK_PATH}"         \
	  -DBUILD_UNIT_TESTS="${BUILD_UNIT_TESTS}" \
  	  -DCMAKE_SKIP_RPATH="${NO_RPATH}"     ..; \
      make lrelease;                             \
      make -j ${CPUS};                           \


#freetype:
#	cd build.debug; \
#      mkdir freetype; \
#      cd freetype; \
#      cmake ../../thirdparty/freetype; \
#      make -j ${CPUS}

debug:
	if test ! -d build.debug; then mkdir build.debug; fi; \
      cd build.debug;                                       \
      export PATH=${BINPATH};                               \
      cmake -DCMAKE_BUILD_TYPE=DEBUG	                    \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}"                  \
  	  -DMSCORE_INSTALL_SUFFIX="${SUFFIX}"                 \
  	  -DMUSESCORE_LABEL="${LABEL}"                        \
  	  -DCMAKE_BUILD_NUMBER="${BUILD_NUMBER}"              \
	  -DBUILD_JACK="${BUILD_JACK}"             	      \
   	  -DBUILD_WEBENGINE="${BUILD_WEBENGINE}"              \
   	  -DUSE_SYSTEM_FREETYPE="${USE_SYSTEM_FREETYPE}"      \
   	  -DCOVERAGE="${COVERAGE}"                          \
   	  -DDOWNLOAD_SOUNDFONT="${DOWNLOAD_SOUNDFONT}"      \
  	  -DCMAKE_SKIP_RPATH="${NO_RPATH}"     ..;            \
      make lrelease;                                        \
      make -j ${CPUS};                                      \


utests:
	if test ! -d build.debug; then mkdir build.debug; fi; \
      cd build.debug;                                       \
      export PATH=${BINPATH};                               \
      cmake -DCMAKE_BUILD_TYPE=DEBUG	                    \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}"                  \
  	  -DMSCORE_INSTALL_SUFFIX="${SUFFIX}"                 \
  	  -DMUSESCORE_LABEL="${LABEL}"                        \
  	  -DCMAKE_BUILD_NUMBER="${BUILD_NUMBER}"              \
	  -DBUILD_JACK="${BUILD_JACK}"             	      \
   	  -DBUILD_WEBENGINE="${BUILD_WEBENGINE}"              \
   	  -DUSE_SYSTEM_FREETYPE="${USE_SYSTEM_FREETYPE}"      \
   	  -DBUILD_UNIT_TESTS="${BUILD_UNIT_TESTS}" \
   	  -DDOWNLOAD_SOUNDFONT=OFF \
	  ..; \
      make -j ${CPUS};                                      \

#
#  win32
#     cross compile windows package
#     NOTE: there are some hardcoded path in CMake - files
#           will probably only work on my setup (ws)
#
win32:
	if test ! -d win32build;                         \
         then                                          \
            mkdir win32build;                          \
      	if test ! -d win32install;                 \
               then                                    \
                  mkdir win32install;                  \
            fi;                                        \
            cd win32build;                             \
            cmake -DCMAKE_TOOLCHAIN_FILE=../build/mingw32.cmake -DCMAKE_INSTALL_PREFIX=../win32install -DCMAKE_BUILD_TYPE=DEBUG  ..; \
            make lrelease;                             \
            make -j ${CPUS};                           \
            make install;                              \
            make package;                              \
         else                                          \
            echo "build directory win32build does already exist, please remove first"; \
         fi

#
# clean out of source build
#

clean:
	-rm -rf build.debug build.release
	-rm -rf win32build win32install

revision:
	@git rev-parse --short=7 HEAD | tr -d '\n' > local_build_revision.env

version:
	@echo ${VERSION}

install: release
	cd build.release \
	&& make install/strip \
	&& if [ ${UPDATE_CACHE} = "TRUE" ]; then \
	     update-mime-database "${PREFIX}/share/mime"; \
	     gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor"; \
	fi

# Portable target: build AppDir ready to be turned into a portable AppImage.
# Creating the AppImage requires https://github.com/probonopd/AppImageKit
# Portable target requires both build and runtime dependencies,
# if Qt is in a non-standard location then be sure to add its
# "bin" folder to PATH and "lib" folder to LD_LIBRARY_PATH. i.e.:
#   $  export $PATH="/path/to/Qt/bin:${PATH}"
#   $  export $LD_LIBRARY_PATH="/path/to/Qt/lib:${LD_LIBRARY_PATH}"
#   $  make portable
# PREFIX sets install location *and* the name of the resulting AppDir.
# Version is appended to PREFIX in CMakeLists.txt if MSCORE_UNSTABLE=FALSE.
portable: PREFIX=MuseScore
portable: SUFFIX=-portable
portable: LABEL=Portable AppImage
portable: NO_RPATH=TRUE
portable: UPDATE_CACHE=FALSE
portable: install
	build_dir="$$(pwd)/build.release" && cd "$$(cat $${build_dir}/PREFIX.txt)" \
	&& [ -L usr ] || ln -s . usr && mscore="mscore${SUFFIX}" \
	&& dsktp="$${mscore}.desktop" icon="$${mscore}.svg" mani="install_manifest.txt" \
	&& cp "share/applications/$${dsktp}" "$${dsktp}" \
	&& cp "share/icons/hicolor/scalable/apps/$${icon}" "$${icon}" \
	&& <"$${build_dir}/$${mani}" >"$${mani}" \
	   sed -rn 's/.*(share\/)(man|mime|icons|applications)(.*)/\1\2\3/p'

installdebug: debug
	cd build.debug \
	&& make install

uninstall:
	cd build.release \
	&& xargs rm < install_manifest.txt \
	&& if [ ${UPDATE_CACHE} = "TRUE" ]; then \
	     update-mime-database "${PREFIX}/share/mime"; \
	     gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor"; \
	   fi \
	&& xargs ../build/rm-empty-dirs < install_manifest.txt \
	&& rm install_manifest.txt

uninstalldebug:
	cd build.debug \
	&& xargs rm < install_manifest.txt \
	&& if [ ${UPDATE_CACHE} = "TRUE" ]; then \
	     update-mime-database "${PREFIX}/share/mime"; \
	     gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor"; \
	   fi \
	&& xargs ../build/rm-empty-dirs < install_manifest.txt \
	&& rm install_manifest.txt

#
#  linux
#     linux binary package build
#
unix:
	if test ! -d linux;                          \
         then                                      \
            mkdir linux;                           \
            cd linux; \
            cmake -DCMAKE_BUILD_TYPE=RELEASE  ../mscore; \
            make -j${CPUS} -f Makefile;            \
            make package;                          \
         else                                      \
            echo "build directory linux does already exist, please remove first";  \
         fi

zip:
	zip -q -r MuseScore-${VERSION}.zip * -x .git\* -x vtest/html\*

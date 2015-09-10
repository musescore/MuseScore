#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
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

REVISION  = `cat mscore/revision.h`
CPUS      = `grep -c processor /proc/cpuinfo`
# Avoid build errors when processor=0 (as in m68k)
ifeq ($(CPUS), 0)
  CPUS=1
endif

PREFIX    = "/usr/local"
VERSION   = "2.0.3b-${REVISION}"
#VERSION = 2.0.2

# Override SUFFIX and LABEL when multiple versions are installed to avoid conflicts.
SUFFIX=""# E.g.: SUFFIX="dev" --> "mscore" becomes "mscoredev"
LABEL=""# E.g.: LABEL="Development Build" --> "MuseScore 2" becomes "MuseScore 2 Development Build"

BUILD_LAME="ON"# Non-free, required for MP3 support. Override with "OFF" to disable.
UPDATE_CACHE="TRUE"# Override if building a DEB or RPM, or when installing to a non-standard location.
NO_RPATH="FALSE"# Package maintainers may want to override this (e.g. Debian)

#
# change path to include your Qt5 installation
#
BINPATH      = ${PATH}

release:
	if test ! -d build.release; then mkdir build.release; fi; \
      cd build.release;                          \
      export PATH=${BINPATH};                    \
      cmake -DCMAKE_BUILD_TYPE=RELEASE	       \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}"       \
  	  -DMSCORE_INSTALL_SUFFIX="${SUFFIX}"      \
  	  -DMUSESCORE_LABEL="${LABEL}"             \
  	  -DBUILD_LAME="${BUILD_LAME}"             \
  	  -DCMAKE_SKIP_RPATH="${NO_RPATH}"     ..; \
      make lrelease;                             \
      make manpages;                             \
      make mscore_alias;                         \
      make -j ${CPUS};                           \


debug:
	if test ! -d build.debug; then mkdir build.debug; fi; \
      cd build.debug;                                       \
      export PATH=${BINPATH};                               \
      cmake -DCMAKE_BUILD_TYPE=DEBUG	                    \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}"                  \
  	  -DMSCORE_INSTALL_SUFFIX="${SUFFIX}"                 \
  	  -DMUSESCORE_LABEL="${LABEL}"                        \
  	  -DBUILD_LAME="${BUILD_LAME}"                        \
  	  -DCMAKE_SKIP_RPATH="${NO_RPATH}"     ..;            \
      make lrelease;                                        \
      make manpages;                                        \
      make mscore_alias;                                    \
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
            echo "build directory win32build does alread exist, please remove first"; \
         fi

#
# clean out of source build
#

clean:
	-rm -rf build.debug build.release
	-rm -rf win32build win32install

revision:
	@git rev-parse --short HEAD > mscore/revision.h

version:
	@echo ${VERSION}

install: release
	cd build.release \
	&& make install/strip \
	&& if [ ${UPDATE_CACHE} = "TRUE" ]; then \
	     update-mime-database "${PREFIX}/share/mime"; \
	     gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor"; \
	fi

installdebug: debug
	cd build.debug \
	&& make install \
	&& if [ ${UPDATE_CACHE} = "TRUE" ]; then \
	     update-mime-database "${PREFIX}/share/mime"; \
	     gtk-update-icon-cache -f -t "${PREFIX}/share/icons/hicolor"; \
	fi

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
            echo "build directory linux does alread exist, please remove first";  \
         fi

doxy:
	doxygen build.debug/Doxyfile
doxylib:
	doxygen build.debug/Doxyfile-LibMscore



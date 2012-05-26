#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2007 by Werner Schweer and others
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

PREFIX    = "/usr/local"
VERSION   = "2.0b${REVISION}"
#VERSION   = 2.0

ROOT=`pwd`

release:
	mkdir build.release;                       \
  cd build.release;                          \
  cmake -DCMAKE_BUILD_TYPE=RELEASE	       \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  	   ..; 			               \
  make lrelease;                             \
  make -j ${CPUS};                           \


debug:
	mkdir build.debug;                         \
  cd build.debug;                            \
  cmake -DCMAKE_BUILD_TYPE=DEBUG	       \
  	  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
  	   ..; 		             \
  make lrelease;                             \
  make -j ${CPUS};                           \


qt5:
	if test ! -d qt5;                        \
         then                                          \
            mkdir qt5;                         \
            cd qt5;                            \
            export PATH=/home/ws/qt/qt5/qtbase/bin:${PATH};        \
            cmake -DCMAKE_BUILD_TYPE=DEBUG -DBUILD_SCRIPTGEN=NO        \
            	  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
            	   ..; 		             \
            make -j ${CPUS};                           \
         else                                          \
            echo "build directory does already exist, please remove first with 'make clean'";       \
         fi

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
	@svnversion -n  > mscore/mscore/revision.h

version: revision
	@echo ${VERSION}

install: release revision
	cd build.release; make install

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
	doxygen -f build/Doxyfile.in



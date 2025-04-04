# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Adapted from KDQtInstallPaths.cmake, from the KDAB CMake modules

if(TARGET Qt::qmake)
    get_target_property(QT_QMAKE_EXECUTABLE Qt${Qt_VERSION_MAJOR}::qmake LOCATION)
else()
    message(FATAL_ERROR "No supported Qt version found. Make sure you find Qt before calling this")
endif()

execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE ALL_VARS
)
if(NOT return_code EQUAL 0)
    message(WARNING "Failed call: ${QMAKE_EXECUTABLE} -query")
    message(FATAL_ERROR "QMake call failed: ${return_code}")
endif()

string(REPLACE "\n" ";" VARS_LIST ${ALL_VARS})
foreach(QVAL ${VARS_LIST})
    if(QVAL MATCHES "QT_INSTALL_")
        string(REPLACE ":" ";" QVAL_LIST ${QVAL})
        list(LENGTH QVAL_LIST listlen)
        list(GET QVAL_LIST 0 var)
        if(WIN32 AND ${listlen} GREATER 2)
            list(GET QVAL_LIST 2 path)
            list(GET QVAL_LIST 1 drive)
            set(path "${drive}:${path}")
        else()
            list(GET QVAL_LIST 1 path)
        endif()
        if(NOT ${var}) #if set already on the command line for example
            set(${var} ${path} CACHE PATH "Qt install path for ${var}")
        endif()
    endif()
endforeach()

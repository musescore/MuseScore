#
# SPDX-FileCopyrightText: 2016-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Allen Winter <allen.winter@kdab.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#
# Create variables for all the various install paths for the Qt version in use
# Make sure to have found Qt before using this.
# sets variables like QT_INSTALL_PREFIX, QT_INSTALL_DATA, QT_INSTALL_DOCS, etc.
# run qmake -query to see a full list

if(TARGET Qt${QT_MAJOR_VERSION}::qmake)
  get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_MAJOR_VERSION}::qmake LOCATION)
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
    if(NOT ${var}) #if set aleady on the command line for example
      set(${var} ${path} CACHE PATH "Qt install path for ${var}")
    endif()
  endif()
endforeach()

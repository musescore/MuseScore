# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited and others
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_testflow ALIAS muse::testflow)

set(MODULE_QRC testflow.qrc)

target_sources(muse_testflow PRIVATE
    testflowmodule.cpp
    testflowmodule.h
    testflowtypes.h
    itestflow.h
    itestcasecontext.h
    itestflowconfiguration.h
    itestflowscriptsrepository.h
    testflowutils.h

    internal/testflow.cpp
    internal/testflow.h
    internal/testflowconfiguration.cpp
    internal/testflowconfiguration.h
    internal/testcasecontext.cpp
    internal/testcasecontext.h
    internal/testcaserunner.cpp
    internal/testcaserunner.h
    internal/testcasereport.cpp
    internal/testcasereport.h
    internal/testflowactions.cpp
    internal/testflowactions.h
    internal/testflowactionscontroller.cpp
    internal/testflowactionscontroller.h
    internal/testflowscriptsrepository.cpp
    internal/testflowscriptsrepository.h
    internal/draw/abpaintprovider.cpp
    internal/draw/abpaintprovider.h
    internal/scriptengine.cpp
    internal/scriptengine.h
    internal/jsmoduleloader.cpp
    internal/jsmoduleloader.h
    internal/testflowinteractive.cpp
    internal/testflowinteractive.h

    internal/api/scriptapi.cpp
    internal/api/scriptapi.h
    internal/api/testflowapi.cpp
    internal/api/testflowapi.h
    internal/api/contextapi.cpp
    internal/api/contextapi.h
)

qt_add_resources(muse_testflow muse_testflow_resources
    PREFIX
        /
    FILES
        examples/NewScore.js
        examples/PutNote.js
        examples/SimpleZoom.js
        examples/steps/NewScore.js
)

target_link_libraries(muse_testflow PRIVATE muse::draw Qt::Quick)

if (MUSE_MODULE_TESTFLOW_QML)
    add_subdirectory(qml/Muse/Testflow)
endif()

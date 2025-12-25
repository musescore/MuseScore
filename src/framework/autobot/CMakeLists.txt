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

muse_create_module(muse_autobot ALIAS muse::autobot)

set(MODULE_QRC autobot.qrc)

target_sources(muse_autobot PRIVATE
    autobotmodule.cpp
    autobotmodule.h
    autobottypes.h
    iautobot.h
    itestcasecontext.h
    iautobotconfiguration.h
    iautobotscriptsrepository.h
    autobotutils.h

    internal/autobot.cpp
    internal/autobot.h
    internal/autobotconfiguration.cpp
    internal/autobotconfiguration.h
    internal/testcasecontext.cpp
    internal/testcasecontext.h
    internal/testcaserunner.cpp
    internal/testcaserunner.h
    internal/testcasereport.cpp
    internal/testcasereport.h
    internal/autobotactions.cpp
    internal/autobotactions.h
    internal/autobotactionscontroller.cpp
    internal/autobotactionscontroller.h
    internal/autobotscriptsrepository.cpp
    internal/autobotscriptsrepository.h
    internal/draw/abpaintprovider.cpp
    internal/draw/abpaintprovider.h
    internal/scriptengine.cpp
    internal/scriptengine.h
    internal/jsmoduleloader.cpp
    internal/jsmoduleloader.h
    internal/autobotinteractive.cpp
    internal/autobotinteractive.h

    internal/api/scriptapi.cpp
    internal/api/scriptapi.h
    internal/api/autobotapi.cpp
    internal/api/autobotapi.h
    internal/api/contextapi.cpp
    internal/api/contextapi.h
)

qt_add_resources(muse_autobot muse_autobot_resources
    PREFIX
        /
    FILES
        examples/NewScore.js
        examples/PutNote.js
        examples/SimpleZoom.js
        examples/steps/NewScore.js
)

target_link_libraries(muse_autobot PRIVATE muse::draw Qt::Quick)

if (MUSE_MODULE_AUTOBOT_QML)
    add_subdirectory(qml/Muse/Autobot)
endif()

# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

muse_create_module(muse_learn ALIAS muse::learn)

target_sources(muse_learn PRIVATE 
    learnmodule.cpp
    learnmodule.h
    learnerrors.h
    ilearnconfiguration.h
    ilearnservice.h
    internal/learnconfiguration.cpp
    internal/learnconfiguration.h
    internal/learnservice.cpp
    internal/learnservice.h
)

if (QT_SUPPORT)
    target_link_libraries(muse_learn PRIVATE Qt::Concurrent Qt::Network Qt::Qml)
endif()

add_subdirectory(qml/Muse/Learn)

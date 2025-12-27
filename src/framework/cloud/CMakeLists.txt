# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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

muse_create_module(muse_cloud ALIAS muse::cloud)

target_sources(muse_cloud PRIVATE
    cloudmodule.cpp
    cloudmodule.h
    iauthorizationservice.h
    icloudconfiguration.h
    cloudtypes.h
    clouderrors.h

    internal/cloudconfiguration.h
    internal/cloudconfiguration.cpp
    internal/oauthhttpserverreplyhandler.h
    internal/oauthhttpserverreplyhandler.cpp
    internal/abstractcloudservice.h
    internal/abstractcloudservice.cpp
    musescorecom/imusescorecomservice.h
    audiocom/iaudiocomservice.h
    audiocom/audiocomservice.h
    audiocom/audiocomservice.cpp
)

if (MUSE_MODULE_CLOUD_MUSESCORECOM)
    target_sources(muse_cloud PRIVATE
        musescorecom/musescorecomservice.cpp
        musescorecom/musescorecomservice.h
    )
    target_compile_definitions(muse_cloud PUBLIC MUSE_MODULE_CLOUD_MUSESCORECOM)
endif()

target_link_libraries(muse_cloud PRIVATE Qt::Network Qt::NetworkAuth)

if (MUSE_MODULE_CLOUD_QML)
    add_subdirectory(qml/Muse/Cloud)
endif()

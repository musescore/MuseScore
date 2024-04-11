# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

declare_module(muse_cloud)
set(MODULE_ALIAS muse::cloud)

set(MODULE_QRC cloud.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/cloudmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cloudmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iauthorizationservice.h
    ${CMAKE_CURRENT_LIST_DIR}/icloudconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/cloudtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/cloudqmltypes.h
    ${CMAKE_CURRENT_LIST_DIR}/clouderrors.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/cloudconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/cloudconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/oauthhttpserverreplyhandler.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/oauthhttpserverreplyhandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractcloudservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractcloudservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musescorecom/imusescorecomservice.h
    ${CMAKE_CURRENT_LIST_DIR}/audiocom/iaudiocomservice.h
    ${CMAKE_CURRENT_LIST_DIR}/audiocom/audiocomservice.h
    ${CMAKE_CURRENT_LIST_DIR}/audiocom/audiocomservice.cpp

    ${CMAKE_CURRENT_LIST_DIR}/view/cloudsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/cloudsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/musescorecomauthorizationmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/musescorecomauthorizationmodel.cpp
    )

if (MUSE_MODULE_CLOUD_MUSESCORECOM)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/musescorecom/musescorecomservice.h
        ${CMAKE_CURRENT_LIST_DIR}/musescorecom/musescorecomservice.cpp
    )
    set(MODULE_DEF ${MODULE_DEF} -DMUSE_MODULE_CLOUD_MUSESCORECOM)
endif()

setup_module()

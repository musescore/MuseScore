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

muse_create_module(muse_uicomponents ALIAS muse::uicomponents)

target_sources(muse_uicomponents PRIVATE
    uicomponentsmodule.cpp
    uicomponentsmodule.h
    view/modelutils.h
    view/topleveldialog.cpp
    view/topleveldialog.h
    view/widgets/radiobuttongroupbox.cpp
    view/widgets/radiobuttongroupbox.h
    view/quickpaintedview.cpp
    view/quickpaintedview.h
    view/widgetview.cpp
    view/widgetview.h
)

add_subdirectory(view/treeview)

if(QT_SUPPORT)
    target_link_libraries(muse_uicomponents PRIVATE Qt::Widgets Qt::Quick Qt::QmlIntegration)
endif()

qt_add_resources(muse_uicomponents "muse_uicomponents_api"
    PREFIX 
        "/"
    FILES
        "api/MuseApi/Controls/qmldir"
)

add_subdirectory(qml/Muse/UiComponents)
add_subdirectory(qml/Muse/GraphicalEffects)

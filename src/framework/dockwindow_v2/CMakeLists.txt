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

muse_create_module(muse_dockwindow ALIAS muse::dockwindow)

target_sources(muse_dockwindow PRIVATE
    dockmodule.cpp
    dockmodule.h
    docktypes.h
    idockwindow.h
    idockwindowprovider.h

    internal/dockbase.cpp
    internal/dockbase.h
    internal/dockseparator.cpp
    internal/dockseparator.h
    internal/docktabbar.cpp
    internal/docktabbar.h
    internal/docktitlebar.cpp
    internal/docktitlebar.h
    internal/dockwindowactionscontroller.cpp
    internal/dockwindowactionscontroller.h
    internal/dockwindowprovider.cpp
    internal/dockwindowprovider.h
    internal/dropcontroller.cpp
    internal/dropcontroller.h
    )

###########################################
# Setup the KDDockWidgets lib
###########################################
target_link_libraries(muse_dockwindow PRIVATE kddockwidgets)

get_property(kddockwidgets_src_dir GLOBAL PROPERTY kddockwidgets_SOURCE_DIR)
target_include_directories(muse_dockwindow SYSTEM PRIVATE
    ${kddockwidgets_src_dir}
    ${kddockwidgets_src_dir}/kddockwidgets/src
    ${kddockwidgets_src_dir}/kddockwidgets/src/3rdparty
    ${kddockwidgets_src_dir}/kddockwidgets/src/3rdparty/nlohmann
)

if (MUSE_MODULE_DOCKWINDOW_QML)
    add_subdirectory(qml/Muse/Dock)
endif()

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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(palette)

target_sources(palette PRIVATE
    palettemodule.cpp
    palettemodule.h
    ipaletteconfiguration.h
    palettetypes.h

    internal/paletteconfiguration.cpp
    internal/paletteconfiguration.h
    internal/paletteworkspacesetup.cpp
    internal/paletteworkspacesetup.h
    internal/paletteuiactions.cpp
    internal/paletteuiactions.h
    internal/paletteactionscontroller.cpp
    internal/paletteactionscontroller.h

    internal/ipaletteprovider.h
    internal/paletteprovider.cpp
    internal/paletteprovider.h
    internal/palettemodel.cpp
    internal/palettemodel.h
    internal/palettecreator.cpp
    internal/palettecreator.h
    internal/palettetree.cpp
    internal/palettetree.h
    internal/palette.cpp
    internal/palette.h
    internal/palettecell.cpp
    internal/palettecell.h
    internal/palettecelliconengine.cpp
    internal/palettecelliconengine.h
    internal/mimedatautils.h
    internal/palettecompat.h
    internal/palettecompat.cpp

    widgets/palettewidget.h
    widgets/palettewidget.cpp
    widgets/masterpalette.h
    widgets/masterpalette.cpp
    widgets/timedialog.h
    widgets/timedialog.cpp
    widgets/timesignaturepropertiesdialog.h
    widgets/timesignaturepropertiesdialog.cpp
    widgets/keyedit.h
    widgets/keyedit.cpp
    widgets/keycanvas.h
    widgets/noteGroups.h
    widgets/noteGroups.cpp
    widgets/notegroupsexampleview.cpp
    widgets/notegroupsexampleview.h
    widgets/symboldialog.h
    widgets/symboldialog.cpp
    widgets/specialcharactersdialog.cpp
    widgets/specialcharactersdialog.h
    widgets/customizekitdialog.h
    widgets/customizekitdialog.cpp
    widgets/drumsetpalette.cpp
    widgets/drumsetpalette.h
)

target_link_libraries(palette PRIVATE
    engraving
    Qt::StateMachine
    Qt::Widgets
)

if (MUE_BUILD_PALETTE_QML)
    add_subdirectory(qml/MuseScore/Palette)
endif()

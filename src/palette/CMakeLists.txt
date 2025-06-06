# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

declare_module(palette)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/palette.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${WIDGETS_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/palettemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/palettemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ipaletteconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/palettetypes.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteworkspacesetup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteworkspacesetup.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteactionscontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/ipaletteprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecreator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettetree.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettetree.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palette.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/palette.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecell.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecelliconengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecelliconengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mimedatautils.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecompat.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palettecompat.cpp

    ${CMAKE_CURRENT_LIST_DIR}/view/paletterootmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/paletterootmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/palettemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettepropertiesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettepropertiesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/palettecellpropertiesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettecellpropertiesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/palettespanelcontextmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettespanelcontextmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/drumsetpanelview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/drumsetpanelview.cpp

    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/palettewidget.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/palettewidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/masterpalette.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/masterpalette.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/timedialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/timedialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/timesignaturepropertiesdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/timesignaturepropertiesdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/keyedit.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/keyedit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/keycanvas.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/noteGroups.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/noteGroups.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/notegroupsexampleview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/notegroupsexampleview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/symboldialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/symboldialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/specialcharactersdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/specialcharactersdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/customizekitdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/customizekitdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/drumsetpalette.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/drumsetpalette.h
    )

set(MODULE_LINK
    engraving
    commonscene
    Qt::Quick
    Qt::StateMachine
    Qt::Widgets
    )

setup_module()

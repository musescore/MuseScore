/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

ListView {

    height: contentItem.childrenRect.height
    width: contentWidth
    orientation: Qt.Horizontal

    model: appMenuModel.items

    AppMenuModel {
        id: appMenuModel
    }

    Component.onCompleted: {
        appMenuModel.load()
    }

    QtObject {
        id: prv

        property var showedMenu: null
    }

    delegate: FlatButton {
        id: radioButtonDelegate

        width: 60

        normalStateColor: !menu.isOpened ? "transparent" : ui.theme.accentColor
        hoveredStateColor: ui.theme.accentColor
        text: modelData["title"]

        mouseArea.onContainsMouseChanged: {
            if (!mouseArea.containsMouse || !prv.showedMenu ||
                prv.showedMenu === menu) {
                return
            }

            toggleMenuOpened()
        }

        onClicked: {
            toggleMenuOpened()
        }

        function toggleMenuOpened() {
            if (prv.showedMenu && prv.showedMenu !== menu) {
                prv.showedMenu.close()
            }

            menu.toggleOpened()
        }

        StyledMenu {
            id: menu

            model: modelData["subitems"]

            onOpened: {
                prv.showedMenu = menu
            }

            onClosed: {
                prv.showedMenu = null
            }

            onHandleAction: {
                Qt.callLater(appMenuModel.handleAction, actionCode, actionIndex)
                menu.close()
            }
        }
    }
}

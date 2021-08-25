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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

ListView {

    property alias navigation: navPanel

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

    NavigationPanel {
        id: navPanel
        name: "AppMenuBar"
    }

    delegate: FlatButton {
        id: radioButtonDelegate

        property var item: Boolean(modelData) ? modelData : null

        width: 60

        normalStateColor: !menuLoader.isMenuOpened ? "transparent" : ui.theme.accentColor
        hoveredStateColor: ui.theme.accentColor
        text: Boolean(item) ? item.title : ""

        navigation.panel: navPanel
        navigation.order: index

        mouseArea.onContainsMouseChanged: {
            if (!mouseArea.containsMouse || !prv.showedMenu ||
                    prv.showedMenu === menuLoader.menu) {
                return
            }

            toggleMenuOpened()
        }

        onClicked: {
            toggleMenuOpened()
        }

        function toggleMenuOpened() {
            if (prv.showedMenu && prv.showedMenu !== menuLoader.menu) {
                prv.showedMenu.close()
            }

            menuLoader.toggleOpened(item.subitems)

            if (menuLoader.isMenuOpened) {
                prv.showedMenu = menuLoader.menu
            } else {
                prv.showedMenu = null
            }
        }

        StyledMenuLoader {
            id: menuLoader

            onHandleMenuItem: {
                Qt.callLater(appMenuModel.handleMenuItem, item.id)
            }
        }
    }
}

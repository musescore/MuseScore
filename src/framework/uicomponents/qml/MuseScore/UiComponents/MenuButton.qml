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

FlatButton {
    id: root

    property var menuModel
    property alias isMenuOpened: menuLoader.isMenuOpened

    signal handleAction(string actionCode, int actionIndex)

    function toggleMenu(item, x, y) {
        menuLoader.parent = item
        menuLoader.toggleOpened(menuModel, null, x, y)
    }

    enabled: visible

    icon: IconCode.MENU_THREE_DOTS
    normalStateColor: isMenuOpened ? ui.theme.accentColor : "transparent"

    StyledMenuLoader {
        id: menuLoader

        menuAnchorItem: ui.rootItem

        onHandleAction: {
            root.handleAction(actionCode, actionIndex)
        }
    }

    onClicked: {
        menuLoader.parent = root
        menuLoader.toggleOpened(root.menuModel, root.navigation)
    }
}

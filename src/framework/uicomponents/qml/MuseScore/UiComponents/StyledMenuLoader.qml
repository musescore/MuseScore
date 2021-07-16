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

Loader {

    id: loader

    signal handleAction(string actionCode, int actionIndex)

    property alias menu: loader.item
    property var menuAnchorItem: null

    property bool isMenuOpened: Boolean(loader.menu) && loader.menu.isOpened

    function toggleOpened(model, navigationParentControl, x = 0, y = 0) {
        if (!prv.initMenu(model, navigationParentControl)) {
            return
        }

        if (x !== 0) {
            loader.menu.x = x
        }

        if (y !== 0) {
            loader.menu.y = y
        }

        prv.openMenu()
    }

    function toggleOpenedWithAlign(model, navigationParentControl, align) {
        if (!prv.initMenu(model, navigationParentControl)) {
            return
        }

        loader.menu.preferredAlign = align

        prv.openMenu()
    }

    function unloadMenu() {
        loader.sourceComponent = null
    }

    QtObject {
        id: prv

        function initMenu(model, navigationParentControl) {
            if (!loader.sourceComponent) {
                loader.sourceComponent = itemMenuComp
            }

            var menu = loader.menu
            if (menu.isOpened) {
                menu.close()
                return false
            }

            menu.parent = loader.parent
            if (navigationParentControl) {
                menu.navigationParentControl = navigationParentControl
                menu.navigation.name = navigationParentControl.name + "PopupMenu"
            }
            menu.anchorItem = menuAnchorItem
            menu.model = model

            return true
        }

        function openMenu() {
            loader.menu.open()

            if (!loader.menu.focusOnSelected()) {
                loader.menu.focusOnFirstItem()
            }
        }
    }

    Component {
        id: itemMenuComp
        StyledMenu {
            id: itemMenu

            onHandleAction: {
                Qt.callLater(loader.handleAction, actionCode, actionIndex)
                itemMenu.close()
            }

            onClosed: {
                Qt.callLater(loader.unloadMenu)
            }
        }
    }
}

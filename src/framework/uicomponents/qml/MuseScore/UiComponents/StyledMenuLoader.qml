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

    QtObject {
        id: prv

        function loadMenu() {
            if (!loader.sourceComponent) {
                loader.sourceComponent = itemMenuComp
            }
        }

        function unloadMenu() {
            loader.sourceComponent = null
        }
    }

    function open(model, navigationParentControl, x = 0, y = 0) {
        prv.loadMenu()

        var menu = loader.menu
        menu.parent = loader.parent
        if (navigationParentControl) {
            menu.navigationParentControl = navigationParentControl
            menu.navigation.name = navigationParentControl.name + "PopupMenu"
        }
        menu.anchorItem = menuAnchorItem

        update(model, x, y)
        menu.open()

        if (!menu.focusOnSelected()) {
            menu.focusOnFirstItem()
        }
    }

    function toggleOpened(model, navigationParentControl, x = 0, y = 0) {
        prv.loadMenu()

        var menu = loader.menu
        if (menu.isOpened) {
            menu.close()
            return
        }

        open(model, navigationParentControl, x, y)
    }

    function toggleOpenedWithAlign(model, navigationParentControl, align) {
        prv.loadMenu()

        loader.menu.preferredAlign = align

        toggleOpened(model, navigationParentControl)
    }

    function close() {
        if (loader.isMenuOpened) {
            loader.menu.close()
        }
    }

    function update(model, x = 0, y = 0) {
        var menu = loader.menu
        if (!Boolean(menu)) {
            return
        }

        menu.model = model

        if (x !== 0) {
            menu.x = x
        }

        if (y !== 0) {
            menu.y = y
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
                Qt.callLater(prv.unloadMenu)
            }
        }
    }
}

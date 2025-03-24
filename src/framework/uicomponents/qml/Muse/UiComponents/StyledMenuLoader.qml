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

import Muse.UiComponents 1.0

import "internal"

Loader {
    id: loader

    signal handleMenuItem(string itemId)
    signal openPrevMenu()
    signal openNextMenu()
    signal opened()
    signal closed(bool force)

    property StyledMenu menu: loader.item as StyledMenu
    property Item menuAnchorItem: null
    property bool hasSiblingMenus: false

    property alias isMenuOpened: loader.active

    property string accessibleName: ""

    QtObject {
        id: prv

        function loadMenu() {
            loader.active = true
        }

        function unloadMenu(force) {
            loader.active = false
            Qt.callLater(loader.closed, force)
        }
    }

    active: false

    sourceComponent: StyledMenu {
        id: itemMenu

        openPolicies: PopupView.NoActivateFocus
        focusPolicies: PopupView.NoFocus

        accessibleName: loader.accessibleName

        onHandleMenuItem: function(itemId) {
            itemMenu.close()
            Qt.callLater(loader.handleMenuItem, itemId)
        }

        onOpenPrevMenu: {
           Qt.callLater(loader.openPrevMenu)
        }  

        onOpenNextMenu: {
            Qt.callLater(loader.openNextMenu)
        }

        onClosed: function(force) {
            Qt.callLater(prv.unloadMenu, force)
        }

        onOpened: {
            focusOnOpenedMenuTimer.start()
        }
    }

    function open(model, x = -1, y = -1) {
        prv.loadMenu()

        var menu = loader.menu
        menu.parent = loader.parent
        menu.anchorItem = menuAnchorItem

        update(model, x, y)
        menu.open()

        Qt.callLater(loader.opened)
    }

    function toggleOpened(model, x = -1, y = -1) {
        prv.loadMenu()

        var menu = loader.menu
        if (menu.isOpened) {
            menu.close()
            return
        }

        open(model, x, y)
    }

    function toggleOpenedWithAlign(model, align) {
        prv.loadMenu()

        loader.menu.preferredAlign = align

        toggleOpened(model)
    }

    function close() {
        if (loader.isMenuOpened) {
            loader.menu.close()
        }
    }

    function update(model, x = -1, y = -1) {
        var menu = loader.menu
        if (!Boolean(menu)) {
            return
        }

        menu.closeSubMenu()

        if (x !== -1) {
            menu.x = x
        }

        if (y !== -1) {
            menu.y = y
        }

        menu.model = model

        menu.calculateSize()
    }

    Timer {
        id: focusOnOpenedMenuTimer

        interval: 50
        running: false
        repeat: false

        onTriggered: {
            if (loader.menu) {
                loader.menu.requestFocus()
            }
        }
    }
}

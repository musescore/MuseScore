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
import Qt.labs.platform 1.1 as PLATFORM

import MuseScore.AppShell 1.0

Item {
    readonly property bool available: menuModel.isGlobalMenuAvailable()

    PLATFORM.MenuBar {
        id: menuBar
    }

    PlatformAppMenuModel {
        id: menuModel
    }

    function load() {
        menuModel.load()

        var items = menuModel.items
        for (var i in items) {
            var item = items[i]
            var menu = makeMenu(item)

            for (var j in item.subitems) {
                var menuItem = makeMenuItem(menu, item.subitems[j])
                menu.addItem(menuItem)
            }

            item.subitemsChanged.connect(function(subitems, menuId) {
                for (var l in menuBar.menus) {
                    var menu = menuBar.menus[l]
                    if (menu.id === menuId) {
                        menuBar.menus[l].subitems = subitems
                    }
                }
            })

            menuBar.addMenu(menu)
        }

        menuModel.itemsChanged.connect(function() {
            for (var i in menuModel.items) {
                menuBar.menus[i].subitems = menuModel.items[i].subitems
            }
        })
    }

    function makeMenu(menuInfo) {
        var menu = menuComponent.createObject(menuBar)

        menu.id = menuInfo.id
        menu.title = menuInfo.title
        menu.enabled = menuInfo.enabled
        menu.subitems = menuInfo.subitems

        return menu
    }

    function makeMenuItem(parentMenu, itemInfo) {
        var menuItem = menuItemComponent.createObject(parentMenu)

        menuItem.id = itemInfo.id
        menuItem.text = itemInfo.title + "\t" + itemInfo.portableShortcuts
        menuItem.enabled = itemInfo.enabled
        menuItem.checked = itemInfo.checked
        menuItem.checkable = itemInfo.checkable
        menuItem.separator = !Boolean(itemInfo.title)
        menuItem.role = itemInfo.role

        return menuItem
    }

    Component {
        id: menuComponent

        PLATFORM.Menu {
            property string id: ""
            property var subitems: []

            onAboutToShow: {
                clear()

                for (var i in subitems) {
                    var item = subitems[i]
                    var isMenu = Boolean(item.subitems) && item.subitems.length > 0

                    if (isMenu) {
                        var subMenu = makeMenu(item)

                        addMenu(subMenu)
                    } else {
                        var menuItem = makeMenuItem(this, item)

                        addItem(menuItem)
                    }
                }
            }
        }
    }

    Component {
        id: menuItemComponent

        PLATFORM.MenuItem {
            property string id: ""

            onTriggered: {
                Qt.callLater(menuModel.handleMenuItem, id)
            }
        }
    }
}

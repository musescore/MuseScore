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
    id: root

    property alias navigation: navPanel

    height: contentItem.childrenRect.height
    width: contentWidth
    orientation: Qt.Horizontal

    interactive: false

    model: appMenuModel.items

    AppMenuModel {
        id: appMenuModel

        onOpenMenu: {
            prv.openMenu(menuId)
        }

        onActiveChanged: {
            prv.activeMenu()
        }
    }

    Component.onCompleted: {
        appMenuModel.load()
    }

    QtObject {
        id: prv

        property var showedMenu: null

        //! Navigation

        function openMenu(menuId) {
            for (var i = 0; i < root.count; ++i) {
                var item = root.itemAtIndex(i)
                if (Boolean(item) && item.menuId === menuId) {
                    item.navigation.requestActive()
                    item.navigation.triggered()
                }
            }
        }

        function activeMenu() {
            if (appMenuModel.active) {
                if (prv.hasNavigatedItem()) {
                    return
                }

                var firstItem = root.itemAtIndex(0)
                firstItem.navigation.requestActive()
            } else {
                var _navigatedItem = navigatedItem()
                _navigatedItem.navigation.requestDeactive()
            }
        }

        function hasNavigatedItem() {
            return navigatedItem() !== null
        }

        function navigatedItem() {
            for (var i = 0; i < root.count; ++i) {
                var item = root.itemAtIndex(i)

                if (!Boolean(item)) {
                    continue
                }

                if (item.navigation.active) {
                    return item
                }
            }

            return null
        }

        function itemByKey(key) {
            for (var i = 0; i < root.count; ++i) {
                var item = root.itemAtIndex(i)

                if (!Boolean(item)) {
                    continue
                }

                var title = item.item.title
                if (Boolean(title)) {
                    title = title.toLowerCase()
                    var index = title.indexOf('&')
                    if (index === -1) {
                        continue
                    }

                    var activateKey = title[index + 1]
                    if (activateKey === key) {
                        return item
                    }
                }
            }

            return null
        }
    }

    NavigationPanel {
        id: navPanel
        name: "AppMenuBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "App menu bar")
    }

    delegate: FlatButton {
        id: radioButtonDelegate

        property var item: Boolean(modelData) ? modelData : null
        property string menuId: Boolean(item) ? item.id : ""

        narrowMargins: true
        drawFocusBorderInsideRect: true

        transparent: !menuLoader.isMenuOpened
        accentButton: menuLoader.isMenuOpened

        contentItem: StyledTextLabel {
            id: textLabel

            text: Boolean(radioButtonDelegate.item) ? correctText(radioButtonDelegate.item.title) : ""
            textFormat: Text.RichText

            width: textMetrics.width

            TextMetrics {
                id: textMetrics

                font: textLabel.font
                text: textLabel.removeAmpersands(radioButtonDelegate.item.title)
            }

            function correctText(text) {
                if (!prv.hasNavigatedItem()) {
                    return removeAmpersands(text)
                }

                return makeMnemonicText(text)
            }

            function removeAmpersands(text) {
                return Utils.removeAmpersands(text)
            }

            function makeMnemonicText(text) {
                return Utils.makeMnemonicText(text)
            }
        }

        navigation.name: text
        navigation.panel: navPanel
        navigation.order: index
        accessible.name: {
            return Utils.removeAmpersands(text)
        }

        Keys.onShortcutOverride: {
            if (!prv.hasNavigatedItem()) {
                return
            }

            var activatedItem = prv.itemByKey(event.text)
            event.accepted = Boolean(activatedItem)
        }

        Keys.onPressed: {
            var activatedItem = prv.itemByKey(event.text)
            if (Boolean(activatedItem)) {
                activatedItem.navigation.requestActive()
                activatedItem.navigation.triggered()
            }
        }

        mouseArea.onContainsMouseChanged: {
            if (!mouseArea.containsMouse || !prv.showedMenu || prv.showedMenu === menuLoader.menu) {
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

            navigation: radioButtonDelegate.navigation

            onHandleMenuItem: {
                Qt.callLater(appMenuModel.handleMenuItem, itemId)
            }
        }
    }
}

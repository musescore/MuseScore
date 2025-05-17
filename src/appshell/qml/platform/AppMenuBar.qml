/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0

ListView {
    id: root

    height: Math.max(1,contentItem.childrenRect.height)
    width: contentWidth

    property alias appWindow: appMenuModel.appWindow

    orientation: Qt.Horizontal

    interactive: false

    model: appMenuModel

    function openedArea(menuLoader) {
        if (menuLoader.isMenuOpened) {
            if (menuLoader.menu.subMenuLoader && menuLoader.menu.subMenuLoader.isMenuOpened)
                return openedArea(menuLoader.menu.subMenuLoader)
            return Qt.rect(menuLoader.menu.x, menuLoader.menu.y, menuLoader.menu.width, menuLoader.menu.height)
        }
        return Qt.rect(0, 0, 0, 0)
    }

    AppMenuModel {
        id: appMenuModel

        appMenuAreaRect: Qt.rect(root.x, root.y, root.width, root.height)
        openedMenuAreaRect: openedArea(menuLoader)

        onOpenMenuRequested: function(menuId) {
            prv.openMenu(menuId)
        }

        onCloseOpenedMenuRequested: {
            menuLoader.close()
        }

        onNavigateWithSymbolRequested: function(symbol) {
            if (menuLoader.isMenuOpened) {
                menuLoader.menu.navigateWithSymbolRequested(symbol, appMenuModel)
            }
        }
    }

    AccessibleItem {
        id: panelAccessibleInfo

        visualItem: root
        role: MUAccessible.Panel
        name: qsTrc("appshell", "Application menu")
    }

    Component.onCompleted: {
        appMenuModel.load()
    }

    QtObject {
        id: prv

        property var openedMenu: null
        property bool needRestoreNavigationAfterClose: false
        property string lastOpenedMenuId: ""

        function openMenu(menuId, byHover) {
            for (var i = 0; i < root.count; ++i) {
                var item = root.itemAtIndex(i)
                if (Boolean(item) && item.menuId === menuId) {
                    needRestoreNavigationAfterClose = true
                    lastOpenedMenuId = menuId

                    if (!byHover) {
                        if (menuLoader.isMenuOpened && menuLoader.parent === item) {
                            menuLoader.close()
                            return
                        }
                    }

                    menuLoader.menuId = menuId
                    menuLoader.parent = item
                    menuLoader.accessibleName = item.title

                    Qt.callLater(menuLoader.open, item.item.subitems)

                    return
                }
            }
        }

        function hasNavigatedItem() {
            return appMenuModel.highlightedMenuId !== ""
        }
    }

    delegate: FlatButton {
        id: radioButtonDelegate

        property var item: model ? model.itemRole : null
        property string menuId: Boolean(item) ? item.id : ""
        property string title: Boolean(item) ? item.title : ""
        property string titleWithMnemonicUnderline: Boolean(item) ? item.titleWithMnemonicUnderline : ""

        property bool isMenuOpened: menuLoader.isMenuOpened && menuLoader.parent === this
        property bool highlight: appMenuModel.highlightedMenuId === menuId

        property int viewIndex: index

        buttonType: FlatButton.TextOnly
        isNarrow: true
        margins: 8
        drawFocusBorderInsideRect: true

        transparent: !isMenuOpened
        accentButton: isMenuOpened

        navigation.accessible.ignored: true

        AccessibleItem {
            id: accessibleInfo

            accessibleParent: panelAccessibleInfo
            visualItem: radioButtonDelegate
            role: MUAccessible.Button
            name: radioButtonDelegate.title

            property bool active: radioButtonDelegate.highlight && !radioButtonDelegate.isMenuOpened
            onActiveChanged: {
                if (active) {
                    forceActiveFocus()
                    accessibleInfo.readInfo()
                } else {
                    accessibleInfo.resetFocus()
                }
            }

            function readInfo() {
                accessibleInfo.ignored = false
                accessibleInfo.focused = true
            }

            function resetFocus() {
                accessibleInfo.focused = false
                accessibleInfo.ignored = true
            }
        }

        contentItem: StyledTextLabel {
            id: textLabel

            text: appMenuModel.isNavigationStarted ? radioButtonDelegate.titleWithMnemonicUnderline : radioButtonDelegate.title
            textFormat: Text.RichText
            font: ui.theme.defaultFont
        }

        backgroundItem: AppButtonBackground {
            mouseArea: radioButtonDelegate.mouseArea

            highlight: radioButtonDelegate.highlight

            color: radioButtonDelegate.normalColor
        }

        mouseArea.onHoveredChanged: {
            if (!mouseArea.containsMouse) {
                return
            }

            if (menuLoader.isMenuOpened && menuLoader.parent !== this) {
                appMenuModel.openMenu(radioButtonDelegate.menuId, true)
            }
        }

        onClicked: {
            appMenuModel.openMenu(radioButtonDelegate.menuId, false)
        }
    }

    StyledMenuLoader {
        id: menuLoader

        property string menuId: ""
        property bool hasSiblingMenus: true

        onHandleMenuItem: function(itemId) {
            Qt.callLater(appMenuModel.handleMenuItem, itemId)
        }

        onOpenPrevMenu: {
            appMenuModel.openPrevMenu()
        }

        onOpenNextMenu: {
            appMenuModel.openNextMenu()
        }

        onOpened: {
            appMenuModel.openedMenuId = menuLoader.menuId
        }

        onClosed: {
            appMenuModel.openedMenuId = ""
        }
    }
}

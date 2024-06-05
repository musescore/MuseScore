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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

FlatButton {
    id: root

    property var item: null

    property bool hasMenu: item.menuItems.length !== 0
    property bool isMenuSecondary: item.isMenuSecondary

    width: item.showTitle ? implicitWidth : 32
    height: item.showTitle ? implicitHeight : 32

    accentButton: item.checked || menuLoader.isMenuOpened

    text: item.showTitle ? item.title : ""

    icon: item.icon
    iconFont: ui.theme.toolbarIconsFont

    toolTipTitle: item.title
    toolTipDescription: item.description
    toolTipShortcut: item.shortcuts

    orientation: Qt.Horizontal
    transparent: true

    enabled: item.enabled

    drawFocusBorderInsideRect: true

    navigation.name: item.id
    isClickOnKeyNavTriggered: false
    navigation.onTriggered: {
        if (menuLoader.isMenuOpened || hasMenu) {
            toggleMenuOpened()
        } else {
            activateToolBarItem()
        }
    }

    mouseArea.acceptedButtons: hasMenu && isMenuSecondary
                               ? Qt.LeftButton | Qt.RightButton
                               : Qt.LeftButton

    function toggleMenuOpened() {
        menuLoader.toggleOpened(item.menuItems)
    }

    function activateToolBarItem() {
        Qt.callLater(root.item.activate)
    }

    onClicked: function(mouse) {
        if (menuLoader.isMenuOpened // If already menu open, close it
                || (hasMenu // Or if can open menu
                    && (!isMenuSecondary // And _should_ open menu
                        || mouse.button === Qt.RightButton))) {
            toggleMenuOpened()
            return
        }

        if (mouse.button === Qt.LeftButton) {
            activateToolBarItem()
        }
    }

    Connections {
        target: root.mouseArea

        enabled: root.hasMenu && !menuLoader.isMenuOpened

        function onPressAndHold() {
            if (menuLoader.isMenuOpened || !root.hasMenu) {
                return
            }

            root.toggleMenuOpened()
        }
    }

    Canvas {
        visible: isMenuSecondary

        property color fillColor: ui.theme.fontPrimaryColor
        onFillColorChanged: {
            requestPaint()
        }

        width: 4
        height: 4

        anchors.margins: 2
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onPaint: {
            const ctx = getContext("2d");
            ctx.fillStyle = fillColor;
            ctx.moveTo(width, 0);
            ctx.lineTo(width, height);
            ctx.lineTo(0, height);
            ctx.closePath();
            ctx.fill();
        }
    }

    StyledMenuLoader {
        id: menuLoader

        onHandleMenuItem: function(itemId) {
            root.item.handleMenuItem(itemId)
        }
    }
}

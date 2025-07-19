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

    property var itemData: null

    property bool hasMenu: Boolean(itemData) ? itemData.menuItems.length !== 0 : false
    property bool isMenuSecondary: Boolean(itemData) ? itemData.isMenuSecondary : false

    width: Boolean(itemData) ? implicitWidth : 32
    height: Boolean(itemData) ? implicitHeight : 32

    accentButton: !transparent && Boolean(itemData) && (itemData.checked || menuLoader.isMenuOpened)

    text: Boolean(itemData) && itemData.showTitle ? itemData.title : ""

    textFont: Boolean(itemData) && itemData.isTitleBold ? ui.theme.largeBodyFont : ui.theme.bodyFont

    icon: Boolean(itemData) ? itemData.icon : IconCode.NONE
    iconFont: ui.theme.toolbarIconsFont

    toolTipTitle: Boolean(itemData) ? itemData.title : ""
    toolTipDescription: Boolean(itemData) ? itemData.description : ""
    toolTipShortcut: Boolean(itemData) ? itemData.shortcuts : ""

    orientation: Qt.Horizontal
    transparent: Boolean(itemData) ? itemData.isTransparent : false

    enabled: Boolean(itemData) ? itemData.enabled : false

    drawFocusBorderInsideRect: true

    navigation.name: Boolean(itemData) ? itemData.id : ""
    accessible.name: {
        if (!Boolean(itemData)) {
            return ""
        }

        if (itemData.checkable) {
            return itemData.title + "  " + (itemData.checked ? qsTrc("global", "On") : qsTrc("global", "Off"))
        }

        return itemData.title
    }
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
        menuLoader.toggleOpened(itemData.menuItems)
    }

    function activateToolBarItem() {
        Qt.callLater(root.itemData.activate)
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

    mouseArea.onPressAndHold: function(event) {
        if (menuLoader.isMenuOpened || !root.hasMenu) {
            event.accepted = false // do not suppress the click event
            return
        }

        root.toggleMenuOpened()
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
            root.itemData.handleMenuItem(itemId)
        }
    }
}

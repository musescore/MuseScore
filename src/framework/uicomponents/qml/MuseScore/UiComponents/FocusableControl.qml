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

FocusScope {
    id: root

    default property alias content: contentItemContaner.data
    property alias contentItem: contentItemContaner
    property alias background: focusRectItem

    property int itemBorderWidth: 0
    property color itemBorderColor: ui.theme.focusColor

    property alias mouseArea: mouseAreaItem
    property alias pressAndHoldInterval: mouseAreaItem.pressAndHoldInterval

    property alias navigation: keynavItem

    signal navigationActived()
    signal navigationTriggered()

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: keynavItem
        name: root.objectName
        enabled: root.enabled && root.visible

        onActiveChanged: {
            if (keynavItem.active) {
                root.ensureActiveFocus()
                root.navigationActived()
            }
        }

        onTriggered: {
            root.navigationTriggered()
        }
    }

    Rectangle {
        id: focusRectItem
        anchors.fill: parent

        Rectangle {
            id: navCtrlBorderRect

            anchors.fill: parent
            anchors.margins: -ui.theme.navCtrlBorderWidth

            visible: keynavItem.active

            color: "transparent"
            radius: parent.radius + navCtrlBorderRect.border.width

            border.width: ui.theme.navCtrlBorderWidth
            border.color: ui.theme.fontPrimaryColor
        }

        border.color: root.itemBorderColor
        border.width: root.itemBorderWidth
    }

    MouseArea {
        id: mouseAreaItem
        anchors.fill: parent

        onClicked: {
            root.ensureActiveFocus()
        }
    }

    Item {
        id: contentItemContaner
        objectName: "FocusableControlContent"
        anchors.fill: focusRectItem
        anchors.margins: 2 //! NOTE margin needed to show focus border
    }
}

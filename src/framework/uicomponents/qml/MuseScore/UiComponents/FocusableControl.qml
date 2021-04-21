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

    default property alias content: contentItem.data
    property alias background: focusRectItem

    property alias mouseArea: mouseAreaItem
    property alias pressAndHoldInterval: mouseAreaItem.pressAndHoldInterval

    property alias navigation: keynavItem

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }

        if (!keynavItem.active) {
            keynavItem.forceActive()
        }
    }

    NavigationControl {
        id: keynavItem
        name: root.objectName
        enabled: root.enabled

        onActiveChanged: {
            if (keynavItem.active) {
                root.ensureActiveFocus()
            }
        }
    }

    Rectangle {
        id: focusRectItem
        anchors.fill: parent
        border.color: ui.theme.focusColor
        border.width: keynavItem.active ? 2 : 0
    }

    MouseArea {
        id: mouseAreaItem
        anchors.fill: parent

        onClicked: {
            root.ensureActiveFocus()
        }
    }

    Item {
        id: contentItem
        objectName: "FocusableControlContent"
        anchors.fill: focusRectItem
        anchors.margins: 2 //! NOTE margin needed to show focus border
    }
}

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
import QtQuick.Layouts 1.12

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    required property QtObject titleBarCpp

    property alias contextMenuModel: contextMenuButton.menuModel

    property alias navigation: contextMenuButton.navigation

    signal handleContextMenuItemRequested(string itemId)

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        cursorShape: Qt.SizeAllCursor
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12

        spacing: 4

        StyledTextLabel {
            id: titleLabel
            Layout.fillWidth: true

            text: root.titleBarCpp ? root.titleBarCpp.title : ""
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        MenuButton {
            id: contextMenuButton

            width: 20
            height: width

            onHandleMenuItem: function(itemId) {
                root.handleContextMenuItemRequested(itemId)
            }
        }
    }
}

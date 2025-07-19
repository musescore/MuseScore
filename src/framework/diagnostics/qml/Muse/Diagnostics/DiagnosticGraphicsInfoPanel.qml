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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Diagnostics 1.0

Rectangle {

    anchors.fill: parent

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        graphicsInfo.init()
    }

    GraphicsInfoModel {
        id: graphicsInfo
    }

    Item {
        id: toolPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        height: 48

        Row {
            id: btnRow
            anchors.fill: parent
            spacing: 8

            FlatButton {
                anchors.verticalCenter: parent.verticalCenter
                text: "Copy to clipboard"
                onClicked: graphicsInfo.copyToClipboard()
            }
        }
    }

    StyledTextLabel {
        anchors.top: toolPanel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
        text: graphicsInfo.info
    }

}

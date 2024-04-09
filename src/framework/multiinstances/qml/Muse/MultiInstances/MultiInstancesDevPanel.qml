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
import Muse.MultiInstances 1.0

Rectangle {

    id: root

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        miModel.init()
    }

    MultiInstancesDevModel {
        id: miModel
    }

    Item {
        id: header
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 16
        height: 40

        StyledTextLabel {
            anchors.fill: parent
            horizontalAlignment: Text.AlignLeft
            text: "selfID: " + miModel.selfID
        }

        FlatButton {
            anchors.right: parent.right
            anchors.margins: 16
            anchors.verticalCenter: parent.verticalCenter

            text: "ping"
            onClicked: miModel.ping()
        }
    }

    ListView {
        id: view
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left

        model: miModel.instances
        delegate: ListItemBlank {

            property var item: modelData

            anchors.right: parent.right
            anchors.left: parent.left
            anchors.margins: 16
            height: 40

            StyledTextLabel {
                anchors.fill: parent
                horizontalAlignment: Text.AlignLeft
                text: "id: " + item.id + ", isServer: " + item.isServer
            }
        }
    }

}

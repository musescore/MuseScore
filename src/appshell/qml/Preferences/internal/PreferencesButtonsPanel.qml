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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    signal revertFactorySettingsRequested()
    signal applyRequested()
    signal rejectRequested()

    color: ui.theme.backgroundPrimaryColor

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 20

        height: childrenRect.height

        FlatButton {
            anchors.left: parent.left

            width: 160

            text: qsTrc("appshell", "Reset preferences")

            onClicked: {
                root.revertFactorySettingsRequested()
            }
        }

        FlatButton {
            anchors.right: applyButton.left
            anchors.rightMargin: 12

            width: 132
            text: qsTrc("global", "Cancel")

            onClicked: {
                root.rejectRequested()
            }
        }

        FlatButton {
            id: applyButton

            anchors.right: parent.right

            width: 132
            accentButton: true

            text: qsTrc("global", "OK")

            onClicked: {
                root.applyRequested()
            }
        }
    }
}

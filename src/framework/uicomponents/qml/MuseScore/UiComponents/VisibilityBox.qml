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

FocusScope {
    id: root

    property alias text: label.text
    property bool isVisible: true

    property alias navigation: eyeButton.navigation

    signal visibleToggled()

    height: 30
    width: content.implicitWidth

    opacity: root.enabled ? 1.0 : 0.7

    RowLayout {
        id: content

        anchors.fill: parent

        spacing: 2

        FlatButton {
            id: eyeButton

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: width

            icon: root.isVisible ? IconCode.VISIBILITY_ON : IconCode.VISIBILITY_OFF

            normalStateColor: "transparent"
            pressedStateColor: ui.theme.accentColor

            onClicked: {
                root.visibleToggled()
            }
        }

        StyledTextLabel {
            id: label

            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            maximumLineCount: 2

            visible: Boolean(text)
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: content
        anchors.margins: -4

        hoverEnabled: true

        onClicked: {
            root.visibleToggled()
        }
    }
}

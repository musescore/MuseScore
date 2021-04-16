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

import MuseScore.UiComponents 1.0

Row {
    id: root

    property alias colors: view.model
    property alias currentColorIndex: view.currentIndex

    property int firstColumnWidth: 0

    signal accentColorChangeRequested(var newColorIndex)

    height: 36
    spacing: 0

    StyledTextLabel {
        width: root.firstColumnWidth

        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Qt.AlignLeft

        text: qsTrc("appshell", "Accent colour:")
    }

    RadioButtonGroup {
        id: view

        spacing: 10

        delegate: RoundedRadioButton {
            width: 36
            height: width

            checked: view.currentIndex === model.index

            onClicked: {
                root.accentColorChangeRequested(model.index)
            }

            indicator: Rectangle {
                anchors.fill: parent

                radius: width / 2

                border.color: ui.theme.fontPrimaryColor
                border.width: parent.checked ? 1 : 0

                color: "transparent"

                Rectangle {
                    anchors.centerIn: parent

                    width: 30
                    height: width
                    radius: width / 2

                    border.color: ui.theme.strokeColor
                    border.width: 1

                    color: modelData
                }
            }

            background: Item {}
        }
    }
}

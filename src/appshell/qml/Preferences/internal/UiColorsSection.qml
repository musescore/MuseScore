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

import MuseScore.UiComponents 1.0

Column {
    id: root
    spacing: 18

    property int firstColumnWidth: 0

    signal colorChangeRequested(var newColor, var propertyName)

    StyledTextLabel {
        text: qsTrc("appshell", "UI Colors")
        font: ui.theme.bodyBoldFont
    }

    Column {
        width: parent.width
        spacing: 12

        Repeater {
            width: parent.width

            model: [
                { textRole: "Accent Color:", colorRole: ui.theme.accentColor},
                { textRole: "Text and Icons:", colorRole: ui.theme.fontPrimaryColor},
                { textRole: "Disabled Text:", colorRole: "#000000"},
                { textRole: "Stroke:", colorRole: ui.theme.strokeColor}
            ]

            delegate: Row {
                anchors.left: parent.left
                anchors.right: parent.right

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData["textRole"]
                    width: root.firstColumnWidth
                    horizontalAlignment: Text.AlignLeft
                }

                ColorPicker {
                    width: 112
                    color: modelData["colorRole"]

                    onNewColorSelected: {
                        root.colorChangeRequested(newColor, modelData.textRole)
                    }
                }
            }
        }
    }
}

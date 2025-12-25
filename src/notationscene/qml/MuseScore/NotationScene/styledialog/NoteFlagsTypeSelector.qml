/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

Rectangle {
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    NotesPageModel {
        id: notesPageModel
    }

    RadioButtonGroup {
        width: 224
        height: 70
        spacing: 12

        model: [
            { iconCode: IconCode.NOTEFLAGS_TRADITIONAL, text: qsTrc("notation", "Traditional"), value: false },
            { iconCode: IconCode.NOTEFLAGS_STRAIGHT, text: qsTrc("notation", "Straight"), value: true }
        ]

        delegate: FlatRadioButton {
            id: delegateButton

            required property var modelData

            width: 106
            height: 70

            checked: modelData.value === notesPageModel.useStraightNoteFlags.value

            Column {
                anchors.centerIn: parent
                spacing: 8

                StyledIconLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    iconCode: delegateButton.modelData.iconCode
                    font.pixelSize: 32
                }

                StyledTextLabel {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: delegateButton.modelData.text
                }
            }

            onToggled: {
                notesPageModel.useStraightNoteFlags.value = modelData.value
            }
        }
    }
}

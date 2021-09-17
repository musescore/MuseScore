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
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

InspectorPropertyView {
    id: root
    titleText: qsTrc("inspector", "Notehead type")

    RadioButtonGroup {
        id: headTypeButtonList

        height: 40
        width: parent.width

        model: [
            { iconRole: IconCode.NONE, textRole: qsTrc("inspector", "Auto"), typeRole: NoteHead.TYPE_AUTO },
            { iconRole: IconCode.NOTE_HEAD_QUARTER, typeRole: NoteHead.TYPE_QUARTER },
            { iconRole: IconCode.NOTE_HEAD_HALF, typeRole: NoteHead.TYPE_HALF },
            { iconRole: IconCode.NOTE_HEAD_WHOLE, typeRole: NoteHead.TYPE_WHOLE },
            { iconRole: IconCode.NOTE_HEAD_BREVIS, typeRole: NoteHead.TYPE_BREVIS }
        ]

        delegate: FlatRadioButton {
            ButtonGroup.group: headTypeButtonList.radioButtonGroup

            text: modelData["textRole"]
            iconCode: modelData["iconRole"]
            iconFontSize: 30

            checked: root.propertyItem && !root.propertyItem.isUndefined ? root.propertyItem.value === modelData["typeRole"]
                                                                         : false
            onToggled: {
                root.propertyItem.value = modelData["typeRole"]
            }
        }
    }
}

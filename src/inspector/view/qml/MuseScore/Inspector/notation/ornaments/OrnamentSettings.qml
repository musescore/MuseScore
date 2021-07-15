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
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "OrnamentSettings"

    spacing: 12

    InspectorPropertyView {

        titleText: qsTrc("inspector", "Performance")
        propertyItem: root.model ? root.model.performanceType : null

        RadioButtonGroup {
            id: radioButtonList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTrc("inspector", "Standard"), valueRole: OrnamentTypes.STYLE_STANDARD },
                { textRole: qsTrc("inspector", "Baroque"), valueRole: OrnamentTypes.STYLE_BAROQUE }
            ]

            delegate: FlatRadioButton {
                id: radioButtonDelegate

                ButtonGroup.group: radioButtonList.radioButtonGroup

                checked: root.model && !root.model.performanceType.isUndefined ? root.model.performanceType.value === modelData["valueRole"]
                                                                               : false
                onToggled: {
                    root.model.performanceType.value = modelData["valueRole"]
                }

                StyledTextLabel {
                    text: modelData["textRole"]

                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Placement")
        propertyItem: root.model ? root.model.placement : null

        Dropdown {
            id: placements

            width: parent.width

            model: [
                { text: qsTrc("inspector", "Above staff"), value: ArticulationTypes.TYPE_ABOVE_STAFF },
                { text: qsTrc("inspector", "Below staff"), value: ArticulationTypes.TYPE_BELOW_STAFF },
                { text: qsTrc("inspector", "Chord automatic"), value: ArticulationTypes.TYPE_CHORD_AUTO },
                { text: qsTrc("inspector", "Above chord"), value: ArticulationTypes.TYPE_ABOVE_CHORD },
                { text: qsTrc("inspector", "Below chord"), value: ArticulationTypes.TYPE_BELOW_CHORD }
            ]

            currentIndex: root.model && !root.model.placement.isUndefined ? placements.indexOfValue(root.model.placement.value) : -1

            onCurrentValueChanged: {
                root.model.placement.value = placements.currentValue
            }
        }
    }

    FlatButton {
        width: parent.width

        text: qsTrc("inspector", "Channel & MIDI properties")

        onClicked: {
            if (root.model) {
                root.model.openChannelAndMidiProperties()
            }
        }
    }
}

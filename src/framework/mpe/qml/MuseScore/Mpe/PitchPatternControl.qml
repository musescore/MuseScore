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
import QtQuick.Controls 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Column {
    id: root

    property QtObject patternModel: null

    spacing: 12

    RowLayout {
        id: offsetPointIndexRow

        width: root.width

        spacing: 12

        StyledTextLabel {
            id: offsetPointIndexLabel

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            text: /*qsTrc*/ "Point index"
        }

        StyledSlider {
            id: offsetPointIndexSlider

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            snapMode: Slider.SnapAlways

            from: 0
            to: 10
            stepSize: 1
            value: root.patternModel ? root.patternModel.selectedPitchOffsetIndex : 0

            onMoved: {
                if (root.patternModel) {
                    root.patternModel.selectedPitchOffsetIndex = offsetPointIndexSlider.value
                }
            }
        }
    }

    RowLayout {
        id: offsetPointValueRow

        width: root.width

        spacing: 12

        StyledTextLabel {
            id: offsetPointValueLabel

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            text: /*qsTrc*/ "Pitch offset"
        }

        StyledSlider {
            id: offsetPointValueSlider

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            implicitWidth: 120

            stepSize: root.patternModel ? root.patternModel.singlePercentValue * 0.1 : 0

            from: root.patternModel ? root.patternModel.singlePercentValue * -50 : 0
            to: root.patternModel ? root.patternModel.singlePercentValue * 50 : 0
            value: root.patternModel ? root.patternModel.pitchOffsetValueAt(offsetPointIndexSlider.value) : 0

            onMoved: {
                if (root.patternModel) {
                    root.patternModel.updatePitchOffsetValue(offsetPointIndexSlider.value,
                                                             offsetPointValueSlider.value)
                }
            }
        }

        TextInputField {
            id: offsetPointValueInput

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: 64

            currentText: offsetPointValueSlider.value / 100

            validator: DoubleInputValidator {
                top: 50
                bottom: -50
                decimal: 1
            }

            onTextChanged: function(newTextValue) {
                if (root.patternModel) {
                    root.patternModel.updatePitchOffsetValue(offsetPointIndexSlider.value,
                                                             newTextValue * 100)
                }
            }
        }
    }
}

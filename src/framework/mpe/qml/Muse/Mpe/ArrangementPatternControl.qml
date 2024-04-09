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
import QtQuick.Layouts 1.12
import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    property QtObject patternModel: null

    spacing: 12

    RowLayout {
        id: timestampOffsetRow

        width: root.width

        spacing: 12

        StyledTextLabel {
            id: timestampOffsetLabel

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            text: /*qsTrc*/ "Timestamp offset"
        }

        StyledSlider {
            id: timestampOffsetSlider

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            implicitWidth: 120

            stepSize: root.patternModel ? root.patternModel.singlePercentValue * 10 : 0

            from: root.patternModel ? root.patternModel.singlePercentValue * -50 : 0
            to: root.patternModel ? root.patternModel.singlePercentValue * 100 : 0
            value: root.patternModel ? root.patternModel.timestampShiftFactor : 0

            onMoved: {
                if (root.patternModel) {
                    root.patternModel.timestampShiftFactor = timestampOffsetSlider.value
                }
            }
        }

        IncrementalPropertyControl {
            id: timestampOffsetInput

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: 64

            currentValue: root.patternModel ? root.patternModel.timestampShiftFactor / 100 : 0
            minValue: -50
            maxValue: 50
            decimals: 0
            step: 1

            validator: IntInputValidator {
                top: timestampOffsetInput.maxValue
                bottom: timestampOffsetInput.minValue
            }

            onValueEdited: function(newValue) {
                if (root.patternModel) {
                    root.patternModel.timestampShiftFactor = newValue * 100
                }
            }
        }
    }

    RowLayout {
        id: durationFactorRow

        width: root.width

        spacing: 12

        StyledTextLabel {
            id: durationFactorLabel

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            text: /*qsTrc*/ "Duration factor"
        }

        StyledSlider {
            id: durationFactorSlider

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            implicitWidth: 120

            stepSize: root.patternModel ? root.patternModel.singlePercentValue * 10 : 0

            from: 0
            to: root.patternModel ? root.patternModel.singlePercentValue * 300 : 0
            value: root.patternModel ? root.patternModel.durationFactor : 0

            onMoved: {
                if (root.patternModel) {
                    root.patternModel.durationFactor = durationFactorSlider.value
                }
            }
        }

        IncrementalPropertyControl {
            id: durationFactorInput

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: 64

            currentValue: root.patternModel ? root.patternModel.durationFactor / 100 : 0
            minValue: 0
            maxValue: 300
            decimals: 0
            step: 1

            validator: IntInputValidator {
                top: durationFactorInput.maxValue
                bottom: durationFactorInput.minValue
            }

            onValueEdited: function(newValue) {
                if (root.patternModel) {
                    root.patternModel.durationFactor = newValue * 100
                }
            }
        }
    }
}

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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

FlatButton {
    id: root

    property var model: null
    property alias popupAnchorItem: popup.anchorItem

    height: 96
    accentButton: popup.isOpened

    StyledTextLabel {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        property string pickupMessage: {
            if (withPickupMeasure.checked) {
                return qsTrc("project", "Pickup: ") +
                        model.pickupTimeSignature.numerator + "/" + model.pickupTimeSignature.denominator
            }

            return qsTrc("project", "No pickup")
        }

        font: ui.theme.tabFont
        text: model.measureCount + qsTrc("project", " measures, \n") + pickupMessage
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopupView {
        id: popup

        padding: 8
        margins: 0

        contentWidth: content.width
        contentHeight: content.height

        ColumnLayout {
            id: content

            spacing: 0

            CheckBox {
                id: withPickupMeasure

                Layout.topMargin: 26
                Layout.leftMargin: 32

                checked: root.model.withPickupMeasure

                text: qsTrc("project", "Show pickup measure")

                onClicked: {
                    root.model.withPickupMeasure = !checked
                }
            }

            TimeSignatureFraction {
                Layout.topMargin: 12
                Layout.leftMargin: 32

                numerator: root.model.pickupTimeSignature.numerator
                denominator: root.model.pickupTimeSignature.denominator
                availableDenominators: root.model.timeSignatureDenominators()
                enabled: withPickupMeasure.checked

                onNumeratorSelected: {
                    root.model.setPickupTimeSignatureNumerator(value)
                }

                onDenominatorSelected: {
                    root.model.setPickupTimeSignatureDenominator(value)
                }
            }

            SeparatorLine {
                Layout.topMargin: 26
            }

            StyledTextLabel {
                Layout.topMargin: 26
                Layout.leftMargin: 32
                Layout.rightMargin: 32

                horizontalAlignment: Text.AlignLeft
                text: qsTrc("project", "Initial number of measures")
            }

            IncrementalPropertyControl {
                id: measuresCountControl

                Layout.topMargin: 12
                Layout.leftMargin: 32
                Layout.rightMargin: 32

                implicitWidth: 68

                currentValue: root.model.measureCount
                step: 1
                decimals: 0
                maxValue: root.model.measureCountRange().max
                minValue: root.model.measureCountRange().min

                onValueEdited: {
                    root.model.measureCount = newValue
                }
            }

            StyledTextLabel {
                Layout.topMargin: 18
                Layout.leftMargin: 32
                Layout.rightMargin: 32
                Layout.bottomMargin: 24
                Layout.preferredWidth: 246

                horizontalAlignment: Text.AlignLeft
                text: qsTrc("project", "Hint: You can also add & delete measures after you have created your score")
                wrapMode: Text.WordWrap
                maximumLineCount: 2
            }
        }
    }
}

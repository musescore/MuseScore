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

    height: 96
    accentButton: popup.visible

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

    StyledPopup {
        id: popup

        implicitHeight: 310
        implicitWidth: 320

        x: root.x - (width - root.width)
        y: root.height
        arrowX: root.x + root.width

        Column {
            anchors.fill: parent
            anchors.margins: 10

            spacing: 30

            Column {
                anchors.left: parent.left
                anchors.right: parent.right

                spacing: 14

                CheckBox {
                    id: withPickupMeasure

                    anchors.left: parent.left
                    anchors.right: parent.right

                    checked: root.model.withPickupMeasure

                    text: qsTrc("project", "Show pickup measure")

                    onClicked: {
                        root.model.withPickupMeasure = !checked
                    }
                }

                TimeSignatureFraction {
                    anchors.left: parent.left
                    anchors.right: parent.right

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
            }

            SeparatorLine {
                anchors.leftMargin: -(parent.anchors.leftMargin + popup.leftPadding)
                anchors.rightMargin: -(parent.anchors.rightMargin + popup.rightPadding)
            }

            Column {
                anchors.left: parent.left
                anchors.right: parent.right

                spacing: 14

                StyledTextLabel {
                    horizontalAlignment: Text.AlignLeft
                    text: qsTrc("project", "Initial number of measures")
                }

                IncrementalPropertyControl {
                    id: measuresCountControl

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
                    anchors.left: parent.left
                    anchors.right: parent.right

                    horizontalAlignment: Text.AlignLeft
                    text: qsTrc("project", "Hint: You can also add & delete measures after you have created your score")
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                }
            }
        }
    }
}

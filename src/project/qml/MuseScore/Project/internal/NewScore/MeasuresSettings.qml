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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

FlatButton {
    id: root

    property var model: null
    property string currentValueAccessibleName: title.text

    property alias popupAnchorItem: popup.anchorItem

    height: 96
    accentButton: popup.isOpened

    StyledTextLabel {
        id: title

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        property string pickupMessage: {
            if (withPickupMeasure.checked) {
                return qsTrc("project/newscore", "pickup:") + " " +
                        model.pickupTimeSignature.numerator + "/" + model.pickupTimeSignature.denominator
            }

            return qsTrc("project/newscore", "no pickup")
        }

        font: ui.theme.largeBodyFont
        text: qsTrc("project/newscore", "%n measure(s),", "", model.measureCount) + "\n" + pickupMessage
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

        margins: 0

        contentWidth: content.width
        contentHeight: content.height

        onOpened: {
            withPickupMeasure.navigation.requestActive()
        }

        ColumnLayout {
            id: content

            spacing: 0

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "MeasuresSettingsPanel"
                section: popup.navigationSection
                direction: NavigationPanel.Both
                order: 1
            }

            CheckBox {
                id: withPickupMeasure

                Layout.topMargin: 26
                Layout.leftMargin: 32

                checked: root.model.withPickupMeasure

                text: qsTrc("project/newscore", "Create pickup measure")

                navigation.name: "WithPickupMeasure"
                navigation.panel: content.navigationPanel
                navigation.row: 0
                navigation.column: 0

                onClicked: {
                    root.model.withPickupMeasure = !checked
                }
            }

            TimeSignatureFraction {
                id: pickupMeasure

                Layout.topMargin: 12
                Layout.leftMargin: 32

                numerator: root.model.pickupTimeSignature.numerator
                denominator: root.model.pickupTimeSignature.denominator
                availableDenominators: root.model.timeSignatureDenominators()
                enabled: withPickupMeasure.checked

                navigationSection: popup.navigationSection
                navigationPanelOrderStart: 2

                onNumeratorSelected: function(value) {
                    root.model.setPickupTimeSignatureNumerator(value)
                }

                onDenominatorSelected: function(value) {
                    root.model.setPickupTimeSignatureDenominator(value)
                }
            }

            SeparatorLine {
                Layout.topMargin: 26
            }

            StyledTextLabel {
                id: numberOfMeasuresLabel
                Layout.topMargin: 26
                Layout.leftMargin: 32
                Layout.rightMargin: 32

                horizontalAlignment: Text.AlignLeft
                text: qsTrc("project/newscore", "Initial number of measures")
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

                navigation.name: "MeasuresCountControl"
                navigation.panel: NavigationPanel {
                    name: "MeasuresCountPanel"
                    section: popup.navigationSection
                    order: pickupMeasure.navigationPanelOrderEnd + 1
                }
                navigation.row: 2
                navigation.column: 0
                navigation.accessible.name: numberOfMeasuresLabel.text + " " + currentValue

                onValueEdited: function(newValue) {
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
                text: qsTrc("project/newscore", "Hint: You can also add & delete measures after you have created your score")
                wrapMode: Text.WordWrap
                maximumLineCount: 2
            }
        }
    }
}

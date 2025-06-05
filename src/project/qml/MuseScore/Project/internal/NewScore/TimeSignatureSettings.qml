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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

FlatButton {
    id: root

    property var model: null
    property string currentValueAccessibleName: root.model.timeSignatureAccessibleName(root.model.timeSignatureType,
                                                                                       root.model.timeSignature.numerator,
                                                                                       root.model.timeSignature.denominator)

    property alias popupAnchorItem: popup.anchorItem

    height: 96
    accentButton: popup.isOpened

    TimeSignatureView {
        id: timeSignatureView

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        numerator: root.model.musicSymbolCodes(root.model.timeSignature.numerator)
        denominator: root.model.musicSymbolCodes(root.model.timeSignature.denominator)
        type: root.model.timeSignatureType
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

        margins: 36

        contentWidth: 200
        contentHeight: radioButtonList.contentHeight

        RadioButtonGroup {
            id: radioButtonList

            anchors.fill: parent

            spacing: 32

            orientation: ListView.Vertical

            model: [
                { comp: fractionComp, valueRole: AdditionalInfoModel.Fraction },
                { comp: commonComp, valueRole: AdditionalInfoModel.Common },
                { comp: cutComp, valueRole: AdditionalInfoModel.Cut }
            ]

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "TimeSignatureTabPanel"
                section: popup.navigationSection
                direction: NavigationPanel.Both
                order: 1
            }

            delegate: RoundedRadioButton {
                id: timeFractionButton

                property bool isCurrent: radioButtonList.currentIndex === model.index

                ButtonGroup.group: radioButtonList.radioButtonGroup
                width: parent.width

                spacing: 30
                leftPadding: 0

                contentComponent: modelData["comp"]
                checked: (root.model.timeSignatureType === modelData["valueRole"]) && popup.isOpened

                navigation.name: modelData["valueRole"]
                navigation.panel: radioButtonList.navigationPanel
                navigation.row: model.index
                navigation.column: 0

                onToggled: {
                    root.model.timeSignatureType = modelData["valueRole"]
                }

                onCheckedChanged: {
                    if (checked && !navigation.active) {
                        navigation.requestActive()
                    }
                }
            }
        }
    }

    Component {
        id: fractionComp

        TimeSignatureFraction {
            anchors.fill: parent

            property string accessibleName: root.model.timeSignatureAccessibleName(AdditionalInfoModel.Fraction,
                                                                                   numerator, denominator)

            enabled: (root.model.timeSignatureType === AdditionalInfoModel.Fraction)
            availableDenominators: root.model.timeSignatureDenominators()

            numerator: enabled ? root.model.timeSignature.numerator : numerator
            denominator: enabled ? root.model.timeSignature.denominator : denominator

            navigationSection: popup.navigationSection
            navigationPanelOrderStart: 2

            onNumeratorSelected: function(value) {
                root.model.setTimeSignatureNumerator(value)
            }

            onDenominatorSelected: function(value) {
                root.model.setTimeSignatureDenominator(value)
            }
        }
    }

    Component {
        id: commonComp

        Item {
            property string accessibleName: root.model.timeSignatureAccessibleName(AdditionalInfoModel.Common)

            implicitWidth: commonLabel.implicitWidth
            implicitHeight: 30

            StyledIconLabel {
                id: commonLabel
                anchors.verticalCenter: parent.verticalCenter
                font.family: ui.theme.musicalFont.family
                font.pixelSize: 30
                horizontalAlignment: Text.AlignLeft
                iconCode: MusicalSymbolCodes.TIMESIG_COMMON
            }
        }
    }

    Component {
        id: cutComp

        Item {
            property string accessibleName: root.model.timeSignatureAccessibleName(AdditionalInfoModel.Cut)

            implicitWidth: cutLabel.implicitWidth
            implicitHeight: 30

            StyledIconLabel {
                id: cutLabel
                anchors.verticalCenter: parent.verticalCenter
                font.family: ui.theme.musicalFont.family
                font.pixelSize: 30
                horizontalAlignment: Text.AlignLeft
                iconCode: MusicalSymbolCodes.TIMESIG_CUT
            }
        }
    }
}

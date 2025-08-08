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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

StyledPopupView {
    id: root

    property bool needActiveFirstItem: false

    signal replaceInstrumentRequested()
    signal resetAllFormattingRequested()

    contentHeight: contentColumn.childrenRect.height

    onOpened: {
        if (root.needActiveFirstItem) {
            instrNameField.navigation.requestActive()
        }
    }

    function load(instrument) {
        settingsModel.load(instrument)
    }

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "InstrumentSettingsPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    InstrumentSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        anchors.fill: parent
        spacing: 12

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                id: nameLabel
                width: parent.width
                text: settingsModel.isMainScore ? qsTrc("instruments", "Name on main score") :
                                                  qsTrc("instruments", "Name on part score")
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                id: instrNameField

                objectName: "InstrNameField"

                navigation.panel: root.navigationPanel
                navigation.row: 1
                navigation.accessible.name: nameLabel.text + " " + currentText

                currentText: settingsModel.instrumentName

                onTextEditingFinished: function(newTextValue) {
                    settingsModel.instrumentName = newTextValue
                }
            }
        }

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                id: abbreviatureLabel
                width: parent.width
                text: qsTrc("instruments", "Abbreviated name")
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                objectName: "AbbreviatureField"

                navigation.panel: root.navigationPanel
                navigation.row: 2
                navigation.accessible.name: abbreviatureLabel.text + " " + currentText

                currentText: settingsModel.abbreviature

                onTextEditingFinished: function(newTextValue) {
                    settingsModel.abbreviature = newTextValue
                }
            }
        }

        SeparatorLine {}

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                id: hideEmptyStavesLabel
                width: parent.width
                text: qsTrc("instruments", "Hide empty staves")
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                id: hideEmptyStavesGroup

                width: parent.width
                orientation: ListView.Vertical

                model: [
                    { text: qsTrc("instruments", "Auto"), value: 0 },
                    { text: qsTrc("instruments", "Always hide"), value: 1 },
                    { text: qsTrc("instruments", "Never hide"), value: 2 }
                ]

                delegate: FlatRadioButton {
                    required property var modelData
                    required property int index

                    navigation.panel: root.navigationPanel
                    navigation.row: 3 + index
                    navigation.accessible.name: hideEmptyStavesLabel.text + " " + text

                    text: modelData.text

                    checked: settingsModel.hideWhenEmpty === modelData.value
                    onToggled: {
                        settingsModel.hideWhenEmpty = modelData.value
                    }
                }
            }
        }

        SeparatorLine {}

        Column {
            width: parent.width
            spacing: 8

            FlatButton {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: 6

                text: qsTrc("instruments", "Replace instrument")

                visible: settingsModel.isMainScore

                onClicked: {
                    root.replaceInstrumentRequested()
                    root.close()
                }
            }

            FlatButton {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: 7

                text: qsTrc("instruments", "Reset all formatting")

                visible: !settingsModel.isMainScore

                onClicked: {
                    root.resetAllFormattingRequested()
                    root.close()
                }
            }
        }
    }
}

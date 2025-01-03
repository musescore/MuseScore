/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    PercussionPreferencesModel {
        id: percussionPreferencesModel
    }

    Component.onCompleted: {
        percussionPreferencesModel.init()
    }

    BaseSection {
        id: percussionPanelPreferences

        title: qsTrc("appshell/preferences", "Percussion")

        navigation.section: root.navigationSection

        CheckBox {
            id: unpitchedSelectedCheckbox

            visible: percussionPreferencesModel.useNewPercussionPanel
            width: parent.width

            text: qsTrc("appshell/preferences", "Open the percussion panel when an unpitched staff is selected")

            navigation.name: "UnpitchedSelectedCheckbox"
            navigation.panel: percussionPanelPreferences.navigation
            navigation.row: 0

            checked: percussionPreferencesModel.autoShowPercussionPanel

            onClicked:  {
                percussionPreferencesModel.autoShowPercussionPanel = !unpitchedSelectedCheckbox.checked
            }
        }

        StyledTextLabel {
            id: padSwapInfo

            visible: percussionPreferencesModel.useNewPercussionPanel
            width: parent.width

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
            text: qsTrc("notation/percussion", "When swapping the positions of two drum pads:")
        }

        RadioButtonGroup {
            id: radioButtons

            property int navigationRowStart: unpitchedSelectedCheckbox.navigation.row + 1
            property int navigationRowEnd: radioButtons.navigationRowStart + model.length

            visible: percussionPreferencesModel.useNewPercussionPanel

            width: parent.width
            spacing: percussionPanelPreferences.spacing

            orientation: ListView.Vertical

            model: [
                { text: qsTrc("notation/percussion", "Move MIDI notes and keyboard shortcuts with their sounds"), value: true },
                { text: qsTrc("notation/percussion", "Leave MIDI notes and keyboard shortcuts fixed to original pad positions"), value: false }
            ]

            delegate: Row {
                width: parent.width
                spacing: 6

                RoundedRadioButton {
                    id: radioButton

                    anchors.verticalCenter: parent.verticalCenter

                    navigation.name: modelData.text
                    navigation.panel: percussionPanelPreferences.navigation
                    navigation.row: radioButtons.navigationRowStart + model.index

                    checked: modelData.value === percussionPreferencesModel.percussionPanelMoveMidiNotesAndShortcuts

                    onToggled: {
                        percussionPreferencesModel.percussionPanelMoveMidiNotesAndShortcuts = modelData.value
                    }
                }

                //! NOTE: Can't use radioButton.text because it won't wrap
                StyledTextLabel {
                    width: parent.width - parent.spacing - radioButton.width

                    anchors.verticalCenter: parent.verticalCenter

                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                    text: modelData.text

                    MouseArea {
                        id: mouseArea

                        anchors.fill: parent

                        onClicked: {
                            percussionPreferencesModel.percussionPanelMoveMidiNotesAndShortcuts = modelData.value
                        }
                    }
                }
            }
        }

        CheckBox {
            id: alwaysAsk

            visible: percussionPreferencesModel.useNewPercussionPanel
            width: parent.width

            text: qsTrc("global", "Always ask")

            navigation.name: "AlwaysAskCheckBox"
            navigation.panel: percussionPanelPreferences.navigation
            navigation.row: radioButtons.navigationRowEnd

            checked: percussionPreferencesModel.showPercussionPanelPadSwapDialog

            onClicked:  {
                percussionPreferencesModel.showPercussionPanelPadSwapDialog = !alwaysAsk.checked
            }
        }

        FlatButton {
            id: useNewPercussionPanel

            text: percussionPreferencesModel.useNewPercussionPanel ? qsTrc("notation/percussion", "Switch to old percussion panel")
                                                                   : qsTrc("notation/percussion", "Switch to new percussion panel")

            navigation.name: "SwitchPercussionPanels"
            navigation.panel: percussionPanelPreferences.navigation
            navigation.row: alwaysAsk.navigation.row + 1

            onClicked: {
                percussionPreferencesModel.useNewPercussionPanel = !percussionPreferencesModel.useNewPercussionPanel
            }
        }
    }
}

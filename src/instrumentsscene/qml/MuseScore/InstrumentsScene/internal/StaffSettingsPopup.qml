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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

StyledPopupView {
    id: root

    contentHeight: contentColumn.childrenRect.height
    contentWidth: 240

    navigation.name: "StaffSettingsPopup"
    navigation.direction: NavigationPanel.Vertical

    function load(staff) {
        settingsModel.load(staff)
    }

    onOpened: {
        staffTypesComboBox.ensureActiveFocus()
    }

    StaffSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        anchors.fill: parent
        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTrc("instruments", "Staff type")
        }

        Dropdown {
            id: staffTypesComboBox
            width: parent.width

            navigation.panel: root.navigation
            navigation.row: 1

            model: settingsModel.allStaffTypes()

            onCurrentValueChanged: {
                settingsModel.setStaffType(staffTypesComboBox.currentValue)
            }
        }

        SeparatorLine {}

        StyledTextLabel {
            text: qsTrc("instruments", "Voices visible in the score")
        }

        Row {
            height: childrenRect.height
            width: parent.width

            spacing: 26

            Repeater {
                model: settingsModel.voices

                delegate: CheckBox {
                    id: item

                    objectName: "Voice" + modelData.title + "CheckBox"

                    navigation.panel: root.navigation
                    navigation.row: model.index + 2 //! NOTE after staffTypesComboBox

                    text: modelData.title
                    checked: modelData.visible

                    onClicked: {
                        item.checked = !item.checked
                        settingsModel.setVoiceVisible(model.index, !checked)
                    }
                }
            }
        }

        SeparatorLine {}

        CheckBox {
            navigation.panel: root.navigation
            navigation.row: 20 // Should be more than a voices checkbox

            text: qsTrc("instruments", "Small staff")
            checked: settingsModel.isSmallStaff

            onClicked: {
                settingsModel.setIsSmallStaff(!checked)
            }
        }

        CheckBox {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 20

            navigation.panel: root.navigation
            navigation.row: 21 // after small staff CheckBox

            text: qsTrc("instruments", "Hide all measures that do not contain notation (cutaway)")
            wrapMode: Text.WordWrap

            checked: settingsModel.cutawayEnabled

            onClicked: {
                settingsModel.setCutawayEnabled(!checked)
            }
        }

        SeparatorLine {}

        FlatButton {
            width: parent.width

            navigation.panel: root.navigation
            navigation.row: 22 // after cutaway CheckBox

            text: qsTrc("instruments", "Create a linked staff")

            onClicked: {
                settingsModel.createLinkedStaff()
                root.close()
            }
        }

        StyledTextLabel {
            width: parent.width

            text: qsTrc("instruments", "Note: linked staves contain identical information.")
            wrapMode: Text.WordWrap
        }
    }
}

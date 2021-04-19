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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopupView {
    id: root

    contentHeight: contentColumn.childrenRect.height
    contentWidth: 240

    function load(staff) {
        settingsModel.load(staff)
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

        StyledComboBox {
            width: parent.width

            textRoleName: "text"
            valueRoleName: "value"

            model: {
                var types = settingsModel.allStaffTypes()
                var result = []

                for (var i = 0; i < types.length; ++i) {
                    result.push({ text: types[i].title, value: types[i].value })
                }

                return result
            }

            onValueChanged: {
                settingsModel.setStaffType(value)
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Voices visible in the score")
        }

        ListView {
            height: contentItem.childrenRect.height
            width: parent.width

            spacing: 26
            orientation: ListView.Horizontal

            model: settingsModel.voices

            delegate: CheckBox {
                text: modelData.title
                checked: modelData.visible

                onClicked: {
                    settingsModel.setVoiceVisible(model.index, !checked)
                }
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        CheckBox {
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

            text: qsTrc("instruments", "Hide all measures that do not contain notation (cutaway)")
            wrapMode: Text.WordWrap

            checked: settingsModel.cutawayEnabled

            onClicked: {
                settingsModel.setCutawayEnabled(!checked)
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        FlatButton {
            width: parent.width

            navigation.panel: root.navigation

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

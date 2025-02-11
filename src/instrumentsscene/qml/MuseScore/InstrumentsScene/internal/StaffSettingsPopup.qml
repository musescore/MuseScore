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

    contentHeight: contentColumn.childrenRect.height
    contentWidth: 240

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "StaffSettingsPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    function load(staff) {
        settingsModel.load(staff.id)
    }

    onOpened: {
        if (root.needActiveFirstItem) {
            staffTypesDropdown.navigation.requestActive()
        }
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
            id: typeLabel
            text: qsTrc("layout", "Staff type")
        }

        StyledDropdown {
            id: staffTypesDropdown

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.row: 1
            navigation.accessible.name: typeLabel.text + " " + currentValue

            currentIndex: staffTypesDropdown.indexOfValue(settingsModel.staffType)
            model: settingsModel.allStaffTypes
            enabled: staffTypesDropdown.count > 1

            onActivated: function(index, value) {
                settingsModel.staffType = value
            }
        }

        SeparatorLine {
            visible: !settingsModel.isMainScore
        }

        StyledTextLabel {
            visible: !settingsModel.isMainScore
            text: qsTrc("layout", "Voices visible in the score")
        }

        Row {
            height: childrenRect.height
            width: parent.width

            spacing: 26

            visible: !settingsModel.isMainScore

            Repeater {
                model: settingsModel.voices

                delegate: CheckBox {
                    id: item

                    property int index: model.index

                    objectName: "Voice" + modelData.title + "CheckBox"

                    navigation.panel: root.navigationPanel
                    navigation.row: model.index + 2 //! NOTE after staffTypesDropdown

                    text: modelData.title
                    checked: modelData.visible

                    onClicked: {
                        settingsModel.setVoiceVisible(model.index, !checked)
                    }

                    Connections {
                        target: settingsModel
                        function onVoiceVisibilityChanged(voiceIndex, visible) {
                            if (item.index === voiceIndex) {
                                item.checked = visible
                            }
                        }
                    }
                }
            }
        }

        SeparatorLine {}

        CheckBox {
            navigation.panel: root.navigationPanel
            navigation.row: 20 // Should be more than a voices checkbox

            text: qsTrc("layout", "Small staff")
            checked: settingsModel.isSmallStaff

            onClicked: {
                settingsModel.isSmallStaff = !checked
            }
        }

        CheckBox {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 20

            navigation.panel: root.navigationPanel
            navigation.row: 21 // after small staff CheckBox

            text: qsTrc("layout", "Hide all measures that do not contain notation (cutaway)")

            checked: settingsModel.cutawayEnabled

            onClicked: {
                settingsModel.cutawayEnabled = !checked
            }
        }

        SeparatorLine {}

        FlatButton {
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.row: 22 // after cutaway CheckBox

            text: qsTrc("layout", "Create a linked staff")

            onClicked: {
                settingsModel.createLinkedStaff()
                root.close()
            }
        }

        StyledTextLabel {
            width: parent.width

            text: qsTrc("layout", "Note: linked staves contain identical information.")
            wrapMode: Text.WordWrap
        }
    }
}

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

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                id: typeLabel
                width: parent.width
                text: qsTrc("layoutpanel", "Staff type")
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
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
        }

        SeparatorLine {
            visible: !settingsModel.isMainScore
        }

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                visible: !settingsModel.isMainScore
                text: qsTrc("layoutpanel", "Voices visible in the score")
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
        }

        SeparatorLine {}

        Column {
            width: parent.width
            spacing: 8

            CheckBox {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: 20 // Should be more than a voices checkbox

                text: qsTrc("layoutpanel", "Small staff")
                checked: settingsModel.isSmallStaff

                onClicked: {
                    settingsModel.isSmallStaff = !checked
                }
            }

            CheckBox {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: 21 // after small staff CheckBox

                text: qsTrc("layoutpanel", "Hide all measures that do not contain notation (cutaway)")

                checked: settingsModel.cutawayEnabled

                onClicked: {
                    settingsModel.cutawayEnabled = !checked
                }
            }
        }

        SeparatorLine {}

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                width: parent.width
                text: qsTrc("layoutpanel", "Hide empty staves")
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                id: hideEmptyStavesGroup

                width: parent.width
                orientation: ListView.Vertical

                model: [
                    { text: qsTrc("layoutpanel", "Follow instrument"), value: 0 },
                    { text: qsTrc("layoutpanel", "Always hide"), value: 1 },
                    { text: qsTrc("layoutpanel", "Never hide"), value: 2 }
                ]

                delegate: FlatRadioButton {
                    required property var modelData
                    required property int index

                    navigation.panel: root.navigationPanel
                    navigation.row: 22 + index
                    navigation.accessible.name: qsTrc("layoutpanel", "Hide empty staves") + " " + text

                    text: modelData.text

                    checked: settingsModel.hideWhenEmpty === modelData.value
                    onToggled: {
                        settingsModel.hideWhenEmpty = modelData.value
                    }
                }
            }

            CheckBox {
                id: showIfEntireSystemEmptyCheckBox
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: 25 // after hideEmptyStavesGroup

                text: qsTrc("layoutpanel", "If the entire system is empty, show this staff")
                checked: settingsModel.showIfEntireSystemEmpty

                onClicked: {
                    settingsModel.showIfEntireSystemEmpty = !checked
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
                navigation.row: 26 // after showIfEntireSystemEmptyCheckBox

                text: qsTrc("layoutpanel", "Create a linked staff")

                onClicked: {
                    settingsModel.createLinkedStaff()
                    root.close()
                }
            }

            StyledTextLabel {
                width: parent.width

                text: qsTrc("layoutpanel", "Linked staves contain identical notation (e.g. for guitar tablature)")
                wrapMode: Text.WordWrap
            }
        }
    }
}

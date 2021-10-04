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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "BarlineSettings"

    property QtObject barlineSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_BARLINE) : null
    property QtObject staffSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_STAFF) : null

    property NavigationPanel navigationPanel: null
    property int navigationRowOffset: 1

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    DropdownPropertyView {
        id: styleSection
        titleText: qsTrc("inspector", "Style")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.type : null

        navigation.name: "Style"
        navigation.panel: root.navigationPanel
        navigationRowStart: root.navigationRowOffset

        model: [
            { text: qsTrc("symUserNames", "Single barline"), value: BarlineTypes.TYPE_NORMAL },
            { text: qsTrc("symUserNames", "Double barline"), value: BarlineTypes.TYPE_DOUBLE },
            { text: qsTrc("symUserNames", "Left (start) repeat sign"), value: BarlineTypes.TYPE_START_REPEAT },
            { text: qsTrc("symUserNames", "Right (end) repeat sign"), value: BarlineTypes.TYPE_END_REPEAT },
            { text: qsTrc("symUserNames", "Right and left repeat sign"), value: BarlineTypes.TYPE_END_START_REPEAT },
            { text: qsTrc("symUserNames", "Dashed barline"), value: BarlineTypes.TYPE_DASHED },
            { text: qsTrc("symUserNames", "Final barline"), value: BarlineTypes.TYPE_FINAL },
            { text: qsTrc("symUserNames", "Dotted barline"), value: BarlineTypes.TYPE_DOTTED },
            { text: qsTrc("symUserNames", "Reverse final barline"), value: BarlineTypes.TYPE_REVERSE_END },
            { text: qsTrc("symUserNames", "Heavy barline"), value: BarlineTypes.TYPE_HEAVY },
            { text: qsTrc("symUserNames", "Heavy double barline"), value: BarlineTypes.TYPE_DOUBLE_HEAVY },
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: repeatStyleSection
        titleText: qsTrc("inspector", "Repeat style")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.hasToShowTips : null

        visible: root.barlineSettingsModel && root.barlineSettingsModel.isRepeatStyleChangingAllowed

        navigation.name: "RepeatStyle"
        navigation.panel: root.navigationPanel
        navigationRowStart: styleSection.navigationRowEnd + 1

        model: [
            { iconCode: IconCode.BARLINE_UNWINGED, value: false },
            { iconCode: IconCode.BARLINE_WINGED, value: true }
        ]
    }

    CheckBox {
        id: spanToNextStaffCheckBox

        isIndeterminate: root.barlineSettingsModel ? root.barlineSettingsModel.isSpanToNextStaff.isUndefined : false
        checked: root.barlineSettingsModel && !isIndeterminate ? root.barlineSettingsModel.isSpanToNextStaff.value : false
        text: qsTrc("inspector", "Span to next staff")

        navigation.name: "SpanToStaffCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: repeatStyleSection.navigationRowEnd + 1
        navigation.enabled: root.enabled

        onClicked: {
            if (root.barlineSettingsModel) {
                root.barlineSettingsModel.isSpanToNextStaff.value = !checked
            }
        }
    }

    SeparatorLine {
        anchors.margins: -10
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: spanFrom
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Span from")
            propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.spanFrom : null

            navigation.name: "SpanFrom"
            navigation.panel: root.navigationPanel
            navigationRowStart: spanToNextStaffCheckBox.navigation.row + 1
        }

        SpinBoxPropertyView {
            id: spanTo
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Span to")
            propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.spanTo : null

            navigation.name: "SpanTo"
            navigation.panel: root.navigationPanel
            navigationRowStart: spanFrom.navigationRowEnd + 1
        }
    }

    FlatButton {
        id: applyToAllStaffsButton
        width: parent.width

        text: qsTrc("inspector", "Apply to all staffs")

        enabled: root.barlineSettingsModel
                 && !root.barlineSettingsModel.spanFrom.isUndefined
                 && !root.barlineSettingsModel.spanTo.isUndefined

        navigation.name: "ApplyToAllStaffs"
        navigation.panel: root.navigationPanel
        navigation.row: spanTo.navigationRowEnd + 1

        onClicked: {
            if (!root.staffSettingsModel || !root.barlineSettingsModel) {
                return
            }

            root.staffSettingsModel.barlinesSpanFrom.value = barlineSettingsModel.spanFrom.currentValue
            root.staffSettingsModel.barlinesSpanTo.value = barlineSettingsModel.spanTo.currentValue
        }
    }

    Column {
        id: presetButtons

        spacing: 8

        width: parent.width

        StyledTextLabel {
            id: spanPresetsLabel
            text: qsTrc("inspector", "Span presets")
        }

        RowLayout {
            width: parent.width
            spacing: 4

            FlatButton {
                text: qsTrc("inspector", "Default")
                Layout.fillWidth: true

                navigation.name: "Default"
                navigation.panel: root.navigationPanel
                navigation.row: applyToAllStaffsButton.navigation.row + 1
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_DEFAULT) }
            }

            FlatButton {
                text: qsTrc("inspector", "Tick 1")
                Layout.fillWidth: true

                navigation.name: "Tick1"
                navigation.panel: root.navigationPanel
                navigation.row: applyToAllStaffsButton.navigation.row + 2
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_1) }
            }

            FlatButton {
                text: qsTrc("inspector", "Tick 2")
                Layout.fillWidth: true

                navigation.name: "Tick2"
                navigation.panel: root.navigationPanel
                navigation.row: applyToAllStaffsButton.navigation.row + 3
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_2) }
            }
        }

        RowLayout {
            width: parent.width
            spacing: 4

            FlatButton {
                text: qsTrc("inspector", "Short 1")
                Layout.fillWidth: true

                navigation.name: "Short1"
                navigation.panel: root.navigationPanel
                navigation.row: applyToAllStaffsButton.navigation.row + 4
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_1) }
            }

            FlatButton {
                text: qsTrc("inspector", "Short 2")
                Layout.fillWidth: true

                navigation.name: "Short2"
                navigation.panel: root.navigationPanel
                navigation.row: applyToAllStaffsButton.navigation.row + 5
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_2) }
            }
        }
    }
}

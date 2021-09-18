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

    spacing: 12

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Style")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.type : null

        Dropdown {
            id: styles

            width: parent.width

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

            currentIndex: root.barlineSettingsModel && !root.barlineSettingsModel.type.isUndefined ? styles.indexOfValue(root.barlineSettingsModel.type.value) : -1

            onCurrentValueChanged: {
                if (currentIndex == -1) {
                    return
                }

                if (root.barlineSettingsModel) {
                    root.barlineSettingsModel.type.value = styles.currentValue
                }
            }
        }
    }

    FlatRadioButtonGroupPropertyView {
        titleText: qsTrc("inspector", "Repeat style")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.hasToShowTips : null

        visible: root.barlineSettingsModel && root.barlineSettingsModel.isRepeatStyleChangingAllowed

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
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Span from")
            propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.spanFrom : null
        }

        SpinBoxPropertyView {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Span to")
            propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.spanTo : null
        }
    }

    FlatButton {
        width: parent.width

        text: qsTrc("inspector", "Apply to all staffs")

        enabled: root.barlineSettingsModel
                 && !root.barlineSettingsModel.spanFrom.isUndefined
                 && !root.barlineSettingsModel.spanTo.isUndefined

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
            text: qsTrc("inspector", "Span presets")
        }

        RowLayout {
            width: parent.width
            spacing: 4

            FlatButton {
                text: qsTrc("inspector", "Default")
                Layout.fillWidth: true
                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_DEFAULT) }
            }

            FlatButton {
                text: qsTrc("inspector", "Tick 1")
                Layout.fillWidth: true
                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_1) }
            }

            FlatButton {
                text: qsTrc("inspector", "Tick 2")
                Layout.fillWidth: true
                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_2) }
            }
        }

        RowLayout {
            width: parent.width
            spacing: 4

            FlatButton {
                text: qsTrc("inspector", "Short 1")
                Layout.fillWidth: true
                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_1) }
            }

            FlatButton {
                text: qsTrc("inspector", "Short 2")
                Layout.fillWidth: true
                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_2) }
            }
        }
    }
}

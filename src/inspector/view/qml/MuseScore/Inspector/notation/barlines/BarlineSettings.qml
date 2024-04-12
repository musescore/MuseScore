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
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "BarlineSettings"

    property QtObject barlineSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_BARLINE) : null
    property QtObject staffSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_STAFF) : null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    DropdownPropertyView {
        id: styleSection
        titleText: qsTrc("inspector", "Style")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.type : null

        navigationName: "Style"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "Single barline"), value: BarlineTypes.TYPE_NORMAL },
            { text: qsTrc("inspector", "Double barline"), value: BarlineTypes.TYPE_DOUBLE },
            { text: qsTrc("inspector", "Dashed barline"), value: BarlineTypes.TYPE_DASHED },
            { text: qsTrc("inspector", "Final barline"), value: BarlineTypes.TYPE_FINAL },
            { text: qsTrc("inspector", "Dotted barline"), value: BarlineTypes.TYPE_DOTTED },
            { text: qsTrc("inspector", "Reverse final barline"), value: BarlineTypes.TYPE_REVERSE_END },
            { text: qsTrc("inspector", "Heavy barline"), value: BarlineTypes.TYPE_HEAVY },
            { text: qsTrc("inspector", "Heavy double barline"), value: BarlineTypes.TYPE_DOUBLE_HEAVY },
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: repeatStyleSection
        titleText: qsTrc("inspector", "Repeat style")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.hasToShowTips : null

        visible: root.barlineSettingsModel && root.barlineSettingsModel.isRepeatStyleChangingAllowed

        navigationName: "RepeatStyle"
        navigationPanel: root.navigationPanel
        navigationRowStart: styleSection.navigationRowEnd + 1

        model: [
            { iconCode: IconCode.BARLINE_UNWINGED, value: false },
            { iconCode: IconCode.BARLINE_WINGED, value: true }
        ]
    }

    PropertyCheckBox {
        id: spanToNextStaffCheckBox

        navigation.name: "SpanToStaffCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: repeatStyleSection.navigationRowEnd + 1

        text: qsTrc("inspector", "Span to next staff")
        propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.isSpanToNextStaff : null
    }

    SeparatorLine { anchors.margins: -12 }

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

            decimals: 0
            step: 1
            minValue: -4
            maxValue: 99

            navigationName: "SpanFrom"
            navigationPanel: root.navigationPanel
            navigationRowStart: spanToNextStaffCheckBox.navigation.row + 1
        }

        SpinBoxPropertyView {
            id: spanTo
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Span to")
            propertyItem: root.barlineSettingsModel ? root.barlineSettingsModel.spanTo : null

            decimals: 0
            step: 1
            minValue: -99
            maxValue: 99

            navigationName: "SpanTo"
            navigationPanel: root.navigationPanel
            navigationRowStart: spanFrom.navigationRowEnd + 1
        }
    }

    FlatButton {
        id: setAsStaffDefaultButton
        width: parent.width

        text: qsTrc("inspector", "Set as staff default")

        enabled: root.barlineSettingsModel

        navigation.name: "SetAsStaffDefault"
        navigation.panel: root.navigationPanel
        navigation.row: spanTo.navigationRowEnd + 1

        onClicked: {
            root.barlineSettingsModel.setSpanIntervalAsStaffDefault()
        }
    }

    Column {
        id: presetButtons

        spacing: 8

        width: parent.width

        StyledTextLabel {
            id: spanPresetsLabel
            width: parent.width
            text: qsTrc("inspector", "Span presets")
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            width: parent.width
            spacing: 4

            FlatButton {
                text: qsTrc("inspector", "Default")
                Layout.fillWidth: true

                navigation.name: "Default"
                navigation.panel: root.navigationPanel
                navigation.row: setAsStaffDefaultButton.navigation.row + 1
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_DEFAULT) }
            }

            FlatButton {
                text: qsTrc("inspector", "Tick 1")
                Layout.fillWidth: true

                navigation.name: "Tick1"
                navigation.panel: root.navigationPanel
                navigation.row: setAsStaffDefaultButton.navigation.row + 2
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_TICK_1) }
            }

            FlatButton {
                text: qsTrc("inspector", "Tick 2")
                Layout.fillWidth: true

                navigation.name: "Tick2"
                navigation.panel: root.navigationPanel
                navigation.row: setAsStaffDefaultButton.navigation.row + 3
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
                navigation.row: setAsStaffDefaultButton.navigation.row + 4
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_1) }
            }

            FlatButton {
                text: qsTrc("inspector", "Short 2")
                Layout.fillWidth: true

                navigation.name: "Short2"
                navigation.panel: root.navigationPanel
                navigation.row: setAsStaffDefaultButton.navigation.row + 5
                navigation.accessible.name: spanPresetsLabel.text + " " + text

                onClicked: { root.barlineSettingsModel.applySpanPreset(BarlineTypes.PRESET_SHORT_2) }
            }
        }
    }
}

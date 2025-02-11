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
import QtQuick.Layouts 1.15
import QtQuick.Controls

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledFlickable {
    id: root

    contentWidth: column.width
    contentHeight: column.height

    ClefKeyTimeSigPageModel {
        id: pageModel
    }

    readonly property StyleItem numeralStyle: pageModel.timeSigPlacement.value === 0 ? pageModel.timeSigNormalStyle
                                            : pageModel.timeSigPlacement.value === 1 ? pageModel.timeSigAboveStyle
                                                                                     : pageModel.timeSigAcrossStyle

    readonly property StyleItem timeSigAlignment: pageModel.timeSigPlacement.value === 0 ? null
                                                : pageModel.timeSigPlacement.value === 1 ? pageModel.timeSigCenterOnBarline
                                                                                         : pageModel.timeSigCenterAcrossStaveGroup

    readonly property StyleItem timeSigScale: pageModel.timeSigPlacement.value === 0 ? pageModel.timeSigNormalScale
                                            : pageModel.timeSigPlacement.value === 1 ? pageModel.timeSigAboveScale
                                                                                     : pageModel.timeSigAcrossScale

    readonly property StyleItem scaleLock: pageModel.timeSigPlacement.value === 0 ? pageModel.timeSigNormalScaleLock
                                         : pageModel.timeSigPlacement.value === 1 ? pageModel.timeSigAboveScaleLock
                                                                                  : pageModel.timeSigAcrossScaleLock

    readonly property StyleItem numDist: pageModel.timeSigPlacement.value === 0 ? pageModel.timeSigNormalNumDist
                                       : pageModel.timeSigPlacement.value === 1 ? pageModel.timeSigAboveNumDist
                                                                                : pageModel.timeSigAcrossNumDist

    readonly property StyleItem yPos: pageModel.timeSigPlacement.value === 0 ? pageModel.timeSigNormalY
                                    : pageModel.timeSigPlacement.value === 1 ? pageModel.timeSigAboveY
                                                                             : pageModel.timeSigAcrossY

    readonly property StyleItem timeSigVsMargin: pageModel.timeSigCenterOnBarline.value === true ? pageModel.timeSigVSMarginCentered
                                                                                                 : pageModel.timeSigVSMarginNonCentered

    ColumnLayout {
        id: column
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/timesignatures", "Clefs")

            ColumnLayout {
                width: parent.width
                spacing: 10

                ItemWithTitle {
                    title: qsTrc("notation/editstyle/timesignatures", "Visibility:")

                    RadioButtonGroup {
                        orientation: ListView.Vertical
                        spacing: 8

                        model: [
                            {text: qsTrc("notation/editstyle/timesignatures", "Show all clefs on every system"), value: true },
                            {text: qsTrc("notation/editstyle/timesignatures", "Hide all clefs after the first system where they appear"), value: false },
                        ]

                        delegate: RoundedRadioButton {
                            text: modelData.text
                            checked: pageModel.genClef.value === modelData.value
                            onToggled: pageModel.genClef.value = modelData.value
                        }
                    }
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/timesignatures", "Hide tab clefs after the first system where they appear")
                    enabled: pageModel.genClef.value === true
                    checked: pageModel.hideTabClefAfterFirst.value === true
                    onClicked: pageModel.hideTabClefAfterFirst.value = !pageModel.hideTabClefAfterFirst.value
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/timesignatures", "Show courtesy clefs")
                    checked: pageModel.genCourtesyClef.value === true
                    onClicked: pageModel.genCourtesyClef.value = !pageModel.genCourtesyClef.value
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/timesignatures", "Default TAB clef")

                    RadioButtonGroup {
                        orientation: ListView.Vertical
                        spacing: 8

                        model: [
                            {text: qsTrc("notation/editstyle/timesignatures", "Standard TAB clef"), value: 31 },
                            {text: qsTrc("notation/editstyle/timesignatures", "Serif TAB clef"), value: 33 },
                        ]

                        delegate: RoundedRadioButton {
                            text: modelData.text
                            checked: pageModel.tabClef.value === modelData.value
                            onToggled: pageModel.tabClef.value = modelData.value
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation/editstyle/timesignatures", "Time signatures")

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.preferredHeight: 144
                    spacing: 12

                    ItemWithTitle {
                        Layout.alignment: Qt.AlignTop
                        Layout.fillWidth: true
                        spacing: 8
                        title: qsTrc("notation/editstyle/timesignatures", "Position:")

                        RadioButtonGroup {
                            orientation: ListView.Vertical
                            spacing: 8

                            model: [
                                {text: qsTrc("notation/editstyle/timesignatures", "On all staves"), value: 0 },
                                {text: qsTrc("notation/editstyle/timesignatures", "Above staves"), value: 1 },
                                {text: qsTrc("notation/editstyle/timesignatures", "Across staves"), value: 2 },
                            ]

                            delegate: RoundedRadioButton {
                                text: modelData.text
                                checked: pageModel.timeSigPlacement.value === modelData.value
                                onToggled: pageModel.timeSigPlacement.value = modelData.value
                            }
                        }

                        StyledTextLabel {
                            visible: pageModel.timeSigPlacement.value !== 0
                            text: qsTrc("notation/editstyle/timesignatures", "Set where time signatures appear using the Layout panel.")
                        }
                    }

                    StyledImage {
                        forceHeight: 120
                        source: pageModel.timeSigPlacement.value === 0 ? "timeSigImages/timesig-on_all_staves.png"
                              : pageModel.timeSigPlacement.value === 1 ? "timeSigImages/timesig-above_staves.png"
                                                                       : "timeSigImages/timesig-across_staves.png"
                    }
                }

                StyledGroupBox {
                    id: styleGroupBox
                    Layout.fillWidth: true

                    title: qsTrc("notation/editstyle/timesignatures", "Style and size")

                    property double labelWidth: 150

                    ColumnLayout {
                        width: parent.width
                        spacing: 12

                        RowLayout {
                            spacing: 12

                            StyledTextLabel {
                                Layout.preferredWidth: styleGroupBox.labelWidth
                                horizontalAlignment: Text.AlignLeft
                                text: qsTrc("notation/editstyle/timesignatures", "Numeral style:")
                            }

                            RadioButtonGroup {
                                Layout.preferredHeight: 56
                                Layout.fillWidth: true

                                model: [
                                    { iconCode: IconCode.TIMESIG_STANDARD, value: 0 },
                                    { iconCode: IconCode.TIMESIG_NARROW, value: 1 },
                                    { iconCode: IconCode.TIMESIG_SANSSERIF, value: 2 },
                                ]

                                delegate: FlatRadioButton {
                                    width: 40

                                    StyledIconLabel {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                        iconCode: modelData.iconCode
                                        font.pixelSize: 16
                                    }

                                    checked: root.numeralStyle.value === modelData.value
                                    onToggled: root.numeralStyle.value = modelData.value
                                }
                            }

                            FlatButton {
                                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                                icon: IconCode.UNDO
                                enabled: !(root.numeralStyle.isDefault && (!root.timeSigAlignment || root.timeSigAlignment.isDefault)
                                           && root.timeSigScale.isDefault && root.scaleLock.isDefault
                                           && root.numDist.isDefault && root.yPos.isDefault)
                                onClicked: pageModel.resetStyleAndSize()
                            }
                        }

                        RowLayout {
                            id: alignmentWithBarlines
                            visible: pageModel.timeSigPlacement.value !== 0
                            spacing: 12

                            StyledTextLabel {
                                Layout.preferredWidth: styleGroupBox.labelWidth
                                horizontalAlignment: Text.AlignLeft
                                text: pageModel.timeSigPlacement.value === 1 ? qsTrc("notation/editstyle/timesignatures", "Alignment with barlines:")
                                                                             : qsTrc("notation/editstyle/timesignatures", "Alignment across staves:")
                            }

                            RadioButtonGroup {
                                Layout.preferredHeight: 28

                                model: pageModel.timeSigPlacement.value === 1 ? [
                                    { iconCode: IconCode.ALIGN_LEFT, text: "", value: false },
                                    { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, text: "", value: true },
                                ] : [
                                    { iconCode: IconCode.ALIGN_TOP, text: "", value: false },
                                    { iconCode: IconCode.ALIGN_VERTICAL_CENTER, text: "", value: true },
                                ]

                                delegate: FlatRadioButton {
                                    width: 28

                                    StyledIconLabel {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                        iconCode: modelData.iconCode
                                        font.pixelSize: 16
                                    }

                                    checked: root.timeSigAlignment ? root.timeSigAlignment.value === modelData.value : false
                                    onToggled: function() {
                                        if (root.timeSigAlignment) {
                                            root.timeSigAlignment.value = modelData.value
                                        }
                                    }
                                }
                            }
                        }

                        RowLayout {
                            spacing: 12

                            StyledTextLabel {
                                Layout.preferredWidth: styleGroupBox.labelWidth
                                horizontalAlignment: Text.AlignLeft
                                text: qsTrc("notation/editstyle/timesignatures", "Scale:")
                            }

                            RowLayout {
                                spacing: 6

                                IncrementalPropertyControl {
                                    Layout.preferredWidth: 100
                                    decimals: 2
                                    measureUnitsSymbol: 'x'
                                    prefixIcon: IconCode.HORIZONTAL
                                    minValue: 1.0

                                    currentValue: root.timeSigScale.value.width
                                    onValueEdited: function(newValue) {
                                        if (root.scaleLock.value === true) {
                                            root.timeSigScale.value = Qt.size(newValue, newValue)
                                        } else {
                                            root.timeSigScale.value.width = newValue
                                        }
                                    }
                                }

                                IncrementalPropertyControl {
                                    Layout.preferredWidth: 100
                                    decimals: 2
                                    measureUnitsSymbol: 'x'
                                    prefixIcon: IconCode.VERTICAL
                                    minValue: 1.0

                                    currentValue: root.timeSigScale.value.height
                                    onValueEdited: function(newValue) {
                                        if (root.scaleLock.value === true) {
                                            root.timeSigScale.value = Qt.size(newValue, newValue)
                                        } else {
                                            root.timeSigScale.value.height = newValue
                                        }
                                    }
                                }

                                FlatButton {
                                    Layout.preferredWidth: 20
                                    Layout.preferredHeight: 20

                                    icon: root.scaleLock.value === true ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

                                    accentButton: root.scaleLock.value === true
                                    onClicked: function () {
                                        root.scaleLock.value = !root.scaleLock.value
                                        if (root.scaleLock.value === true) {
                                            root.timeSigScale.value.height = root.timeSigScale.value.width
                                        }
                                    }
                                }
                            }
                        }

                        RowLayout {
                            spacing: 12

                            StyledTextLabel {
                                Layout.preferredWidth: styleGroupBox.labelWidth
                                horizontalAlignment: Text.AlignLeft
                                text: qsTrc("notation/editstyle/timesignatures", "Gap between numbers\n(scaled):")
                            }

                            IncrementalPropertyControl {
                                Layout.preferredWidth: 100
                                decimals: 2
                                step: 0.1
                                measureUnitsSymbol: 'sp'

                                currentValue: root.numDist.value
                                onValueEdited: function(newValue) {
                                    root.numDist.value = newValue
                                }
                            }
                        }

                        RowLayout {
                            spacing: 12

                            enabled: !(pageModel.timeSigPlacement.value === 2 && pageModel.timeSigCenterAcrossStaveGroup.value === true)

                            StyledTextLabel {
                                Layout.preferredWidth: styleGroupBox.labelWidth
                                horizontalAlignment: Text.AlignLeft
                                text: qsTrc("notation/editstyle/timesignatures", "Vertical placement:")
                            }

                            IncrementalPropertyControl {
                                Layout.preferredWidth: 100
                                decimals: 2
                                measureUnitsSymbol: 'sp'

                                currentValue: root.yPos.value
                                onValueEdited: function(newValue) {
                                    root.yPos.value = newValue
                                }
                            }
                        }
                    }
                }

                CheckBox {
                    id: showCourtesy
                    text: qsTrc("notation/editstyle/timesignatures", "Show courtesy time signatures")
                    checked: pageModel.genCourtesyTimesig.value === true
                    onClicked: pageModel.genCourtesyTimesig.value = !pageModel.genCourtesyTimesig.value
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    visible: pageModel.timeSigPlacement.value === 1
                    label: null

                    RowLayout {
                        anchors.topMargin: 20
                        enabled: pageModel.genCourtesyTimesig.value === true
                        width: parent.width

                        ItemWithTitle {
                            Layout.alignment: Qt.AlignTop
                            Layout.fillWidth: true
                            spacing: 8
                            title: qsTrc("notation/editstyle/timesignatures", "End-of-staff alignment:")

                            RoundedRadioButton {
                                enabled: pageModel.timeSigCenterOnBarline.value === true
                                text: qsTrc("notation/editstyle/timesignatures", "Hang into page margin")
                                checked: root.timeSigVsMargin.value === 0
                                onToggled: root.timeSigVsMargin.value = 0
                            }
                            RoundedRadioButton {
                                text: qsTrc("notation/editstyle/timesignatures", "Inset time signature")
                                checked: root.timeSigVsMargin.value === 1
                                onToggled: root.timeSigVsMargin.value = 1
                            }
                            RoundedRadioButton {
                                text: qsTrc("notation/editstyle/timesignatures", "Inset barline")
                                checked: root.timeSigVsMargin.value === 2
                                onToggled: root.timeSigVsMargin.value = 2
                            }
                        }

                        StyledImage {
                            forceWidth: 120
                            source: root.timeSigVsMargin.value === 0 ? "timeSigImages/timesig-courtesy-hang.png"
                                  : root.timeSigVsMargin.value === 1 ? "timeSigImages/timesig-courtesy-right_align.png"
                                                                     : "timeSigImages/timesig-courtesy-create_space.png"
                        }
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation/editstyle/timesignatures", "Key signatures")

            ColumnLayout {
                spacing: 8

                ItemWithTitle {
                    spacing: 8
                    title: qsTrc("notation/editstyle/timesignatures", "Visibility:")

                    RadioButtonGroup {
                        orientation: ListView.Vertical
                        spacing: 8

                        model: [
                            {text: qsTrc("notation/editstyle/timesignatures", "Show on every system"), value: true },
                            {text: qsTrc("notation/editstyle/timesignatures", "Hide after the first system where they appear"), value: false },
                        ]

                        delegate: RoundedRadioButton {
                            text: modelData.text
                            checked: pageModel.genKeysig.value === modelData.value
                            onToggled: pageModel.genKeysig.value = modelData.value
                        }
                    }
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/timesignatures", "Show courtesy key signatures")
                    checked: pageModel.genCourtesyKeysig.value === true
                    onClicked: pageModel.genCourtesyKeysig.value = !pageModel.genCourtesyKeysig.value
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            title: qsTrc("notation/editstyle/timesignatures", "Clefs, key & time signatures at repeats and jumps")

            component CourtesyShowAndParenToggle: GridLayout {
                required property StyleItem showStyleItem
                required property StyleItem parensStyleItem
                property alias text: toggleText.text

                flow: GridLayout.TopToBottom

                required property string imageON
                required property string imageOFF
                required property string imageParens
                width: parent.width
                rowSpacing: 8

                RowLayout {
                    Layout.row: 0
                    Layout.column: 0
                    Layout.maximumWidth: 232
                    Layout.preferredWidth: 232
                    Layout.fillWidth: false
                    ToggleButton {
                        id: toggleButton
                        checked: showStyleItem.value === true
                        onToggled: {
                            showStyleItem.value = !showStyleItem.value
                        }
                    }

                    StyledTextLabel {
                        id: toggleText
                        Layout.alignment: Qt.AlignLeft
                        horizontalAlignment: Text.AlignLeft
                        Layout.maximumWidth: 232 - toggleButton.width
                        Layout.preferredWidth: 232 - toggleButton.width
                        wrapMode: Text.Wrap
                    }
                }


                CheckBox {
                    id: parensCheckbox
                    Layout.row: 1
                    Layout.column: 0
                    text: qsTrc("notation/editstyle/timesignatures", "Use parentheses")
                    checked: parensStyleItem.value === true
                    onClicked: parensStyleItem.value = !parensStyleItem.value

                    enabled: toggleButton.checked
                }

                Rectangle {
                    Layout.row: 0
                    Layout.column: 1
                    Layout.rowSpan: 3
                    Layout.alignment: Qt.AlignRight

                    id: display
                    width: image.width + 25
                    height: image.height + 20

                    color: "#ffffff"
                    border.color: ui.theme.strokeColor
                    radius: ui.theme.borderWidth

                    Image {
                        id: image

                        width: 370
                        height: 98
                        anchors.centerIn: parent
                        mipmap: true

                        fillMode: Image.PreserveAspectFit
                        source: toggleButton.checked ? (parensCheckbox.checked ? imageParens : imageON) : imageOFF
                    }
                }
            }

            ColumnLayout {
                spacing: 12
                anchors.fill: parent

                StyledTextLabel {
                    text: qsTrc("notation/editstyle/timesignatures", "Place all changes before the barline")
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/timesignatures", "At repeats")
                    checked: pageModel.changesBeforeBarlineRepeats.value === true
                    onClicked: pageModel.changesBeforeBarlineRepeats.value = !pageModel.changesBeforeBarlineRepeats.value
                }

                CheckBox {
                    text: qsTrc("notation/editstyle/timesignatures", "At all other jumps")
                    checked: pageModel.changesBeforeBarlineOtherJumps.value === true
                    onClicked: pageModel.changesBeforeBarlineOtherJumps.value = !pageModel.changesBeforeBarlineOtherJumps.value
                }

                StyledGroupBox {
                    Layout.fillWidth: true

                    title: qsTrc("notation/editstyle/timesignatures", "Changes that apply only after repeats and jumps")

                    GridLayout {
                        width: parent.width
                        rowSpacing: 8

                        RoundedRadioButton {
                            id: clefBeforeRepeatsButton
                            Layout.row: 0
                            Layout.column: 0
                            text: qsTrc("notation/editstyle/timesignatures", "Place clefs before repeats")
                            checked: pageModel.placeClefsBeforeRepeats.value === true
                            onToggled: pageModel.placeClefsBeforeRepeats.value = true
                        }

                        RoundedRadioButton {
                            Layout.row: 1
                            Layout.column: 0
                            Layout.alignment: Qt.AlignTop
                            text: qsTrc("notation/editstyle/timesignatures", "Place clefs after repeats")
                            checked: pageModel.placeClefsBeforeRepeats.value === false
                            onToggled: pageModel.placeClefsBeforeRepeats.value = false
                        }

                        Rectangle {
                            Layout.row: 0
                            Layout.column: 1
                            Layout.rowSpan: 2
                            Layout.alignment: Qt.AlignRight

                            id: clefBeforeDisplay
                            width: clefBeforeImage.width + 25
                            height: clefBeforeImage.height + 20

                            color: "#ffffff"
                            border.color: ui.theme.strokeColor
                            radius: ui.theme.borderWidth

                            Image {
                                id: clefBeforeImage

                                width: 136
                                height: 68
                                anchors.centerIn: parent
                                mipmap: true

                                fillMode: Image.PreserveAspectFit
                                source: clefBeforeRepeatsButton.checked ? "courtesyImages/clef-before-true.png" : "courtesyImages/clef-before-false.png"
                            }
                        }

                        CheckBox {
                            id: betweenRepeatsCheckbox
                            Layout.row: 2
                            Layout.column: 0
                            text: qsTrc("notation/editstyle/timesignatures", "Allow changes between end-start repeats")
                            checked: pageModel.changesBetweenEndStartRepeat.value === true
                            onClicked: pageModel.changesBetweenEndStartRepeat.value = !pageModel.changesBetweenEndStartRepeat.value
                        }

                        Rectangle {
                            Layout.row: 2
                            Layout.column: 1
                            Layout.rowSpan: 2
                            Layout.alignment: Qt.AlignRight

                            id: endStartRepeatDisplay
                            width: endStartRepeatImage.width + 25
                            height: endStartRepeatImage.height + 20

                            color: "#ffffff"
                            border.color: ui.theme.strokeColor
                            radius: ui.theme.borderWidth

                            Image {
                                id: endStartRepeatImage

                                width: 136
                                height: 68
                                anchors.centerIn: parent
                                mipmap: true

                                fillMode: Image.PreserveAspectFit
                                source: clefBeforeRepeatsButton.checked ? (betweenRepeatsCheckbox.checked ? "courtesyImages/end-start-repeats-4"
                                                                                                          : "courtesyImages/end-start-repeats-3")
                                                                        : (betweenRepeatsCheckbox.checked ? "courtesyImages/end-start-repeats-1"
                                                                                                          : "courtesyImages/end-start-repeats-2")
                            }
                        }

                    }
                }

                StyledGroupBox {
                    Layout.fillWidth: true
                    title: qsTrc("notation/editstyle/timesignatures", "Courtesies")
                    width: parent.width

                    ColumnLayout {
                        width: parent.width
                        spacing: 12

                        CourtesyShowAndParenToggle {
                            showStyleItem: pageModel.showCourtesiesRepeats
                            parensStyleItem: pageModel.useParensRepeatCourtesies

                            imageON: "courtesyImages/repeat-2"
                            imageOFF: "courtesyImages/repeat-1"
                            imageParens: "courtesyImages/repeat-3"

                            text: qsTrc("notation/editstyle/timesignatures", "Show at repeats")
                        }

                        CourtesyShowAndParenToggle {
                            showStyleItem: pageModel.showCourtesiesOtherJumps
                            parensStyleItem: pageModel.useParensOtherJumpCourtesies

                            imageON: "courtesyImages/other-jump-2"
                            imageOFF: "courtesyImages/other-jump-1"
                            imageParens: "courtesyImages/other-jump-3"

                            text: qsTrc("notation/editstyle/timesignatures", "Show at all other jumps")
                        }

                        CourtesyShowAndParenToggle {
                            showStyleItem: pageModel.showCourtesiesAfterCancellingRepeats
                            parensStyleItem: pageModel.useParensRepeatCourtesiesAfterCancelling

                            imageON: "courtesyImages/repeat-cancel-2"
                            imageOFF: "courtesyImages/repeat-cancel-1"
                            imageParens: "courtesyImages/repeat-cancel-3"

                            text: qsTrc("notation/editstyle/timesignatures", "Show when cancelling a change before repeats")
                        }

                        CourtesyShowAndParenToggle {
                            showStyleItem: pageModel.showCourtesiesAfterCancellingOtherJumps
                            parensStyleItem: pageModel.useParensOtherJumpCourtesiesAfterCancelling

                            imageON: "courtesyImages/other-jump-cancel-2"
                            imageOFF: "courtesyImages/other-jump-cancel-1"
                            imageParens: "courtesyImages/other-jump-cancel-3"

                            text: qsTrc("notation/editstyle/timesignatures", "Show when cancelling a change before all other jumps")
                        }

                        CheckBox {
                            Layout.column: 0
                            Layout.row: 2
                            text: qsTrc("notation", "Small parentheses")
                            checked: pageModel.smallParens.value === true
                            onClicked: pageModel.smallParens.value = !pageModel.smallParens.value
                        }
                    }
                }
            }
        }
    }
}

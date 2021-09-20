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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PlaybackSettings"
        direction: NavigationPanel.Both
    }

    width: parent.width

    spacing: 12

    function focusOnFirst() {
        matchStaffSize.navigation.requestActive()
    }

    Item {
        height: childrenRect.height
        width: parent.width

        CheckBox {
            id: matchStaffSize
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2
            anchors.verticalCenter: subscriptOptionsButtonList.verticalCenter

            navigation.name: "Match staff size"
            navigation.panel: root.navigationPanel
            navigation.row: 1

            isIndeterminate: root.model ? root.model.isSizeSpatiumDependent.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isSizeSpatiumDependent.value : false
            text: qsTrc("inspector", "Match staff size")

            onClicked: { root.model.isSizeSpatiumDependent.value = !checked }
        }

        RadioButtonGroup {
            id: subscriptOptionsButtonList

            anchors.right: parent.right
            height: 30

            model: [
                { iconRole: IconCode.TEXT_SUBSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_BOTTOM },
                { iconRole: IconCode.TEXT_SUPERSCRIPT, typeRole: TextTypes.TEXT_SUBSCRIPT_TOP }
            ]

            delegate: FlatRadioButton {
                navigation.name: "ScriptOptions" + model.index
                navigation.panel: root.navigationPanel
                navigation.row: 2 + model.index

                width: 30
                normalStateColor: "transparent"

                iconCode: modelData["iconRole"]
                checked: root.model && !root.model.textScriptAlignment.isUndefined ? root.model.textScriptAlignment.value === modelData["typeRole"]
                                                                                   : false
                onClicked: {
                    if (root.model.textScriptAlignment.value === modelData["typeRole"]) {
                        root.model.textScriptAlignment.value = TextTypes.TEXT_SUBSCRIPT_NORMAL
                    } else {
                        root.model.textScriptAlignment.value = modelData["typeRole"]
                    }
                }
            }
        }
    }

    FlatRadioButtonGroupPropertyView {
        titleText: qsTrc("inspector", "Frame")
        propertyItem: root.model ? root.model.frameType : null

        navigation.name: "FrameMenu"
        navigation.panel: root.navigationPanel
        navigation.row: 4

        model: [
            { text: qsTrc("inspector", "None"), value: TextTypes.FRAME_TYPE_NONE },
            { iconCode: IconCode.FRAME_SQUARE, value: TextTypes.FRAME_TYPE_SQUARE },
            { iconCode: IconCode.FRAME_CIRCLE, value: TextTypes.FRAME_TYPE_CIRCLE }
        ]
    }

    Item {
        height: childrenRect.height
        width: parent.width

        ColorSection {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigation.name: "BorderColorMenu"
            navigation.panel: root.navigationPanel
            navigation.row: 9

            visible: root.model ? root.model.frameBorderColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Border")
            propertyItem: root.model ? root.model.frameBorderColor : null
        }

        ColorSection {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigation.name: "HighlightColorMenu"
            navigation.panel: root.navigationPanel
            navigation.row: 11

            visible: root.model ? root.model.frameHighlightColor.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Highlight")
            propertyItem: root.model ? root.model.frameHighlightColor : null
        }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigation.name: "Thickness"
            navigation.panel: root.navigationPanel
            navigation.row: 13

            visible: root.model ? root.model.frameThickness.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.model ? root.model.frameThickness : null

            step: 0.1
            minValue: 0.1
            maxValue: 5
        }

        SpinBoxPropertyView {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            navigation.name: "Margin"
            navigation.panel: root.navigationPanel
            navigation.row: 15

            visible: root.model ? root.model.frameMargin.isEnabled : false
            height: visible ? implicitHeight : 0

            titleText: qsTrc("inspector", "Margin")
            propertyItem: root.model ? root.model.frameMargin : null

            step: 0.1
            minValue: 0
            maxValue: 5
        }
    }

    SpinBoxPropertyView {
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        navigation.name: "Corner radius"
        navigation.panel: root.navigationPanel
        navigation.row: 17

        visible: root.model ? root.model.frameCornerRadius.isEnabled : false
        height: visible ? implicitHeight : 0

        titleText: qsTrc("inspector", "Corner radius")
        propertyItem: root.model ? root.model.frameCornerRadius : null

        step: 0.1
        minValue: 0
        maxValue: 5
    }

    SeparatorLine { anchors.margins: -10 }

    DropdownPropertyView {
        titleText: qsTrc("inspector", "Text style")
        propertyItem: root.model ? root.model.textType : null

        navigation.name: "Text style"
        navigation.panel: root.navigationPanel
        navigation.row: 19

        model: [
            { text: qsTrc("inspector", "Title"), value: TextTypes.TEXT_TYPE_TITLE },
            { text: qsTrc("inspector", "Subtitle"), value: TextTypes.TEXT_TYPE_SUBTITLE},
            { text: qsTrc("inspector", "Composer"), value: TextTypes.TEXT_TYPE_COMPOSER },
            { text: qsTrc("inspector", "Lyricist"), value: TextTypes.TEXT_TYPE_LYRICS_ODD },
            { text: qsTrc("inspector", "Translator"), value: TextTypes.TEXT_TYPE_TRANSLATOR },
            { text: qsTrc("inspector", "Frame"), value: TextTypes.TEXT_TYPE_FRAME },
            { text: qsTrc("inspector", "Header"), value: TextTypes.TEXT_TYPE_HEADER },
            { text: qsTrc("inspector", "Footer"), value: TextTypes.TEXT_TYPE_FOOTER },
            { text: qsTrc("inspector", "Measure number"), value: TextTypes.TEXT_TYPE_MEASURE_NUMBER },
            { text: qsTrc("inspector", "Instrument name (Part)"), value: TextTypes.TEXT_TYPE_INSTRUMENT_EXCERPT },
            { text: qsTrc("inspector", "Instrument change"), value: TextTypes.TEXT_TYPE_INSTRUMENT_CHANGE },
            { text: qsTrc("inspector", "Staff"), value: TextTypes.TEXT_TYPE_STAFF },
            { text: qsTrc("inspector", "System"), value: TextTypes.TEXT_TYPE_SYSTEM },
            { text: qsTrc("inspector", "Expression"), value: TextTypes.TEXT_TYPE_EXPRESSION },
            { text: qsTrc("inspector", "Dynamics"), value: TextTypes.TEXT_TYPE_DYNAMICS },
            { text: qsTrc("inspector", "Hairpin"), value: TextTypes.TEXT_TYPE_HAIRPIN },
            { text: qsTrc("inspector", "Tempo"), value: TextTypes.TEXT_TYPE_TEMPO },
            { text: qsTrc("inspector", "Rehearshal mark"), value: TextTypes.TEXT_TYPE_REHEARSAL_MARK },
            { text: qsTrc("inspector", "Repeat text left"), value: TextTypes.TEXT_TYPE_REPEAT_LEFT },
            { text: qsTrc("inspector", "Repeat text right"), value: TextTypes.TEXT_TYPE_REPEAT_RIGHT },
            { text: qsTrc("inspector", "Lyrics odd lines"), value: TextTypes.TEXT_TYPE_LYRICS_ODD },
            { text: qsTrc("inspector", "Lyrics even lines"), value: TextTypes.TEXT_TYPE_LYRICS_EVEN },
            { text: qsTrc("inspector", "Chord symbol"), value: TextTypes.TEXT_TYPE_HARMONY_A },
            { text: qsTrc("inspector", "Chord symbol (Alternate)"), value: TextTypes.TEXT_TYPE_HARMONY_B },
            { text: qsTrc("inspector", "Roman numeral analysis"), value: TextTypes.TEXT_TYPE_HARMONY_ROMAN },
            { text: qsTrc("inspector", "Nashville number"), value: TextTypes.TEXT_TYPE_HARMONY_NASHVILLE },
            { text: qsTrc("inspector", "Sticking"), value: TextTypes.TEXT_TYPE_STICKING }
        ]
    }

    PlacementSection {
        propertyItem: root.model ? root.model.textPlacement : null
    }

    FlatButton {
        width: parent.width

        navigation.name: "Staff text properties"
        navigation.panel: root.navigationPanel
        navigation.row: 24

        text: qsTrc("inspector", "Staff text properties")

        visible: root.model ? root.model.areStaffTextPropertiesAvailable : false

        onClicked: {
            root.model.showStaffTextProperties()
        }
    }
}

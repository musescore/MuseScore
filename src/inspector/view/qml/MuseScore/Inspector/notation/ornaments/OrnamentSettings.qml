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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "OrnamentSettings"

    spacing: 12

    function focusOnFirst() {
        intervalAbove.focusOnFirst()
    }

    DropdownPropertyView {
        id: intervalAbove
        visible: root.model ? root.model.isIntervalAboveAvailable : false

        titleText: qsTrc("inspector", "Interval above")
        propertyItem: root.model ? root.model.intervalAbove : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "Auto (diatonic)"), value: OrnamentTypes.TYPE_AUTO_DIATONIC},
            { text: qsTrc("inspector", "Minor second"), value: OrnamentTypes.TYPE_MINOR_SECOND},
            { text: qsTrc("inspector", "Major second"), value: OrnamentTypes.TYPE_MAJOR_SECOND},
            { text: qsTrc("inspector", "Augmented second"), value: OrnamentTypes.TYPE_AUGMENTED_SECOND},
        ]
    }

    DropdownPropertyView {
        id: intervalBelow
        visible: root.model ? root.model.isIntervalBelowAvailable : false

        titleText: qsTrc("inspector", "Interval below")
        propertyItem: root.model ? root.model.intervalBelow : null

        navigationPanel: root.navigationPanel
        navigationRowStart: intervalAbove.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto (diatonic)"), value: OrnamentTypes.TYPE_AUTO_DIATONIC},
            { text: qsTrc("inspector", "Minor second"), value: OrnamentTypes.TYPE_MINOR_SECOND},
            { text: qsTrc("inspector", "Major second"), value: OrnamentTypes.TYPE_MAJOR_SECOND},
            { text: qsTrc("inspector", "Augmented second"), value: OrnamentTypes.TYPE_AUGMENTED_SECOND},
        ]
    }

    InspectorPropertyView {
        id: interval
        visible: root.model ? root.model.isFullIntervalChoiceAvailable : false

        titleText: qsTrc("inspector", "Interval")
        propertyItem: root.model ? root.model.intervalAbove : null

        onRequestResetToDefault: {
            root.model.intervalStep.resetToDefault()
            root.model.intervalType.resetToDefault()
        }

        navigationName: "Interval"
        navigationPanel: root.navigationPanel
        navigationRowStart: intervalBelow.navigationRowEnd + 1
        navigationRowEnd: intervalStep.navigation.row

        function focusOnFirst() {
            intervalType.navigation.requestActive()
        }

        Item {
            width: parent.width
            height: childrenRect.height

            StyledDropdown {
                id: intervalType

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigation.name: interval.navigationName + " Dropdown"
                navigation.panel: interval.navigationPanel
                navigation.row: interval.navigationRowStart + 1
                navigation.accessible.name: interval.titleText + " " + currentText

                model: root.model && root.model.isPerfectStep ? [
                    { text: qsTrc("inspector", "Auto (diatonic)"), value: OrnamentTypes.TYPE_AUTO},
                    { text: qsTrc("inspector", "Augmented"), value: OrnamentTypes.TYPE_AUGMENTED},
                    { text: qsTrc("inspector", "Perfect"), value: OrnamentTypes.TYPE_PERFECT},
                    { text: qsTrc("inspector", "Diminished"), value: OrnamentTypes.TYPE_DIMINISHED},
                ] : [
                    { text: qsTrc("inspector", "Auto (diatonic)"), value: OrnamentTypes.TYPE_AUTO},
                    { text: qsTrc("inspector", "Augmented"), value: OrnamentTypes.TYPE_AUGMENTED},
                    //: Interval, not the mode of a key signature
                    { text: qsTrc("inspector", "Major", "interval quality"), value: OrnamentTypes.TYPE_MAJOR},
                    //: Interval, not the mode of a key signature
                    { text: qsTrc("inspector", "Minor", "interval quality"), value: OrnamentTypes.TYPE_MINOR},
                    { text: qsTrc("inspector", "Diminished"), value: OrnamentTypes.TYPE_DIMINISHED},
                ]

                currentIndex: root.model.intervalType && !root.model.intervalType.isUndefined
                              ? indexOfValue(root.model.intervalType.value) : -1

                onActivated: function(index, value) {
                    root.model.intervalType.value = value
                }
            }

            StyledDropdown {
                id: intervalStep

                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2

                navigation.name: interval.navigationName + " Dropdown"
                navigation.panel: interval.navigationPanel
                navigation.row: intervalType.navigation.row + 1
                navigation.accessible.name: interval.titleText + " " + currentText

                model:  [
                    { text: qsTrc("inspector", "Unison"), value: OrnamentTypes.STEP_UNISON},
                    { text: qsTrc("inspector", "Second"), value: OrnamentTypes.STEP_SECOND},
                    { text: qsTrc("inspector", "Third"), value: OrnamentTypes.STEP_THIRD},
                    { text: qsTrc("inspector", "Fourth"), value: OrnamentTypes.STEP_FOURTH},
                    { text: qsTrc("inspector", "Fifth"), value: OrnamentTypes.STEP_FIFTH},
                    { text: qsTrc("inspector", "Sixth"), value: OrnamentTypes.STEP_SIXTH},
                    { text: qsTrc("inspector", "Seventh"), value: OrnamentTypes.STEP_SEVENTH},
                    { text: qsTrc("inspector", "Octave"), value: OrnamentTypes.STEP_OCTAVE},
                ]

                currentIndex: root.model.intervalStep && !root.model.intervalStep.isUndefined
                              ? indexOfValue(root.model.intervalStep.value) : -1

                onActivated: function(index, value) {
                    root.model.intervalStep.value = value
                }
            }
        }
    }

    DropdownPropertyView {
        id: showAccidental

        titleText: qsTrc("inspector", "Accidental visibility")
        propertyItem: root.model ? root.model.showAccidental : null

        navigationPanel: root.navigationPanel
        navigationRowStart: interval.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Default"), value: OrnamentTypes.SHOW_ACCIDENTAL_DEFAULT},
            { text: qsTrc("inspector", "Show any alteration"), value: OrnamentTypes.SHOW_ACCIDENTAL_ANY_ALTERATION},
            { text: qsTrc("inspector", "Always display an accidental"), value: OrnamentTypes.SHOW_ACCIDENTAL_ALWAYS},
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: showCueNote
        visible: root.model ? root.model.isFullIntervalChoiceAvailable : false

        titleText: qsTrc("inspector", "Cue note visibility")
        propertyItem: root.model ? root.model.showCueNote : null

        navigationPanel: root.navigationPanel
        navigationRowStart: interval.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: CommonTypes.AUTO_ON_OFF_AUTO},
            { text: qsTrc("inspector", "On"), value: CommonTypes.AUTO_ON_OFF_ON},
            { text: qsTrc("inspector", "Off"), value: CommonTypes.AUTO_ON_OFF_OFF},
        ]
    }

    PropertyCheckBox {
        id: startOnUpperNote
        visible: false // prepared for future option but not implemented yet

        navigation.name: "StartOnUpperNote"
        navigation.panel: root.navigationPanel
        navigation.row: showAccidental.navigationRowEnd + 1

        text: qsTrc("inspector", "Start on upper note")
        propertyItem: root.model ? root.model.startOnUpperNote : null
    }

    PlacementSection {
        titleText: qsTrc("inspector", "Placement")
        propertyItem: root.model ? root.model.placement : null

        navigationPanel: root.navigationPanel
        navigationRowStart: startOnUpperNote.navigation.row + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: ArticulationTypes.TYPE_AUTO },
            { text: qsTrc("inspector", "Above"), value: ArticulationTypes.TYPE_TOP },
            { text: qsTrc("inspector", "Below"), value: ArticulationTypes.TYPE_BOTTOM }
        ]
    }
}

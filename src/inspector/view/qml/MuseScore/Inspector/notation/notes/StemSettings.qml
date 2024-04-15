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

FocusableItem {
    id: root

    //@note Current design assumes that stems and hooks should be represented at the same tab,
    //      but semantically it's different things, so they should have different models
    property QtObject chordModel: null
    property QtObject stemModel: null
    property QtObject hookModel: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: root.width

        spacing: 12

        PropertyCheckBox {
            text: qsTrc("inspector", "Stemless")
            propertyItem: root.chordModel ? root.chordModel.isStemless : null

            navigation.name: "Stemless"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }

        PropertyCheckBox {
            text: qsTrc("inspector", "Show stem slash")
            propertyItem: root.chordModel ? root.chordModel.showStemSlash : null
            visible: root.chordModel ? root.chordModel.showStemSlashVisible : false
            enabled: root.chordModel ? root.chordModel.showStemSlashEnabled : false

            navigation.name: "Show stem slash"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2
        }

        DirectionSection {
            id: stemDirectionGroup

            titleText: qsTrc("inspector", "Stem direction")
            propertyItem: root.stemModel ? root.stemModel.stemDirection : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 3
        }

        Column {
            width: parent.width
            height: childrenRect.height
            spacing: 8

            StyledTextLabel {
                id: flagStyleLabel
                width: parent.width
                text: qsTrc("inspector", "Flag style")
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                id: flagStyleGroup

                width: parent.width
                height: 70

                readonly property int navigationRowStart: stemDirectionGroup.navigationRowEnd + 1
                readonly property int navigationRowEnd: navigationRowStart + count

                model: [
                    { iconCode: IconCode.NOTEFLAGS_TRADITIONAL, text: qsTrc("inspector", "Traditional", "Note flags"), value: false },
                    { iconCode: IconCode.NOTEFLAGS_STRAIGHT, text: qsTrc("inspector", "Straight", "Note flags"), value: true }
                ]

                delegate: FlatRadioButton {
                    height: 70

                    navigation.name: "FlagStyle" + modelData.text
                    navigation.panel: root.navigationPanel
                    navigation.row: flagStyleGroup.navigationRowStart + index
                    navigation.accessible.name: flagStyleLabel.text + " " + modelData.text

                    Column {
                        anchors.centerIn: parent
                        height: childrenRect.height
                        spacing: 8

                        StyledIconLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            iconCode: modelData.iconCode
                            font.pixelSize: 32
                        }

                        StyledTextLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.text
                        }
                    }

                    checked: root.stemModel && root.stemModel.useStraightNoteFlags === modelData.value
                    onToggled: {
                        if (root.stemModel) {
                            root.stemModel.useStraightNoteFlags = modelData.value
                        }
                    }
                }
            }
        }

        ExpandableBlank {
            id: showItem
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.row: flagStyleGroup.navigationRowEnd + 1

            contentItemComponent: Column {
                height: implicitHeight
                width: root.width

                spacing: 12

                Item {
                    height: childrenRect.height
                    width: parent.width

                    SpinBoxPropertyView {
                        id: thicknessView
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2

                        titleText: qsTrc("inspector", "Thickness")
                        propertyItem: root.stemModel ? root.stemModel.thickness : null

                        maxValue: 4
                        minValue: 0.01
                        step: 0.01

                        navigationName: "Thickness"
                        navigationPanel: root.navigationPanel
                        navigationRowStart: showItem.navigation.row + 1
                    }

                    SpinBoxPropertyView {
                        id: lengthView
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Length")
                        propertyItem: root.stemModel ? root.stemModel.length : null

                        maxValue: 10
                        minValue: -10

                        navigationName: "Length"
                        navigationPanel: root.navigationPanel
                        navigationRowStart: thicknessView.navigationRowEnd + 1
                    }
                }

                OffsetSection {
                    id: stemOffsetSection
                    titleText: qsTrc("inspector", "Stem offset")
                    propertyItem: root.stemModel ? root.stemModel.offset : null

                    navigationName: "StemOffset"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: lengthView.navigationRowEnd + 1
                }

                OffsetSection {
                    titleText: qsTrc("inspector", "Flag offset")
                    propertyItem: root.hookModel ? root.hookModel.offset : null

                    navigationName: "FlagOffset"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: stemOffsetSection.navigationRowEnd + 1
                }
            }
        }
    }
}

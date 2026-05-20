/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
pragma ComponentBehavior: Bound

import QtQuick

import MuseScore.PropertiesPanel
import Muse.UiComponents
import Muse.Ui

FocusableItem {
    id: root

    required property NoteheadSettingsModel headModel
    required property ChordSettingsModel chordModel

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        FlatRadioButtonGroupPropertyView {
            id: noteHeadParenthesesView

            titleText: qsTrc("propertiespanel", "Notehead parentheses")
            propertyItem: root.headModel ? root.headModel.hasHeadParentheses : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1

            model: [
                { iconCode: IconCode.NOTE_HEAD, value: NoteHead.PAREN_NONE, title: qsTrc("propertiespanel", "Normal notehead") },
                { iconCode: IconCode.NOTE_HEAD_PARENTHESES, value: NoteHead.PAREN_BOTH, title: qsTrc("propertiespanel", "Notehead with parentheses") }
            ]
        }

        NoteheadGroupSelector {
            id: noteHeadSection

            propertyItem: root.headModel ? root.headModel.headGroup : null

            navigationPanel: root.navigationPanel
            navigationRowStart: noteHeadParenthesesView.navigationRowEnd + 1
        }

        PropertyCheckBox {
            id: hideNoteheadCheckBox
            visible: root.headModel ? !root.headModel.isTrillCueNote : true

            text: qsTrc("propertiespanel", "Hide notehead")
            propertyItem: root.headModel ? root.headModel.isHeadHidden : null

            navigation.name: "HideNoteHeadBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadSection.navigationRowEnd + 1
        }

        PropertyCheckBox {
            id: smallNoteheadCheckBox

            text: qsTrc("propertiespanel", "Small notehead")
            propertyItem: root.headModel ? root.headModel.isHeadSmall : null

            navigation.name: "SmallNoteHeadBox"
            navigation.panel: root.navigationPanel
            navigation.row: hideNoteheadCheckBox.navigation.row + 1
        }

        FlatRadioButtonGroupPropertyView {
            id: durationDotPosition
            visible: root.headModel ? !root.headModel.isTrillCueNote : true

            titleText: qsTrc("propertiespanel", "Duration dot position")
            propertyItem: root.headModel ? root.headModel.dotPosition : null

            navigationName: "DottedNote"
            navigationPanel: root.navigationPanel
            navigationRowStart: smallNoteheadCheckBox.navigation.row + 1

            model: [
                { text: qsTrc("propertiespanel", "Auto"), value: NoteHead.DOT_POSITION_AUTO, title: qsTrc("propertiespanel", "Auto", "augmentation dot position") },
                { iconCode: IconCode.DOT_BELOW_LINE, value: NoteHead.DOT_POSITION_DOWN, title: qsTrc("propertiespanel", "Down", "augmentation dot position") },
                { iconCode: IconCode.DOT_ABOVE_LINE, value: NoteHead.DOT_POSITION_UP, title: qsTrc("propertiespanel", "Up", "augmentation dot position") }
            ]
        }

        ExpandableBlank {
            id: showItem
            visible: root.headModel ? !root.headModel.isTrillCueNote : true

            isExpanded: false

            title: isExpanded ? qsTrc("propertiespanel", "Show less") : qsTrc("propertiespanel", "Show more")

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.row: durationDotPosition.navigationRowEnd + 1

            contentItemComponent: Column {
                height: implicitHeight
                width: parent.width

                spacing: 12

                DropdownPropertyView {
                    id: noteHeadSystemSection

                    titleText: qsTrc("propertiespanel", "Notehead scheme")
                    propertyItem: root.headModel ? root.headModel.headSystem : null

                    model: root.headModel.possibleHeadSystemTypes()

                    navigationName: "NoteHeadSystemSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: showItem.navigation.row + 1
                }

                NoteheadTypeSelector {
                    id: noteHeadTypeSection
                    titleText: qsTrc("propertiespanel", "Override visual duration")
                    propertyItem: root.headModel ? root.headModel.headType : null

                    navigationName: "NoteHeadTypeSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteHeadSystemSection.navigationRowEnd + 1
                }

                DirectionSection {
                    id: noteDirectionSection

                    titleText: qsTrc("propertiespanel", "Note direction")
                    propertyItem: root.headModel ? root.headModel.headDirection : null

                    orientation: Qt.Horizontal

                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteHeadTypeSection.navigationRowEnd + 1
                }

                OffsetSection {
                    id: noteOffsetSection
                    titleText: qsTrc("propertiespanel", "Notehead offset")
                    propertyItem: root.headModel ? root.headModel.offset : null
                    measurementUnits: root.headModel?.measurementUnits ?? CommonTypes.UNITS_UNKNOWN

                    navigationName: "NoteHeadOffsetSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteDirectionSection.navigationRowEnd + 1
                }

                FlatRadioButtonGroupPropertyView {
                    id: centerStavesSection
                    propertyItem: root.chordModel ? root.chordModel.combineVoice : null

                    showTitle: true;
                    titleLabelComponent: Component {
                        id: centerStavesTitleLabel

                        StyledTextLabel {
                            width: parent.width
                            text: qsTrc("propertiespanel", "Combine with voices that share the same stem direction")
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideNone
                            wrapMode: Text.Wrap
                        }
                    }

                    navigationName: "Combine with voices that share the same stem direction"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteOffsetSection.navigationRowEnd + 1

                    model: [
                        { text: qsTrc("propertiespanel", "Auto"), value: CommonTypes.AUTO_ON_OFF_AUTO },
                        { text: qsTrc("propertiespanel", "On"), value: CommonTypes.AUTO_ON_OFF_ON },
                        { text: qsTrc("propertiespanel", "Off"), value: CommonTypes.AUTO_ON_OFF_OFF }
                    ]
                }
            }
        }
    }
}

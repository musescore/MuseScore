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
import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null

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

            titleText: qsTrc("inspector", "Notehead parentheses")
            propertyItem: root.model ? root.model.hasHeadParentheses : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1

            model: [
                { iconCode: IconCode.NOTE_HEAD, value: false, title: qsTrc("inspector", "Normal notehead") },
                { iconCode: IconCode.NOTE_HEAD_PARENTHESES, value: true, title: qsTrc("inspector", "Notehead with parentheses") }
            ]
        }

        NoteheadGroupSelector {
            id: noteHeadSection

            propertyItem: root.model ? root.model.headGroup : null

            navigationPanel: root.navigationPanel
            navigationRowStart: noteHeadParenthesesView.navigationRowEnd + 1
        }

        PropertyCheckBox {
            id: hideNoteheadCheckBox
            visible: root.model ? !root.model.isTrillCueNote : true

            text: qsTrc("inspector", "Hide notehead")
            propertyItem: root.model ? root.model.isHeadHidden : null

            navigation.name: "HideNoteHeadBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadSection.navigationRowEnd + 1
        }

        PropertyCheckBox {
            id: smallNoteheadCheckBox

            text: qsTrc("inspector", "Small notehead")
            propertyItem: root.model ? root.model.isHeadSmall : null

            navigation.name: "SmallNoteHeadBox"
            navigation.panel: root.navigationPanel
            navigation.row: hideNoteheadCheckBox.navigation.row + 1
        }

        FlatRadioButtonGroupPropertyView {
            id: durationDotPosition
            visible: root.model ? !root.model.isTrillCueNote : true

            titleText: qsTrc("inspector", "Duration dot position")
            propertyItem: root.model ? root.model.dotPosition : null

            navigationName: "DottedNote"
            navigationPanel: root.navigationPanel
            navigationRowStart: smallNoteheadCheckBox.navigation.row + 1

            model: [
                { text: qsTrc("inspector", "Auto"), value: NoteHead.DOT_POSITION_AUTO, title: qsTrc("inspector", "Auto") },
                { iconCode: IconCode.DOT_BELOW_LINE, value: NoteHead.DOT_POSITION_DOWN, title: qsTrc("inspector", "Down") },
                { iconCode: IconCode.DOT_ABOVE_LINE, value: NoteHead.DOT_POSITION_UP, title: qsTrc("inspector", "Up") }
            ]
        }

        OffsetSection {
            id: trillCueNoteOffsetSection
            visible: root.model ? root.model.isTrillCueNote : false

            titleText: qsTrc("inspector", "Notehead offset")
            propertyItem: root.model ? root.model.offset : null

            navigationName: "NoteHeadOffsetSection"
            navigationPanel: root.navigationPanel
            navigationRowStart: durationDotPosition.navigationRowEnd + 1
        }

        ExpandableBlank {
            id: showItem
            visible: root.model ? !root.model.isTrillCueNote : true

            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.row: trillCueNoteOffsetSection.navigationRowEnd + 1

            contentItemComponent: Column {
                height: implicitHeight
                width: parent.width

                spacing: 12

                DropdownPropertyView {
                    id: noteHeadSystemSection

                    titleText: qsTrc("inspector", "Notehead system")
                    propertyItem: root.model ? root.model.headSystem : null

                    model: root.model.possibleHeadSystemTypes()

                    navigationName: "NoteHeadSystemSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: showItem.navigation.row + 1
                }

                NoteheadTypeSelector {
                    id: noteHeadTypeSection
                    titleText: qsTrc("inspector", "Override visual duration")
                    propertyItem: root.model ? root.model.headType : null

                    navigationName: "NoteHeadTypeSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteHeadSystemSection.navigationRowEnd + 1
                }

                DirectionSection {
                    id: noteDirectionSection

                    titleText: qsTrc("inspector", "Note direction")
                    propertyItem: root.model ? root.model.headDirection : null

                    orientation: Qt.Horizontal

                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteHeadTypeSection.navigationRowEnd + 1
                }

                OffsetSection {
                    titleText: qsTrc("inspector", "Notehead offset")
                    propertyItem: root.model ? root.model.offset : null

                    navigationName: "NoteHeadOffsetSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteDirectionSection.navigationRowEnd + 1
                }
            }
        }
    }
}

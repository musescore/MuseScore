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

        CheckBoxPropertyView {
            text: qsTrc("inspector", "Hide notehead")
            propertyItem: root.model ? root.model.isHeadHidden : null

            navigation.name: "HideNoteHeadBox"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }

        NoteheadGroupSelector {
            id: noteHeadSection
            propertyItem: root.model ? root.model.headGroup : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 2
        }

        FlatRadioButtonGroupPropertyView {
            id: dottedNotePositionSection
            titleText: qsTrc("inspector", "Duration dot position")
            propertyItem: root.model ? root.model.dotPosition : null

            navigationName: "DottedNote"
            navigationPanel: root.navigationPanel
            navigationRowStart: noteHeadSection.navigationRowEnd + 1

            model: [
                { text: qsTrc("inspector", "Auto"), value: NoteHead.DOT_POSITION_AUTO, title: qsTrc("inspector", "Auto") },
                { iconCode: IconCode.DOT_BELOW_LINE, value: NoteHead.DOT_POSITION_DOWN, title: qsTrc("inspector", "Down") },
                { iconCode: IconCode.DOT_ABOVE_LINE, value: NoteHead.DOT_POSITION_UP, title: qsTrc("inspector", "Up") }
            ]
        }

        ExpandableBlank {
            id: showItem
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.row: dottedNotePositionSection.navigationRowEnd + 1

            contentItemComponent: Column {
                height: implicitHeight
                width: parent.width

                spacing: 12

                NoteheadTypeSelector {
                    id: noteHeadTypeSection
                    titleText: qsTrc("inspector", "Notehead type (visual only)")
                    propertyItem: root.model ? root.model.headType : null

                    navigationName: "NoteHeadTypeSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: showItem.navigation.row + 1
                }

                FlatRadioButtonGroupPropertyView {
                    id: noteDirectionSection
                    titleText: qsTrc("inspector", "Note direction")
                    propertyItem: root.model ? root.model.headDirection : null

                    navigationName: "NoteDirectionSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteHeadTypeSection.navigationRowEnd + 1

                    model: [
                        { text: qsTrc("inspector", "Auto"), value: NoteHead.DIRECTION_H_AUTO, title: qsTrc("inspector", "Auto") },
                        { iconCode: IconCode.ARROW_LEFT, value: NoteHead.DIRECTION_H_LEFT, title: qsTrc("inspector", "Left") },
                        { iconCode: IconCode.ARROW_RIGHT, value: NoteHead.DIRECTION_H_RIGHT, title: qsTrc("inspector", "Right") }
                    ]
                }

                OffsetSection {
                    titleText: qsTrc("inspector", "Notehead offset")
                    horizontalOffset: root.model ? root.model.horizontalOffset : null
                    verticalOffset: root.model ? root.model.verticalOffset : null

                    navigationName: "NoteHeadOffsetSection"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: noteDirectionSection.navigationRowEnd + 1
                }
            }
        }
    }
}

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

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        CheckBox {
            isIndeterminate: root.model ? root.model.isHeadHidden.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isHeadHidden.value : false
            text: qsTrc("inspector", "Hide notehead")

            onClicked: { root.model.isHeadHidden.value = !checked }
        }

        NoteheadGroupSelector {
            propertyItem: root.model ? root.model.headGroup : null
        }

        FlatRadioButtonGroupPropertyView {
            titleText: qsTrc("inspector", "Dotted note position")
            propertyItem: root.model ? root.model.dotPosition : null

            model: [
                { text: qsTrc("inspector", "Auto"), value: NoteHead.DOT_POSITION_AUTO },
                { iconCode: IconCode.DOT_ABOVE_LINE, value: NoteHead.DOT_POSITION_DOWN },
                { iconCode: IconCode.DOT_BELOW_LINE, value: NoteHead.DOT_POSITION_UP }
            ]
        }

        ExpandableBlank {
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            contentItemComponent: Column {
                height: implicitHeight
                width: parent.width

                spacing: 12

                NoteheadTypeSelector {
                    titleText: qsTrc("inspector", "Notehead type (visual only)")
                    propertyItem: root.model ? root.model.headType : null
                }

                FlatRadioButtonGroupPropertyView {
                    titleText: qsTrc("inspector", "Note direction")
                    propertyItem: root.model ? root.model.headDirection : null

                    model: [
                        { text: qsTrc("inspector", "Auto"), value: NoteHead.DIRECTION_H_AUTO },
                        { iconCode: IconCode.ARROW_LEFT, value: NoteHead.DIRECTION_H_LEFT },
                        { iconCode: IconCode.ARROW_RIGHT, value: NoteHead.DIRECTION_H_RIGHT }
                    ]
                }

                OffsetSection {
                    titleText: qsTrc("inspector", "Notehead offset")
                    horizontalOffset: root.model ? root.model.horizontalOffset : null
                    verticalOffset: root.model ? root.model.verticalOffset : null
                }
            }
        }
    }
}

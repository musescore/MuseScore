/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents

StyledPopupView {
    id: root

    //! NOTE: list of { title: string, items: array<string> }; content is opaque to this popup
    property var sections: []

    contentWidth: 320
    contentHeight: content.implicitHeight
    padding: 8

    //! NOTE: purely informational content — must never steal keyboard focus
    openPolicies: PopupView.NoActivateFocus
    navigationSection: null

    Column {
        id: content

        width: parent.width
        spacing: 14

        Repeater {
            model: root.sections

            Column {
                id: section

                required property var modelData

                readonly property int itemIndent: 18

                width: content.width
                spacing: 4

                StyledTextLabel {
                    width: parent.width
                    text: section.modelData.title
                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                Repeater {
                    model: section.modelData.items

                    Row {
                        id: requirementItem

                        required property string modelData

                        x: section.itemIndent
                        width: section.width - section.itemIndent
                        spacing: 4

                        StyledTextLabel {
                            id: bulletLabel
                            text: "•"
                        }

                        StyledTextLabel {
                            width: requirementItem.width - bulletLabel.width - requirementItem.spacing
                            wrapMode: Text.WordWrap
                            text: requirementItem.modelData
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            }
        }
    }
}

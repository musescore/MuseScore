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

    contentWidth: 280
    contentHeight: content.implicitHeight

    showArrow: false

    NavigationPanel {
        id: navPanel

        name: "FileRequirementsPopup"
        section: root.navigationSection
        order: 1
    }

    Column {
        id: content

        width: parent.width
        spacing: 12

        StyledTextLabel {
            width: parent.width

            text: qsTrc("project/convert", "File requirements")
            font.family: ui.theme.bodyBoldFont.family
            font.pixelSize: ui.theme.bodyBoldFont.pixelSize
            font.bold: true
            font.underline: true
            horizontalAlignment: Text.AlignLeft
        }

        Repeater {
            model: root.sections

            Column {
                id: section

                required property var modelData

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

                        x: 12
                        width: section.width - 12
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

        FlatButton {
            width: parent.width

            text: qsTrc("global", "Close")

            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                root.close()
            }
        }
    }
}

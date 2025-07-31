/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property alias notationViewNavigationSection: staffVisibilityNavPanel.section
    property alias navigationOrderStart: staffVisibilityNavPanel.order
    readonly property int navigationOrderEnd: 0

    contentWidth: contentColumn.width
    contentHeight: contentColumn.height

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        root.x = root.parent.width / 2 - root.contentWidth / 2
        root.y = root.parent.height + 4 // 4 for spacing
    }

    NavigationPanel {
        id: staffVisibilityNavPanel
        name: "StaffVisibility"
        direction: NavigationPanel.Horizontal
        accessible.name: qsTrc("notation", "Staff visibility popup")

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.Escape) {
                root.close()
            }
        }
    }

    Column {
        id: contentColumn

        width: 276
        spacing: 6

        Item {
            id: headerArea

            width: parent.width
            height: 36

            StyledTextLabel {
                id: title

                anchors.top: parent.top
                anchors.left: parent.left

                text: qsTrc("notation", "Hide empty staves")
                font: ui.theme.largeBodyBoldFont
            }

            StyledTextLabel {
                id: systemNumberLabel

                anchors.bottom: parent.bottom
                anchors.left: parent.left

                text: "System #" // TODO
                font: ui.theme.bodyFont
            }

            FlatButton {
                id: resetAllButton

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                icon: IconCode.UNDO
            }
        }

        SeparatorLine {
            width: root.parent.width
        }

        Column {
            id: staffVisibilityControlsColumn

            width: parent.width
            spacing: 4

            Repeater {
                model: 5

                VisibilityControls {
                    id: visibilityControls

                    navigationPanel: staffVisibilityNavPanel
                    navigationRow: 0

                    width: parent.width
                    height: implicitHeight

                    // TODO: Placeholder values/model...

                    title: "Staff/part name"

                    useVisibilityButton: true
                    isVisible: true

                    isExpandable: true
                    isExpanded: false

                    rightSideButtonComp: FlatButton {
                        id: resetButton

                        // TODO: visible: (can reset)

                        navigation.panel: visibilityControls.navigationPanel
                        navigation.row: visibilityControls.navigationRow
                        navigation.column: 3
                        navigation.accessible.name: qsTrc("notation", "Reset staff visibility")

                        icon: IconCode.UNDO

                        onClicked: {
                            console.log("Reset staff visibility")
                        }
                    }
                }
            }
        }
    }
}

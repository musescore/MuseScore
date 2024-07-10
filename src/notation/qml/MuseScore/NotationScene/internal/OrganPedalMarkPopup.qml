/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledPopupView {
    id: root

    property QtObject model: organPedalMarkModel

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: organPedalMarksNavPanel.order

    contentWidth: content.width
    contentHeight: content.height

    margins: 0

    showArrow: true

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        contentWidth = Math.min(content.implicitWidth, 300 - margins * 2)
        contentHeight = content.implicitHeight

        let isAbove = organPedalMarkModel.isAbove()

        root.x = root.parent.width / 2 - (contentWidth + padding * 2 + margins * 2) / 2
        root.y = isAbove ? -(contentHeight + 25) : root.parent.height + 1

        root.setOpensUpward(isAbove)
    }

    GridLayout {
        id: content
        columns:  5
        flow: GridLayout.TopToBottom
        columnSpacing: 10
        rowSpacing: 10

        OrganPedalMarkPopupModel {
            id: organPedalMarkModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            organPedalMarkModel.init()
        }

        NavigationPanel {
            id: organPedalMarksNavPanel
            name: "OrganPedalMarks"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Organ pedal marks")
        }

        Repeater {
            id: pedalMarkRepeater

            model: [
                { name: "keyboardPedalToe2", sym: "\uE665", offset: 8},
                { name: "keyboardPedalHeel1", sym: "\uE661", offset: 8},
                { name: "keyboardPedalHeel3", sym: "\uE663", offset: 8},
                { name: "keyboardPedalParensLeft", sym: "\uE676", offset: 8},
                { name: "keyboardPedalParensRight", sym: "\uE677", offset: 8},

                { name: "keyboardPedalToe1", sym: "\uE664", offset: 10},
                { name: "keyboardPedalHeel2", sym: "\uE662", offset: 9},
                { name: "keyboardPedalHeelToe", sym: "\uE666", offset: 12},
                { name: "keyboardPedalToeToHeel", sym: "\uE675", offset: 10},
                { name: "keyboardPedalHeelToToe", sym: "\uE674", offset: 10},

            ]

            FlatButton {
                id: pedalMarkButton

                Layout.column: index % content.columns
                Layout.row: index / content.columns

                Layout.topMargin: Layout.row == 0 ? 10 : 0
                Layout.bottomMargin: Layout.row == 1 ? 10 : 0
                Layout.leftMargin: Layout.column == 0 ? 10 : 0
                Layout.rightMargin: Layout.column == 4 ? 10 : 0

                minWidth: 12
                transparent: true

                StyledTextLabel {
                    id: pedalMarkLabel

                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: modelData.offset

                    text: modelData.sym
                    font.pixelSize: 30
                }

                navigation.name: modelData.name // TODO
                navigation.panel: organPedalMarksNavPanel
                navigation.order: index
                navigation.accessible.name: modelData.name // TODO

                onClicked: {
                    organPedalMarkModel.updatePedalMark(modelData.name)
                }
            }
        }
    }
}

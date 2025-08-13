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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root
    property variant pedalState: harpModel.pedalState

    property alias notationViewNavigationSection: pedalSettingsNavPanel.section
    property alias navigationOrderStart: pedalSettingsNavPanel.order
    readonly property alias navigationOrderEnd: isDiagramNavPanel.order

    contentWidth: menuItems.width
    contentHeight: menuItems.height

    margins: 0

    showArrow: false

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        const marginFromElement = 12
        var popupHeight = root.contentHeight + root.padding * 2

        // Above diagram
        let yUp = Math.min(-popupHeight - marginFromElement,
                           (harpModel.staffPos.y - root.parent.y) - contentHeight - marginFromElement)
        let yDown = Math.max(root.parent.height + marginFromElement,
                             (harpModel.staffPos.y - root.parent.y) + harpModel.staffPos.height + marginFromElement)

        // not enough room on window to open above so open below stave
        let opensUp = true
        let globPos = root.parent.mapToItem(ui.rootItem, Qt.point(root.x, yUp))
        if (globPos.y < 0) {
            opensUp = false;
            globPos = root.parent.mapToItem(ui.rootItem, Qt.point(root.x, yDown))
        }

        // not enough room below stave to open so open above
        if (globPos + contentHeight > ui.rootItem.height) {
            opensUp = true;
        }

        root.setPopupPosition(opensUp ? PopupPosition.Top : PopupPosition.Bottom)

        root.y = opensUp ? yUp : yDown
    }

    function checkPedalState(string, state) {
        return harpModel.pedalState[string] === state
    }

    function updatePedalState(string, state) {
        root.pedalState[string] = state
        harpModel.setDiagramPedalState(root.pedalState)
    }

    function getNoteName(string, state) {

        var noteNames = [
                [qsTrc("notation", "D flat"), qsTrc("notation", "D natural"), qsTrc("notation", "D sharp")],
                [qsTrc("notation", "C flat"), qsTrc("notation", "C natural"), qsTrc("notation", "C sharp")],
                [qsTrc("notation", "B flat"), qsTrc("notation", "B natural"), qsTrc("notation", "B sharp")],
                [qsTrc("notation", "E flat"), qsTrc("notation", "E natural"), qsTrc("notation", "E sharp")],
                [qsTrc("notation", "F flat"), qsTrc("notation", "F natural"), qsTrc("notation", "F sharp")],
                [qsTrc("notation", "G flat"), qsTrc("notation", "G natural"), qsTrc("notation", "G sharp")],
                [qsTrc("notation", "A flat"), qsTrc("notation", "A natural"), qsTrc("notation", "A sharp")]
        ]

        return noteNames[string][state]
    }

    GridLayout {
        id: menuItems
        rows: 5
        flow: GridLayout.TopToBottom
        columnSpacing: 10
        rowSpacing: 10

        HarpPedalPopupModel {
            id: harpModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            harpModel.init()
        }

        NavigationPanel {
            id: pedalSettingsNavPanel
            name: "PedalSettings"
            direction: NavigationPanel.Vertical
            accessible.name: qsTrc("notation", "Pedal settings buttons")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
        }

        // Accidental symbols
        Repeater {
            model: [IconCode.FLAT, IconCode.NATURAL, IconCode.SHARP]
            StyledIconLabel {
                Layout.row: index + 1
                Layout.leftMargin: 30
                Layout.preferredWidth: 20
                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                iconCode: modelData
            }
        }

        // String names
        Repeater {
            width: parent.width

            model: [
                { str: qsTrc("notation", "D"), col: 1 },
                { str: qsTrc("notation", "C"), col: 2 },
                { str: qsTrc("notation", "B"), col: 3 },
                { str: qsTrc("notation", "E"), col: 5 },
                { str: qsTrc("notation", "F"), col: 6 },
                { str: qsTrc("notation", "G"), col: 7 },
                { str: qsTrc("notation", "A"), col: 8 }
            ]

            StyledTextLabel {
                Layout.column: modelData.col
                Layout.row: 0
                Layout.topMargin: 15
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.rightMargin: (modelData.col === 8) ? 30 : 0

                font: ui.theme.largeBodyBoldFont

                text: modelData.str
            }
        }

        ButtonGroup {
            id: dGroup
        }

        ButtonGroup {
            id: cGroup
        }

        ButtonGroup {
            id: bGroup
        }

        ButtonGroup {
            id: eGroup
        }

        ButtonGroup {
            id: fGroup
        }

        ButtonGroup {
            id: gGroup
        }

        ButtonGroup {
            id: aGroup
        }

        // Button repeater
        Repeater {
            width: parent.width

            model: [
                // Strings: 0 1 2 3 4 5 6   Positions: 0    1   2       3
                //          D C B E F G A              Flat Nat Sharp   Unset
                { buttonId: "DFlatButton",  stringId: 0, pos: 0, col: 1, btnGroup: dGroup },
                { buttonId: "DNatButton",   stringId: 0, pos: 1, col: 1, btnGroup: dGroup },
                { buttonId: "DSharpButton", stringId: 0, pos: 2, col: 1, btnGroup: dGroup },

                { buttonId: "CFlatButton",  stringId: 1, pos: 0, col: 2, btnGroup: cGroup },
                { buttonId: "CNatButton",   stringId: 1, pos: 1, col: 2, btnGroup: cGroup },
                { buttonId: "CSharpButton", stringId: 1, pos: 2, col: 2, btnGroup: cGroup },

                { buttonId: "BFlatButton",  stringId: 2, pos: 0, col: 3, btnGroup: bGroup },
                { buttonId: "BNatButton",   stringId: 2, pos: 1, col: 3, btnGroup: bGroup },
                { buttonId: "BSharpButton", stringId: 2, pos: 2, col: 3, btnGroup: bGroup },

                { buttonId: "EFlatButton",  stringId: 3, pos: 0, col: 5, btnGroup: eGroup },
                { buttonId: "ENatButton",   stringId: 3, pos: 1, col: 5, btnGroup: eGroup },
                { buttonId: "ESharpButton", stringId: 3, pos: 2, col: 5, btnGroup: eGroup },

                { buttonId: "FFlatButton",  stringId: 4, pos: 0, col: 6, btnGroup: fGroup },
                { buttonId: "FNatButton",   stringId: 4, pos: 1, col: 6, btnGroup: fGroup },
                { buttonId: "FSharpButton", stringId: 4, pos: 2, col: 6, btnGroup: fGroup },

                { buttonId: "GFlatButton",  stringId: 5, pos: 0, col: 7, btnGroup: gGroup },
                { buttonId: "GNatButton",   stringId: 5, pos: 1, col: 7, btnGroup: gGroup },
                { buttonId: "GSharpButton", stringId: 5, pos: 2, col: 7, btnGroup: gGroup },

                { buttonId: "AFlatButton",  stringId: 6, pos: 0, col: 8, btnGroup: aGroup },
                { buttonId: "ANatButton",   stringId: 6, pos: 1, col: 8, btnGroup: aGroup },
                { buttonId: "ASharpButton", stringId: 6, pos: 2, col: 8, btnGroup: aGroup },
            ]

            RoundedRadioButton {
                Layout.row: modelData.pos + 1
                Layout.column: modelData.col
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.rightMargin: (modelData.col === 8) ? 30 : 0

                checked: checkPedalState(modelData.stringId, modelData.pos)
                ButtonGroup.group: modelData.btnGroup

                navigation.name: getNoteName(modelData.stringId, modelData.pos)
                navigation.panel: pedalSettingsNavPanel
                navigation.order: modelData.stringId * 3 + modelData.pos
                navigation.accessible.name: getNoteName(modelData.stringId, modelData.pos)

                onToggled: updatePedalState(modelData.stringId, modelData.pos)
            }
        }

        SeparatorLine {
            Layout.row: 0
            Layout.column: 4
            Layout.rowSpan: 4
            Layout.leftMargin: 9
            Layout.rightMargin: 10
            Layout.topMargin: 15

            orientation: Qt.Vertical
            Layout.alignment: Qt.AlignCenter
        }

        // Diagram or text

        NavigationPanel {
            id: isDiagramNavPanel
            name: "HarpPedalIsDiagramButtons"
            section: root.notationViewNavigationSection
            direction: NavigationPanel.Horizontal
            order: pedalSettingsNavPanel.order + 1
            accessible.name: qsTrc("notation", "Diagram type buttons")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
        }

        RoundedRadioButton {
            id: diagramButton
            Layout.row: 5
            Layout.column: 0
            Layout.columnSpan: 4

            Layout.topMargin: 10
            Layout.bottomMargin: 15
            Layout.leftMargin: 30
            Layout.fillWidth: true

            checked: harpModel.isDiagram
            text: qsTrc("notation", "Diagram")

            navigation.name: "diagramButton"
            navigation.panel: isDiagramNavPanel
            navigation.order: 1
            navigation.accessible.name: qsTrc("notation", "Diagram")

            onToggled: {
                harpModel.setIsDiagram(true)
            }
        }

        RoundedRadioButton {
            id: textButton
            Layout.row: 5
            Layout.column: 4
            Layout.columnSpan: 4

            Layout.topMargin: 10
            Layout.bottomMargin: 15
            Layout.fillWidth: true

            checked: !harpModel.isDiagram
            text: qsTrc("notation", "Text")

            navigation.name: "textButton"
            navigation.panel: isDiagramNavPanel
            navigation.order: 2
            navigation.accessible.name: qsTrc("notation", "Text")

            onToggled: {
                harpModel.setIsDiagram(false)
            }
        }
    }
}

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    HarpPedalPopupModel {
        id: harpModel

        onIsDiagramChanged: updatePosition(harpModel.pos, harpModel.size)
    }

    property QtObject model: harpModel

    property variant pedalState: harpModel.pedalState

    property int navigationOrderStart: 0
    property int navigationOrderEnd: isDiagramNavPanel.order

    contentWidth: menuItems.width

    contentHeight: menuItems.height

    margins: 0

    openPolicy: PopupView.NoActivateFocus
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    showArrow: false

    function updatePosition(pos, size) {

        // Default: open above position of diagram
        setOpensUpward(true)
        root.x = (pos.x + size.x / 2) - contentWidth / 2
        root.y = pos.y - size.y - contentHeight

        // For diagrams below stave, position above stave to not obscure it
        if (harpModel.belowStave) {
            root.y = harpModel.staffPos.y - size.y - contentHeight
        }

        // not enough room on window to open above so open below stave
        var globPos = mapToItem(notationView, Qt.point(root.x, root.y))
        if (globPos.y < 0) {
            setOpensUpward(false)
            root.y = harpModel.staffPos.y + harpModel.staffPos.height + 10
        }

        // not enough room below stave to open so open above
        if (root.y > notationView.height) {
            root.y = pos.y - size.y
        }
    }

    function checkPedalState(string, state) {
        return harpModel.pedalState[string] == state
    }

    function updatePedalState(string, state) {
        root.pedalState[string] = state
        harpModel.setDiagramPedalState(root.pedalState)
    }

    function getNoteName(string, state) {

        var noteNames = [
                [qsTrc("notation", "D Flat"), qsTrc("notation", "D Natural"), qsTrc("notation", "D Sharp")],
                [qsTrc("notation", "C Flat"), qsTrc("notation", "C Natural"), qsTrc("notation", "C Sharp")],
                [qsTrc("notation", "B Flat"), qsTrc("notation", "B Natural"), qsTrc("notation", "B Sharp")],
                [qsTrc("notation", "E Flat"), qsTrc("notation", "E Natural"), qsTrc("notation", "E Sharp")],
                [qsTrc("notation", "F Flat"), qsTrc("notation", "F Natural"), qsTrc("notation", "F Sharp")],
                [qsTrc("notation", "G Flat"), qsTrc("notation", "G Natural"), qsTrc("notation", "G Sharp")],
                [qsTrc("notation", "A Flat"), qsTrc("notation", "A Natural"), qsTrc("notation", "A Sharp")]
        ]

        return noteNames[string][state]
    }

    GridLayout {
        id: menuItems
        rows: 5
        flow: GridLayout.TopToBottom
        columnSpacing: 10
        rowSpacing: 10

        NavigationPanel {
            id: pedalSettingsNavPanel
            name: "PedalSettings"
            direction: NavigationPanel.Vertical
            section: root.navigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Pedal Settings buttons")
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
            section: root.navigationSection
            direction: NavigationPanel.Horizontal
            order: pedalSettingsNavPanel.order + 1
            accessible.name: qsTrc("notation", "Diagram type buttons")
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

            checked: model.isDiagram
            text: qsTrc("notation", "Diagram")

            navigation.name: "diagramButton"
            navigation.panel: isDiagramNavPanel
            navigation.order: 1
            navigation.accessible.name: qsTrc("notation", "Diagram")

            onToggled: {
                model.setIsDiagram(true)
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

            checked: !model.isDiagram
            text: qsTrc("notation", "Text")

            navigation.name: "textButton"
            navigation.panel: isDiagramNavPanel
            navigation.order: 2
            navigation.accessible.name: qsTrc("notation", "Text")

            onToggled: {
                model.setIsDiagram(false)
            }
        }
    }
}

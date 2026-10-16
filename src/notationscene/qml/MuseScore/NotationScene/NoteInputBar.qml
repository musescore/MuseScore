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

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

Item {
    id: root

    property alias orientation: gridView.orientation

    property bool floating: false

    property int maximumWidth: 0
    property int maximumHeight: 0

    width: gridView.isHorizontal ? childrenRect.width : 76
    height: !gridView.isHorizontal ? childrenRect.height : 40

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "NoteInputBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("notation", "Note input toolbar")
    }

    NoteInputBarModel {
        id: noteInputModel
    }

    QtObject {
        id: prv

        function resolveHorizontalGridViewWidth() {
            if (root.floating) {
                return gridView.contentWidth
            }

            var requiredFreeSpace = gridView.cellWidth * 3 + gridView.rowSpacing * 4

            if (root.maximumWidth - gridView.contentWidth < requiredFreeSpace) {
                return gridView.contentWidth - requiredFreeSpace
            }

            return gridView.contentWidth
        }

        function resolveVerticalGridViewHeight() {
            if (root.floating) {
                return gridView.contentHeight
            }

            var requiredFreeSpace = gridView.cellHeight * 3 + gridView.rowSpacing * 4

            if (root.maximumHeight - gridView.contentHeight < requiredFreeSpace) {
                return gridView.contentHeight - requiredFreeSpace
            }

            return gridView.contentHeight
        }
    }

    GridViewSectional {
        id: gridView

        sectionRole: "section"

        rowSpacing: 4
        columnSpacing: 4

        cellWidth: 32
        cellHeight: cellWidth

        sectionWidth: isHorizontal ? 1 : width
        sectionHeight: isHorizontal ? height : 1

        clip: true

        model: noteInputModel

        sectionDelegate: SeparatorLine {
            required property int itemIndex

            orientation: gridView.isHorizontal ? Qt.Vertical : Qt.Horizontal
            visible: itemIndex !== 0
        }

        itemDelegate: FlatButton {
            id: btn

            required property var itemModel

            readonly property MenuItem item: Boolean(itemModel) ? itemModel.item : null
            readonly property bool hasMenu: Boolean(item) && item.subitems.length !== 0

            width: gridView.cellWidth
            height: gridView.cellWidth

            enabled: noteInputModel.isInputAllowed

            accentButton: (Boolean(item) && item.checked) || menuLoader.isMenuOpened
            transparent: !accentButton

            icon: Boolean(item) ? item.icon : IconCode.NONE
            iconFont: ui.theme.toolbarIconsFont

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            navigation.panel: root.navigationPanel
            navigation.name: Boolean(item) ? item.id : ""
            navigation.order: Boolean(itemModel) ? itemModel.order : 0
            isClickOnKeyNavTriggered: false
            navigation.onTriggered: {
                if (btn.hasMenu) {
                    toggleMenuOpened()
                } else {
                    handleMenuItem()
                }
            }

            function toggleMenuOpened() {
                menuLoader.toggleOpened(item.subitems)
            }

            function handleMenuItem() {
                Qt.callLater(noteInputModel.handleMenuItem, item.id)
            }

            onClicked: {
                if (btn.hasMenu) {
                    toggleMenuOpened()
                } else {
                    handleMenuItem()
                }
            }

            mouseArea.onPressAndHold: function(event) {
                if (menuLoader.isMenuOpened || !btn.hasMenu) {
                    event.accepted = false // do not suppress the click event
                    return
                }

                btn.toggleMenuOpened()
            }

            StyledMenuLoader {
                id: menuLoader

                onHandleMenuItem: function(itemId) {
                    noteInputModel.handleMenuItem(itemId)
                }
            }
        }
    }

    Grid {
        id: automationControls

        columns: gridView.isHorizontal ? children.length : 1
        spacing: 4
        verticalItemAlignment: Grid.AlignVCenter

        visible: noteInputModel.isAutomationToolbarActive

        // Don't contribute to the toolbar's size while hidden
        width: visible ? implicitWidth : 0
        height: visible ? implicitHeight : 0

        component ToolbarSeparator: Rectangle {
            width: 1
            height: root.height
            color: ui.theme.strokeColor

            visible: gridView.isHorizontal
        }

        ToolbarSeparator {}

        // Automation type
        Row {
            leftPadding: 4
            spacing: 8

            StyledTextLabel {
                id: automateLabel
                anchors.verticalCenter: parent.verticalCenter
                text: "Automate"
            }

            StyledDropdown {
                id: automationTypeDropdown

                anchors.verticalCenter: parent.verticalCenter

                width: 120

                model: noteInputModel.automationTypes
                currentIndex: indexOfValue(noteInputModel.currentAutomationType)

                navigation.panel: root.navigationPanel
                navigation.name: "AutomationType"
                navigation.order: 101
                navigation.accessible.name: automateLabel.text + " " + currentText

                onActivated: function(index, value) {
                    noteInputModel.currentAutomationType = value
                }
            }
        }

        ToolbarSeparator {}

        // Voice selection (Dynamics only; UI only for now)
        Row {
            leftPadding: 4
            spacing: 8

            visible: noteInputModel.isVoiceSelectionAvailable

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter
                text: "Voice"
            }

            Row {
                spacing: 4

                FlatButton {
                    width: 38
                    height: gridView.cellHeight

                    text: "All"
                    toolTipTitle: "All voices"
                    accentButton: noteInputModel.currentAutomationVoice === -1

                    navigation.panel: root.navigationPanel
                    navigation.name: "AutomationAllVoices"
                    navigation.order: 102

                    onClicked: {
                        noteInputModel.currentAutomationVoice = -1
                    }
                }

                Repeater {
                    model: noteInputModel.automationVoicesCount

                    FlatButton {
                        required property int index

                        width: gridView.cellWidth
                        height: gridView.cellHeight

                        icon: [IconCode.VOICE_1, IconCode.VOICE_2, IconCode.VOICE_3, IconCode.VOICE_4][index]
                        iconFont: ui.theme.toolbarIconsFont
                        toolTipTitle: "Voice " + (index + 1)
                        accentButton: noteInputModel.currentAutomationVoice === index

                        navigation.panel: root.navigationPanel
                        navigation.name: "AutomationVoice" + (index + 1)
                        navigation.order: 103 + index

                        onClicked: {
                            noteInputModel.currentAutomationVoice = index
                        }
                    }
                }
            }
        }

        ToolbarSeparator {
            visible: gridView.isHorizontal && noteInputModel.isVoiceSelectionAvailable
        }

        // Tempo only (UI only for now)
        FlatButton {
            height: 30

            visible: noteInputModel.isTempoRangeAvailable

            text: "Set tempo range…"

            navigation.panel: root.navigationPanel
            navigation.name: "SetTempoRange"
            navigation.order: 110

            onClicked: {
                noteInputModel.openTempoRangeDialog()
            }
        }

        FlatButton {
            id: resetButton

            width: gridView.cellWidth
            height: gridView.cellHeight

            icon: IconCode.UNDO
            iconFont: ui.theme.toolbarIconsFont
            toolTipTitle: "Reset"
            accentButton: resetMenuLoader.isMenuOpened

            navigation.panel: root.navigationPanel
            navigation.name: "AutomationReset"
            navigation.order: 111

            onClicked: {
                if (resetMenuLoader.isMenuOpened) {
                    resetMenuLoader.close()
                } else {
                    resetMenuLoader.open(noteInputModel.automationResetMenuItems())
                }
            }

            StyledMenuLoader {
                id: resetMenuLoader

                onHandleMenuItem: function(itemId) {
                    noteInputModel.handleAutomationMenuItem(itemId)
                }
            }

            // Menu indicator
            Canvas {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 2

                width: 4
                height: 4

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = ui.theme.fontPrimaryColor
                    ctx.beginPath()
                    ctx.moveTo(width, 0)
                    ctx.lineTo(width, height)
                    ctx.lineTo(0, height)
                    ctx.closePath()
                    ctx.fill()
                }
            }
        }
    }

    PopupButton {
        id: customizeButton

        anchors.margins: 4

        width: gridView.cellWidth
        height: gridView.cellHeight

        icon: IconCode.SETTINGS_COG
        iconFont: ui.theme.toolbarIconsFont
        toolTipTitle: qsTrc("notation", "Customize toolbar")
        toolTipDescription: qsTrc("notation", "Show/hide toolbar buttons")
        transparent: true

        enabled: noteInputModel.isInputAllowed
        visible: !noteInputModel.isAutomationToolbarActive

        navigation.panel: root.navigationPanel
        navigation.order: 100
        navigation.accessible.name: qsTrc("notation", "Customize toolbar")

        popupAnchorItem: root.floating ? null : ui.rootItem
        popupComponent: NoteInputBarCustomisePopup {}
    }

    states: [
        State {
            when: gridView.isHorizontal

            PropertyChanges {
                target: gridView
                width: prv.resolveHorizontalGridViewWidth()
                height: root.height
                sectionWidth: 1
                sectionHeight: root.height
                rows: 1
                columns: gridView.noLimit
            }

            AnchorChanges {
                target: customizeButton
                anchors.left: gridView.right
                anchors.verticalCenter: root.verticalCenter
            }

            AnchorChanges {
                target: automationControls
                anchors.left: gridView.right
                anchors.verticalCenter: root.verticalCenter
            }

            PropertyChanges {
                target: automationControls
                anchors.leftMargin: 4
            }
        },
        State {
            when: !gridView.isHorizontal

            PropertyChanges {
                target: gridView
                width: root.width
                height: prv.resolveVerticalGridViewHeight()
                sectionWidth: root.width
                sectionHeight: 1
                rows: gridView.noLimit
                columns: 2
            }

            AnchorChanges {
                target: customizeButton
                anchors.top: gridView.bottom
                anchors.right: parent.right
            }

            AnchorChanges {
                target: automationControls
                anchors.top: gridView.bottom
                anchors.left: root.left
            }

            PropertyChanges {
                target: automationControls
                anchors.topMargin: 4
            }
        }
    ]
}

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
    Shortcut {sequence:"Ctrl+Alt+D";context:Qt.WindowShortcut;enabled:drawingButton.enabled;onActivated:DrawingSession.setDrawingMode(!DrawingSession.modeActive)}

    property bool floating: false

    property int maximumWidth: 0
    property int maximumHeight: 0

    implicitWidth: gridView.isHorizontal ? gridView.contentWidth + 104 : 76
    implicitHeight: gridView.isHorizontal ? 40 : gridView.contentHeight + 104
    width: gridView.isHorizontal ? (floating ? implicitWidth : maximumWidth) : 76
    height: !gridView.isHorizontal ? childrenRect.height : 40
    clip: true

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

            var available = root.maximumWidth - customizeButton.width - drawingButton.width - 20
            return Math.max(0, Math.min(gridView.contentWidth, available))
        }

        function resolveVerticalGridViewHeight() {
            if (root.floating) {
                return gridView.contentHeight
            }

            var available = root.maximumHeight - customizeButton.height - drawingButton.height - 20
            return Math.max(0, Math.min(gridView.contentHeight, available))
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
            // MenuItemList is a QML sequence; its generated type omits length.
            // qmllint disable missing-property
            readonly property bool hasMenu: Boolean(item) && item.subitems.length !== 0
            // qmllint enable missing-property

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
                DrawingSession.setDrawingMode(false)
                menuLoader.toggleOpened(item.subitems)
            }

            function handleMenuItem() {
                DrawingSession.setDrawingMode(false)
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
                    DrawingSession.setDrawingMode(false)
                    noteInputModel.handleMenuItem(itemId)
                }
            }
        }
    }

    FlatButton {
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

        navigation.panel: root.navigationPanel
        navigation.order: 100
        navigation.accessible.name: qsTrc("notation", "Customize toolbar")

        onClicked: {
            customizePopup.toggleOpened()
        }

        NoteInputBarCustomisePopup {
            id: customizePopup

            anchorItem: !root.floating ? ui.rootItem : null
        }
    }

    FlatButton {
        id: drawingButton
        width: 56; height: gridView.cellHeight
        anchors.margins: 8
        enabled: DrawingSession.editor !== null && noteInputModel.isInputAllowed
        accentButton: DrawingSession.modeActive
        transparent: !accentButton
        iconFont: ui.theme.toolbarIconsFont
        toolTipTitle: qsTrc("drawing-tools", "Draw on score")
        toolTipDescription: qsTrc("drawing-tools", "Draw and edit vector shapes on the score. Ctrl+Alt+D toggles drawing mode; Esc exits.")
        navigation.panel: root.navigationPanel
        navigation.order: 101
        navigation.name: "vector-drawing-mode"
        toolTipShortcut:"Ctrl+Alt+D"
        onClicked: DrawingSession.setDrawingMode(!DrawingSession.modeActive)
        contentItem: Component {
            Item {
                width: 56; height: 32
                Canvas {
                    anchors.fill: parent
                    property color ink: drawingButton.iconColor
                    onInkChanged: requestPaint()
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset(); ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = ink; ctx.fillStyle = ink; ctx.lineWidth = 1.4
                        ctx.lineCap = "round"
                        ctx.beginPath(); ctx.arc(4, 24, 2.4, 0, Math.PI * 2); ctx.fill()
                        ctx.beginPath(); ctx.moveTo(4, 24)
                        ctx.bezierCurveTo(12, 13, 18, 32, 25, 24)
                        ctx.quadraticCurveTo(29, 20, 33, 24); ctx.stroke()
                    }
                }
                StyledIconLabel {
                    x: 28; width: 28; height: 32
                    iconCode: IconCode.EDIT
                    font: drawingButton.iconFont
                    color: drawingButton.iconColor
                }
            }
        }
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
                target: drawingButton
                anchors.right: root.right
                anchors.verticalCenter: root.verticalCenter
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
                target: drawingButton
                anchors.top: customizeButton.bottom
                anchors.horizontalCenter: root.horizontalCenter
            }
        }
    ]
}

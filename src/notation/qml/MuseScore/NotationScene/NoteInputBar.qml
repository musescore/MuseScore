/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

import "internal"

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

    Component.onCompleted: {
        noteInputModel.load()
    }

    GridViewSectional {
        id: gridView

        sectionRole: "section"

        rowSpacing: 4
        columnSpacing: 4

        cellWidth: 32
        cellHeight: cellWidth

        clip: true

        model: noteInputModel

        sectionDelegate: SeparatorLine {
            orientation: gridView.orientation
            visible: itemIndex !== 0
        }

        itemDelegate: FlatButton {
            id: btn

            readonly property QtObject item: Boolean(itemModel) ? itemModel.itemRole : null
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

            Connections {
                target: btn.mouseArea

                // Make sure we only connect to `pressAndHold` if necessary
                // See https://github.com/musescore/MuseScore/issues/16012
                enabled: btn.hasMenu && !menuLoader.isMenuOpened

                function onPressAndHold() {
                    if (menuLoader.isMenuOpened || !btn.hasMenu) {
                        return
                    }

                    btn.toggleMenuOpened()
                }
            }

            StyledMenuLoader {
                id: menuLoader

                onHandleMenuItem: function(itemId) {
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
        }
    ]
}

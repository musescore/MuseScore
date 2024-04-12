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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    height: 26
    visible: notationsView.count > 0
    color: ui.theme.backgroundSecondaryColor

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "NotationViewTabs"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal
    }

    function ensureActive() {
        var item = notationsView.itemAtIndex(notationsView.currentIndex)
        item.navigation.requestActive()
    }

    NotationSwitchListModel {
        id: notationSwitchModel

        onCurrentNotationIndexChanged: function(index) {
            notationsView.currentIndex = index
        }
    }

    Component.onCompleted: {
        notationSwitchModel.load()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        RadioButtonGroup {
            id: notationsView

            // - 1: don't need to see the right separator of the rightmost tab
            readonly property bool needsScrollArrowButtons: contentWidth - 1 > root.width

            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.horizontal: null

            model: notationSwitchModel
            currentIndex: 0
            spacing: 0

            clip: true
            interactive: true

            function scrollLeft() {
                let leftmostTab = indexAt(contentX, 0)
                let newLeftmostTab = Math.max(0, leftmostTab - 1)
                positionViewAtIndex(newLeftmostTab, ListView.Contain)
            }

            function scrollRight() {
                let rightmostTab = indexAt(contentX + width - 1, 0)
                let newRightmostTab = Math.min(count - 1, rightmostTab + 1)
                positionViewAtIndex(newRightmostTab, ListView.Contain)
            }

            delegate: NotationSwitchButton {
                id: button

                navigation.name: "NotationTab" + index
                navigation.panel: root.navigationPanel
                navigation.row: index * 10  + 1 // * 10 - for close button
                navigation.accessible.name: text + (needSave ? (" " + qsTrc("notation", "Not saved")) : "")

                text: model.title
                needSave: model.needSave
                isCloud: model.isCloud

                ButtonGroup.group: notationsView.radioButtonGroup
                checked: index === notationsView.currentIndex

                onToggled: {
                    notationSwitchModel.setCurrentNotation(index)
                }

                onCloseRequested: {
                    notationSwitchModel.closeNotation(index)
                }

                onContextMenuItemsRequested: {
                    contextMenuItems = notationSwitchModel.contextMenuItems(index)
                }

                onHandleContextMenuItem: function(itemId) {
                    notationSwitchModel.handleContextMenuItem(index, itemId)
                }
            }
        }

        Row {
            Layout.fillHeight: true
            Layout.leftMargin: -1
            spacing: 0

            visible: notationsView.needsScrollArrowButtons

            SeparatorLine {}

            NotationSwitchScrollArrowButton {
                enabled: !notationsView.atXBeginning

                navigation.name: "ScrollLeftButton"
                navigation.panel: root.navigationPanel
                navigation.row: notationsView.count * 10 + 1
                navigation.accessible.name: qsTrc("notation", "Scroll left")

                icon: IconCode.CHEVRON_LEFT

                onScrollRequested: {
                    notationsView.scrollLeft()
                }
            }

            SeparatorLine {}

            NotationSwitchScrollArrowButton {
                enabled: !notationsView.atXEnd

                navigation.name: "ScrollRightButton"
                navigation.panel: root.navigationPanel
                navigation.row: notationsView.count * 10 + 2
                navigation.accessible.name: qsTrc("notation", "Scroll right")

                icon: IconCode.CHEVRON_RIGHT

                onScrollRequested: {
                    notationsView.scrollRight()
                }
            }
        }
    }
}

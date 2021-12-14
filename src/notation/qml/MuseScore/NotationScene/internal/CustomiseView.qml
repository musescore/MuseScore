/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

ListView {
    id: root

    spacing: 0

    boundsBehavior: Flickable.StopAtBounds
    clip: true

    signal selectRowRequested(int index)
    signal clearSelectionRequested()

    function positionViewAtSelectedItems() {
        var selectedIndexes = root.model.selectionModel.selectedIndexes
        for (var _index in selectedIndexes) {
            positionViewAtIndex(selectedIndexes[_index].row, ListView.Contain)
        }
    }

    function focusOnFirst() {
        var selectedIndexes = root.model.selectionModel.selectedIndexes
        if (selectedIndexes.lenght > 0) {
            root.selectRowRequested(selectedIndexes[0])
        } else {
            root.selectRowRequested(0)
        }

        root.positionViewAtSelectedItems()
    }

    function clearFocus() {
        root.clearSelectionRequested()
    }

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "CustomiseView"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlName", prv.currentItemNavigationName)
            }
        }
    }

    QtObject {
        id: prv

        property var currentItemNavigationName: []
    }

    ScrollBar.vertical: StyledScrollBar {}

    delegate: ListItemBlank {
        id: itemDelegate

        property var item: model.item

        height: 38

        isSelected: model.isSelected

        onClicked: {
            root.selectRowRequested(index)
        }

        navigation.name: item.title
        navigation.panel: root.navigationPanel
        navigation.row: model.index
        navigation.column: 0
        navigation.accessible.name: item.title
        navigation.onActiveChanged: {
            if (navigation.active) {
                prv.currentItemNavigationName = navigation.name
                root.positionViewAtIndex(index, ListView.Contain)
            }
        }

        onIsSelectedChanged: {
            if (isSelected && !navigation.active) {
                navigation.requestActive()
            }
        }

        Loader {
            property var delegateType: Boolean(itemDelegate.item) ? itemDelegate.item.type : NoteInputBarCustomiseItem.UNDEFINED

            anchors.fill: parent
            sourceComponent: delegateType === NoteInputBarCustomiseItem.ACTION ? actionComponent : separatorLineComponent

            Component {
                id: actionComponent

                NoteInputBarActionDelegate {
                    item: itemDelegate.item

                    navigationPanel: root.navigationPanel
                    navigationRow: index
                }
            }

            Component {
                id: separatorLineComponent

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: Boolean(itemDelegate.item) ? itemDelegate.item.title : ""
                }
            }
        }
    }
}

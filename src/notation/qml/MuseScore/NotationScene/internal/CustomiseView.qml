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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledListView {
    id: root

    spacing: 0
    scrollBarPolicy: ScrollBar.AlwaysOn

    signal selectRowRequested(int index)
    signal clearSelectionRequested()
    signal removeSelectionRequested()

    function positionViewAtSelectedItems() {
        var selectedIndexes = root.model.selectionModel.selectedIndexes
        for (var _index in selectedIndexes) {
            positionViewAtIndex(selectedIndexes[_index].row, ListView.Contain)
        }
    }

    function focusOnFirst() {
        var firstItem = root.itemAtIndex(0)
        if (Boolean(firstItem)) {
            firstItem.navigation.requestActive()
        }
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

    delegate: ListItemBlank {
        id: itemDelegate

        property var item: model.item

        height: 38

        isSelected: model.isSelected

        onClicked: {
            root.selectRowRequested(index)
        }

        onRemoveSelectionRequested: {
            root.removeSelectionRequested()
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

        Loader {
            property int delegateType: Boolean(itemDelegate.item) ? itemDelegate.item.type : NoteInputBarCustomiseItem.UNDEFINED

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

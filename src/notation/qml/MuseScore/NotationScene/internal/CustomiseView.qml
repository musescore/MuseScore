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

    function positionViewAtSelectedItems() {
        var selectedIndexes = root.model.selectionModel.selectedIndexes
        for (var _index in selectedIndexes) {
            positionViewAtIndex(selectedIndexes[_index].row, ListView.Contain)
        }
    }

    ScrollBar.vertical: StyledScrollBar {

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        visible: root.contentHeight > root.height
        z: 1
    }

    delegate: ListItemBlank {
        id: itemDelegate

        height: 38

        isSelected: model.isSelected

        onClicked: {
            root.selectRowRequested(index)
        }

        property var item: model.item

        Loader {
            property var delegateType: Boolean(itemDelegate.item) ? itemDelegate.item.type : NoteInputBarCustomiseItem.UNDEFINED

            anchors.fill: parent
            sourceComponent: delegateType === NoteInputBarCustomiseItem.ACTION ? actionComponent : separatorLineComponent

            Component {
                id: actionComponent

                NoteInputBarActionDelegate {
                    item: itemDelegate.item
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

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.Inspector 1.0

FocusableItem {
    id: root

    property QtObject model: null

    property int sideMargin: 0

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        FretFrameChordsControlPanel {
            id: controlPanel

            anchors.left: parent.left
            anchors.leftMargin: root.sideMargin
            anchors.right: parent.right
            anchors.rightMargin: root.sideMargin

            listOrderItem: root.model ? root.model.listOrder : null
            hasInvisibleChords: root.model ? root.model.hasInvisibleChords : null

            isMovingUpAvailable: view.model ? view.model.isMovingUpAvailable : false
            isMovingDownAvailable: view.model ? view.model.isMovingDownAvailable : false

            visible: !view.isEmpty

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart

            onMoveSelectionUpRequested: {
                view.model.moveSelectionUp()
                Qt.callLater(view.positionViewAtSelectedItems)
            }

            onMoveSelectionDownRequested: {
                view.model.moveSelectionDown()
                Qt.callLater(view.positionViewAtSelectedItems)
            }

            onResetListRequested: {
                root.model.resetList()
            }
        }

        FretFrameChordsView {
            id: view

            property bool isEmpty: count === 0

            width: parent.width
            height: Math.min(contentHeight, 400)

            model: root.model ? root.model.chordListModel : null

            navigationPanel: root.navigationPanel
            navigationRowStart: controlPanel.navigationRowEnd + 1

            onChangeChordVisibilityRequested: function(index, visible) {
                view.model.setChordVisible(index, visible)
            }

            onSelectRowRequested: function(index) {
                view.model.selectRow(index)
            }

            onClearSelectionRequested: {
                view.model.clearSelection()
            }
        }

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            height: implicitHeight

            text: qsTrc("inspector", "Add chord symbols to your score and they’ll automatically appear in the legend.")
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap

            visible: view.isEmpty
        }
    }
}

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Extensions 1.0

Column {
    id: root

    property alias model: view.model
    readonly property alias count: view.count

    property alias title: titleLabel.text

    property var flickableItem: null
    property int headerHeight: titleLabel.height + spacing

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "MuseSoundsListView"
        direction: NavigationPanel.Both
        accessible.name: root.title
    }

    spacing: 16

    signal navigationActivated(var itemRect)

    function focusOnFirst() {
        view.itemAtIndex(0).requestActive()
    }

    StyledTextLabel {
        id: titleLabel
        width: parent.width
        font: ui.theme.headerFont
        horizontalAlignment: Text.AlignLeft
    }

    GridView {
        id: view

        readonly property int columns: Math.max(0, Math.floor(width / cellWidth))

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: -spacingBetweenColumns / 2
        anchors.rightMargin: -spacingBetweenColumns / 2

        height: contentHeight - spacingBetweenRows

        // We don't want this GridView to be scrollable; it would conflict with the containing Flickable.
        interactive: false
        highlightFollowsCurrentItem: false // prevent automatic scrolling

        readonly property real spacingBetweenColumns: 22
        readonly property real spacingBetweenRows: 16

        readonly property real actualCellWidth: 350
        readonly property real actualCellHeight: 150

        cellWidth: actualCellWidth + spacingBetweenColumns
        cellHeight: actualCellHeight + spacingBetweenRows

        delegate: Item {
            id: item

            height: view.cellHeight
            width: view.cellWidth

            function requestActive() {
                soundLibraryItem.navigation.requestActive()
            }

            SoundLibraryItem {
                id: soundLibraryItem

                width: view.actualCellWidth
                height: view.actualCellHeight

                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter

                title: modelData.title
                subtitle: modelData.subtitle
                thumbnailUrl: modelData.thumbnail

                navigation.panel: root.navigationPanel
                navigation.row: view.columns === 0 ? 0 : Math.floor(model.index / view.columns)
                navigation.column: model.index - (navigation.row * view.columns)
                navigation.onActiveChanged: {
                    var pos = soundLibraryItem.mapToItem(root.flickableItem, 0, 0)
                    var rect = Qt.rect(pos.x, pos.y, soundLibraryItem.width, soundLibraryItem.height)
                    root.navigationActivated(rect)
                }

                onGetSoundLibraryRequested: {
                    forceActiveFocus()
                    api.launcher.openUrl(modelData.uri)
                }
            }
        }
    }
}

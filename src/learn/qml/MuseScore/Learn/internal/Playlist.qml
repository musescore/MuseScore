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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Learn 1.0

FocusScope {
    id: root

    property alias playlist: view.model

    property alias navigation: navPanel
    property int sideMargin: 46

    signal requestOpenVideo(string videoId)

    NavigationPanel {
        id: navPanel
        name: "Playlist"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor
    }

    Rectangle {
        anchors.top: view.top

        width: parent.width
        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: background.color
            }

            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    GridView {
        id: view

        anchors.fill: parent
        anchors.leftMargin: root.sideMargin - itemMargin
        anchors.rightMargin: root.sideMargin - itemMargin
        anchors.topMargin: -itemMargin
        anchors.bottomMargin: -itemMargin

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        cellHeight: itemMargin + itemHeight + itemMargin
        cellWidth: itemMargin + itemWidth + itemMargin

        property int itemMargin: 25
        property int itemWidth: 256
        property int itemHeight: 218

        property int rows: Math.max(0, Math.floor(height / cellHeight))
        property int columns: Math.max(0, Math.floor(width / cellWidth))

        ScrollBar.vertical: StyledScrollBar {
            parent: root

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            visible: view.contentHeight > view.height
            z: 1
        }

        delegate: Item {
            height: view.cellHeight
            width: view.cellWidth

            PlaylistItem {
                anchors.centerIn: parent

                width: view.itemWidth
                height: view.itemHeight

                navigation.panel: navPanel
                navigation.row: view.columns === 0 ? 0 : Math.floor(model.index / view.columns)
                navigation.column: model.index - (navigation.row * view.columns)
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        view.positionViewAtIndex(index, ListView.Contain)
                    }
                }

                title: modelData.title
                author: modelData.author
                duration: modelData.duration
                thumbnail: modelData.thumbnailUrl

                onClicked: {
                    root.requestOpenVideo(modelData.videoId)
                }
            }
        }
    }
}

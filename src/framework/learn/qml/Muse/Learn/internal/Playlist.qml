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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Learn 1.0

FocusScope {
    id: root

    property alias playlist: view.model
    property color backgroundColor: ui.theme.backgroundPrimaryColor

    property alias navigation: navPanel
    property int sideMargin: 46

    NavigationPanel {
        id: navPanel
        name: "Playlist"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
    }

    GradientRectangle {
        id: topGradient

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: view.top

        startColor: root.backgroundColor
        endColor: "transparent"
    }

    StyledGridView {
        id: view

        readonly property int columns: Math.max(0, Math.floor(width / cellWidth))

        visible: count > 0
        anchors.fill: parent
        anchors.leftMargin: root.sideMargin - spacingBetweenColumns / 2
        anchors.rightMargin: root.sideMargin - spacingBetweenColumns / 2

        topMargin: topGradient.height

        readonly property real spacingBetweenColumns: 50
        readonly property real spacingBetweenRows: 24

        readonly property real actualCellWidth: 256
        readonly property real actualCellHeight: 232

        cellWidth: actualCellWidth + spacingBetweenColumns
        cellHeight: actualCellHeight + spacingBetweenRows

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
                width: view.actualCellWidth
                height: view.actualCellHeight

                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter

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
                    api.launcher.openUrl(modelData.url)
                }
            }
        }
    }

    Column {
        id: errorMessageColumn
        visible: !view.visible

        anchors.top: parent.top
        anchors.topMargin: topGradient.height + Math.max(parent.height / 3 - height / 2, 0)
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin

        spacing: 16

        StyledTextLabel {
            width: parent.width
            font: ui.theme.tabBoldFont
            text: qsTrc("learn", "Sorry, we are unable to load these videos right now")
        }

        StyledTextLabel {
            width: parent.width
            text: qsTrc("global", "Please check your internet connection or try again later.")
        }
    }
}

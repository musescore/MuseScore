/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

Row {
    id: root

    property bool headerVisible: true
    property real headerWidth: 98
    property real channelItemWidth: 108
    property NavigationSection navigationSection: null
    property int navigationOrder: 0

    visible: videoModel.hasVideo
    height: visible ? 44 : 0
    spacing: 1

    VideoPanelModel {
        id: videoModel
    }

    Component.onCompleted: {
        videoModel.load()
    }

    StyledTextLabel {
        width: root.headerVisible ? root.headerWidth : 0
        height: parent.height
        visible: root.headerVisible
        text: qsTrc("playback", "Video")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Rectangle {
        width: root.channelItemWidth
        height: parent.height
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 6

            FlatToggleButton {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                icon: IconCode.MUTE
                checked: videoModel.muted
                enabled: videoModel.hasVideo
                navigation.panel: navigationPanel
                navigation.row: 1
                navigation.accessible.name: qsTrc("playback", "Video mute")

                onToggled: {
                    videoModel.muted = !checked
                }
            }

            FlatToggleButton {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                icon: IconCode.SOLO
                checked: videoModel.solo
                enabled: videoModel.hasVideo
                navigation.panel: navigationPanel
                navigation.row: 2
                navigation.accessible.name: qsTrc("playback", "Video solo")

                onToggled: {
                    videoModel.solo = !videoModel.solo
                }
            }

            StyledSlider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: videoModel.volumePercent
                enabled: videoModel.hasVideo && !videoModel.muted
                navigation.panel: navigationPanel
                navigation.row: 3
                navigation.accessible.name: qsTrc("playback", "Video volume") + " " + videoModel.volumePercent + "%"

                onMoved: {
                    videoModel.volumePercent = value
                }
            }
        }

        StyledTextLabel {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 8
            text: videoModel.volumePercent + "%"
            font: ui.theme.captionFont
            enabled: videoModel.hasVideo
        }
    }

    NavigationPanel {
        id: navigationPanel
        name: "VideoMixerPanel"
        section: root.navigationSection
        order: root.navigationOrder
        direction: NavigationPanel.Horizontal
    }
}

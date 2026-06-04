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
import QtMultimedia

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 0

    readonly property int contentMargin: 8
    readonly property bool compactMode: width < 620
    readonly property int controlsHeight: controlsColumn.implicitHeight

    clip: true

    VideoPanelModel {
        id: videoModel
    }

    readonly property int syncToleranceMs: 180

    Component.onCompleted: {
        videoModel.load()
    }

    function targetVideoPositionMs() {
        return Math.max(0, Math.min(video.duration, videoModel.scorePlaybackPositionMs + videoModel.offsetMs))
    }

    function syncVideoToScore(forceSeek) {
        if (!videoModel.hasVideo || video.duration <= 0) {
            return
        }

        if (videoModel.scorePlaybackPositionMs + videoModel.offsetMs < 0) {
            if (video.position !== 0) {
                video.seek(0)
            }

            if (video.playbackState === MediaPlayer.PlayingState) {
                video.pause()
            }

            return
        }

        var targetPosition = targetVideoPositionMs()
        if (forceSeek || Math.abs(video.position - targetPosition) > syncToleranceMs) {
            video.seek(targetPosition)
        }

        if (videoModel.scorePlaying) {
            if (targetPosition < video.duration && video.playbackState !== MediaPlayer.PlayingState) {
                video.play()
            }
        } else if (video.playbackState === MediaPlayer.PlayingState) {
            video.pause()
        }
    }

    Connections {
        target: videoModel

        function onPlaybackSyncChanged() {
            root.syncVideoToScore(false)
        }

        function onVideoSettingsChanged() {
            root.syncVideoToScore(true)
        }
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: root.contentMargin

        columns: 1
        rowSpacing: 8
        columnSpacing: 12

        Rectangle {
            Layout.preferredWidth: -1
            Layout.maximumWidth: 16777215
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: Math.max(140, root.height - root.controlsHeight - (root.contentMargin * 2) - 8)
            Layout.minimumHeight: 72

            radius: 4
            color: "#111111"
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
            clip: true

            Video {
                id: video

                anchors.fill: parent
                anchors.margins: 1
                source: videoModel.videoUrl
                muted: videoModel.muted
                volume: videoModel.volumePercent / 100
                fillMode: VideoOutput.PreserveAspectFit
                visible: videoModel.hasVideo

                onSourceChanged: {
                    stop()
                }

                onDurationChanged: {
                    root.syncVideoToScore(true)
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width - 24, 200)
                spacing: 6
                visible: !videoModel.hasVideo

                StyledIconLabel {
                    Layout.alignment: Qt.AlignHCenter
                    iconCode: IconCode.PLAY
                    font.pixelSize: 24
                    opacity: 0.45
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTrc("playback", "No video attached")
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                }
            }

            FlatButton {
                anchors.centerIn: parent
                width: 44
                height: 44
                visible: videoModel.hasVideo && video.playbackState !== MediaPlayer.PlayingState
                icon: IconCode.PLAY_FILL
                buttonType: FlatButton.IconOnly
                transparent: true
                iconColor: "white"
                toolTipTitle: qsTrc("playback", "Play video")
                navigation.panel: navigationPanel
                navigation.order: root.contentNavigationPanelOrderStart + 1

                onClicked: {
                    video.play()
                }
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 28
                visible: videoModel.hasVideo && video.duration > 0 && videoModel.hitPoints.length > 0

                Rectangle {
                    anchors.fill: parent
                    color: "#99000000"
                }

                Repeater {
                    model: videoModel.hitPoints

                    Rectangle {
                        required property var modelData

                        x: Math.max(0, Math.min(parent.width - width, (modelData.timeMs / Math.max(video.duration, 1)) * parent.width - (width / 2)))
                        y: 3
                        width: 2
                        height: parent.height - 6
                        color: "#" + ("000000" + modelData.color.toString(16)).slice(-6)

                        StyledTextLabel {
                            anchors.left: parent.right
                            anchors.leftMargin: 3
                            anchors.verticalCenter: parent.verticalCenter
                            text: parent.modelData.label
                            maximumLineCount: 1
                            font.pixelSize: 10
                            color: "white"
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: controlsColumn

            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.minimumWidth: 0
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                FlatButton {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 30
                    enabled: videoModel.hasVideo
                    icon: video.playbackState === MediaPlayer.PlayingState ? IconCode.PAUSE : IconCode.PLAY
                    buttonType: FlatButton.IconOnly
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 2

                    onClicked: {
                        if (video.playbackState === MediaPlayer.PlayingState) {
                            video.pause()
                        } else {
                            video.play()
                        }
                    }
                }

                StyledSlider {
                    Layout.fillWidth: true
                    from: 0
                    to: Math.max(video.duration, 1)
                    stepSize: 100
                    value: video.position
                    enabled: videoModel.hasVideo && video.seekable
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 3

                    onMoved: {
                        video.seek(value)
                    }
                }

                StyledTextLabel {
                    Layout.preferredWidth: root.compactMode ? 64 : 92
                    horizontalAlignment: Text.AlignRight
                    text: videoModel.hasVideo ? Math.floor(video.position / 1000) + " / " + Math.floor(video.duration / 1000) + " s" : ""
                }
            }

            FilePicker {
                Layout.fillWidth: true
                path: videoModel.videoPath
                dialogTitle: qsTrc("playback", "Choose video")
                filter: qsTrc("playback", "Video files (*.mp4 *.mov *.m4v *.avi *.mkv *.webm);;All files (*)")
                buttonType: FlatButton.IconOnly
                navigation: navigationPanel
                navigationRowOrderStart: root.contentNavigationPanelOrderStart + 4

                onPathEdited: function(newPath) {
                    videoModel.videoPath = newPath
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledTextLabel {
                    text: qsTrc("playback", "Offset")
                }

                FlatButton {
                    text: root.compactMode ? qsTrc("playback", "-100") : qsTrc("playback", "-100 ms")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 6

                    onClicked: {
                        videoModel.nudgeOffset(-100)
                    }
                }

                TextInputField {
                    Layout.preferredWidth: root.compactMode ? 64 : 88
                    currentText: videoModel.offsetMs.toString()
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 7

                    onTextEditingFinished: function(newTextValue) {
                        var parsedValue = parseInt(newTextValue, 10)
                        if (!isNaN(parsedValue)) {
                            videoModel.offsetMs = parsedValue
                        }
                    }
                }

                StyledTextLabel {
                    text: qsTrc("playback", "ms")
                }

                FlatButton {
                    text: root.compactMode ? qsTrc("playback", "+100") : qsTrc("playback", "+100 ms")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 8

                    onClicked: {
                        videoModel.nudgeOffset(100)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledTextLabel {
                    text: qsTrc("playback", "Timecode")
                }

                FlatButton {
                    text: qsTrc("playback", "Off")
                    enabled: videoModel.hasVideo
                    accentButton: videoModel.timecodeDisplayMode === 0
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 9

                    onClicked: {
                        videoModel.timecodeDisplayMode = 0
                    }
                }

                FlatButton {
                    text: qsTrc("playback", "Above")
                    enabled: videoModel.hasVideo
                    accentButton: videoModel.timecodeDisplayMode === 1
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 10

                    onClicked: {
                        videoModel.timecodeDisplayMode = 1
                    }
                }

                FlatButton {
                    text: qsTrc("playback", "Below")
                    enabled: videoModel.hasVideo
                    accentButton: videoModel.timecodeDisplayMode === 2
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 11

                    onClicked: {
                        videoModel.timecodeDisplayMode = 2
                    }
                }

                StyledTextLabel {
                    text: qsTrc("playback", "fps")
                }

                TextInputField {
                    Layout.preferredWidth: 56
                    currentText: videoModel.frameRate.toString()
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 12

                    onTextEditingFinished: function(newTextValue) {
                        var parsedValue = parseFloat(newTextValue)
                        if (!isNaN(parsedValue)) {
                            videoModel.frameRate = parsedValue
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                visible: videoModel.hasVideo

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: videoModel.hasVideo ? videoModel.formatTimecode(video.position) : ""
                    }

                    FlatButton {
                        text: qsTrc("playback", "Add hit point")
                        enabled: videoModel.hasVideo
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 13

                        onClicked: {
                            videoModel.addHitPoint(video.position)
                        }
                    }
                }

                Repeater {
                    model: videoModel.hitPoints

                    RowLayout {
                        required property var modelData
                        required property int index

                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: 92
                            text: parent.modelData.timecode
                            maximumLineCount: 1
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: parent.modelData.label + "  " + parent.modelData.musicalPosition
                            maximumLineCount: 1
                        }

                        FlatButton {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 28
                            icon: IconCode.DELETE_TANK
                            buttonType: FlatButton.IconOnly
                            navigation.panel: navigationPanel
                            navigation.order: root.contentNavigationPanelOrderStart + 14 + parent.index

                            onClicked: {
                                videoModel.removeHitPoint(parent.index)
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                FlatButton {
                    text: qsTrc("playback", "Clear")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 30

                    onClicked: {
                        video.stop()
                        videoModel.clearVideo()
                    }
                }
            }
        }
    }

    NavigationPanel {
        id: navigationPanel
        name: "VideoPanel"
        section: root.navigationSection
        order: root.contentNavigationPanelOrderStart
        direction: NavigationPanel.Vertical
    }
}

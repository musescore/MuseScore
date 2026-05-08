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

        columns: root.compactMode ? 1 : 2
        rowSpacing: 8
        columnSpacing: 12

        Rectangle {
            Layout.preferredWidth: 230
            Layout.maximumWidth: 280
            Layout.fillWidth: root.compactMode
            Layout.fillHeight: !root.compactMode
            Layout.preferredHeight: root.compactMode ? 96 : -1
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
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: !root.compactMode
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
                        var parsedValue = parseInt(newTextValue)
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

                Item {
                    Layout.fillWidth: true
                }

                FlatButton {
                    text: qsTrc("playback", "Clear")
                    enabled: videoModel.hasVideo
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 9

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

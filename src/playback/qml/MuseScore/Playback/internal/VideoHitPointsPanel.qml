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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

Item {
    id: root

    required property VideoPanelModel videoModel
    // function(videoPositionMs) -- seeks both the video element and the score
    required property var seekToVideoPositionMs
    // function() -> number (detected fps of the attached video, 0 if unknown)
    required property var detectFrameRate
    required property real currentVideoPositionMs
    // QtMultimedia loads video duration/metadata asynchronously, after
    // videoModel.hasVideo already turns true -- this gives the info/
    // detectFrameRate() bindings below an explicit, notifying dependency so
    // they recompute once that metadata actually arrives, instead of staying
    // frozen at whatever they read before it loaded.
    required property real currentVideoDurationMs
    // function() -- stops and detaches the current video
    required property var clearAttachedVideo
    // function() -> {path, durationMs, resolutionText, frameRate, fileFormat,
    // videoCodec, videoBitRate, audioCodec, audioChannels, audioSampleRate,
    // audioBitRate} -- technical info about the attached video, for the
    // Information tab. Fields are "" / 0 when not available.
    required property var videoInfo

    required property NavigationPanel navigationPanel
    required property int navigationOrderStart

    readonly property int hitPointsRowStride: 3

    // NOTE: deliberately NOT `enabled: videoModel.hasVideo` at this root level --
    // that would also disable the tab bar itself (enabled propagates to all
    // children), leaving no way to switch to/view the Hit points tab once a
    // video is cleared. The controls that actually need a video already have
    // their own hasVideo-gated `enabled` (Add hit point, fps/offset fields, etc).
    implicitHeight: contentColumn.implicitHeight + 16
    // NOTE: if the floating window gets squeezed narrower than this panel and
    // the video pane's combined minimums, clip so content is cut off at this
    // panel's own edge instead of bleeding into the video pane.
    clip: true

    ColumnLayout {
        id: contentColumn

        // NOTE: anchored on all 4 sides (not just left/right/top) so the tab
        // content below actually stretches to fill whatever height the
        // SplitView gives this panel -- previously this only sized itself to
        // its own content's natural height, leaving the rest of the panel's
        // real (taller) height as dead space below the hit-point list.
        anchors.fill: parent
        anchors.margins: 8
        anchors.rightMargin: 16
        spacing: 8

        StyledTabBar {
            id: tabBar

            Layout.fillWidth: true

            StyledTabButton {
                text: qsTrc("playback", "Hit points")

                navigation.name: "VideoHitPointsTab"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationOrderStart
            }

            StyledTabButton {
                text: qsTrc("playback", "Settings")

                navigation.name: "VideoSettingsTab"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationOrderStart + 1
            }

            StyledTabButton {
                text: qsTrc("playback", "Information")

                navigation.name: "VideoInformationTab"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationOrderStart + 2
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            ColumnLayout {
                spacing: 8

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("playback", "Add hit point")
                    enabled: root.videoModel.hasVideo
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 2

                    onClicked: {
                        root.videoModel.addHitPoint(root.currentVideoPositionMs)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 4
                    visible: root.videoModel.hitPoints.length > 0

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: 88
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Timecode")
                            font: ui.theme.bodyBoldFont
                            maximumLineCount: 1
                        }

                        StyledTextLabel {
                            Layout.preferredWidth: 64
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Measure")
                            font: ui.theme.bodyBoldFont
                            maximumLineCount: 1
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 80
                            horizontalAlignment: Text.AlignLeft
                            text: qsTrc("playback", "Name")
                            font: ui.theme.bodyBoldFont
                            maximumLineCount: 1
                        }

                        Item {
                            Layout.preferredWidth: 28
                        }
                    }

                    StyledFlickable {
                        id: hitPointsFlickable

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        contentHeight: hitPointsColumn.implicitHeight
                        interactive: contentHeight > height
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        readonly property bool overflowing: contentHeight > height + 1

                        ScrollBar.vertical: StyledScrollBar {
                            policy: hitPointsFlickable.overflowing ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                            padding: 0
                        }

                        ColumnLayout {
                            id: hitPointsColumn

                            width: Math.max(0, hitPointsFlickable.width - (hitPointsFlickable.overflowing ? 12 : 0))
                            spacing: 4

                            Repeater {
                                model: root.videoModel.hitPoints

                                VideoHitPointRow {
                                    id: hitPointRow

                                    required property var modelData
                                    required property int index

                                    Layout.fillWidth: true

                                    hitPoint: modelData
                                    videoModel: root.videoModel
                                    seekToVideoPositionMs: root.seekToVideoPositionMs
                                    navigationPanel: root.navigationPanel
                                    navigationOrderStart: root.navigationOrderStart + 10 + (hitPointRow.index * root.hitPointsRowStride)
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                id: settingsPage

                spacing: 8

                readonly property int settingsLabelWidth: 50
                readonly property int settingsFieldWidth: 72

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    StyledTextLabel {
                        Layout.preferredWidth: settingsPage.settingsLabelWidth
                        text: qsTrc("playback", "fps")
                    }

                    TextInputField {
                        Layout.preferredWidth: settingsPage.settingsFieldWidth
                        currentText: root.videoModel.frameRate.toString()
                        enabled: root.videoModel.hasVideo
                        navigation.panel: root.navigationPanel
                        navigation.order: root.navigationOrderStart + 3

                        onTextEditingFinished: function(newTextValue) {
                            var parsedValue = parseFloat(newTextValue)
                            if (!isNaN(parsedValue)) {
                                root.videoModel.frameRate = parsedValue
                            }
                        }
                    }

                    FlatButton {
                        Layout.fillWidth: true
                        text: qsTrc("playback", "Detect")
                        enabled: root.videoModel.hasVideo && (root.currentVideoDurationMs, root.detectFrameRate() > 0)
                        navigation.panel: root.navigationPanel
                        navigation.order: root.navigationOrderStart + 4

                        onClicked: {
                            var rate = root.detectFrameRate()
                            if (rate > 0) {
                                root.videoModel.frameRate = rate
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    StyledTextLabel {
                        Layout.preferredWidth: settingsPage.settingsLabelWidth
                        text: qsTrc("playback", "Offset")
                    }

                    TextInputField {
                        Layout.preferredWidth: settingsPage.settingsFieldWidth
                        currentText: root.videoModel.offsetMs.toString()
                        enabled: root.videoModel.hasVideo
                        navigation.panel: root.navigationPanel
                        navigation.order: root.navigationOrderStart + 5

                        onTextEditingFinished: function(newTextValue) {
                            var parsedValue = parseInt(newTextValue, 10)
                            if (!isNaN(parsedValue)) {
                                root.videoModel.offsetMs = parsedValue
                            }
                        }
                    }

                    StyledTextLabel {
                        text: qsTrc("playback", "ms")
                    }

                    FlatButton {
                        Layout.preferredWidth: 56
                        textFont.pixelSize: 9
                        text: qsTrc("playback", "-100 ms")
                        enabled: root.videoModel.hasVideo
                        navigation.panel: root.navigationPanel
                        navigation.order: root.navigationOrderStart + 6

                        onClicked: {
                            root.videoModel.nudgeOffset(-100)
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 56
                        textFont.pixelSize: 9
                        text: qsTrc("playback", "+100 ms")
                        enabled: root.videoModel.hasVideo
                        navigation.panel: root.navigationPanel
                        navigation.order: root.navigationOrderStart + 7

                        onClicked: {
                            root.videoModel.nudgeOffset(100)
                        }
                    }
                }

                FlatButton {
                    Layout.fillWidth: true
                    text: qsTrc("playback", "Clear video")
                    enabled: root.videoModel.hasVideo
                    navigation.panel: root.navigationPanel
                    navigation.order: root.navigationOrderStart + 8

                    onClicked: {
                        root.clearAttachedVideo()
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            ColumnLayout {
                id: informationPage

                spacing: 8

                // NOTE: recomputed whenever hasVideo changes (load/clear/swap)
                // or currentVideoDurationMs changes -- the latter is read here
                // purely to give this binding an explicit dependency on
                // QtMultimedia's (async) metadata load, since videoInfo() itself
                // reads video.duration/video.metaData through a plain function
                // call that wouldn't otherwise re-trigger this binding once
                // hasVideo is already true.
                readonly property var info: root.videoModel.hasVideo ? (root.currentVideoDurationMs, root.videoInfo()) : ({})

                readonly property int labelWidth: 70

                function durationText(ms) {
                    if (!(ms > 0)) {
                        return "—"
                    }

                    var totalSeconds = Math.floor(ms / 1000)
                    var minutes = Math.floor(totalSeconds / 60)
                    var seconds = totalSeconds % 60
                    return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
                }

                function channelsText(count) {
                    switch (count) {
                    case 0: return "—"
                    case 1: return qsTrc("playback", "Mono")
                    case 2: return qsTrc("playback", "Stereo")
                    default: return qsTrc("playback", "%1 channels").arg(count)
                    }
                }

                function bitRateText(bitsPerSecond) {
                    return bitsPerSecond > 0 ? Math.round(bitsPerSecond / 1000) + " kb/s" : "—"
                }

                function codecText(codec, extra) {
                    if (!codec) {
                        return "—"
                    }

                    return extra ? codec + " (" + extra + ")" : codec
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    visible: !root.videoModel.hasVideo
                    wrapMode: Text.WordWrap
                    opacity: 0.75
                    text: qsTrc("playback", "No video attached")
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    visible: root.videoModel.hasVideo

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "File")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideMiddle
                            maximumLineCount: 1
                            displayTruncatedTextOnHover: true
                            text: informationPage.info.path || "—"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Duration")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.durationText(informationPage.info.durationMs)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Video")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.codecText(informationPage.info.videoCodec, informationPage.info.resolutionText)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Frame rate")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.info.frameRate > 0 ? informationPage.info.frameRate + " fps" : "—"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Video rate")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.bitRateText(informationPage.info.videoBitRate)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Audio")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.codecText(informationPage.info.audioCodec,
                                                             informationPage.info.audioSampleRate > 0
                                                             ? informationPage.info.audioSampleRate + " Hz" : "")
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Channels")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.channelsText(informationPage.info.audioChannels)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: informationPage.labelWidth
                            horizontalAlignment: Text.AlignRight
                            text: qsTrc("playback", "Audio rate")
                            font: ui.theme.bodyBoldFont
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: informationPage.bitRateText(informationPage.info.audioBitRate)
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }
}

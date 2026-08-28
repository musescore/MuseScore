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
import QtMultimedia

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 0

    readonly property int contentMargin: 8
    readonly property int hitPointsPanelMinWidth: 240
    readonly property int hitPointsPanelMaxWidth: 420
    property int hitPointsPanelWidth: 260
    readonly property int timelineZoomMax: 8
    property real timelineZoom: 1
    readonly property int timelineFrameRate: Math.max(1, Math.round(videoModel.frameRate))
    readonly property int timelineFrameCount: videoModel.hasVideo && video.duration > 0 ? Math.floor((video.duration / 1000) * root.timelineFrameRate) + 1 : 0

    clip: true

    VideoPanelModel {
        id: videoModel
    }

    readonly property int syncToleranceMs: 180

    Component.onCompleted: {
        videoModel.load()
        root.hitPointsPanelWidth = videoModel.hitPointsPanelWidth()
    }

    function colorFromInt(value) {
        return "#" + value.toString(16).padStart(6, "0")
    }

    function targetVideoPositionMs() {
        return Math.max(0, Math.min(video.duration, videoModel.scorePlaybackPositionMs + videoModel.offsetMs))
    }

    function clearAttachedVideo() {
        video.stop()
        video.source = ""
        Qt.callLater(videoModel.clearVideo)
    }

    // Seeks both the video element and the score to the given video-timeline position.
    function seekToVideoPositionMs(videoPositionMs) {
        video.seek(videoPositionMs)
        videoModel.seekScoreToVideoPositionMs(videoPositionMs)
    }

    function detectedFrameRate() {
        try {
            if (!video.metaData) {
                return 0
            }

            var candidates = [
                video.metaData.videoFrameRate,
                video.metaData.VideoFrameRate,
                video.metaData.frameRate,
                video.metaData.FrameRate
            ]

            if (typeof MediaMetaData !== "undefined" && video.metaData.value) {
                candidates.push(video.metaData.value(MediaMetaData.VideoFrameRate))
            }

            for (var i = 0; i < candidates.length; ++i) {
                var rate = Number(candidates[i])
                if (!isNaN(rate) && rate > 0) {
                    return Math.round(rate * 1000) / 1000
                }
            }
        } catch (error) {
            return 0
        }

        return 0
    }

    function videoAspectRatio() {
        try {
            if (!video.metaData) {
                return 16 / 9
            }

            var sizeCandidates = [
                video.metaData.resolution,
                video.metaData.Resolution,
                video.metaData.videoResolution,
                video.metaData.VideoResolution
            ]

            if (typeof MediaMetaData !== "undefined" && video.metaData.value) {
                sizeCandidates.push(video.metaData.value(MediaMetaData.Resolution))
                sizeCandidates.push(video.metaData.value(MediaMetaData.VideoResolution))
            }

            for (var i = 0; i < sizeCandidates.length; ++i) {
                var size = sizeCandidates[i]
                if (size && size.width > 0 && size.height > 0) {
                    return size.width / size.height
                }
            }

            var widthCandidates = [
                video.metaData.videoWidth,
                video.metaData.VideoWidth,
                video.metaData.width,
                video.metaData.Width
            ]
            var heightCandidates = [
                video.metaData.videoHeight,
                video.metaData.VideoHeight,
                video.metaData.height,
                video.metaData.Height
            ]

            for (var w = 0; w < widthCandidates.length; ++w) {
                for (var h = 0; h < heightCandidates.length; ++h) {
                    var videoWidth = Number(widthCandidates[w])
                    var videoHeight = Number(heightCandidates[h])
                    if (!isNaN(videoWidth) && !isNaN(videoHeight) && videoWidth > 0 && videoHeight > 0) {
                        return videoWidth / videoHeight
                    }
                }
            }
        } catch (error) {
            return 16 / 9
        }

        return 16 / 9
    }

    function timelineTickHeight(frameIndex) {
        var oneSecond = root.timelineFrameRate
        if (frameIndex % (oneSecond * 60) === 0) {
            return 14
        }

        if (frameIndex % (oneSecond * 30) === 0) {
            return 13
        }

        if (frameIndex % (oneSecond * 15) === 0) {
            return 12
        }

        if (frameIndex % (oneSecond * 10) === 0) {
            return 10
        }

        if (frameIndex % (oneSecond * 5) === 0) {
            return 8
        }

        if (frameIndex % oneSecond === 0) {
            return 6
        }

        return 2
    }

    function timelineTickWidth(frameIndex) {
        var oneSecond = root.timelineFrameRate
        if (frameIndex % (oneSecond * 30) === 0) {
            return 2
        }

        if (frameIndex % oneSecond === 0) {
            return 1.5
        }

        return 1
    }

    function snappedTimelinePositionMs(positionMs) {
        var frameDurationMs = 1000 / Math.max(root.timelineFrameRate, 1)
        return Math.max(0, Math.min(video.duration, Math.round(positionMs / frameDurationMs) * frameDurationMs))
    }

    function timelinePositionForX(x, timelineWidth) {
        if (timelineWidth <= 0 || video.duration <= 0) {
            return 0
        }

        return snappedTimelinePositionMs((Math.max(0, Math.min(timelineWidth, x)) / timelineWidth) * video.duration)
    }

    function timelineLabel(seconds) {
        if (seconds < 60) {
            return seconds + qsTrc("playback", "s")
        }

        var minutes = Math.floor(seconds / 60)
        var remainingSeconds = seconds % 60
        return minutes + ":" + (remainingSeconds < 10 ? "0" : "") + remainingSeconds
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

    SplitView {
        id: contentSplitView

        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Rectangle {
            id: resizingHandle

            implicitWidth: 4
            implicitHeight: 4

            color: ui.theme.strokeColor

            states: [
                State {
                    name: "PRESSED"
                    when: resizingHandle.SplitHandle.pressed
                    PropertyChanges {
                        target: resizingHandle
                        opacity: ui.theme.accentOpacityHit
                    }
                },
                State {
                    name: "HOVERED"
                    when: resizingHandle.SplitHandle.hovered
                    PropertyChanges {
                        target: resizingHandle
                        opacity: ui.theme.accentOpacityHover
                    }
                }
            ]
        }

        Item {
            id: leftPane

            SplitView.fillWidth: true
            SplitView.fillHeight: true
            SplitView.minimumWidth: 320

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.contentMargin
                spacing: 8

            Rectangle {
                id: previewSlot

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 96

                radius: 4
                color: "#111111"
                border.width: ui.theme.borderWidth
                border.color: ui.theme.strokeColor
                clip: true

                StyledTextLabel {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 10
                    visible: videoModel.hasVideo
                    text: {
                        var timecode = videoModel.formatTimecode(video.position)
                        var musicalPosition = videoModel.musicalPositionText(video.position)
                        return musicalPosition.length > 0 ? (timecode + "   |   " + musicalPosition) : timecode
                    }
                    font.pixelSize: 20
                    font.bold: true
                    color: "#F0F0F0"
                }

                Item {
                    id: videoFrame

                    readonly property real aspectRatio: Math.max(0.1, root.videoAspectRatio())
                    readonly property real availableWidth: Math.max(0, previewSlot.width - 2)
                    readonly property real availableHeight: Math.max(0, previewSlot.height - 2)
                    readonly property bool limitedByHeight: availableHeight > 0 && availableWidth / availableHeight > aspectRatio

                    anchors.centerIn: parent
                    width: Math.round(limitedByHeight ? availableHeight * aspectRatio : availableWidth)
                    height: Math.round(limitedByHeight ? availableHeight : availableWidth / aspectRatio)

                    Video {
                        id: video

                        anchors.fill: parent
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

                    MouseArea {
                        anchors.fill: parent
                        enabled: videoModel.hasVideo
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            videoModel.toggleScorePlay()
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
                }

                FlatButton {
                    anchors.centerIn: videoFrame
                    visible: videoModel.hasVideo && video.playbackState !== MediaPlayer.PlayingState
                    icon: IconCode.PLAY_FILL
                    buttonType: FlatButton.IconOnly
                    transparent: true
                    toolTipTitle: qsTrc("playback", "Play")
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 1

                    onClicked: {
                        videoModel.toggleScorePlay()
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                FlatButton {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    enabled: videoModel.hasVideo
                    icon: video.playbackState === MediaPlayer.PlayingState ? IconCode.PAUSE : IconCode.PLAY
                    iconFont: ui.theme.toolbarIconsFont
                    buttonType: FlatButton.IconOnly
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 2

                    onClicked: {
                        videoModel.toggleScorePlay()
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: videoModel.hitPoints.length > 0 ? 70 : 58

                    StyledFlickable {
                        id: timelineFlickable

                        anchors.fill: parent
                        contentWidth: Math.max(width, width * root.timelineZoom)
                        contentHeight: height
                        interactive: contentWidth > width
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds

                        ScrollBar.horizontal: StyledScrollBar {
                            policy: timelineFlickable.contentWidth > timelineFlickable.width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                            padding: 0
                        }

                        Item {
                            id: timelineContent

                            width: timelineFlickable.contentWidth
                            height: timelineFlickable.height

                            StyledSlider {
                                id: positionSlider

                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.topMargin: videoModel.hitPoints.length > 0 ? 17 : 8
                                from: 0
                                to: Math.max(video.duration, 1)
                                stepSize: 100
                                value: video.position
                                enabled: videoModel.hasVideo && video.seekable
                                navigation.panel: navigationPanel
                                navigation.order: root.contentNavigationPanelOrderStart + 3

                                onMoved: {
                                    root.seekToVideoPositionMs(value)
                                }
                            }

                            Canvas {
                                id: frameTickCanvas

                                anchors.left: parent.left
                                anchors.right: parent.right
                                y: 41
                                height: 28
                                visible: videoModel.hasVideo && video.duration > 0

                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)

                                    var tickCount = root.timelineFrameCount
                                    if (tickCount <= 0) {
                                        return
                                    }

                                    var primaryColor = ui.theme.fontPrimaryColor.toString()
                                    var secondaryColor = ui.theme.strokeColor.toString()
                                    for (var i = 0; i < tickCount; ++i) {
                                        var tickWidth = root.timelineTickWidth(i)
                                        var tickHeight = root.timelineTickHeight(i)
                                        var x = Math.max(0, Math.min(width - tickWidth, (i / Math.max(tickCount - 1, 1)) * width - (tickWidth / 2)))
                                        ctx.globalAlpha = i % root.timelineFrameRate === 0 ? 0.62 : 0.24
                                        ctx.fillStyle = i % root.timelineFrameRate === 0 ? primaryColor : secondaryColor
                                        ctx.fillRect(x, 0, tickWidth, tickHeight)
                                    }

                                    var durationSeconds = Math.floor(video.duration / 1000)
                                    ctx.font = "10px sans-serif"
                                    ctx.textBaseline = "bottom"
                                    ctx.fillStyle = primaryColor
                                    ctx.globalAlpha = 0.74
                                    for (var second = 0; second <= durationSeconds; second += 5) {
                                        var labelX = (second * 1000 / Math.max(video.duration, 1)) * width
                                        if (labelX < 16) {
                                            ctx.textAlign = "left"
                                        } else if (labelX > width - 16) {
                                            ctx.textAlign = "right"
                                        } else {
                                            ctx.textAlign = "center"
                                        }
                                        ctx.fillText(root.timelineLabel(second), labelX, height)
                                    }
                                    ctx.globalAlpha = 1
                                }

                                Connections {
                                    target: videoModel
                                    function onVideoSettingsChanged() {
                                        frameTickCanvas.requestPaint()
                                    }
                                }

                                Connections {
                                    target: video
                                    function onDurationChanged() {
                                        frameTickCanvas.requestPaint()
                                    }
                                }

                                onWidthChanged: requestPaint()
                                onVisibleChanged: requestPaint()
                            }

                            Repeater {
                                model: videoModel.hitPoints

                                Rectangle {
                                    id: hitPointMarker

                                    required property var modelData
                                    required property int index
                                    property bool dragging: false
                                    property bool editingLabel: false
                                    property real dragTimeMs: modelData.timeMs
                                    readonly property real displayTimeMs: dragging ? dragTimeMs : modelData.timeMs

                                    x: Math.max(0, Math.min(parent.width - width, (displayTimeMs / Math.max(video.duration, 1)) * parent.width - (width / 2)))
                                    y: 13
                                    width: 3
                                    height: 34
                                    radius: 1
                                    visible: videoModel.hasVideo && video.duration > 0
                                    color: root.colorFromInt(modelData.color)

                                    StyledTextLabel {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottom: parent.top
                                        anchors.bottomMargin: 1
                                        text: parent.modelData.label
                                        maximumLineCount: 1
                                        font.pixelSize: 10
                                        color: root.colorFromInt(parent.modelData.color)
                                        visible: !hitPointMarker.editingLabel

                                        MouseArea {
                                            anchors.fill: parent
                                            anchors.margins: -4

                                            onDoubleClicked: {
                                                hitPointMarker.editingLabel = true
                                                markerLabelEditor.forceActiveFocus()
                                                markerLabelEditor.selectAll()
                                            }
                                        }
                                    }

                                    TextInputField {
                                        id: markerLabelEditor

                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottom: parent.top
                                        anchors.bottomMargin: 1
                                        width: 84
                                        currentText: hitPointMarker.modelData.label
                                        visible: hitPointMarker.editingLabel

                                        onTextEditingFinished: function(newTextValue) {
                                            videoModel.renameHitPoint(hitPointMarker.modelData.id, newTextValue)
                                            hitPointMarker.editingLabel = false
                                        }

                                        Keys.onEscapePressed: {
                                            hitPointMarker.editingLabel = false
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        anchors.leftMargin: -8
                                        anchors.rightMargin: -8
                                        cursorShape: Qt.SizeHorCursor
                                        preventStealing: true

                                        onPressed: function(mouse) {
                                            var mapped = mapToItem(positionSlider, mouse.x, mouse.y)
                                            hitPointMarker.dragging = true
                                            hitPointMarker.dragTimeMs = root.timelinePositionForX(mapped.x, positionSlider.width)
                                        }

                                        onPositionChanged: function(mouse) {
                                            if (!pressed) {
                                                return
                                            }

                                            var mapped = mapToItem(positionSlider, mouse.x, mouse.y)
                                            hitPointMarker.dragTimeMs = root.timelinePositionForX(mapped.x, positionSlider.width)
                                        }

                                        onReleased: {
                                            videoModel.setHitPointTimeMs(hitPointMarker.modelData.id, hitPointMarker.dragTimeMs)
                                            root.seekToVideoPositionMs(hitPointMarker.dragTimeMs)
                                            hitPointMarker.dragging = false
                                        }

                                        onCanceled: {
                                            hitPointMarker.dragging = false
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 2

                    FlatButton {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        icon: IconCode.ZOOM_OUT
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        enabled: videoModel.hasVideo && root.timelineZoom > 1
                        toolTipTitle: qsTrc("playback", "Zoom out timeline")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 3

                        onClicked: {
                            root.timelineZoom = Math.max(1, root.timelineZoom - 1)
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        icon: IconCode.ZOOM_IN
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        enabled: videoModel.hasVideo && root.timelineZoom < root.timelineZoomMax
                        toolTipTitle: qsTrc("playback", "Zoom in timeline")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 3

                        onClicked: {
                            root.timelineZoom = Math.min(root.timelineZoomMax, root.timelineZoom + 1)
                        }
                    }
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
            }
        }

        VideoHitPointsPanel {
            id: hitPointsPanel

            SplitView.preferredWidth: root.hitPointsPanelWidth
            SplitView.minimumWidth: root.hitPointsPanelMinWidth
            SplitView.maximumWidth: root.hitPointsPanelMaxWidth
            SplitView.fillHeight: true

            videoModel: videoModel
            seekToVideoPositionMs: root.seekToVideoPositionMs
            detectFrameRate: root.detectedFrameRate
            currentVideoPositionMs: video.position
            clearAttachedVideo: root.clearAttachedVideo
            navigationPanel: navigationPanel
            navigationOrderStart: root.contentNavigationPanelOrderStart + 20

            onWidthChanged: {
                if (width > 0) {
                    root.hitPointsPanelWidth = width
                    videoModel.setHitPointsPanelWidth(width)
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

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
    readonly property bool compactMode: width < 620
    readonly property color hitPointColor: "#3B94E5"
    readonly property real previewHeightRatio: 0.42
    readonly property int controlsScrollbarReserve: 12
    readonly property int minimumControlsHeight: 168
    readonly property int timelineFrameRate: Math.max(1, Math.round(videoModel.frameRate))
    readonly property int timelineFrameCount: videoModel.hasVideo && video.duration > 0 ? Math.floor((video.duration / 1000) * root.timelineFrameRate) + 1 : 0

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

    function clearAttachedVideo() {
        video.stop()
        video.source = ""
        Qt.callLater(videoModel.clearVideo)
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

    function autodetectFrameRate() {
        var rate = detectedFrameRate()
        if (rate > 0) {
            videoModel.frameRate = rate
        }
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

    Item {
        id: contentItem

        anchors.fill: parent
        anchors.margins: root.contentMargin

        Rectangle {
            id: previewSlot

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: Math.round(Math.max(96, Math.min(parent.height * root.previewHeightRatio, parent.height - root.minimumControlsHeight)))

            radius: 4
            color: "#111111"
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
            clip: true

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

        StyledFlickable {
            id: controlsFlickable

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: previewSlot.bottom
            anchors.topMargin: 8
            anchors.bottom: parent.bottom

            contentHeight: controlsColumn.implicitHeight
            interactive: contentHeight > height
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            readonly property bool overflowing: contentHeight > height + 1

            ScrollBar.vertical: StyledScrollBar {
                policy: controlsFlickable.overflowing ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                padding: 0
            }
        ColumnLayout {
            id: controlsColumn

            width: Math.max(0, controlsFlickable.width - (controlsFlickable.overflowing ? root.controlsScrollbarReserve : 0))
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

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: videoModel.hitPoints.length > 0 ? 70 : 58

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
                            video.seek(value)
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
                            ctx.textAlign = "center"
                            ctx.textBaseline = "bottom"
                            ctx.fillStyle = primaryColor
                            ctx.globalAlpha = 0.74
                            for (var second = 0; second <= durationSeconds; second += 5) {
                                var labelX = (second * 1000 / Math.max(video.duration, 1)) * width
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
                            property real dragTimeMs: modelData.timeMs
                            readonly property real displayTimeMs: dragging ? dragTimeMs : modelData.timeMs

                            x: Math.max(0, Math.min(parent.width - width, (displayTimeMs / Math.max(video.duration, 1)) * parent.width - (width / 2)))
                            y: 13
                            width: 3
                            height: 34
                            radius: 1
                            visible: videoModel.hasVideo && video.duration > 0
                            color: root.hitPointColor

                            StyledTextLabel {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.top
                                anchors.bottomMargin: 1
                                text: parent.modelData.label
                                maximumLineCount: 1
                                font.pixelSize: 10
                                color: root.hitPointColor
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
                                    videoModel.setHitPointTimeMs(hitPointMarker.index, hitPointMarker.dragTimeMs)
                                    hitPointMarker.dragging = false
                                }

                                onCanceled: {
                                    hitPointMarker.dragging = false
                                }
                            }
                        }
                    }
                }

                StyledTextLabel {
                    Layout.preferredWidth: root.compactMode ? 86 : 116
                    horizontalAlignment: Text.AlignRight
                    text: videoModel.hasVideo ? videoModel.formatTimecode(video.position) : ""
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

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                visible: videoModel.hasVideo

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: videoModel.hasVideo ? videoModel.hitPoints.length + " " + qsTrc("playback", "hit points") : ""
                    }

                    StyledTextLabel {
                        text: qsTrc("playback", "fps")
                    }

                    TextInputField {
                        Layout.preferredWidth: 56
                        currentText: videoModel.frameRate.toString()
                        enabled: videoModel.hasVideo
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 9

                        onTextEditingFinished: function(newTextValue) {
                            var parsedValue = parseFloat(newTextValue)
                            if (!isNaN(parsedValue)) {
                                videoModel.frameRate = parsedValue
                            }
                        }
                    }

                    FlatButton {
                        text: qsTrc("playback", "Detect FPS")
                        enabled: videoModel.hasVideo && root.detectedFrameRate() > 0
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 10

                        onClicked: {
                            root.autodetectFrameRate()
                        }
                    }

                    FlatButton {
                        text: qsTrc("playback", "Add hit point")
                        enabled: videoModel.hasVideo
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 11

                        onClicked: {
                            videoModel.addHitPoint(video.position)
                        }
                    }
                }

                StyledFlickable {
                    id: hitPointsFlickable

                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(hitPointsColumn.implicitHeight, 148)
                    Layout.maximumHeight: 132
                    visible: videoModel.hitPoints.length > 0

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

                        width: Math.max(0, hitPointsFlickable.width - (hitPointsFlickable.overflowing ? root.controlsScrollbarReserve : 0))
                        spacing: 4

                        RowLayout {
                            width: hitPointsColumn.width
                            spacing: 8

                            StyledTextLabel {
                                Layout.preferredWidth: 92
                                text: qsTrc("playback", "Timecode")
                                font: ui.theme.bodyBoldFont
                                maximumLineCount: 1
                            }

                            StyledTextLabel {
                                Layout.preferredWidth: 68
                                text: qsTrc("playback", "Measure")
                                font: ui.theme.bodyBoldFont
                                maximumLineCount: 1
                            }

                            StyledTextLabel {
                                Layout.fillWidth: true
                                text: qsTrc("playback", "Name")
                                font: ui.theme.bodyBoldFont
                                maximumLineCount: 1
                            }

                            Item {
                                Layout.preferredWidth: 32
                            }
                        }

                        Repeater {
                            model: videoModel.hitPoints

                            RowLayout {
                                id: hitPointDelegate

                                required property var modelData
                                required property int index

                                property bool editingLabel: false
                                property bool editingTimecode: false

                                width: hitPointsColumn.width
                                spacing: 8

                                Item {
                                    Layout.preferredWidth: 92
                                    Layout.preferredHeight: 28

                                    StyledTextLabel {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        text: hitPointDelegate.modelData.timecode
                                        maximumLineCount: 1
                                        visible: !hitPointDelegate.editingTimecode

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                hitPointDelegate.editingTimecode = true
                                                timecodeEditor.forceActiveFocus()
                                            }
                                        }
                                    }

                                    TextInputField {
                                        id: timecodeEditor

                                        anchors.fill: parent
                                        currentText: hitPointDelegate.modelData.timecode
                                        visible: hitPointDelegate.editingTimecode
                                        navigation.panel: navigationPanel
                                        navigation.order: root.contentNavigationPanelOrderStart + 12 + hitPointDelegate.index

                                        onTextEditingFinished: function(newTextValue) {
                                            videoModel.setHitPointTimecode(hitPointDelegate.index, newTextValue)
                                            hitPointDelegate.editingTimecode = false
                                        }

                                        Keys.onEscapePressed: {
                                            hitPointDelegate.editingTimecode = false
                                        }
                                    }
                                }

                                StyledTextLabel {
                                    Layout.preferredWidth: 68
                                    text: hitPointDelegate.modelData.musicalPosition
                                    maximumLineCount: 1
                                }

                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 28

                                    StyledTextLabel {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        text: hitPointDelegate.modelData.label
                                        maximumLineCount: 1
                                        visible: !hitPointDelegate.editingLabel

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                hitPointDelegate.editingLabel = true
                                                labelEditor.forceActiveFocus()
                                            }
                                        }
                                    }

                                    TextInputField {
                                        id: labelEditor

                                        anchors.fill: parent
                                        currentText: hitPointDelegate.modelData.label
                                        visible: hitPointDelegate.editingLabel
                                        navigation.panel: navigationPanel
                                        navigation.order: root.contentNavigationPanelOrderStart + 11 + hitPointDelegate.index

                                        onTextEditingFinished: function(newTextValue) {
                                            videoModel.renameHitPoint(hitPointDelegate.index, newTextValue)
                                            hitPointDelegate.editingLabel = false
                                        }

                                        Keys.onEscapePressed: {
                                            hitPointDelegate.editingLabel = false
                                        }
                                    }
                                }

                                FlatButton {
                                    Layout.preferredWidth: 32
                                    Layout.preferredHeight: 28
                                    icon: IconCode.DELETE_TANK
                                    buttonType: FlatButton.IconOnly
                                    navigation.panel: navigationPanel
                                    navigation.order: root.contentNavigationPanelOrderStart + 18 + hitPointDelegate.index

                                    onClicked: {
                                        videoModel.removeHitPoint(hitPointDelegate.index)
                                    }
                                }
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
                        root.clearAttachedVideo()
                    }
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

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
    readonly property int timelineZoomMax: 10
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

                FontMetrics {
                    id: timerFontMetrics
                    font.family: "monospace"
                    font.pixelSize: 20
                    font.bold: true
                }

                RowLayout {
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 10
                    visible: videoModel.hasVideo
                    spacing: 8

                    // NOTE: each zone below has a fixed width so digits changing width
                    // (e.g. "1" vs "0") during playback can't shift anything else --
                    // same technique as MeasureAndBeatFields.qml (see its comment/the
                    // linked issue #9633) applied to plain read-only labels here.
                    Item {
                        Layout.preferredWidth: timerFontMetrics.advanceWidth("00:00:00:00")
                        Layout.preferredHeight: timecodeLabel.implicitHeight

                        StyledTextLabel {
                            id: timecodeLabel
                            anchors.right: parent.right
                            text: videoModel.formatTimecode(video.position)
                            font.family: "monospace"
                            font.pixelSize: 20
                            font.bold: true
                            color: "#F0F0F0"
                        }
                    }

                    StyledTextLabel {
                        text: "|"
                        visible: musicalPositionLabel.text.length > 0
                        font.pixelSize: 20
                        font.bold: true
                        opacity: 0.6
                        color: "#F0F0F0"
                    }

                    Item {
                        Layout.preferredWidth: timerFontMetrics.advanceWidth("9999.9")
                        Layout.preferredHeight: musicalPositionLabel.implicitHeight
                        visible: musicalPositionLabel.text.length > 0

                        StyledTextLabel {
                            id: musicalPositionLabel
                            anchors.left: parent.left
                            text: videoModel.musicalPositionText(video.position)
                            font.family: "monospace"
                            font.pixelSize: 20
                            font.bold: true
                            color: "#F0F0F0"
                        }
                    }
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

                FlatButton {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    enabled: videoModel.hasVideo
                    text: qsTrc("playback", "M", "Mark: add a hit point at the current position")
                    toolTipTitle: qsTrc("playback", "Add hit point at current position")
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 3

                    onClicked: {
                        videoModel.addHitPoint(video.position)
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76

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

                            // Zone 1: ruler -- hit-point badges. Double-click empty space to add
                            // a new hit point there; right-click a badge for Edit/Delete.
                            Rectangle {
                                id: hitPointsRuler

                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                height: 20
                                color: ui.theme.backgroundSecondaryColor
                                visible: videoModel.hasVideo && video.duration > 0

                                MouseArea {
                                    anchors.fill: parent

                                    onDoubleClicked: function(mouse) {
                                        videoModel.addHitPoint(root.timelinePositionForX(mouse.x, width))
                                    }
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
                                        readonly property real tickX: (displayTimeMs / Math.max(video.duration, 1)) * parent.width

                                        x: Math.max(0, Math.min(parent.width - width, tickX - width / 2))
                                        y: 2
                                        width: Math.max(20, badgeLabel.implicitWidth + 12)
                                        height: 16
                                        radius: height / 2
                                        color: ui.theme.accentColor
                                        border.width: hitPointMarker.dragging ? 2 : 0
                                        border.color: "white"

                                        StyledTextLabel {
                                            id: badgeLabel
                                            anchors.centerIn: parent
                                            text: parent.modelData.label
                                            maximumLineCount: 1
                                            font.pixelSize: 9
                                            font.bold: true
                                            color: "white"
                                            visible: !hitPointMarker.editingLabel
                                        }

                                        TextInputField {
                                            id: markerLabelEditor

                                            anchors.centerIn: parent
                                            width: Math.max(70, parent.width)
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
                                            id: hitPointMouseArea

                                            anchors.fill: parent
                                            anchors.margins: -4
                                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                                            cursorShape: Qt.SizeHorCursor
                                            preventStealing: true

                                            onPressed: function(mouse) {
                                                if (mouse.button === Qt.RightButton) {
                                                    hitPointContextMenu.show(Qt.point(mouse.x, mouse.y))
                                                    return
                                                }

                                                var mapped = mapToItem(positionSlider, mouse.x, mouse.y)
                                                hitPointMarker.dragging = true
                                                hitPointMarker.dragTimeMs = root.timelinePositionForX(mapped.x, positionSlider.width)
                                            }

                                            onPositionChanged: function(mouse) {
                                                if (!pressed) {
                                                    return
                                                }

                                                var mapped = mapToItem(timelineTrack, mouse.x, mouse.y)
                                                hitPointMarker.dragTimeMs = root.timelinePositionForX(mapped.x, timelineTrack.width)
                                            }

                                            onReleased: function(mouse) {
                                                if (mouse.button === Qt.RightButton) {
                                                    return
                                                }

                                                videoModel.setHitPointTimeMs(hitPointMarker.modelData.id, hitPointMarker.dragTimeMs)
                                                root.seekToVideoPositionMs(hitPointMarker.dragTimeMs)
                                                hitPointMarker.dragging = false
                                            }

                                            onCanceled: {
                                                hitPointMarker.dragging = false
                                            }

                                            onDoubleClicked: function(mouse) {
                                                if (mouse.button === Qt.RightButton) {
                                                    return
                                                }

                                                hitPointMarker.editingLabel = true
                                                markerLabelEditor.forceActiveFocus()
                                                markerLabelEditor.selectAll()
                                            }

                                            ContextMenuLoader {
                                                id: hitPointContextMenu

                                                items: [
                                                    { id: "edit", title: qsTrc("playback", "Edit"), icon: IconCode.EDIT },
                                                    { id: "delete", title: qsTrc("playback", "Delete"), icon: IconCode.DELETE_TANK }
                                                ]

                                                onHandleMenuItem: function(itemId) {
                                                    switch (itemId) {
                                                    case "edit":
                                                        hitPointMarker.editingLabel = true
                                                        markerLabelEditor.forceActiveFocus()
                                                        markerLabelEditor.selectAll()
                                                        break
                                                    case "delete":
                                                        videoModel.removeHitPoint(hitPointMarker.modelData.id)
                                                        break
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // Zone 2: timeline -- frame-tick ruler with duration labels, a thin
                            // position line per hit point (reference only; the ruler above is
                            // interactive), and the playhead itself as a full-height bar rather
                            // than a slider thumb, for more precise visual tracking during
                            // playback. Click or drag anywhere in this zone to seek.
                            Rectangle {
                                id: timelineTrack

                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: hitPointsRuler.bottom
                                anchors.topMargin: 2
                                anchors.bottom: parent.bottom
                                color: ui.theme.backgroundTertiaryColor

                            MouseArea {
                                anchors.fill: parent
                                enabled: videoModel.hasVideo && video.seekable
                                cursorShape: Qt.PointingHandCursor

                                onPressed: function(mouse) {
                                    root.seekToVideoPositionMs(root.timelinePositionForX(mouse.x, width))
                                }

                                onPositionChanged: function(mouse) {
                                    if (!pressed) {
                                        return
                                    }

                                    root.seekToVideoPositionMs(root.timelinePositionForX(mouse.x, width))
                                }
                            }

                            Repeater {
                                model: videoModel.hitPoints

                                Rectangle {
                                    required property var modelData

                                    x: Math.max(0, Math.min(parent.width - width,
                                                             (modelData.timeMs / Math.max(video.duration, 1)) * parent.width - (width / 2)))
                                    y: 2
                                    width: 1
                                    height: parent.height - y
                                    color: ui.theme.accentColor
                                    opacity: 0.8
                                    visible: videoModel.hasVideo && video.duration > 0
                                }
                            }

                            Canvas {
                                id: frameTickCanvas

                                anchors.left: parent.left
                                anchors.right: parent.right
                                y: 6
                                height: 30
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

                                    // The total duration is always shown, right-aligned at the very
                                    // end -- reserve its space so no regular 5s-grid label (which
                                    // may not land on a "nice" interval, especially when zoomed) can
                                    // be drawn overlapping it.
                                    var minLabelGap = 6
                                    var finalLabel = root.timelineLabel(durationSeconds)
                                    var finalLabelLeftEdge = width - ctx.measureText(finalLabel).width

                                    var lastLabelRightEdge = -Infinity
                                    var lastGridSecond = durationSeconds - (durationSeconds % 5)

                                    for (var second = 0; second <= lastGridSecond; second += 5) {
                                        var label = root.timelineLabel(second)
                                        var labelWidth = ctx.measureText(label).width
                                        var labelX = (second * 1000 / Math.max(video.duration, 1)) * width

                                        var align = labelX < 16 ? "left" : (labelX > width - 16 ? "right" : "center")
                                        var leftEdge = align === "left" ? labelX : (align === "right" ? labelX - labelWidth : labelX - labelWidth / 2)
                                        var rightEdge = leftEdge + labelWidth

                                        if (leftEdge < lastLabelRightEdge + minLabelGap) {
                                            continue
                                        }
                                        if (second !== lastGridSecond && rightEdge + minLabelGap > finalLabelLeftEdge) {
                                            continue
                                        }

                                        ctx.textAlign = align
                                        ctx.fillText(label, labelX, height)
                                        lastLabelRightEdge = rightEdge
                                    }

                                    ctx.textAlign = "right"
                                    ctx.fillText(finalLabel, width, height)
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

                            Rectangle {
                                id: playheadBar

                                readonly property real tickX: (video.position / Math.max(video.duration, 1)) * parent.width

                                x: Math.max(0, Math.min(parent.width - width, tickX - width / 2))
                                y: 0
                                width: 2
                                height: parent.height
                                color: ui.theme.focusColor
                                visible: videoModel.hasVideo && video.duration > 0
                            }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 4

                    FlatButton {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        icon: IconCode.ZOOM_OUT
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        enabled: videoModel.hasVideo && root.timelineZoom > 1
                        toolTipTitle: qsTrc("playback", "Zoom out timeline")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 5

                        onClicked: {
                            root.timelineZoom = Math.max(1, root.timelineZoom - 0.25)
                        }
                    }

                    TextInputField {
                        Layout.preferredWidth: 44
                        currentText: Math.round(root.timelineZoom * 100).toString()
                        enabled: videoModel.hasVideo
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 6

                        onTextEditingFinished: function(newTextValue) {
                            var parsedValue = parseInt(newTextValue, 10)
                            if (!isNaN(parsedValue) && parsedValue > 0) {
                                root.timelineZoom = Math.max(1, Math.min(root.timelineZoomMax, parsedValue / 100))
                            }
                        }
                    }

                    StyledTextLabel {
                        text: "%"
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
                        navigation.order: root.contentNavigationPanelOrderStart + 7

                        onClicked: {
                            root.timelineZoom = Math.min(root.timelineZoomMax, root.timelineZoom + 0.25)
                        }
                    }

                    Repeater {
                        model: [100, 250, 500, 750]

                        FlatButton {
                            required property int modelData
                            required property int index

                            Layout.preferredHeight: 24
                            text: modelData + "%"
                            enabled: videoModel.hasVideo
                            navigation.panel: navigationPanel
                            navigation.order: root.contentNavigationPanelOrderStart + 8 + index

                            onClicked: {
                                root.timelineZoom = modelData / 100
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: filePicker.implicitHeight + 8
                color: ui.theme.backgroundSecondaryColor
                radius: 2

                FilePicker {
                    id: filePicker

                    anchors.left: parent.left
                    anchors.right: recentFilesButton.left
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 4
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

                FlatButton {
                    id: recentFilesButton

                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 22
                    height: 22
                    icon: IconCode.SMALL_ARROW_DOWN
                    buttonType: FlatButton.IconOnly
                    transparent: true
                    toolTipTitle: qsTrc("playback", "Recently opened videos")
                    navigation.panel: navigationPanel
                    navigation.order: root.contentNavigationPanelOrderStart + 4 + 2

                    onClicked: {
                        recentFilesMenu.items = videoModel.recentVideoFiles().map(function(path) {
                            return { id: path, title: path }
                        })
                        recentFilesMenu.show(Qt.point(0, height))
                    }

                    ContextMenuLoader {
                        id: recentFilesMenu

                        onHandleMenuItem: function(itemId) {
                            videoModel.videoPath = itemId
                        }
                    }
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

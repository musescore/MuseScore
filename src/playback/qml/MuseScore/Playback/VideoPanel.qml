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
    // NOTE: the button row below the video is laid out as 3 independently
    // anchored groups (left/centerIn/right) so the middle group stays truly
    // centered on the video regardless of how wide the outer groups are --
    // but that means, unlike a plain RowLayout, nothing here prevents the
    // groups from painting on top of each other if the row gets narrower than
    // all three combined need. This floor is sized for that: left cluster
    // (Load+Recent, ~60) + centered cluster (#, M, Rewind, Play, Stop, Loop,
    // ~220) + right cluster (zoom out/value/presets-dropdown/zoom in/sidebar
    // toggle, ~136), plus content margins and slack for font-metrics rounding.
    // NOTE: whenever a button is added to/removed from any of the three
    // clusters, this needs revisiting too -- it does NOT update itself.
    readonly property int previewPaneMinWidth: 510
    // NOTE: must fit the hit-point table's own minimum row content (timecode 88
    // + measure 64 + name's own 80 floor + delete button 28, plus spacing and
    // panel margins) without clipping the delete button -- narrower than this
    // and VideoHitPointsPanel's clip:true (needed so a squeezed floating window
    // can't bleed the sidebar over the video) cuts the delete icons off instead.
    readonly property int hitPointsPanelMinWidth: 320
    readonly property int hitPointsPanelMaxWidth: 420
    property int hitPointsPanelWidth: 320
    property bool hitPointsPanelVisible: true
    readonly property int timelineZoomMax: 10
    property real timelineZoom: 1
    readonly property int timelineFrameRate: Math.max(1, Math.round(videoModel.frameRate))
    readonly property int timelineFrameCount: videoModel.hasVideo && video.duration > 0 ? Math.floor((video.duration / 1000) * root.timelineFrameRate) + 1 : 0

    // NOTE: shared drag state (rather than each hit point's own Repeater-instance-
    // local state) so the ruler's marker line AND the timeline's reference line for
    // the SAME hit point move together while it's being dragged -- otherwise the
    // timeline's line (bound to the not-yet-committed modelData.timeMs) visibly
    // lags behind the ruler's line (bound to the live drag position) until release.
    property int draggingHitPointId: -1
    property real draggingHitPointTimeMs: 0
    property bool editingZoom: false

    // NOTE: when this panel is floated into its own window, the dock framework
    // derives that window's minimum resize size from this root item's own
    // implicitWidth/Height rather than from the DockPanel's own minimumWidth
    // property (which only constrains the DOCKED case) -- so the floating
    // window's floor has to be declared here, or the SplitView below can be
    // resized narrower than its two panes' own minimums, making the sidebar
    // cover the video. Half the current sidebar width on top of the two panes'
    // combined minimum leaves enough slack that this shouldn't get hit in
    // normal use, per the user's own sizing rule of thumb. When the sidebar is
    // hidden there's no sidebar minimum to reserve space for.
    implicitWidth: root.previewPaneMinWidth
                   + (root.hitPointsPanelVisible ? root.hitPointsPanelMinWidth + Math.round(root.hitPointsPanelWidth / 2) : 0)

    // NOTE: the generic "monospace" family alias doesn't reliably resolve to an
    // actual fixed-pitch font on every platform -- when it doesn't, digits with
    // different glyph widths (e.g. "1" vs "0") make the timer's rendered text
    // width fluctuate every frame, which reads as the numbers "dancing" even
    // though they sit in a fixed-width container. Name a real fixed-pitch font
    // per platform instead.
    readonly property string monoFontFamily: Qt.platform.os === "osx" ? "Menlo"
                                              : (Qt.platform.os === "windows" ? "Consolas" : "monospace")

    clip: true

    VideoPanelModel {
        id: videoModel
    }

    readonly property int syncToleranceMs: 180
    // NOTE: the score reports its playback position far more often (every
    // audio-engine tick, tens of times a second) than the video's own
    // position actually advances between updates, so a plain "drift >
    // tolerance -> seek" check ends up re-seeking the video repeatedly
    // during perfectly normal playback -- each of those hits the hardware
    // decoder (VideoToolbox on macOS, similarly on other platforms) with
    // seeks faster than it can settle, which is what was flooding the log
    // with "Failed to seek"/decode-failure spam. Throttling how often a
    // drift-correction seek can actually fire (below) keeps sync within
    // syncToleranceMs on average without hammering the decoder; a forced
    // seek (explicit user action -- offset change, hit point click, initial
    // load) always still happens immediately, this only throttles the
    // continuous background drift correction during ordinary playback.
    readonly property int minResyncIntervalMs: 500
    property real lastResyncTime: 0

    // NOTE: `videoSettingsChanged` fires on ANY attachment mutation, including
    // ones with nothing to do with playback position (add/rename/retime a hit
    // point, mute, volume...). Its handler below forces an unconditional
    // reseek every time. That collides with e.g. the hit-point ruler drag's
    // onReleased, which already does its own explicit seekToVideoPositionMs()
    // to the new hit-point time immediately before setHitPointTimeMs() fires
    // this same notification -- without a guard, that's two unthrottled
    // video.seek() calls per release, to two different positions, and several
    // quick release attempts (e.g. while fumbling with two overlapping
    // hit-point labels) burst multiple such pairs with zero rate limit,
    // reproducing the same decoder-flooding "Failed to seek" spam the
    // drift-correction throttle above was meant to prevent. This debounce
    // skips a forced reseek that lands right after any other seek just
    // happened (any seek path stamps lastResyncTime) -- short enough to stay
    // unnoticeable for genuine standalone forced cases (offset change, initial
    // load) that aren't immediately preceded by another seek.
    readonly property int forceSeekDebounceMs: 150

    // NOTE: same flood as the drift-correction case above, different trigger --
    // dragging across the timeline track calls seekToVideoPositionMs() on every
    // pixel of mouse movement (onPositionChanged fires dozens of times/sec),
    // which hammered the hardware decoder with "Failed to seek" spam just like
    // the unthrottled background resync used to. Throttled to a shorter
    // interval than minResyncIntervalMs since this is direct user scrubbing --
    // needs to still feel responsive -- with a final unthrottled seek on release
    // so the video always lands exactly on the pixel the user let go of.
    readonly property int scrubSeekIntervalMs: 80
    property real lastScrubSeekTime: 0

    Component.onCompleted: {
        videoModel.load()
        root.hitPointsPanelWidth = Math.max(root.hitPointsPanelMinWidth, Math.min(root.hitPointsPanelMaxWidth, videoModel.hitPointsPanelWidth()))
        root.hitPointsPanelVisible = videoModel.hitPointsPanelVisible()
    }

    function toggleHitPointsPanelVisible() {
        root.hitPointsPanelVisible = !root.hitPointsPanelVisible
        videoModel.setHitPointsPanelVisible(root.hitPointsPanelVisible)
    }

    function targetVideoPositionMs() {
        return Math.max(0, Math.min(video.duration, videoModel.scorePlaybackPositionMs + videoModel.offsetMs))
    }

    function clearAttachedVideo() {
        // NOTE: deliberately does NOT also do `video.source = ""` here -- `source`
        // is declaratively bound to `videoModel.videoUrl` below; an imperative
        // assignment to a bound property permanently breaks that binding for the
        // rest of this Video element's lifetime. That silently broke reloading a
        // video after Clear: videoModel.videoUrl kept changing correctly, but
        // video.source was stuck at "" forever, so nothing ever loaded again.
        // clearVideo() clearing the attachment already drives source to empty
        // reactively through the binding, with no need for this to help.
        video.stop()
        Qt.callLater(videoModel.clearVideo)
    }

    // Seeks both the video element and the score to the given video-timeline position.
    function seekToVideoPositionMs(videoPositionMs) {
        video.seek(videoPositionMs)
        lastResyncTime = Date.now()
        videoModel.seekScoreToVideoPositionMs(videoPositionMs)
        scrollTimelineToPositionMs(videoPositionMs)
    }

    // Scrolls the zoomed timeline so the given position is visible -- a no-op
    // when it already is (e.g. seeking by clicking directly on the ruler/track,
    // which is already in view by definition). Re-centers on the position
    // rather than just nudging it into view, since a jump usually means the
    // user wants to look around that point (e.g. clicking a hit point in the
    // sidebar that's currently scrolled out of view).
    function scrollTimelineToPositionMs(videoPositionMs) {
        if (!videoModel.hasVideo || video.duration <= 0 || timelineFlickable.contentWidth <= timelineFlickable.width) {
            return
        }

        var targetX = (videoPositionMs / video.duration) * timelineFlickable.contentWidth
        var viewStart = timelineFlickable.contentX
        var viewEnd = viewStart + timelineFlickable.width

        if (targetX >= viewStart && targetX <= viewEnd) {
            return
        }

        var centeredX = targetX - (timelineFlickable.width / 2)
        timelineFlickable.contentX = Math.max(0, Math.min(timelineFlickable.contentWidth - timelineFlickable.width, centeredX))
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

    // Read-only technical info about the currently attached video, for the
    // sidebar's Information tab -- path/duration are already tracked
    // elsewhere in this file, the rest comes from the same video.metaData
    // this file already reads for frame rate/aspect ratio (see
    // detectedFrameRate()/videoAspectRatio() above), with the same
    // defensive multi-candidate-key + try/catch approach since not every
    // key is populated on every platform/codec.
    function videoInfo() {
        var info = {
            path: videoModel.videoPath,
            durationMs: video.duration,
            resolutionText: "",
            frameRate: root.detectedFrameRate(),
            fileFormat: "",
            videoCodec: "",
            videoBitRate: 0,
            audioCodec: "",
            audioChannels: 0,
            audioSampleRate: 0,
            audioBitRate: 0
        }

        if (!video.metaData) {
            return info
        }

        function stringFor(key) {
            try {
                if (typeof MediaMetaData !== "undefined" && video.metaData.stringValue) {
                    var s = video.metaData.stringValue(MediaMetaData[key])
                    if (s) {
                        return s
                    }
                }
            } catch (error) {
                // fall through -- not every Qt version/backend exposes stringValue()
            }

            return ""
        }

        function numberFor(key) {
            try {
                if (typeof MediaMetaData !== "undefined" && video.metaData.value) {
                    var n = Number(video.metaData.value(MediaMetaData[key]))
                    if (!isNaN(n) && n > 0) {
                        return n
                    }
                }
            } catch (error) {
                // fall through
            }

            return 0
        }

        try {
            var size = video.metaData.resolution || video.metaData.Resolution
            if (size && size.width > 0 && size.height > 0) {
                info.resolutionText = size.width + " x " + size.height
            }
        } catch (error) {
            // keep default
        }

        info.fileFormat = stringFor("FileFormat")
        info.videoCodec = stringFor("VideoCodec")
        info.videoBitRate = numberFor("VideoBitRate")
        info.audioCodec = stringFor("AudioCodec")
        info.audioChannels = numberFor("ChannelCount")
        info.audioSampleRate = numberFor("SampleRate")
        info.audioBitRate = numberFor("AudioBitRate")

        return info
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
        if (forceSeek) {
            if (video.position !== targetPosition) {
                if (Date.now() - lastResyncTime >= forceSeekDebounceMs) {
                    video.seek(targetPosition)
                    lastResyncTime = Date.now()
                    pendingForceSeekTimer.stop()
                } else {
                    // The debounce blocked this forced seek -- don't just drop
                    // it, retry once the debounce window has elapsed so the
                    // video still ends up in sync with the score.
                    pendingForceSeekTimer.restart()
                }
            }
        } else if (Math.abs(video.position - targetPosition) > syncToleranceMs) {
            var now = Date.now()
            if (now - lastResyncTime >= minResyncIntervalMs) {
                video.seek(targetPosition)
                lastResyncTime = now
            }
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

    Timer {
        id: pendingForceSeekTimer
        interval: root.forceSeekDebounceMs
        onTriggered: root.syncVideoToScore(true)
    }

    // Keeps the playhead visible while playing a zoomed-in timeline, without a
    // continuous auto-scroll (fighting the user's own manual scrolling) --
    // instead jumps by a whole viewport-width "page" once the playhead actually
    // leaves the visible range, landing on a fixed page boundary (0, one
    // viewport width, two viewport widths, ...) rather than re-centering, so
    // repeated playback of the same region pages consistently.
    Connections {
        target: video

        function onPositionChanged() {
            root.pageTimelineToKeepPlayheadVisible()
        }

        function onPlaybackStateChanged() {
            videoModel.setVideoElementPlaying(video.playbackState === MediaPlayer.PlayingState)
        }
    }

    Component.onDestruction: videoModel.setVideoElementPlaying(false)

    function pageTimelineToKeepPlayheadVisible() {
        if (!videoModel.hasVideo || video.duration <= 0 || timelineFlickable.contentWidth <= timelineFlickable.width) {
            return
        }

        var playheadX = (video.position / video.duration) * timelineFlickable.contentWidth
        var viewStart = timelineFlickable.contentX
        var viewEnd = viewStart + timelineFlickable.width

        if (playheadX >= viewStart && playheadX <= viewEnd) {
            return
        }

        var page = Math.floor(playheadX / timelineFlickable.width)
        var pagedContentX = page * timelineFlickable.width
        timelineFlickable.contentX = Math.max(0, Math.min(timelineFlickable.contentWidth - timelineFlickable.width, pagedContentX))
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

            // NOTE: only capture/persist the sidebar width once the user finishes
            // dragging (not on every intermediate width change) -- so a transient
            // shrink caused by too little available space -- see hitPointsPanel's
            // SplitView.preferredWidth -- never overwrites the user's real
            // preference with a space-constrained value.
            Connections {
                target: resizingHandle.SplitHandle

                function onPressedChanged() {
                    if (!resizingHandle.SplitHandle.pressed && hitPointsPanel.width > 0) {
                        root.hitPointsPanelWidth = hitPointsPanel.width
                        videoModel.setHitPointsPanelWidth(hitPointsPanel.width)
                    }
                }
            }

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

            // NOTE: if the floating window gets resized narrower than this pane's
            // and the sidebar's combined minimums (the OS-level floating-window
            // resize floor isn't reliably enforced by the underlying dock
            // framework), clip so squeezed content is cut off at this pane's own
            // edge instead of visually bleeding into the sidebar.
            clip: true

            SplitView.fillWidth: true
            SplitView.fillHeight: true
            SplitView.minimumWidth: root.previewPaneMinWidth

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
                    font.family: root.monoFontFamily
                    font.pixelSize: 20
                    font.bold: true
                }

                Rectangle {
                    id: timecodeOverlayBackground

                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 10
                    // NOTE: must paint above videoFrame (declared below), which
                    // otherwise covers this row whenever the video fills the top
                    // of previewSlot.
                    z: 2
                    visible: videoModel.hasVideo

                    // NOTE: a plain translucent backdrop behind the readout --
                    // the video underneath can be any color (including near-white),
                    // so the light, near-white timer text needs *some* darkening
                    // behind it to stay legible in all cases. The alpha lives in
                    // the color itself (not this Rectangle's `opacity`), so it
                    // only darkens the video behind it without also fading out
                    // the fully-opaque text drawn on top of it.
                    //
                    // NOTE: sized from timecodeOverlayRow.width, not
                    // .implicitWidth -- a plain Row (below) always resolves its
                    // own width to the sum of its (visible) children's actual
                    // width, whereas a RowLayout's implicitWidth does not
                    // reliably reflect children sized via Layout.preferredWidth
                    // when the RowLayout itself isn't managed by an enclosing
                    // Layout (as was the case here before, and produced a
                    // background narrower than the text it was meant to sit
                    // behind). Row already grows/shrinks correctly as the
                    // reserved timecode/measure-number width or the number of
                    // visible zones changes, so this stays correct regardless
                    // of video length or measure count.
                    radius: 4
                    color: Qt.rgba(0, 0, 0, 0.5)
                    width: timecodeOverlayRow.width + 16
                    height: timecodeOverlayRow.height + 6

                    Row {
                        id: timecodeOverlayRow

                        anchors.centerIn: parent
                        spacing: 8

                        // NOTE: each zone below has a fixed width so digits changing width
                        // (e.g. "1" vs "0") during playback can't shift anything else --
                        // same technique as MeasureAndBeatFields.qml (see its comment/the
                        // linked issue #9633) applied to plain read-only labels here.
                        Item {
                            width: timerFontMetrics.advanceWidth("00:00:00:00")
                            height: timecodeLabel.implicitHeight

                            StyledTextLabel {
                                id: timecodeLabel
                                anchors.right: parent.right
                                text: videoModel.formatTimecode(video.position)
                                font.family: root.monoFontFamily
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
                            width: timerFontMetrics.advanceWidth("9999.9")
                            height: musicalPositionLabel.implicitHeight
                            visible: musicalPositionLabel.text.length > 0

                            StyledTextLabel {
                                id: musicalPositionLabel
                                anchors.left: parent.left
                                // NOTE: bound to the already-deduped/rounded score position
                                // (playbackSyncChanged, tens of Hz at most) rather than the raw
                                // video.position -- QtMultimedia fires positionChanged at its own
                                // frame cadence (tens of times/sec), each of which would otherwise
                                // redo a full tick2measure/utime2utick lookup for a label no one
                                // can read faster than ~10Hz anyway.
                                text: videoModel.musicalPositionText(videoModel.scorePlaybackPositionMs + videoModel.offsetMs)
                                font.family: root.monoFontFamily
                                font.pixelSize: 20
                                font.bold: true
                                color: "#F0F0F0"
                            }
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
                            // NOTE: was stop() -- stopping a freshly-set source (instead of
                            // pausing it) left some backends never reporting duration/
                            // rendering a frame afterward: reloading a video showed a blank
                            // frame and the whole timeline (gated on duration > 0) stayed
                            // hidden. pause() still resets any prior playback state without
                            // that side effect, and is compatible with the seek() that
                            // syncVideoToScore() below does once duration is known.
                            pause()
                        }

                        onDurationChanged: {
                            root.syncVideoToScore(true)

                            // NOTE: same detect-and-apply logic as the Settings
                            // tab's "Detect" button, just run automatically once
                            // a newly loaded video's metadata is available (that's
                            // what duration becoming known indicates), rather than
                            // requiring a manual click every time -- getting fps
                            // wrong is exactly what causes video/score sync drift.
                            var detectedRate = root.detectedFrameRate()
                            if (detectedRate > 0) {
                                videoModel.frameRate = detectedRate
                            }
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

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                RowLayout {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    FilePicker {
                        id: filePicker

                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        showPathField: false
                        path: videoModel.videoPath
                        dialogTitle: qsTrc("playback", "Choose video")
                        filter: qsTrc("playback", "Video files (*.mp4 *.mov *.m4v *.avi *.mkv *.webm);;All files (*)")
                        buttonType: FlatButton.IconOnly
                        navigation: navigationPanel
                        navigationRowOrderStart: root.contentNavigationPanelOrderStart + 2

                        onPathEdited: function(newPath) {
                            videoModel.videoPath = newPath
                        }
                    }

                    FlatButton {
                        id: recentFilesButton

                        Layout.preferredWidth: 22
                        Layout.preferredHeight: 22
                        Layout.alignment: Qt.AlignVCenter
                        icon: IconCode.SMALL_ARROW_DOWN
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        toolTipTitle: qsTrc("playback", "Recently opened videos")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 5

                        onClicked: {
                            var recentFiles = videoModel.recentVideoFiles()
                            var menuItems = recentFiles.map(function(path) {
                                return { id: path, title: path }
                            })
                            if (recentFiles.length > 0) {
                                menuItems.push({ id: "", title: "" }) // separator
                                menuItems.push({ id: "__clearRecent__", title: qsTrc("playback", "Clear list") })
                            }
                            recentFilesMenu.items = menuItems
                            recentFilesMenu.show(Qt.point(0, height))
                        }

                        ContextMenuLoader {
                            id: recentFilesMenu

                            onHandleMenuItem: function(itemId) {
                                if (itemId === "__clearRecent__") {
                                    videoModel.clearRecentVideoFiles()
                                    return
                                }

                                videoModel.videoPath = itemId
                            }
                        }
                    }
                }

                // NOTE: centered on this Item's full width, which matches the video
                // preview's width above -- not on the space left over between the
                // Load/Recent group and the zoom controls (those differ in width, so
                // that would center this group off from the video's true midpoint).
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    FlatButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        enabled: videoModel.hasVideo
                        text: qsTrc("playback", "#", "Mark: add a hit point at the current position")
                        toolTipTitle: qsTrc("playback", "Add hit point at current position")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 6

                        onClicked: {
                            videoModel.addHitPoint(video.position)
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        enabled: videoModel.hasVideo
                        text: qsTrc("playback", "M", "Mute the video's own audio track")
                        accentButton: videoModel.muted
                        toolTipTitle: qsTrc("playback", "Mute video audio")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 7

                        onClicked: {
                            videoModel.muted = !videoModel.muted
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        enabled: videoModel.hasVideo
                        icon: IconCode.REWIND_START_FILL
                        buttonType: FlatButton.IconOnly
                        toolTipTitle: qsTrc("playback", "Rewind to start")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 8

                        onClicked: {
                            root.seekToVideoPositionMs(0)
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        enabled: videoModel.hasVideo
                        icon: video.playbackState === MediaPlayer.PlayingState ? IconCode.PAUSE : IconCode.PLAY
                        iconFont: ui.theme.toolbarIconsFont
                        buttonType: FlatButton.IconOnly
                        toolTipTitle: video.playbackState === MediaPlayer.PlayingState ? qsTrc("playback", "Pause") : qsTrc("playback", "Play")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 9

                        onClicked: {
                            videoModel.toggleScorePlay()
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        enabled: videoModel.hasVideo
                        icon: IconCode.STOP
                        iconFont: ui.theme.toolbarIconsFont
                        buttonType: FlatButton.IconOnly
                        toolTipTitle: qsTrc("playback", "Stop")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 10

                        onClicked: {
                            videoModel.stopScorePlayback()
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        enabled: videoModel.hasVideo
                        icon: IconCode.LOOP
                        iconFont: ui.theme.toolbarIconsFont
                        buttonType: FlatButton.IconOnly
                        accentButton: videoModel.loopEnabled
                        toolTipTitle: qsTrc("playback", "Loop playback")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 11

                        onClicked: {
                            videoModel.toggleLoopPlayback()
                        }
                    }
                }

                RowLayout {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    FlatButton {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        icon: IconCode.ZOOM_OUT
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        // NOTE: deliberately not disabled at the zoom boundary -- disabled
                        // buttons are dimmed via opacity, and at this icon's default tint
                        // that dimming reads as fully invisible against a dark theme's
                        // toolbar. The boundary is still enforced, just in the handler.
                        enabled: videoModel.hasVideo
                        toolTipTitle: qsTrc("playback", "Zoom out")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 12

                        onClicked: {
                            root.timelineZoom = Math.max(1, root.timelineZoom - 0.25)
                        }
                    }

                    // NOTE: plain read-only number when not editing (no box, no "%"
                    // sign) -- click to edit, matching the double-click-to-edit idiom
                    // used elsewhere in this panel (hit point timecode/label).
                    Item {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 24

                        StyledTextLabel {
                            anchors.fill: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 9
                            text: Math.round(root.timelineZoom * 100).toString()
                            visible: !root.editingZoom
                            enabled: videoModel.hasVideo

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    root.editingZoom = true
                                    zoomEditor.forceActiveFocus()
                                    zoomEditor.selectAll()
                                }
                            }
                        }

                        TextInputField {
                            id: zoomEditor

                            anchors.fill: parent
                            currentText: Math.round(root.timelineZoom * 100).toString()
                            visible: root.editingZoom
                            inputField.font.pixelSize: 9
                            navigation.panel: navigationPanel
                            navigation.order: root.contentNavigationPanelOrderStart + 13

                            onTextEditingFinished: function(newTextValue) {
                                var parsedValue = parseInt(newTextValue, 10)
                                if (!isNaN(parsedValue) && parsedValue > 0) {
                                    root.timelineZoom = Math.max(1, Math.min(root.timelineZoomMax, parsedValue / 100))
                                }
                                root.editingZoom = false
                            }

                            Keys.onEscapePressed: {
                                root.editingZoom = false
                            }
                        }
                    }

                    FlatButton {
                        id: zoomPresetsButton

                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 24
                        icon: IconCode.SMALL_ARROW_DOWN
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        enabled: videoModel.hasVideo
                        toolTipTitle: qsTrc("playback", "Zoom presets")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 14

                        onClicked: {
                            zoomPresetsMenu.items = [100, 250, 500, 750].map(function(percent) {
                                return { id: percent.toString(), title: percent + "%" }
                            })
                            zoomPresetsMenu.show(Qt.point(0, height))
                        }

                        ContextMenuLoader {
                            id: zoomPresetsMenu

                            onHandleMenuItem: function(itemId) {
                                root.timelineZoom = parseInt(itemId, 10) / 100
                            }
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        icon: IconCode.ZOOM_IN
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        enabled: videoModel.hasVideo
                        toolTipTitle: qsTrc("playback", "Zoom in")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 15

                        onClicked: {
                            root.timelineZoom = Math.min(root.timelineZoomMax, root.timelineZoom + 0.25)
                        }
                    }

                    FlatButton {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        Layout.leftMargin: 4
                        icon: root.hitPointsPanelVisible ? IconCode.CHEVRON_RIGHT : IconCode.CHEVRON_LEFT
                        buttonType: FlatButton.IconOnly
                        transparent: true
                        toolTipTitle: root.hitPointsPanelVisible ? qsTrc("playback", "Hide sidebar") : qsTrc("playback", "Show sidebar")
                        navigation.panel: navigationPanel
                        navigation.order: root.contentNavigationPanelOrderStart + 16

                        onClicked: {
                            root.toggleHitPointsPanelVisible()
                        }
                    }
                }
            }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 88

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
                                // NOTE: fixed dark color, not a theme token -- this strip hosts the
                                // white hit-point badge text/lines, and under the light theme
                                // backgroundSecondaryColor was too close to white to read against.
                                color: "#2A2A2A"
                                visible: videoModel.hasVideo && video.duration > 0

                                MouseArea {
                                    anchors.fill: parent

                                    onDoubleClicked: function(mouse) {
                                        videoModel.addHitPoint(root.timelinePositionForX(mouse.x, width))
                                    }
                                }

                                Repeater {
                                    model: videoModel.hitPoints

                                    Item {
                                        id: hitPointMarker

                                        required property var modelData
                                        required property int index
                                        readonly property bool dragging: root.draggingHitPointId === modelData.id
                                        property bool editingLabel: false
                                        readonly property real dragTimeMs: root.draggingHitPointTimeMs
                                        readonly property real displayTimeMs: dragging ? dragTimeMs : modelData.timeMs
                                        readonly property real tickX: (displayTimeMs / Math.max(video.duration, 1)) * parent.width
                                        readonly property int labelWidth: Math.max(20, badgeLabel.implicitWidth + 8)

                                        // NOTE: clamped only by the line's own width, not the whole
                                        // item's (line + label) width -- the line marks the exact
                                        // hit-point time and must match the reference line in the
                                        // timeline track below at all times, including near the very
                                        // end. Clamping by the full item width (as this used to) kept
                                        // the label on-screen there, but dragged the line along with
                                        // it, desyncing it from the true time / the track's line.
                                        x: Math.max(0, Math.min(parent.width - markerLine.width, tickX))
                                        y: 0
                                        width: markerLine.width + labelWidth
                                        height: parent.height

                                        Rectangle {
                                            id: markerLine
                                            width: 2
                                            height: parent.height
                                            color: ui.theme.accentColor
                                        }

                                        Rectangle {
                                            id: markerLabelBox
                                            // NOTE: the label -- not the line -- absorbs the edge
                                            // clamping instead, sliding left of its normal position
                                            // (immediately right of the line) only as much as needed to
                                            // stay fully inside the ruler.
                                            x: Math.min(markerLine.width,
                                                        hitPointMarker.parent.width - hitPointMarker.x - hitPointMarker.labelWidth)
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: hitPointMarker.labelWidth
                                            height: 16
                                            radius: 0
                                            color: ui.theme.accentColor
                                            border.width: hitPointMarker.dragging ? 2 : 0
                                            border.color: "white"

                                            StyledTextLabel {
                                                id: badgeLabel
                                                anchors.centerIn: parent
                                                text: hitPointMarker.modelData.label
                                                maximumLineCount: 1
                                                font.pixelSize: 8
                                                font.bold: true
                                                color: "white"
                                                visible: !hitPointMarker.editingLabel
                                            }

                                            TextInputField {
                                                id: markerLabelEditor

                                                // NOTE: left-anchored (not centerIn) so the wider edit
                                                // field grows to the right of the marker line instead of
                                                // straddling the badge's own (narrower) center, which
                                                // visibly shifted it left of the line/text when editing.
                                                anchors.left: parent.left
                                                anchors.verticalCenter: parent.verticalCenter
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
                                                }
                                                // NOTE: deliberately does nothing else on press -- starting
                                                // the drag here (as before) snapped the marker to the click
                                                // position even for a plain click with no movement. Only
                                                // onPositionChanged (real movement) starts an actual drag.
                                            }

                                            onPositionChanged: function(mouse) {
                                                if (!pressed) {
                                                    return
                                                }

                                                var mapped = mapToItem(timelineTrack, mouse.x, mouse.y)
                                                root.draggingHitPointId = hitPointMarker.modelData.id
                                                root.draggingHitPointTimeMs = root.timelinePositionForX(mapped.x, timelineTrack.width)
                                            }

                                            onReleased: function(mouse) {
                                                if (mouse.button === Qt.RightButton) {
                                                    return
                                                }

                                                if (root.draggingHitPointId === hitPointMarker.modelData.id) {
                                                    // NOTE: seek BEFORE persisting the new hit-point time --
                                                    // setHitPointTimeMs() synchronously fires settingsChanged
                                                    // -> onVideoSettingsChanged -> syncVideoToScore(true), so if
                                                    // that ran first, its forced reseek would fire before
                                                    // seekToVideoPositionMs() below had a chance to stamp
                                                    // lastResyncTime, defeating forceSeekDebounceMs and causing
                                                    // the exact double video.seek() decoder-flood this debounce
                                                    // was added to prevent.
                                                    root.seekToVideoPositionMs(root.draggingHitPointTimeMs)
                                                    videoModel.setHitPointTimeMs(hitPointMarker.modelData.id, root.draggingHitPointTimeMs)
                                                    root.draggingHitPointId = -1
                                                }
                                            }

                                            onCanceled: {
                                                if (root.draggingHitPointId === hitPointMarker.modelData.id) {
                                                    root.draggingHitPointId = -1
                                                }
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
                                // NOTE: fixed dark color, not a theme token -- same reasoning as
                                // hitPointsRuler above (this hosts the fixed-light tick/second-label
                                // canvas text, which needs a reliably dark surface under it).
                                color: "#181818"

                            // Shaded region for MuseScore's own score loop-playback range (the
                            // main transport toolbar's Loop button), converted from score ticks to
                            // video-timeline position the same way hit points are.
                            Rectangle {
                                id: loopRegion

                                readonly property real startX: (videoModel.loopStartMs / Math.max(video.duration, 1)) * parent.width
                                readonly property real endX: (videoModel.loopEndMs / Math.max(video.duration, 1)) * parent.width

                                visible: videoModel.hasVideo && video.duration > 0 && videoModel.loopEnabled
                                         && videoModel.loopEndMs > videoModel.loopStartMs
                                x: Math.max(0, Math.min(parent.width, startX))
                                width: Math.max(0, Math.min(parent.width, endX) - x)
                                height: parent.height
                                color: ui.theme.accentColor
                                opacity: 0.18
                            }

                            // Greys out the stretch of video with no corresponding music (e.g.
                            // the video runs longer than the score) -- reads as "nothing to
                            // sync against here" rather than implying hit points still make
                            // sense against real measures out there. Stays correct as measures
                            // are added/removed since scoreEndVideoPositionMs is recomputed on
                            // every score edit (see VideoPanelModel::listenCurrentProject()).
                            Rectangle {
                                id: pastScoreEndRegion

                                readonly property real startX: (videoModel.scoreEndVideoPositionMs / Math.max(video.duration, 1)) * parent.width

                                visible: videoModel.hasVideo && video.duration > 0 && videoModel.scoreEndVideoPositionMs >= 0
                                         && videoModel.scoreEndVideoPositionMs < video.duration
                                x: Math.max(0, Math.min(parent.width, startX))
                                width: parent.width - x
                                height: parent.height
                                color: "#000000"
                                opacity: 0.45
                            }

                            MouseArea {
                                anchors.fill: parent
                                enabled: videoModel.hasVideo && video.seekable
                                cursorShape: Qt.PointingHandCursor

                                onPressed: function(mouse) {
                                    root.lastScrubSeekTime = Date.now()
                                    root.seekToVideoPositionMs(root.timelinePositionForX(mouse.x, width))
                                }

                                onPositionChanged: function(mouse) {
                                    if (!pressed) {
                                        return
                                    }

                                    const now = Date.now()
                                    if (now - root.lastScrubSeekTime < root.scrubSeekIntervalMs) {
                                        return
                                    }

                                    root.lastScrubSeekTime = now
                                    root.seekToVideoPositionMs(root.timelinePositionForX(mouse.x, width))
                                }

                                onReleased: function(mouse) {
                                    root.seekToVideoPositionMs(root.timelinePositionForX(mouse.x, width))
                                }
                            }

                            Repeater {
                                model: videoModel.hitPoints

                                Rectangle {
                                    required property var modelData

                                    // NOTE: follow the live drag position (not the not-yet-committed
                                    // modelData.timeMs) while this hit point is being dragged in the
                                    // ruler above, so the two reference lines never visibly diverge.
                                    readonly property real displayTimeMs: root.draggingHitPointId === modelData.id ? root.draggingHitPointTimeMs : modelData.timeMs

                                    x: Math.max(0, Math.min(parent.width - width,
                                                             (displayTimeMs / Math.max(video.duration, 1)) * parent.width - (width / 2)))
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

                                // NOTE: the top strip is reserved for measure numbers (drawn above
                                // the ticks), so the ticks/second-labels below are shifted down by
                                // that same amount rather than starting at y:0.
                                readonly property int measureRowHeight: 12

                                anchors.left: parent.left
                                anchors.right: parent.right
                                y: 6
                                height: 30 + measureRowHeight
                                visible: videoModel.hasVideo && video.duration > 0

                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)

                                    var tickCount = root.timelineFrameCount
                                    if (tickCount <= 0) {
                                        return
                                    }

                                    // NOTE: a long, high-fps video at low zoom can have far more
                                    // frame-ticks than there are horizontal pixels to draw them in --
                                    // drawing every single one is wasted work (sub-pixel-width rects
                                    // overwriting each other) and can freeze the UI on multi-hour
                                    // footage. Stride through ticks so at most ~1 is drawn per pixel
                                    // column; visually identical since ticks narrower than a pixel
                                    // aren't distinguishable anyway.
                                    var tickStride = Math.max(1, Math.floor(tickCount / Math.max(width, 1)))

                                    // NOTE: fixed light colors, not theme tokens -- this canvas always
                                    // sits on the ruler/track's own background (hitPointsRuler/
                                    // timelineTrack below), which stays dark in both the app's light
                                    // and dark themes, so a theme-adaptive fontPrimaryColor (dark under
                                    // the light theme) goes unreadable here. Same reasoning as the
                                    // hardcoded "#F0F0F0" timer text over previewSlot above.
                                    var primaryColor = "#F0F0F0"
                                    var secondaryColor = "#F0F0F0"
                                    var measureColor = "#FFFFFF"
                                    var tickTop = measureRowHeight
                                    for (var i = 0; i < tickCount; i += tickStride) {
                                        var tickWidth = root.timelineTickWidth(i)
                                        var tickHeight = root.timelineTickHeight(i)
                                        var x = Math.max(0, Math.min(width - tickWidth, (i / Math.max(tickCount - 1, 1)) * width - (tickWidth / 2)))
                                        ctx.globalAlpha = i % root.timelineFrameRate === 0 ? 0.62 : 0.24
                                        ctx.fillStyle = i % root.timelineFrameRate === 0 ? primaryColor : secondaryColor
                                        ctx.fillRect(x, tickTop, tickWidth, tickHeight)
                                    }

                                    var durationSeconds = Math.floor(video.duration / 1000)
                                    ctx.font = "10px sans-serif"
                                    ctx.textBaseline = "bottom"
                                    ctx.fillStyle = primaryColor
                                    ctx.globalAlpha = 0.74

                                    // NOTE: no longer forces a final "total duration" label via a
                                    // dedicated reserved-space calculation -- that broke visibly at
                                    // some zoom levels (labels overlapping/garbled). Instead, the exact
                                    // end position is just one more candidate through the SAME simple
                                    // collision check as every other grid label below, so it's skipped
                                    // like any other label would be if it doesn't fit, and shown
                                    // otherwise -- this is also what makes the measure number for the
                                    // score's true final measure reach the timeline, not just whatever
                                    // measure the last 5s-aligned gridpoint happens to land on.
                                    var minLabelGap = 6
                                    var lastLabelRightEdge = -Infinity
                                    var lastGridSecond = durationSeconds - (durationSeconds % 5)

                                    var gridSeconds = []
                                    for (var s = 0; s <= lastGridSecond; s += 5) {
                                        gridSeconds.push(s)
                                    }
                                    if (durationSeconds !== lastGridSecond) {
                                        gridSeconds.push(durationSeconds)
                                    }

                                    for (var gi = 0; gi < gridSeconds.length; ++gi) {
                                        var second = gridSeconds[gi]
                                        var label = root.timelineLabel(second)
                                        var labelWidth = ctx.measureText(label).width
                                        var labelX = (second * 1000 / Math.max(video.duration, 1)) * width

                                        var align = labelX < 16 ? "left" : (labelX > width - 16 ? "right" : "center")
                                        var leftEdge = align === "left" ? labelX : (align === "right" ? labelX - labelWidth : labelX - labelWidth / 2)
                                        var rightEdge = leftEdge + labelWidth

                                        if (leftEdge < lastLabelRightEdge + minLabelGap) {
                                            continue
                                        }

                                        ctx.textAlign = align
                                        ctx.fillStyle = primaryColor
                                        ctx.globalAlpha = 0.74
                                        ctx.fillText(label, labelX, height)
                                        lastLabelRightEdge = rightEdge
                                    }

                                    // Measure numbers, right above the ticks -- lets a hit point's
                                    // timing be read against the score directly on the timeline, not
                                    // just in the sidebar table. NOTE: each measure's label is placed
                                    // at ITS OWN true tick-accurate position (from measurePositions()),
                                    // not approximated to the nearest 5-second grid interval like the
                                    // time labels above -- that approximation used to visibly desync
                                    // this from other tick-accurate timeline elements, e.g. a measure
                                    // label could sit up to ~2.5s away from where that measure (and so
                                    // e.g. the loop-range shading) actually starts.
                                    var measures = videoModel.measurePositions()
                                    if (measures.length > 0) {
                                        ctx.textBaseline = "top"
                                        ctx.fillStyle = measureColor
                                        ctx.globalAlpha = 0.85

                                        // NOTE: regular "every Nth measure" step instead of a greedy
                                        // fit-as-many-as-fit pass -- measures don't take equal screen
                                        // width, so a greedy pass produced an irregular, unpredictable
                                        // pattern (e.g. "M1, M3, M6, M8..."). The step shrinks as
                                        // zooming in gives each measure more screen space, showing
                                        // every 2nd/1st measure once there's room, rather than staying
                                        // stuck at whatever spacing fit at the very start.
                                        var sampleLabel = "" + measures[measures.length - 1].measureNumber
                                        var estimatedLabelWidth = ctx.measureText(sampleLabel).width + minLabelGap
                                        var avgPxPerMeasure = width / measures.length
                                        var measureStep = Math.max(1, Math.ceil(estimatedLabelWidth / avgPxPerMeasure))

                                        for (var mi = 0; mi < measures.length; mi += measureStep) {
                                            var measureLabel = "" + measures[mi].measureNumber
                                            var measureLabelX = (measures[mi].videoPositionMs / Math.max(video.duration, 1)) * width

                                            ctx.textAlign = measureLabelX < 16 ? "left" : (measureLabelX > width - 16 ? "right" : "center")
                                            ctx.fillText(measureLabel, measureLabelX, 0)
                                        }

                                        ctx.textBaseline = "bottom"
                                    }
                                    ctx.globalAlpha = 1
                                }

                                Connections {
                                    target: videoModel
                                    function onVideoSettingsChanged() {
                                        frameTickCanvas.requestPaint()
                                    }
                                    function onScoreContentChanged() {
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
            }
        }

        VideoHitPointsPanel {
            id: hitPointsPanel

            visible: root.hitPointsPanelVisible

            // NOTE: never request more than (total - leftPane's own minimum) --
            // otherwise a stale/persisted width can force the SplitView to shrink
            // leftPane below what it needs, which reads as the sidebar "covering"
            // the video.
            SplitView.preferredWidth: Math.min(root.hitPointsPanelWidth,
                                                Math.max(root.hitPointsPanelMinWidth, contentSplitView.width - leftPane.SplitView.minimumWidth))
            SplitView.minimumWidth: root.hitPointsPanelMinWidth
            SplitView.maximumWidth: root.hitPointsPanelMaxWidth
            SplitView.fillHeight: true

            videoModel: videoModel
            seekToVideoPositionMs: root.seekToVideoPositionMs
            detectFrameRate: root.detectedFrameRate
            currentVideoPositionMs: video.position
            currentVideoDurationMs: video.duration
            clearAttachedVideo: root.clearAttachedVideo
            videoInfo: root.videoInfo
            navigationPanel: navigationPanel
            navigationOrderStart: root.contentNavigationPanelOrderStart + 20
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

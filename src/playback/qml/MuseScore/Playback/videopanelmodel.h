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

#pragma once

#include <functional>
#include <vector>

#include <QObject>
#include <qqmlintegration.h>
#include <QStringList>
#include <QUrl>
#include <QVariantList>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

#include "notation/inotationplayback.h"
#include "project/inotationproject_fwd.h"

#include "iplaybackconfiguration.h"
#include "iplaybackcontroller.h"

namespace mu::playback {
class VideoPanelModel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY videoSettingsChanged)
    Q_PROPERTY(QString videoPath READ videoPath WRITE setVideoPath NOTIFY videoSettingsChanged)
    Q_PROPERTY(QUrl videoUrl READ videoUrl NOTIFY videoSettingsChanged)
    Q_PROPERTY(int offsetMs READ offsetMs WRITE setOffsetMs NOTIFY videoSettingsChanged)
    Q_PROPERTY(int volumePercent READ volumePercent WRITE setVolumePercent NOTIFY videoSettingsChanged)
    Q_PROPERTY(int balance READ balance WRITE setBalance NOTIFY videoSettingsChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY videoSettingsChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY videoSettingsChanged)
    Q_PROPERTY(double frameRate READ frameRate WRITE setFrameRate NOTIFY videoSettingsChanged)
    Q_PROPERTY(int timecodeDisplayMode READ timecodeDisplayMode WRITE setTimecodeDisplayMode NOTIFY videoSettingsChanged)
    Q_PROPERTY(QVariantList hitPoints READ hitPoints NOTIFY hitPointsChanged)
    Q_PROPERTY(bool scorePlaying READ scorePlaying NOTIFY playbackSyncChanged)
    Q_PROPERTY(int scorePlaybackPositionMs READ scorePlaybackPositionMs NOTIFY playbackSyncChanged)
    //! NOTE MuseScore's own score loop-playback feature (the main transport
    //! toolbar's Loop button) -- loopStartMs/loopEndMs are 0 when there's no
    //! loop range set (check loopEnabled and/or hasVideo before using them).
    Q_PROPERTY(bool loopEnabled READ loopEnabled NOTIFY loopChanged)
    Q_PROPERTY(int loopStartMs READ loopStartMs NOTIFY loopChanged)
    Q_PROPERTY(int loopEndMs READ loopEndMs NOTIFY loopChanged)
    //! NOTE Video position where the score's last measure ends -- lets the
    //! timeline grey out the stretch of video with no corresponding music
    //! (e.g. the video runs longer than the score). -1 if there's no current
    //! project/score.
    Q_PROPERTY(int scoreEndVideoPositionMs READ scoreEndVideoPositionMs NOTIFY scoreContentChanged)

    QML_ELEMENT

    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::ContextInject<IPlaybackController> playbackController = { this };
    muse::GlobalInject<IPlaybackConfiguration> configuration;

public:
    explicit VideoPanelModel(QObject* parent = nullptr);
    //! NOTE Out-of-line (not defaulted here) since m_lastHitPoints is a
    //! vector of a type only forward-declared in this header.
    ~VideoPanelModel() override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void clearVideo();
    Q_INVOKABLE void nudgeOffset(int deltaMs);
    Q_INVOKABLE void addHitPoint(int videoPositionMs);
    //! NOTE hitPointId identifies a hit point by its stable VideoHitPointSettings::id
    //! (see hitPoints()'s "id" role), not its position in the list -- the list is
    //! re-sorted chronologically on every edit, so a position captured before the
    //! call may no longer refer to the same hit point by the time it runs.
    Q_INVOKABLE void removeHitPoint(int hitPointId);
    Q_INVOKABLE void renameHitPoint(int hitPointId, const QString& label);
    Q_INVOKABLE void setHitPointTimeMs(int hitPointId, int videoPositionMs);
    Q_INVOKABLE void setHitPointTimecode(int hitPointId, const QString& timecode);
    Q_INVOKABLE QString formatTimecode(int videoPositionMs) const;
    //! NOTE "<measure>.<beat>" for the score position corresponding to videoPositionMs,
    //! e.g. "34.3" -- same format already used per-hit-point in hitPoints(). Empty
    //! string if there is no current project/score.
    Q_INVOKABLE QString musicalPositionText(int videoPositionMs) const;
    //! NOTE One entry per measure -- {"measureNumber": int, "videoPositionMs": int} --
    //! for drawing each measure's own label at its true tick-accurate timeline
    //! position (approximating it to the nearest 5-second grid interval, like the
    //! plain second/minute labels do, visibly desyncs from tick-accurate
    //! positions elsewhere on the timeline, e.g. the loop range). Empty list if
    //! there is no current project/score.
    Q_INVOKABLE QVariantList measurePositions() const;

    //! NOTE Toggles the score's own playback; the video follows via the existing
    //! score->video sync (playbackSyncChanged), it is not driven directly here.
    Q_INVOKABLE void toggleScorePlay();
    //! NOTE Stops the score's own playback outright (distinct from Play/Pause's
    //! toggle) -- the video follows via the same score->video sync. Useful when
    //! the video is shorter than the score: once the video finishes on its own,
    //! this can stop the still-playing score without switching back to the main
    //! MuseScore window.
    Q_INVOKABLE void stopScorePlayback();
    //! NOTE Toggles MuseScore's own score loop-playback feature -- the same one
    //! the main transport toolbar's Loop button controls, not something
    //! specific to the video panel.
    Q_INVOKABLE void toggleLoopPlayback();
    //! NOTE Seeks the score to the position corresponding to videoPositionMs (minus
    //! the sync offset). Does not touch the video element itself -- QML seeks the
    //! video separately since it owns that QtMultimedia object.
    Q_INVOKABLE void seekScoreToVideoPositionMs(int videoPositionMs);

    //! NOTE Called from VideoPanel.qml whenever the actual Qt Multimedia video element's
    //! playbackState changes -- forwarded to IPlaybackController so the Mixer's Video
    //! channel meter can react to the video really playing, distinct from the score's own
    //! transport state (they can briefly diverge, e.g. the video ending before the score).
    Q_INVOKABLE void setVideoElementPlaying(bool playing);

    //! NOTE Persisted user preference for the hit-points side panel's width (not a
    //! live-bound Q_PROPERTY -- QML reads it once on load and writes it back when
    //! the user finishes dragging the resize handle).
    Q_INVOKABLE int hitPointsPanelWidth() const;
    Q_INVOKABLE void setHitPointsPanelWidth(int width);
    Q_INVOKABLE bool hitPointsPanelVisible() const;
    Q_INVOKABLE void setHitPointsPanelVisible(bool visible);

    Q_INVOKABLE QStringList recentVideoFiles() const;
    Q_INVOKABLE void clearRecentVideoFiles();

    bool hasVideo() const;
    QString videoPath() const;
    QUrl videoUrl() const;
    void setVideoPath(const QString& path);

    int offsetMs() const;
    void setOffsetMs(int offsetMs);

    int volumePercent() const;
    void setVolumePercent(int volumePercent);

    int balance() const;
    void setBalance(int balance);

    bool muted() const;
    void setMuted(bool muted);

    bool solo() const;
    void setSolo(bool solo);

    double frameRate() const;
    void setFrameRate(double frameRate);

    int timecodeDisplayMode() const;
    void setTimecodeDisplayMode(int mode);

    QVariantList hitPoints() const;

    bool scorePlaying() const;
    int scorePlaybackPositionMs() const;

    bool loopEnabled() const;
    int loopStartMs() const;
    int loopEndMs() const;

    int scoreEndVideoPositionMs() const;

signals:
    void videoSettingsChanged();
    void hitPointsChanged();
    void playbackSyncChanged();
    void loopChanged();
    void scoreContentChanged();

private:
    project::IProjectVideoSettingsPtr videoSettings() const;
    project::VideoAttachmentSettings attachment() const;
    void updateAttachment(const project::VideoAttachmentSettings& attachment);
    QVariantMap hitPointToMap(const project::VideoHitPointSettings& hitPoint) const;
    static int indexOfHitPoint(const project::VideoAttachmentSettings& attachment, int hitPointId);
    //! NOTE Shared lookup+guard+persist sequence for every single-hit-point
    //! mutation (rename/retime/remove) -- looks up hitPointId, no-ops if the
    //! attachment/hit point isn't valid, otherwise runs mutate() against the
    //! (mutable) attachment and its index, then persists via updateAttachment().
    void withHitPoint(int hitPointId, const std::function<void(project::VideoAttachmentSettings&, int)>& mutate);
    int parseTimecodeToMs(const QString& timecode) const;
    notation::INotationPlaybackPtr notationPlayback() const;
    //! NOTE Uses the same score->utick2utime() conversion as musicalPositionText()'s
    //! reverse direction (score->utime2utick()), rather than
    //! INotationPlayback::playedTickToSec() (which is repeat-aware and, on at
    //! least one score, produced a different -- wrong -- result for this).
    //! Returns -1 if there's no current project/score.
    int tickToVideoPositionMs(int tick) const;
    void listenCurrentProject();
    void listenPlaybackState();

    int m_scorePlaybackPositionMs = 0;

    //! NOTE Cheap proxy for "did measurePositions()'s output actually change"
    //! -- see listenCurrentProject()'s notationChanged() handler.
    size_t m_lastMeasureCount = 0;
    int m_lastScoreEndTick = -1;

    //! NOTE videoSettingsChanged fires on any attachment field mutation, but
    //! the hitPoints Q_PROPERTY only actually needs rebuilding (and its bound
    //! QML Repeaters only need to tear down/recreate delegates) when the hit
    //! points themselves changed -- see listenCurrentProject().
    std::vector<project::VideoHitPointSettings> m_lastHitPoints;
};
}

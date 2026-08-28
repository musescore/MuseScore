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

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "project/iprojectvideosettings.h"

#include "iplaybackcontroller.h"
#include "iplaybackconfiguration.h"

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
    Q_PROPERTY(QVariantList hitPoints READ hitPoints NOTIFY videoSettingsChanged)
    Q_PROPERTY(bool scorePlaying READ scorePlaying NOTIFY playbackSyncChanged)
    Q_PROPERTY(int scorePlaybackPositionMs READ scorePlaybackPositionMs NOTIFY playbackSyncChanged)

    QML_ELEMENT

    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::ContextInject<IPlaybackController> playbackController = { this };
    muse::GlobalInject<IPlaybackConfiguration> configuration;

public:
    explicit VideoPanelModel(QObject* parent = nullptr);

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

    //! NOTE Toggles the score's own playback; the video follows via the existing
    //! score->video sync (playbackSyncChanged), it is not driven directly here.
    Q_INVOKABLE void toggleScorePlay();
    //! NOTE Seeks the score to the position corresponding to videoPositionMs (minus
    //! the sync offset). Does not touch the video element itself -- QML seeks the
    //! video separately since it owns that QtMultimedia object.
    Q_INVOKABLE void seekScoreToVideoPositionMs(int videoPositionMs);

    //! NOTE Persisted user preference for the hit-points side panel's width (not a
    //! live-bound Q_PROPERTY -- QML reads it once on load and writes it back when
    //! the user finishes dragging the resize handle).
    Q_INVOKABLE int hitPointsPanelWidth() const;
    Q_INVOKABLE void setHitPointsPanelWidth(int width);

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

signals:
    void videoSettingsChanged();
    void playbackSyncChanged();

private:
    project::IProjectVideoSettingsPtr videoSettings() const;
    project::VideoAttachmentSettings attachment() const;
    void updateAttachment(const project::VideoAttachmentSettings& attachment);
    QVariantMap hitPointToMap(const project::VideoHitPointSettings& hitPoint) const;
    static int indexOfHitPoint(const project::VideoAttachmentSettings& attachment, int hitPointId);
    QString musicalPositionText(int videoPositionMs) const;
    int parseTimecodeToMs(const QString& timecode) const;
    void listenCurrentProject();
    void listenPlaybackState();

    int m_scorePlaybackPositionMs = 0;
};
}

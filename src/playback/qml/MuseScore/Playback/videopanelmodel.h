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

public:
    explicit VideoPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void clearVideo();
    Q_INVOKABLE void nudgeOffset(int deltaMs);
    Q_INVOKABLE void addHitPoint(int videoPositionMs);
    Q_INVOKABLE void removeHitPoint(int index);
    Q_INVOKABLE void renameHitPoint(int index, const QString& label);
    Q_INVOKABLE void setHitPointTimeMs(int index, int videoPositionMs);
    Q_INVOKABLE void setHitPointTimecode(int index, const QString& timecode);
    Q_INVOKABLE QString formatTimecode(int videoPositionMs) const;

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
    QString musicalPositionText(int videoPositionMs) const;
    int parseTimecodeToMs(const QString& timecode) const;
    void listenCurrentProject();
    void listenPlaybackState();

    int m_scorePlaybackPositionMs = 0;
};
}

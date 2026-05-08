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
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "project/iprojectvideosettings.h"
#include "playback/iplaybackcontroller.h"

namespace mu::playback {
class VideoPanelModel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY videoSettingsChanged)
    Q_PROPERTY(QString videoPath READ videoPath WRITE setVideoPath NOTIFY videoSettingsChanged)
    Q_PROPERTY(QUrl videoUrl READ videoUrl NOTIFY videoSettingsChanged)
    Q_PROPERTY(int offsetMs READ offsetMs WRITE setOffsetMs NOTIFY videoSettingsChanged)
    Q_PROPERTY(int volumePercent READ volumePercent WRITE setVolumePercent NOTIFY videoSettingsChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY videoSettingsChanged)
    Q_PROPERTY(bool scorePlaying READ scorePlaying NOTIFY playbackSyncChanged)
    Q_PROPERTY(int scorePlaybackPositionMs READ scorePlaybackPositionMs NOTIFY playbackSyncChanged)

    QML_ELEMENT

    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::Inject<IPlaybackController> playbackController = { this };

public:
    explicit VideoPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void clearVideo();
    Q_INVOKABLE void nudgeOffset(int deltaMs);

    bool hasVideo() const;
    QString videoPath() const;
    QUrl videoUrl() const;
    void setVideoPath(const QString& path);

    int offsetMs() const;
    void setOffsetMs(int offsetMs);

    int volumePercent() const;
    void setVolumePercent(int volumePercent);

    bool muted() const;
    void setMuted(bool muted);

    bool scorePlaying() const;
    int scorePlaybackPositionMs() const;

signals:
    void videoSettingsChanged();
    void playbackSyncChanged();

private:
    project::IProjectVideoSettingsPtr videoSettings() const;
    project::VideoAttachmentSettings attachment() const;
    void updateAttachment(const project::VideoAttachmentSettings& attachment);
    void listenCurrentProject();
    void listenPlaybackController();

    int m_scorePlaybackPositionMs = 0;
};
}

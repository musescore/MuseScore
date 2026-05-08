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

#include "videopanelmodel.h"

#include <algorithm>
#include <cmath>

using namespace mu::playback;
using namespace mu::project;

VideoPanelModel::VideoPanelModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void VideoPanelModel::load()
{
    listenPlaybackController();

    context()->currentProjectChanged().onNotify(this, [this]() {
        listenCurrentProject();
        emit videoSettingsChanged();
    });

    listenCurrentProject();
    emit videoSettingsChanged();
}

void VideoPanelModel::clearVideo()
{
    IProjectVideoSettingsPtr settings = videoSettings();
    if (!settings) {
        return;
    }

    settings->clearAttachment();
}

void VideoPanelModel::nudgeOffset(int deltaMs)
{
    setOffsetMs(offsetMs() + deltaMs);
}

bool VideoPanelModel::hasVideo() const
{
    return attachment().isValid();
}

QString VideoPanelModel::videoPath() const
{
    return attachment().path.toQString();
}

QUrl VideoPanelModel::videoUrl() const
{
    const QString path = videoPath();
    return path.isEmpty() ? QUrl() : QUrl::fromLocalFile(path);
}

void VideoPanelModel::setVideoPath(const QString& path)
{
    IProjectVideoSettingsPtr settings = videoSettings();
    if (!settings) {
        return;
    }

    if (path.isEmpty()) {
        settings->clearAttachment();
        return;
    }

    VideoAttachmentSettings updated = attachment();
    updated.path = path;
    updateAttachment(updated);
}

int VideoPanelModel::offsetMs() const
{
    return attachment().offsetMs;
}

void VideoPanelModel::setOffsetMs(int offsetMs)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid() || updated.offsetMs == offsetMs) {
        return;
    }

    updated.offsetMs = offsetMs;
    updateAttachment(updated);
}

int VideoPanelModel::volumePercent() const
{
    return std::clamp(static_cast<int>(attachment().volume * 100.f + 0.5f), 0, 100);
}

void VideoPanelModel::setVolumePercent(int volumePercent)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid()) {
        return;
    }

    volumePercent = std::clamp(volumePercent, 0, 100);
    const float volume = static_cast<float>(volumePercent) / 100.f;
    if (updated.volume == volume) {
        return;
    }

    updated.volume = volume;
    updateAttachment(updated);
}

bool VideoPanelModel::muted() const
{
    return attachment().muted;
}

void VideoPanelModel::setMuted(bool muted)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid() || updated.muted == muted) {
        return;
    }

    updated.muted = muted;
    updateAttachment(updated);
}

bool VideoPanelModel::solo() const
{
    return attachment().solo;
}

void VideoPanelModel::setSolo(bool solo)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid() || updated.solo == solo) {
        return;
    }

    updated.solo = solo;
    if (solo) {
        updated.muted = false;
    }
    updateAttachment(updated);
}

bool VideoPanelModel::scorePlaying() const
{
    return playbackController()->isPlaying();
}

int VideoPanelModel::scorePlaybackPositionMs() const
{
    return m_scorePlaybackPositionMs;
}

IProjectVideoSettingsPtr VideoPanelModel::videoSettings() const
{
    INotationProjectPtr project = context()->currentProject();
    return project ? project->videoSettings() : nullptr;
}

VideoAttachmentSettings VideoPanelModel::attachment() const
{
    IProjectVideoSettingsPtr settings = videoSettings();
    return settings ? settings->attachment() : VideoAttachmentSettings();
}

void VideoPanelModel::updateAttachment(const VideoAttachmentSettings& attachment)
{
    IProjectVideoSettingsPtr settings = videoSettings();
    if (!settings) {
        return;
    }

    settings->setAttachment(attachment);
}

void VideoPanelModel::listenCurrentProject()
{
    IProjectVideoSettingsPtr settings = videoSettings();
    if (!settings) {
        return;
    }

    settings->settingsChanged().onNotify(this, [this]() {
        emit videoSettingsChanged();
    }, Asyncable::Mode::SetReplace);
}

void VideoPanelModel::listenPlaybackController()
{
    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        emit playbackSyncChanged();
    });

    playbackController()->currentPlaybackPositionChanged().onReceive(this, [this](muse::audio::secs_t pos, muse::midi::tick_t) {
        const int positionMs = std::max(0, static_cast<int>(std::lround(pos * 1000.0)));
        if (m_scorePlaybackPositionMs == positionMs) {
            return;
        }

        m_scorePlaybackPositionMs = positionMs;
        emit playbackSyncChanged();
    });
}

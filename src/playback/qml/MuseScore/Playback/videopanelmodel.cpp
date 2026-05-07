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

using namespace mu::playback;
using namespace mu::project;

VideoPanelModel::VideoPanelModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void VideoPanelModel::load()
{
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

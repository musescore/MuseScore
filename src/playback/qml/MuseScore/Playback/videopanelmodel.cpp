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

#include <QStringList>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/types/constants.h"

using namespace mu::playback;
using namespace mu::project;

VideoPanelModel::VideoPanelModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void VideoPanelModel::load()
{
    listenPlaybackState();

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

void VideoPanelModel::addHitPoint(int videoPositionMs)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid()) {
        return;
    }

    videoPositionMs = std::max(0, videoPositionMs);

    VideoHitPointSettings hitPoint;
    hitPoint.timeMs = videoPositionMs;
    hitPoint.label = muse::String(u"Hit %1").arg(static_cast<int>(updated.hitPoints.size()) + 1);
    updated.hitPoints.push_back(hitPoint);

    std::sort(updated.hitPoints.begin(), updated.hitPoints.end(), [](const VideoHitPointSettings& a, const VideoHitPointSettings& b) {
        return a.timeMs < b.timeMs;
    });

    updateAttachment(updated);
}

void VideoPanelModel::removeHitPoint(int index)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid() || index < 0 || index >= static_cast<int>(updated.hitPoints.size())) {
        return;
    }

    updated.hitPoints.erase(updated.hitPoints.begin() + index);
    updateAttachment(updated);
}

void VideoPanelModel::renameHitPoint(int index, const QString& label)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid() || index < 0 || index >= static_cast<int>(updated.hitPoints.size())) {
        return;
    }

    const QString trimmedLabel = label.trimmed();
    const muse::String newLabel = trimmedLabel.isEmpty()
                                  ? muse::String(u"Hit %1").arg(index + 1)
                                  : muse::String::fromQString(trimmedLabel);
    if (updated.hitPoints.at(index).label == newLabel) {
        return;
    }

    updated.hitPoints[index].label = newLabel;
    updateAttachment(updated);
}

void VideoPanelModel::setHitPointTimecode(int index, const QString& timecode)
{
    const int positionMs = parseTimecodeToMs(timecode);
    if (positionMs < 0) {
        return;
    }

    setHitPointTimeMs(index, positionMs);
}

void VideoPanelModel::setHitPointTimeMs(int index, int videoPositionMs)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid() || index < 0 || index >= static_cast<int>(updated.hitPoints.size())) {
        return;
    }

    const int positionMs = std::max(0, videoPositionMs);
    if (updated.hitPoints.at(index).timeMs == positionMs) {
        return;
    }

    updated.hitPoints[index].timeMs = positionMs;
    std::sort(updated.hitPoints.begin(), updated.hitPoints.end(), [](const VideoHitPointSettings& a, const VideoHitPointSettings& b) {
        return a.timeMs < b.timeMs;
    });

    updateAttachment(updated);
}

QString VideoPanelModel::formatTimecode(int videoPositionMs) const
{
    videoPositionMs = std::max(0, videoPositionMs);
    const double framesPerSecond = std::clamp(frameRate(), 1.0, 240.0);
    const int roundedFrameRate = std::max(1, static_cast<int>(std::lround(framesPerSecond)));
    const qint64 totalFrames = static_cast<qint64>(std::floor((videoPositionMs / 1000.0) * roundedFrameRate + 0.5));

    const qint64 frames = totalFrames % roundedFrameRate;
    const qint64 totalSeconds = totalFrames / roundedFrameRate;
    const qint64 seconds = totalSeconds % 60;
    const qint64 minutes = (totalSeconds / 60) % 60;
    const qint64 hours = totalSeconds / 3600;

    return QString("%1:%2:%3:%4")
           .arg(hours, 2, 10, QLatin1Char('0'))
           .arg(minutes, 2, 10, QLatin1Char('0'))
           .arg(seconds, 2, 10, QLatin1Char('0'))
           .arg(frames, 2, 10, QLatin1Char('0'));
}

int VideoPanelModel::parseTimecodeToMs(const QString& timecode) const
{
    const QStringList parts = timecode.trimmed().split(QLatin1Char(':'));
    if (parts.size() != 4) {
        return -1;
    }

    bool ok = false;
    const int hours = parts.at(0).toInt(&ok);
    if (!ok || hours < 0) {
        return -1;
    }

    const int minutes = parts.at(1).toInt(&ok);
    if (!ok || minutes < 0 || minutes > 59) {
        return -1;
    }

    const int seconds = parts.at(2).toInt(&ok);
    if (!ok || seconds < 0 || seconds > 59) {
        return -1;
    }

    const double framesPerSecond = std::clamp(frameRate(), 1.0, 240.0);
    const int roundedFrameRate = std::max(1, static_cast<int>(std::lround(framesPerSecond)));
    const int frames = parts.at(3).toInt(&ok);
    if (!ok || frames < 0 || frames >= roundedFrameRate) {
        return -1;
    }

    const qint64 totalSeconds = static_cast<qint64>(hours) * 3600 + minutes * 60 + seconds;
    const qint64 totalFrames = totalSeconds * roundedFrameRate + frames;
    return static_cast<int>(std::floor((static_cast<double>(totalFrames) * 1000.0 / roundedFrameRate) + 0.5));
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

int VideoPanelModel::balance() const
{
    return std::clamp(static_cast<int>(attachment().balance * 100.f + (attachment().balance >= 0.f ? 0.5f : -0.5f)), -100, 100);
}

void VideoPanelModel::setBalance(int balance)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid()) {
        return;
    }

    balance = std::clamp(balance, -100, 100);
    const float scaledBalance = static_cast<float>(balance) / 100.f;
    if (updated.balance == scaledBalance) {
        return;
    }

    updated.balance = scaledBalance;
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

double VideoPanelModel::frameRate() const
{
    return std::clamp(attachment().frameRate, 1.0, 240.0);
}

void VideoPanelModel::setFrameRate(double frameRate)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid()) {
        return;
    }

    frameRate = std::clamp(frameRate, 1.0, 240.0);
    if (std::abs(updated.frameRate - frameRate) < 0.001) {
        return;
    }

    updated.frameRate = frameRate;
    updateAttachment(updated);
}

int VideoPanelModel::timecodeDisplayMode() const
{
    return static_cast<int>(attachment().timecodeDisplayMode);
}

void VideoPanelModel::setTimecodeDisplayMode(int mode)
{
    VideoAttachmentSettings updated = attachment();
    if (!updated.isValid()) {
        return;
    }

    mode = std::clamp(mode, static_cast<int>(VideoTimecodeDisplayMode::Off), static_cast<int>(VideoTimecodeDisplayMode::BelowBars));
    const VideoTimecodeDisplayMode displayMode = static_cast<VideoTimecodeDisplayMode>(mode);
    if (updated.timecodeDisplayMode == displayMode) {
        return;
    }

    updated.timecodeDisplayMode = displayMode;
    updateAttachment(updated);
}

QVariantList VideoPanelModel::hitPoints() const
{
    QVariantList result;
    for (const VideoHitPointSettings& hitPoint : attachment().hitPoints) {
        result.push_back(hitPointToMap(hitPoint));
    }
    return result;
}

bool VideoPanelModel::scorePlaying() const
{
    context::IPlaybackStatePtr playbackState = context()->playbackState();
    return playbackState && playbackState->playbackStatus() == muse::audio::PlaybackStatus::Running;
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

QVariantMap VideoPanelModel::hitPointToMap(const VideoHitPointSettings& hitPoint) const
{
    QVariantMap result;
    result["label"] = hitPoint.label.toQString();
    result["timeMs"] = hitPoint.timeMs;
    result["timecode"] = formatTimecode(hitPoint.timeMs);
    result["musicalPosition"] = musicalPositionText(hitPoint.timeMs);
    result["color"] = hitPoint.color;
    return result;
}

QString VideoPanelModel::musicalPositionText(int videoPositionMs) const
{
    INotationProjectPtr project = context()->currentProject();
    if (!project || !project->masterNotation() || !project->masterNotation()->masterScore()) {
        return QString();
    }

    auto* score = project->masterNotation()->masterScore();
    const double scoreTimeSeconds = std::max(0.0, static_cast<double>(videoPositionMs - offsetMs()) / 1000.0);
    const int tick = std::max(0, score->utime2utick(scoreTimeSeconds));
    const engraving::Measure* measure = score->tick2measure(engraving::Fraction::fromTicks(tick));
    if (!measure) {
        return QString();
    }

    const int beatTicks = engraving::Constants::DIVISION;
    const int beat = std::max(1, ((tick - measure->tick().ticks()) / beatTicks) + 1);
    return QString("%1.%2").arg(measure->measureNumber() + 1).arg(beat);
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

void VideoPanelModel::listenPlaybackState()
{
    context::IPlaybackStatePtr playbackState = context()->playbackState();
    if (!playbackState) {
        return;
    }

    const int initialPositionMs = std::max(0, static_cast<int>(std::lround(playbackState->playbackPosition() * 1000.0)));
    if (m_scorePlaybackPositionMs != initialPositionMs) {
        m_scorePlaybackPositionMs = initialPositionMs;
    }

    playbackState->playbackStatusChanged().onReceive(this, [this](muse::audio::PlaybackStatus) {
        emit playbackSyncChanged();
    });

    playbackState->playbackPositionChanged().onReceive(this, [this](muse::audio::secs_t pos) {
        const int positionMs = std::max(0, static_cast<int>(std::lround(pos * 1000.0)));
        if (m_scorePlaybackPositionMs == positionMs) {
            return;
        }

        m_scorePlaybackPositionMs = positionMs;
        emit playbackSyncChanged();
    });
}

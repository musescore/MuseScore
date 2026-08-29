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
#include <limits>

#include <QStringList>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/utils.h"

#include "notation/inotation.h"
#include "project/iprojectvideosettings.h"

using namespace mu::playback;
using namespace mu::project;

VideoPanelModel::VideoPanelModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

VideoPanelModel::~VideoPanelModel() = default;

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

    // NOTE: derive the default label from the id the new hit point is about to
    // be assigned (see ProjectVideoSettings::normalizeHitPoints()), not from
    // the current list size -- size is reused after a deletion (e.g. delete
    // "#2" out of "#1"/"#2"/"#3", add a new one: size()+1 is "#3" again,
    // duplicating the still-present "#3"), whereas ids only ever increase and
    // are never reused.
    int nextId = 0;
    for (const VideoHitPointSettings& existing : updated.hitPoints) {
        nextId = std::max(nextId, existing.id);
    }

    VideoHitPointSettings hitPoint;
    hitPoint.timeMs = videoPositionMs;
    hitPoint.label = muse::String(u"#%1").arg(nextId + 1);
    updated.hitPoints.push_back(hitPoint);

    std::sort(updated.hitPoints.begin(), updated.hitPoints.end(), [](const VideoHitPointSettings& a, const VideoHitPointSettings& b) {
        return a.timeMs < b.timeMs;
    });

    updateAttachment(updated);
}

int VideoPanelModel::indexOfHitPoint(const VideoAttachmentSettings& attachment, int hitPointId)
{
    for (int i = 0; i < static_cast<int>(attachment.hitPoints.size()); ++i) {
        if (attachment.hitPoints.at(i).id == hitPointId) {
            return i;
        }
    }

    return -1;
}

void VideoPanelModel::withHitPoint(int hitPointId, const std::function<void(VideoAttachmentSettings&, int)>& mutate)
{
    VideoAttachmentSettings updated = attachment();
    const int index = indexOfHitPoint(updated, hitPointId);
    if (!updated.isValid() || index < 0) {
        return;
    }

    mutate(updated, index);

    // NOTE: updateAttachment()/ProjectVideoSettings::setAttachment() already
    // no-ops (and skips the settingsChanged notify) if the result is
    // identical to what's already stored, and already re-sorts by timeMs via
    // normalizeHitPoints() -- callers don't need to duplicate either check.
    updateAttachment(updated);
}

void VideoPanelModel::removeHitPoint(int hitPointId)
{
    withHitPoint(hitPointId, [](VideoAttachmentSettings& updated, int index) {
        updated.hitPoints.erase(updated.hitPoints.begin() + index);
    });
}

void VideoPanelModel::renameHitPoint(int hitPointId, const QString& label)
{
    withHitPoint(hitPointId, [&label](VideoAttachmentSettings& updated, int index) {
        // NOTE: fall back to the hit point's own (stable, unique, never-reused)
        // id rather than its time-sorted list position -- the position can
        // collide with another hit point's existing label (e.g. clearing the
        // label of the hit point at sorted position 3 produced "Hit 3", which
        // could already be some other hit point's default/user-set label).
        const QString trimmedLabel = label.trimmed();
        updated.hitPoints[index].label = trimmedLabel.isEmpty()
                                         ? muse::String(u"Hit %1").arg(updated.hitPoints.at(index).id)
                                         : muse::String::fromQString(trimmedLabel);
    });
}

void VideoPanelModel::setHitPointTimecode(int hitPointId, const QString& timecode)
{
    const int positionMs = parseTimecodeToMs(timecode);
    if (positionMs < 0) {
        return;
    }

    setHitPointTimeMs(hitPointId, positionMs);
}

void VideoPanelModel::setHitPointTimeMs(int hitPointId, int videoPositionMs)
{
    withHitPoint(hitPointId, [videoPositionMs](VideoAttachmentSettings& updated, int index) {
        updated.hitPoints[index].timeMs = std::max(0, videoPositionMs);
    });
}

QString VideoPanelModel::formatTimecode(int videoPositionMs) const
{
    return project::formatVideoTimecode(videoPositionMs, frameRate());
}

void VideoPanelModel::toggleScorePlay()
{
    if (playbackController()) {
        playbackController()->togglePlay();
    }
}

void VideoPanelModel::stopScorePlayback()
{
    if (playbackController()) {
        playbackController()->stop();
    }
}

void VideoPanelModel::toggleLoopPlayback()
{
    if (playbackController()) {
        playbackController()->toggleLoopPlayback();
    }
}

void VideoPanelModel::seekScoreToVideoPositionMs(int videoPositionMs)
{
    if (!playbackController()) {
        return;
    }

    const double scoreTimeSeconds = std::max(0.0, static_cast<double>(videoPositionMs - offsetMs()) / 1000.0);
    playbackController()->rewind(muse::secs_t(scoreTimeSeconds));
}

int VideoPanelModel::hitPointsPanelWidth() const
{
    return configuration()->videoHitPointsPanelWidth();
}

void VideoPanelModel::setHitPointsPanelWidth(int width)
{
    configuration()->setVideoHitPointsPanelWidth(width);
}

bool VideoPanelModel::hitPointsPanelVisible() const
{
    return configuration()->videoHitPointsPanelVisible();
}

void VideoPanelModel::setHitPointsPanelVisible(bool visible)
{
    configuration()->setVideoHitPointsPanelVisible(visible);
}

QStringList VideoPanelModel::recentVideoFiles() const
{
    return configuration()->recentVideoFiles();
}

void VideoPanelModel::clearRecentVideoFiles()
{
    configuration()->clearRecentVideoFiles();
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
    const qint64 positionMs = static_cast<qint64>(std::floor((static_cast<double>(totalFrames) * 1000.0 / roundedFrameRate) + 0.5));
    if (positionMs > std::numeric_limits<int>::max()) {
        return -1;
    }

    return static_cast<int>(positionMs);
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
    //! NOTE Hit points are timed against the PREVIOUS video's own footage --
    //! loading or reloading a video (even the same file, e.g. after re-editing
    //! it) invalidates them, so start over rather than carrying stale ones.
    updated.hitPoints.clear();
    updateAttachment(updated);

    configuration()->addRecentVideoFile(path);
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

    //! NOTE loopStartMs/loopEndMs/scoreEndVideoPositionMs are all derived from
    //! offsetMs via tickToVideoPositionMs(), but updateAttachment() only
    //! re-emits videoSettingsChanged()/hitPointsChanged() -- explicitly notify
    //! these offset-derived properties so QML bindings actually recompute.
    emit loopChanged();
    emit scoreContentChanged();
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

bool VideoPanelModel::loopEnabled() const
{
    return playbackController() && playbackController()->isLoopEnabled();
}

int VideoPanelModel::loopStartMs() const
{
    notation::INotationPlaybackPtr playback = notationPlayback();
    if (!playback) {
        return 0;
    }

    const notation::LoopBoundaries& bounds = playback->loopBoundaries();
    if (bounds.isNull()) {
        return 0;
    }

    const int videoPositionMs = tickToVideoPositionMs(bounds.loopInTick.ticks());
    return videoPositionMs < 0 ? 0 : videoPositionMs;
}

int VideoPanelModel::loopEndMs() const
{
    notation::INotationPlaybackPtr playback = notationPlayback();
    if (!playback) {
        return 0;
    }

    const notation::LoopBoundaries& bounds = playback->loopBoundaries();
    if (bounds.isNull()) {
        return 0;
    }

    const int videoPositionMs = tickToVideoPositionMs(bounds.loopOutTick.ticks());
    return videoPositionMs < 0 ? 0 : videoPositionMs;
}

int VideoPanelModel::tickToVideoPositionMs(int tick) const
{
    INotationProjectPtr project = context()->currentProject();
    if (!project || !project->masterNotation() || !project->masterNotation()->masterScore()) {
        return -1;
    }

    return mu::project::videoPositionMsForTick(project->masterNotation()->masterScore(), tick, offsetMs());
}

notation::INotationPlaybackPtr VideoPanelModel::notationPlayback() const
{
    INotationProjectPtr project = context()->currentProject();
    if (!project || !project->masterNotation()) {
        return nullptr;
    }

    return project->masterNotation()->playback();
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
    result["id"] = hitPoint.id;
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
    if (!score->checkHasMeasures()) {
        return QString();
    }

    const double scoreTimeSeconds = std::max(0.0, static_cast<double>(videoPositionMs - offsetMs()) / 1000.0);
    const int tick = std::max(0, score->utime2utick(scoreTimeSeconds));

    //! NOTE findBeat() derives the beat length from the actual time signature
    //! in effect at this tick, unlike a fixed Constants::DIVISION (quarter
    //! note) which is wrong in meters like 6/8 or 2/2.
    const engraving::MeasureBeat measureBeat = engraving::findBeat(score, tick);
    const int beat = std::max(1, static_cast<int>(measureBeat.beat) + 1);
    return QString("%1.%2").arg(measureBeat.measureIndex + 1).arg(beat);
}

QVariantList VideoPanelModel::measurePositions() const
{
    QVariantList result;

    INotationProjectPtr project = context()->currentProject();
    if (!project || !project->masterNotation() || !project->masterNotation()->masterScore()) {
        return result;
    }

    auto* score = project->masterNotation()->masterScore();
    for (const engraving::Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        const int videoPositionMs = tickToVideoPositionMs(measure->tick().ticks());
        if (videoPositionMs < 0) {
            continue;
        }

        QVariantMap entry;
        entry["measureNumber"] = measure->measureNumber() + 1;
        entry["videoPositionMs"] = videoPositionMs;
        result.push_back(entry);
    }

    return result;
}

int VideoPanelModel::scoreEndVideoPositionMs() const
{
    INotationProjectPtr project = context()->currentProject();
    if (!project || !project->masterNotation() || !project->masterNotation()->masterScore()) {
        return -1;
    }

    const engraving::Measure* lastMeasure = project->masterNotation()->masterScore()->lastMeasure();
    if (!lastMeasure) {
        return -1;
    }

    return tickToVideoPositionMs(lastMeasure->endTick().ticks());
}

void VideoPanelModel::listenCurrentProject()
{
    if (IProjectVideoSettingsPtr settings = videoSettings()) {
        settings->settingsChanged().onNotify(this, [this]() {
            emit videoSettingsChanged();

            const std::vector<VideoHitPointSettings>& hitPoints = attachment().hitPoints;
            if (hitPoints != m_lastHitPoints) {
                m_lastHitPoints = hitPoints;
                emit hitPointsChanged();
            }
        }, Asyncable::Mode::SetReplace);
    }

    if (notation::INotationPlaybackPtr playback = notationPlayback()) {
        playback->loopBoundariesChanged().onNotify(this, [this]() {
            emit loopChanged();
        }, Asyncable::Mode::SetReplace);
    }

    if (notation::INotationPtr notation = context()->currentNotation()) {
        //! NOTE Measures added/removed (or otherwise re-laid-out) changes
        //! scoreEndVideoPositionMs and every entry from measurePositions() --
        //! this is what keeps the timeline's measure labels and "past the end
        //! of the score" shading correct as the score is edited.
        //!
        //! notationChanged() fires on essentially every undoable edit (a note's
        //! pitch, text, dynamics...), not just ones that actually move measure
        //! boundaries -- re-emitting scoreContentChanged() on all of them forces
        //! measurePositions() (an O(measure count) walk) and a timeline repaint
        //! for edits that don't touch measure positions at all. Only re-emit
        //! when the cheap measure-count/last-measure-end proxy actually changed.
        notation->notationChanged().onReceive(this, [this](const muse::RectF&) {
            INotationProjectPtr project = context()->currentProject();
            const engraving::Score* score = (project && project->masterNotation()) ? project->masterNotation()->masterScore() : nullptr;
            const size_t measureCount = score ? score->nmeasures() : 0;
            const int scoreEndTick = (score && score->lastMeasure()) ? score->lastMeasure()->endTick().ticks() : -1;

            if (measureCount == m_lastMeasureCount && scoreEndTick == m_lastScoreEndTick) {
                return;
            }

            m_lastMeasureCount = measureCount;
            m_lastScoreEndTick = scoreEndTick;
            emit scoreContentChanged();
        }, Asyncable::Mode::SetReplace);
    }
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

    if (playbackController()) {
        playbackController()->loopEnabledChanged().onReceive(this, [this](bool) {
            emit loopChanged();
        });
    }

    playbackState->playbackPositionChanged().onReceive(this, [this](muse::audio::secs_t pos) {
        const int positionMs = std::max(0, static_cast<int>(std::lround(pos * 1000.0)));
        if (m_scorePlaybackPositionMs == positionMs) {
            return;
        }

        m_scorePlaybackPositionMs = positionMs;
        emit playbackSyncChanged();
    });
}

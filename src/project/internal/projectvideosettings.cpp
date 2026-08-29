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

#include "projectvideosettings.h"

#include <algorithm>
#include <cmath>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#include "engraving/dom/score.h"
#include "types/bytearray.h"

using namespace mu::project;
using namespace muse;

static constexpr int VIDEO_SETTINGS_VERSION = 1;

QString mu::project::formatVideoTimecode(int videoPositionMs, double frameRate)
{
    //! NOTE Non-drop-frame SMPTE timecode: frame footage shot at a fractional
    //! rate (23.976, 29.97, 59.94fps...) is still labeled against its nearest
    //! integer nominal rate (24/30/60), so the ROUNDED rate is what's used
    //! below as the frames-per-second convention for the HH:MM:SS:FF display
    //! (frames/seconds rollover). But which real video FRAME is showing at a
    //! given elapsed time is a physical fact of the ACTUAL (fractional) rate --
    //! using the rounded rate for that too (as before) made the displayed
    //! frame number silently drift from the real one over long durations.
    //! This does NOT implement drop-frame compensation (periodic frame-number
    //! skipping to keep the displayed clock matching wall time at 29.97/
    //! 59.94fps) -- that's a separate, larger feature; without it, the
    //! displayed timecode is expected to drift from wall-clock time over long
    //! footage at those rates, same as any other NDF timecode display.
    const double clampedFrameRate = std::clamp(frameRate, 1.0, 240.0);
    const int roundedFrameRate = std::max(1, static_cast<int>(std::lround(clampedFrameRate)));
    const qint64 totalFrames = static_cast<qint64>(std::floor((std::max(0, videoPositionMs) / 1000.0) * clampedFrameRate + 0.5));

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

int mu::project::videoPositionMsForTick(const mu::engraving::Score* score, int tick, int offsetMs)
{
    if (!score) {
        return -1;
    }

    const double timeSeconds = score->utick2utime(tick);
    return std::max(0, static_cast<int>(std::lround(timeSeconds * 1000.0)) + offsetMs);
}

void mu::project::updateVideoAttachment(const IProjectVideoSettingsPtr& settings,
                                        const std::function<void(VideoAttachmentSettings&)>& mutate)
{
    if (!settings || !settings->attachment().isValid()) {
        return;
    }

    VideoAttachmentSettings updated = settings->attachment();
    mutate(updated);
    settings->setAttachment(updated);
}

static void normalizeHitPoints(VideoAttachmentSettings& attachment)
{
    std::stable_sort(
        attachment.hitPoints.begin(), attachment.hitPoints.end(),
        [](const VideoHitPointSettings& a, const VideoHitPointSettings& b) {
        return a.timeMs < b.timeMs;
    });

    int nextId = 0;
    for (const VideoHitPointSettings& hitPoint : attachment.hitPoints) {
        nextId = std::max(nextId, hitPoint.id);
    }
    for (VideoHitPointSettings& hitPoint : attachment.hitPoints) {
        if (hitPoint.id == 0) {
            hitPoint.id = ++nextId;
        }
    }
}

const VideoAttachmentSettings& ProjectVideoSettings::attachment() const
{
    return m_attachment;
}

void ProjectVideoSettings::setAttachment(const VideoAttachmentSettings& attachment)
{
    VideoAttachmentSettings normalized = attachment;
    normalizeHitPoints(normalized);

    if (m_attachment == normalized) {
        return;
    }

    m_attachment = normalized;
    m_settingsChanged.notify();
}

void ProjectVideoSettings::clearAttachment()
{
    setAttachment(VideoAttachmentSettings());
}

muse::async::Notification ProjectVideoSettings::settingsChanged() const
{
    return m_settingsChanged;
}

muse::Ret ProjectVideoSettings::read(const engraving::MscReader& reader)
{
    ByteArray json = reader.readVideoSettingsJsonFile();
    if (json.empty()) {
        makeDefault();
        return make_ret(Ret::Code::Ok);
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json.toQByteArrayNoCopy(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        makeDefault();
        return make_ret(Ret::Code::Ok);
    }

    QJsonObject rootObj = document.object();
    const int version = rootObj.value("version").toInt(0);
    const QJsonValue attachmentValue = rootObj.value("attachment");
    if (version != VIDEO_SETTINGS_VERSION || !attachmentValue.isObject()) {
        makeDefault();
        return make_ret(Ret::Code::Ok);
    }

    m_attachment = attachmentFromJson(attachmentValue.toObject());

    return make_ret(Ret::Code::Ok);
}

muse::Ret ProjectVideoSettings::write(engraving::MscWriter& writer) const
{
    if (!m_attachment.isValid()) {
        return make_ret(Ret::Code::Ok);
    }

    QJsonObject rootObj;
    rootObj["version"] = VIDEO_SETTINGS_VERSION;
    rootObj["attachment"] = attachmentToJson(m_attachment);

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeVideoSettingsJsonFile(ByteArray::fromQByteArrayNoCopy(json));

    return make_ret(Ret::Code::Ok);
}

void ProjectVideoSettings::makeDefault()
{
    m_attachment = VideoAttachmentSettings();
}

VideoHitPointSettings ProjectVideoSettings::hitPointFromJson(const QJsonObject& object) const
{
    VideoHitPointSettings result;
    //! NOTE Hit points saved before the "id" field existed read back as id 0;
    //! normalizeHitPoints() backfills a real id for those on load.
    result.id = object.value("id").toInt(0);
    result.label = object.value("label").toString();
    result.timeMs = std::max(0, object.value("timeMs").toInt());
    result.color = object.value("color").toInt(0x3B94E5);
    return result;
}

QJsonObject ProjectVideoSettings::hitPointToJson(const VideoHitPointSettings& hitPoint) const
{
    QJsonObject object;
    object["id"] = hitPoint.id;
    object["label"] = hitPoint.label.toQString();
    object["timeMs"] = hitPoint.timeMs;
    object["color"] = hitPoint.color;
    return object;
}

VideoAttachmentSettings ProjectVideoSettings::attachmentFromJson(const QJsonObject& object) const
{
    VideoAttachmentSettings result;
    result.path = object.value("path").toString();
    result.offsetMs = object.value("offsetMs").toInt();
    result.volume = static_cast<float>(object.value("volume").toDouble(1.0));
    result.balance = static_cast<float>(object.value("balance").toDouble(0.0));
    result.muted = object.value("muted").toBool(false);
    result.solo = object.value("solo").toBool(false);
    result.frameRate = std::clamp(object.value("frameRate").toDouble(24.0), 1.0, 240.0);
    result.timecodeDisplayMode = static_cast<VideoTimecodeDisplayMode>(
        std::clamp(object.value("timecodeDisplayMode").toInt(static_cast<int>(VideoTimecodeDisplayMode::Off)),
                   static_cast<int>(VideoTimecodeDisplayMode::Off),
                   static_cast<int>(VideoTimecodeDisplayMode::BelowBars)));
    result.showHitPoints = object.value("showHitPoints").toBool(true);

    const QJsonArray hitPoints = object.value("hitPoints").toArray();
    result.hitPoints.reserve(static_cast<size_t>(hitPoints.size()));
    for (const QJsonValue& hitPointValue : hitPoints) {
        if (hitPointValue.isObject()) {
            result.hitPoints.push_back(hitPointFromJson(hitPointValue.toObject()));
        }
    }

    normalizeHitPoints(result);

    return result;
}

QJsonObject ProjectVideoSettings::attachmentToJson(const VideoAttachmentSettings& attachment) const
{
    QJsonObject object;
    object["path"] = attachment.path.toQString();
    object["offsetMs"] = attachment.offsetMs;
    object["volume"] = attachment.volume;
    object["balance"] = attachment.balance;
    object["muted"] = attachment.muted;
    object["solo"] = attachment.solo;
    object["frameRate"] = attachment.frameRate;
    object["timecodeDisplayMode"] = static_cast<int>(attachment.timecodeDisplayMode);
    object["showHitPoints"] = attachment.showHitPoints;

    QJsonArray hitPoints;
    for (const VideoHitPointSettings& hitPoint : attachment.hitPoints) {
        hitPoints.append(hitPointToJson(hitPoint));
    }
    object["hitPoints"] = hitPoints;

    return object;
}

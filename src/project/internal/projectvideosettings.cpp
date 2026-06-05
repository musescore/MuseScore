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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#include "types/bytearray.h"

using namespace mu::project;
using namespace muse;

static constexpr int VIDEO_SETTINGS_VERSION = 1;

static void normalizeHitPoints(VideoAttachmentSettings& attachment)
{
    std::stable_sort(
        attachment.hitPoints.begin(), attachment.hitPoints.end(),
        [](const VideoHitPointSettings& a, const VideoHitPointSettings& b) {
        return a.timeMs < b.timeMs;
    });
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
    result.label = object.value("label").toString();
    result.timeMs = std::max(0, object.value("timeMs").toInt());
    result.color = object.value("color").toInt(0x3B94E5);
    return result;
}

QJsonObject ProjectVideoSettings::hitPointToJson(const VideoHitPointSettings& hitPoint) const
{
    QJsonObject object;
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

    QJsonArray hitPoints;
    for (const VideoHitPointSettings& hitPoint : attachment.hitPoints) {
        hitPoints.append(hitPointToJson(hitPoint));
    }
    object["hitPoints"] = hitPoints;

    return object;
}

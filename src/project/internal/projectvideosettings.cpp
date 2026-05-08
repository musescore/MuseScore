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

#include <QJsonDocument>
#include <QJsonObject>

#include "types/bytearray.h"

using namespace mu::project;
using namespace muse;

static constexpr int VIDEO_SETTINGS_VERSION = 1;

const VideoAttachmentSettings& ProjectVideoSettings::attachment() const
{
    return m_attachment;
}

void ProjectVideoSettings::setAttachment(const VideoAttachmentSettings& attachment)
{
    if (m_attachment == attachment) {
        return;
    }

    m_attachment = attachment;
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

    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();
    m_attachment = attachmentFromJson(rootObj.value("attachment").toObject());

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

VideoAttachmentSettings ProjectVideoSettings::attachmentFromJson(const QJsonObject& object) const
{
    VideoAttachmentSettings result;
    result.path = object.value("path").toString();
    result.offsetMs = object.value("offsetMs").toInt();
    result.volume = static_cast<float>(object.value("volume").toDouble(1.0));
    result.balance = static_cast<float>(object.value("balance").toDouble(0.0));
    result.muted = object.value("muted").toBool(false);
    result.solo = object.value("solo").toBool(false);
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
    return object;
}

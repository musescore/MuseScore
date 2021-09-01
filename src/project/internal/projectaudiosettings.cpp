/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "projectaudiosettings.h"

#include <map>
#include <QJsonDocument>
#include <QJsonObject>

using namespace mu::project;
using namespace mu::audio;

static const std::map<AudioSourceType, QString> SOURCE_TYPE_MAP = {
    { AudioSourceType::Undefined, "undefined" },
    { AudioSourceType::Fluid, "fluid" },
    { AudioSourceType::Vsti, "vsti" }
};

AudioOutputParams ProjectAudioSettings::masterAudioOutputParams() const
{
    return m_masterOutputParams;
}

void ProjectAudioSettings::setMasterAudioOutputParams(const AudioOutputParams& params)
{
    m_masterOutputParams = params;
}

mu::Ret ProjectAudioSettings::read(const engraving::MscReader& reader)
{
    QByteArray json = reader.readAudioSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json).object();

    QJsonObject masterObj = rootObj.value("master").toObject();
    m_masterOutputParams = outputParamsFromJson(masterObj);

    return make_ret(Ret::Code::Ok);
}

mu::Ret ProjectAudioSettings::write(engraving::MscWriter& writer)
{
    QJsonObject rootObj;
    rootObj["master"] = outputParamsToJson(m_masterOutputParams);

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeAudioSettingsJsonFile(json);

    return make_ret(Ret::Code::Ok);
}

void ProjectAudioSettings::makeDefault()
{
    //TODO initialize default audio input params
}

AudioInputParams ProjectAudioSettings::inputParamsFromJson(const QJsonObject& object) const
{
    AudioInputParams result;
    result.resourceMeta.id = object.value("resourceId").toString().toStdString();

    return result;
}

AudioOutputParams ProjectAudioSettings::outputParamsFromJson(const QJsonObject& object) const
{
    AudioOutputParams result;
    result.muted = object.value("muted").toBool();
    result.balance = object.value("balance").toVariant().toFloat();
    result.volume = object.value("volumeDb").toVariant().toFloat();

    return result;
}

QJsonObject ProjectAudioSettings::inputParamsToJson(const audio::AudioInputParams& params) const
{
    QJsonObject result;
    result.insert("type", sourceTypeToString(params.type()));
    result.insert("resourceId", QString::fromStdString(params.resourceMeta.id));

    return result;
}

QJsonObject ProjectAudioSettings::outputParamsToJson(const audio::AudioOutputParams& params) const
{
    QJsonObject result;
    result.insert("muted", params.muted);
    result.insert("balance", params.balance);
    result.insert("volumeDb", params.volume);

    return result;
}

AudioSourceType ProjectAudioSettings::sourceTypeFromString(const QString& string) const
{
    for (const auto& pair : SOURCE_TYPE_MAP) {
        if (pair.second == string) {
            return pair.first;
        }
    }

    return AudioSourceType::Undefined;
}

QString ProjectAudioSettings::sourceTypeToString(const audio::AudioSourceType& type) const
{
    auto search = SOURCE_TYPE_MAP.find(type);

    if (search != SOURCE_TYPE_MAP.end()) {
        return search->second;
    }

    return SOURCE_TYPE_MAP.at(AudioSourceType::Undefined);
}

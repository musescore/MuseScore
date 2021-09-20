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
#include <QJsonArray>
#include <QJsonObject>

using namespace mu::project;
using namespace mu::audio;

static const std::map<AudioSourceType, QString> SOURCE_TYPE_MAP = {
    { AudioSourceType::Undefined, "undefined" },
    { AudioSourceType::Fluid, "fluid" },
    { AudioSourceType::Vsti, "vsti" }
};

static const std::map<AudioResourceType, QString> RESOURCE_TYPE_MAP = {
    { AudioResourceType::Undefined, "undefined" },
    { AudioResourceType::FluidSoundfont, "fluid_soundfont" },
    { AudioResourceType::VstPlugin, "vst_plugin" }
};

AudioOutputParams ProjectAudioSettings::masterAudioOutputParams() const
{
    return m_masterOutputParams;
}

void ProjectAudioSettings::setMasterAudioOutputParams(const AudioOutputParams& params)
{
    m_masterOutputParams = params;
}

AudioInputParams ProjectAudioSettings::trackInputParams(const ID& partId) const
{
    auto search = m_trackInputParamsMap.find(partId);

    if (search == m_trackInputParamsMap.end()) {
        return {};
    }

    return search->second;
}

void ProjectAudioSettings::setTrackInputParams(const ID& partId, const audio::AudioInputParams& params)
{
    m_trackInputParamsMap.insert_or_assign(partId, params);
}

AudioOutputParams ProjectAudioSettings::trackOutputParams(const ID& partId) const
{
    auto search = m_trackOutputParamsMap.find(partId);

    if (search == m_trackOutputParamsMap.end()) {
        return {};
    }

    return search->second;
}

void ProjectAudioSettings::setTrackOutputParams(const ID& partId, const audio::AudioOutputParams& params)
{
    m_trackOutputParamsMap.insert_or_assign(partId, params);
}

void ProjectAudioSettings::removeTrackParams(const ID& partId)
{
    auto inSearch = m_trackInputParamsMap.find(partId);
    if (inSearch != m_trackInputParamsMap.end()) {
        m_trackInputParamsMap.erase(inSearch);
    }

    auto outSearch = m_trackOutputParamsMap.find(partId);
    if (outSearch != m_trackOutputParamsMap.end()) {
        m_trackOutputParamsMap.erase(outSearch);
    }
}

mu::Ret ProjectAudioSettings::read(const engraving::MscReader& reader)
{
    QByteArray json = reader.readAudioSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json).object();

    QJsonObject masterObj = rootObj.value("master").toObject();
    m_masterOutputParams = outputParamsFromJson(masterObj);

    QJsonObject tracksObj = rootObj.value("tracks").toObject();

    for (const QString& key : tracksObj.keys()) {
        QJsonObject trackObject = tracksObj.value(key).toObject();

        ID partId(key);

        audio::AudioInputParams inParams = inputParamsFromJson(trackObject.value("in").toObject());
        audio::AudioOutputParams outParams = outputParamsFromJson(trackObject.value("out").toObject());

        m_trackInputParamsMap.emplace(partId, std::move(inParams));
        m_trackOutputParamsMap.emplace(partId, std::move(outParams));
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret ProjectAudioSettings::write(engraving::MscWriter& writer)
{
    QJsonObject rootObj;
    rootObj["master"] = outputParamsToJson(m_masterOutputParams);

    QJsonObject tracksObj;
    for (const auto& pair : m_trackInputParamsMap) {
        tracksObj.insert(pair.first.toQString(), buildTrackObject(pair.first));
    }

    rootObj["tracks"] = tracksObj;

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
    result.resourceMeta = resourceMetaFromJson(object.value("resourceMeta").toObject());

    return result;
}

AudioOutputParams ProjectAudioSettings::outputParamsFromJson(const QJsonObject& object) const
{
    AudioOutputParams result;
    result.fxChain = fxChainFromJson(object.value("fxChain").toObject());
    result.muted = object.value("muted").toBool();
    result.solo = object.value("solo").toBool();
    result.balance = object.value("balance").toVariant().toFloat();
    result.volume = object.value("volumeDb").toVariant().toFloat();

    return result;
}

AudioFxChain ProjectAudioSettings::fxChainFromJson(const QJsonObject& fxChainObject) const
{
    AudioFxChain result;

    for (const QString& key : fxChainObject.keys()) {
        AudioFxParams params = fxParamsFromJson(fxChainObject.value(key).toObject());
        result.emplace(static_cast<AudioFxChainOrder>(key.toInt()), std::move(params));
    }

    return result;
}

AudioFxParams ProjectAudioSettings::fxParamsFromJson(const QJsonObject& object) const
{
    AudioFxParams result;
    result.active = object.value("active").toBool();
    result.chainOrder = static_cast<AudioFxChainOrder>(object.value("chainOrder").toInt());
    result.resourceMeta = resourceMetaFromJson(object.value("resourceMeta").toObject());

    return result;
}

AudioResourceMeta ProjectAudioSettings::resourceMetaFromJson(const QJsonObject& object) const
{
    AudioResourceMeta result;
    result.id = object.value("id").toString().toStdString();
    result.vendor = object.value("vendor").toString().toStdString();
    result.type = resourceTypeFromString(object.value("type").toString());

    return result;
}

QJsonObject ProjectAudioSettings::inputParamsToJson(const audio::AudioInputParams& params) const
{
    QJsonObject result;
    result.insert("resourceMeta", resourceMetaToJson(params.resourceMeta));

    return result;
}

QJsonObject ProjectAudioSettings::outputParamsToJson(const audio::AudioOutputParams& params) const
{
    QJsonObject result;
    result.insert("fxChain", fxChainToJson(params.fxChain));
    result.insert("muted", params.muted);
    result.insert("solo", params.solo);
    result.insert("balance", params.balance);
    result.insert("volumeDb", params.volume);

    return result;
}

QJsonObject ProjectAudioSettings::fxChainToJson(const audio::AudioFxChain& fxChain) const
{
    QJsonObject result;

    for (const auto& pair : fxChain) {
        result.insert(QString::number(static_cast<int>(pair.first)), fxParamsToJson(pair.second));
    }

    return result;
}

QJsonObject ProjectAudioSettings::fxParamsToJson(const audio::AudioFxParams& fxParams) const
{
    QJsonObject result;
    result.insert("active", fxParams.active);
    result.insert("chainOrder", static_cast<int>(fxParams.chainOrder));
    result.insert("resourceMeta", resourceMetaToJson(fxParams.resourceMeta));

    return result;
}

QJsonObject ProjectAudioSettings::resourceMetaToJson(const audio::AudioResourceMeta& meta) const
{
    QJsonObject result;
    result.insert("id", QString::fromStdString(meta.id));
    result.insert("vendor", QString::fromStdString(meta.vendor));
    result.insert("type", resourceTypeToString(meta.type));

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

AudioResourceType ProjectAudioSettings::resourceTypeFromString(const QString& string) const
{
    for (const auto& pair : RESOURCE_TYPE_MAP) {
        if (pair.second == string) {
            return pair.first;
        }
    }

    return AudioResourceType::Undefined;
}

QString ProjectAudioSettings::sourceTypeToString(const audio::AudioSourceType& type) const
{
    auto search = SOURCE_TYPE_MAP.find(type);

    if (search != SOURCE_TYPE_MAP.end()) {
        return search->second;
    }

    return SOURCE_TYPE_MAP.at(AudioSourceType::Undefined);
}

QString ProjectAudioSettings::resourceTypeToString(const audio::AudioResourceType& type) const
{
    auto search = RESOURCE_TYPE_MAP.find(type);

    if (search != RESOURCE_TYPE_MAP.end()) {
        return search->second;
    }

    return RESOURCE_TYPE_MAP.at(AudioResourceType::Undefined);
}

QJsonObject ProjectAudioSettings::buildTrackObject(const ID& id) const
{
    QJsonObject result;

    auto inputParamsSearch = m_trackInputParamsMap.find(id);
    if (inputParamsSearch != m_trackInputParamsMap.end()) {
        result.insert("in", inputParamsToJson(inputParamsSearch->second));
    }

    auto outputParamsSearch = m_trackOutputParamsMap.find(id);
    if (outputParamsSearch != m_trackOutputParamsMap.end()) {
        result.insert("out", outputParamsToJson(outputParamsSearch->second));
    }

    return result;
}

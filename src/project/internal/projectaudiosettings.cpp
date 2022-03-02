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
using namespace mu::engraving;

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
    if (m_masterOutputParams == params) {
        return;
    }

    m_masterOutputParams = params;
    setNeedSave(true);
}

AudioInputParams ProjectAudioSettings::trackInputParams(const InstrumentTrackId& partId) const
{
    auto search = m_trackInputParamsMap.find(partId);

    if (search == m_trackInputParamsMap.end()) {
        return {};
    }

    return search->second;
}

void ProjectAudioSettings::setTrackInputParams(const InstrumentTrackId& partId, const audio::AudioInputParams& params)
{
    auto it = m_trackInputParamsMap.find(partId);
    if (it != m_trackInputParamsMap.end() && it->second == params) {
        return;
    }

    m_trackInputParamsMap.insert_or_assign(partId, params);
    setNeedSave(true);
}

AudioOutputParams ProjectAudioSettings::trackOutputParams(const InstrumentTrackId& partId) const
{
    auto search = m_trackOutputParamsMap.find(partId);

    if (search == m_trackOutputParamsMap.end()) {
        return {};
    }

    return search->second;
}

void ProjectAudioSettings::setTrackOutputParams(const InstrumentTrackId& partId, const audio::AudioOutputParams& params)
{
    auto it = m_trackOutputParamsMap.find(partId);
    if (it != m_trackOutputParamsMap.end() && it->second == params) {
        return;
    }

    m_trackOutputParamsMap.insert_or_assign(partId, params);
    setNeedSave(true);
}

void ProjectAudioSettings::removeTrackParams(const InstrumentTrackId& partId)
{
    auto inSearch = m_trackInputParamsMap.find(partId);
    if (inSearch != m_trackInputParamsMap.end()) {
        m_trackInputParamsMap.erase(inSearch);
        setNeedSave(true);
    }

    auto outSearch = m_trackOutputParamsMap.find(partId);
    if (outSearch != m_trackOutputParamsMap.end()) {
        m_trackOutputParamsMap.erase(outSearch);
        setNeedSave(true);
    }
}

mu::ValNt<bool> ProjectAudioSettings::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = m_needSave;
    needSave.notification = m_needSaveNotification;

    return needSave;
}

mu::Ret ProjectAudioSettings::read(const engraving::MscReader& reader)
{
    QByteArray json = reader.readAudioSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json).object();

    QJsonObject masterObj = rootObj.value("master").toObject();
    m_masterOutputParams = outputParamsFromJson(masterObj);

    QJsonArray tracksArray = rootObj.value("tracks").toArray();

    for (const QJsonValue& value : tracksArray) {
        QJsonObject trackObject = value.toObject();

        ID partId = trackObject.value("partId").toString();
        std::string instrumentId = trackObject.value("instrumentId").toString().toStdString();

        InstrumentTrackId id = { partId, instrumentId };

        audio::AudioInputParams inParams = inputParamsFromJson(trackObject.value("in").toObject());
        audio::AudioOutputParams outParams = outputParamsFromJson(trackObject.value("out").toObject());

        m_trackInputParamsMap.emplace(id, std::move(inParams));
        m_trackOutputParamsMap.emplace(id, std::move(outParams));
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret ProjectAudioSettings::write(engraving::MscWriter& writer)
{
    QJsonObject rootObj;
    rootObj["master"] = outputParamsToJson(m_masterOutputParams);

    QJsonArray tracksArray;
    for (const auto& pair : m_trackInputParamsMap) {
        tracksArray.append(buildTrackObject(pair.first));
    }

    rootObj["tracks"] = tracksArray;

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeAudioSettingsJsonFile(json);

    setNeedSave(false);

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
    result.configuration = unitConfigFromJson(object.value("unitConfiguration").toObject());

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
    result.configuration = unitConfigFromJson(object.value("unitConfiguration").toObject());

    return result;
}

AudioResourceMeta ProjectAudioSettings::resourceMetaFromJson(const QJsonObject& object) const
{
    AudioResourceMeta result;
    result.id = object.value("id").toString().toStdString();
    result.hasNativeEditorSupport = object.value("hasNativeEditorSupport").toBool();
    result.vendor = object.value("vendor").toString().toStdString();
    result.type = resourceTypeFromString(object.value("type").toString());

    return result;
}

AudioUnitConfig ProjectAudioSettings::unitConfigFromJson(const QJsonObject& object) const
{
    AudioUnitConfig result;

    for (const QString& key : object.keys()) {
        result.emplace(key.toStdString(), object.value(key).toString().toStdString());
    }

    return result;
}

QJsonObject ProjectAudioSettings::inputParamsToJson(const audio::AudioInputParams& params) const
{
    QJsonObject result;
    result.insert("resourceMeta", resourceMetaToJson(params.resourceMeta));
    result.insert("unitConfiguration", unitConfigToJson(params.configuration));

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
    result.insert("unitConfiguration", unitConfigToJson(fxParams.configuration));

    return result;
}

QJsonObject ProjectAudioSettings::resourceMetaToJson(const audio::AudioResourceMeta& meta) const
{
    QJsonObject result;
    result.insert("id", QString::fromStdString(meta.id));
    result.insert("hasNativeEditorSupport", meta.hasNativeEditorSupport);
    result.insert("vendor", QString::fromStdString(meta.vendor));
    result.insert("type", resourceTypeToString(meta.type));

    return result;
}

QJsonObject ProjectAudioSettings::unitConfigToJson(const audio::AudioUnitConfig& config) const
{
    QJsonObject result;

    for (const auto& pair : config) {
        result.insert(QString::fromStdString(pair.first), QString::fromStdString(pair.second));
    }

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

QJsonObject ProjectAudioSettings::buildTrackObject(const InstrumentTrackId& id) const
{
    QJsonObject result;

    result.insert("partId", id.partId.toQString());
    result.insert("instrumentId", QString::fromStdString(id.instrumentId));

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

void ProjectAudioSettings::setNeedSave(bool needSave)
{
    if (m_needSave == needSave) {
        return;
    }

    m_needSave = needSave;
    m_needSaveNotification.notify();
}

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

#include "types/bytearray.h"

using namespace mu::project;
using namespace mu::audio;
using namespace mu::engraving;

static const std::map<AudioSourceType, QString> SOURCE_TYPE_MAP = {
    { AudioSourceType::Undefined, "undefined" },
    { AudioSourceType::MuseSampler, "musesampler" },
    { AudioSourceType::Fluid, "fluid" },
    { AudioSourceType::Vsti, "vsti" }
};

static const std::map<AudioResourceType, QString> RESOURCE_TYPE_MAP = {
    { AudioResourceType::Undefined, "undefined" },
    { AudioResourceType::MuseSamplerSoundPack, "muse_sampler_sound_pack" },
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
    bool needSave = it == m_trackOutputParamsMap.cend();

    if (!needSave) {
        needSave |= (it->second.volume != params.volume);
        needSave |= (it->second.balance != params.balance);
        needSave |= (it->second.fxChain != params.fxChain);
    }

    m_trackOutputParamsMap.insert_or_assign(partId, params);

    if (needSave) {
        setNeedSave(true);
    }
}

IProjectAudioSettings::SoloMuteState ProjectAudioSettings::soloMuteState(const InstrumentTrackId& partId) const
{
    auto search = m_soloMuteStatesMap.find(partId);

    if (search == m_soloMuteStatesMap.end()) {
        return {};
    }

    return search->second;
}

mu::async::Channel<mu::engraving::InstrumentTrackId,
                   IProjectAudioSettings::SoloMuteState> ProjectAudioSettings::soloMuteStateChanged() const
{
    return m_soloMuteStateChanged;
}

void ProjectAudioSettings::setSoloMuteState(const InstrumentTrackId& partId, const SoloMuteState& state)
{
    auto it = m_soloMuteStatesMap.find(partId);
    if (it != m_soloMuteStatesMap.end() && it->second == state) {
        return;
    }

    m_soloMuteStatesMap.insert_or_assign(partId, state);
    m_soloMuteStateChanged.send(partId, state);
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

    auto soloMuteSearch = m_soloMuteStatesMap.find(partId);
    if (soloMuteSearch != m_soloMuteStatesMap.end()) {
        m_soloMuteStatesMap.erase(soloMuteSearch);
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
    ByteArray json = reader.readAudioSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();

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
        SoloMuteState soloMuteState = soloMuteStateFromJson(trackObject.value("soloMuteState").toObject());

        m_trackInputParamsMap.emplace(id, std::move(inParams));
        m_trackOutputParamsMap.emplace(id, std::move(outParams));
        m_soloMuteStatesMap.emplace(id, std::move(soloMuteState));
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
    writer.writeAudioSettingsJsonFile(ByteArray::fromQByteArrayNoCopy(json));

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
    result.balance = object.value("balance").toVariant().toFloat();
    result.volume = object.value("volumeDb").toVariant().toFloat();

    return result;
}

IProjectAudioSettings::SoloMuteState ProjectAudioSettings::soloMuteStateFromJson(const QJsonObject& object) const
{
    SoloMuteState result;
    result.mute = object.value("mute").toBool();
    result.solo = object.value("solo").toBool();

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
        QByteArray base = QByteArray::fromBase64(object.value(key).toString().toUtf8());
        result.emplace(key.toStdString(), base.toStdString());
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
    result.insert("balance", params.balance);
    result.insert("volumeDb", params.volume);

    return result;
}

QJsonObject ProjectAudioSettings::soloMuteStateToJson(const SoloMuteState& state) const
{
    QJsonObject result;
    result.insert("mute", state.mute);
    result.insert("solo", state.solo);

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
        QByteArray byteArray = QByteArray::fromRawData(pair.second.c_str(), pair.second.size());
        result.insert(QString::fromStdString(pair.first), QString(byteArray.toBase64()));
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

    auto soloMuteSearch = m_soloMuteStatesMap.find(id);
    if (soloMuteSearch != m_soloMuteStatesMap.end()) {
        result.insert("soloMuteState", soloMuteStateToJson(soloMuteSearch->second));
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

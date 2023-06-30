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
using namespace mu::playback;

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
    { AudioResourceType::VstPlugin, "vst_plugin" },
    { AudioResourceType::MusePlugin, "muse_plugin" },
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

bool ProjectAudioSettings::containsAuxOutputParams(audio::aux_channel_idx_t index) const
{
    return mu::contains(m_auxOutputParams, index);
}

mu::audio::AudioOutputParams ProjectAudioSettings::auxOutputParams(audio::aux_channel_idx_t index) const
{
    return mu::value(m_auxOutputParams, index);
}

void ProjectAudioSettings::setAuxOutputParams(audio::aux_channel_idx_t index, const audio::AudioOutputParams& params)
{
    auto it = m_auxOutputParams.find(index);
    if (it != m_auxOutputParams.end() && it->second == params) {
        return;
    }

    m_auxOutputParams.insert_or_assign(index, params);
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
        needSave |= !RealIsEqual(it->second.volume, params.volume);
        needSave |= !RealIsEqual(it->second.balance, params.balance);
        needSave |= (it->second.fxChain != params.fxChain);
        needSave |= (it->second.auxSends != params.auxSends);
    }

    m_trackOutputParamsMap.insert_or_assign(partId, params);

    if (needSave) {
        setNeedSave(true);
    }
}

IProjectAudioSettings::SoloMuteState ProjectAudioSettings::trackSoloMuteState(const InstrumentTrackId& partId) const
{
    auto search = m_trackSoloMuteStatesMap.find(partId);

    if (search == m_trackSoloMuteStatesMap.end()) {
        return {};
    }

    return search->second;
}

mu::async::Channel<mu::engraving::InstrumentTrackId,
                   IProjectAudioSettings::SoloMuteState> ProjectAudioSettings::trackSoloMuteStateChanged() const
{
    return m_trackSoloMuteStateChanged;
}

void ProjectAudioSettings::setTrackSoloMuteState(const InstrumentTrackId& partId, const SoloMuteState& state)
{
    auto it = m_trackSoloMuteStatesMap.find(partId);
    if (it != m_trackSoloMuteStatesMap.end() && it->second == state) {
        return;
    }

    m_trackSoloMuteStatesMap.insert_or_assign(partId, state);
    m_trackSoloMuteStateChanged.send(partId, state);
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

    auto soloMuteSearch = m_trackSoloMuteStatesMap.find(partId);
    if (soloMuteSearch != m_trackSoloMuteStatesMap.end()) {
        m_trackSoloMuteStatesMap.erase(soloMuteSearch);
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

void ProjectAudioSettings::markAsSaved()
{
    setNeedSave(false);
}

const SoundProfileName& ProjectAudioSettings::activeSoundProfile() const
{
    return m_activeSoundProfileName;
}

void ProjectAudioSettings::setActiveSoundProfile(const playback::SoundProfileName& profileName)
{
    m_activeSoundProfileName = profileName;
    setNeedSave(true);
}

mu::Ret ProjectAudioSettings::read(const engraving::MscReader& reader)
{
    ByteArray json = reader.readAudioSettingsJsonFile();
    if (json.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();

    QJsonObject masterObj = rootObj.value("master").toObject();
    m_masterOutputParams = outputParamsFromJson(masterObj);

    QJsonArray auxArray = rootObj.value("aux").toArray();
    for (int i = 0; i < auxArray.size(); ++i) {
        QJsonObject auxObject = auxArray[i].toObject();
        audio::AudioOutputParams outParams = outputParamsFromJson(auxObject.value("out").toObject());
        m_auxOutputParams.emplace(static_cast<aux_channel_idx_t>(i), std::move(outParams));
    }

    QJsonArray tracksArray = rootObj.value("tracks").toArray();

    for (const QJsonValue value : tracksArray) {
        QJsonObject trackObject = value.toObject();

        ID partId = trackObject.value("partId").toString();
        std::string instrumentId = trackObject.value("instrumentId").toString().toStdString();

        InstrumentTrackId id = { partId, instrumentId };

        audio::AudioInputParams inParams = inputParamsFromJson(trackObject.value("in").toObject());
        audio::AudioOutputParams outParams = outputParamsFromJson(trackObject.value("out").toObject());
        SoloMuteState soloMuteState = soloMuteStateFromJson(trackObject.value("soloMuteState").toObject());

        m_trackInputParamsMap.emplace(id, std::move(inParams));
        m_trackOutputParamsMap.emplace(id, std::move(outParams));
        m_trackSoloMuteStatesMap.emplace(id, std::move(soloMuteState));
    }

    m_activeSoundProfileName = rootObj.value("activeSoundProfile").toString();
    if (m_activeSoundProfileName.empty()) {
        m_activeSoundProfileName = playbackConfig()->defaultProfileForNewProjects();
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret ProjectAudioSettings::write(engraving::MscWriter& writer)
{
    QJsonObject rootObj;
    rootObj["master"] = outputParamsToJson(m_masterOutputParams);

    QJsonArray auxArray;
    for (const auto& pair : m_auxOutputParams) {
        auxArray.append(buildAuxObject(pair.second));
    }

    rootObj["aux"] = auxArray;

    QJsonArray tracksArray;
    for (const auto& pair : m_trackInputParamsMap) {
        tracksArray.append(buildTrackObject(pair.first));
    }

    rootObj["tracks"] = tracksArray;
    rootObj["activeSoundProfile"] = m_activeSoundProfileName.toQString();

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeAudioSettingsJsonFile(ByteArray::fromQByteArrayNoCopy(json));

    return make_ret(Ret::Code::Ok);
}

void ProjectAudioSettings::makeDefault()
{
    m_activeSoundProfileName = playbackConfig()->defaultProfileForNewProjects();
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
    result.auxSends = auxSendsFromJson(object.value("auxSends").toArray());

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

AuxSendsParams ProjectAudioSettings::auxSendsFromJson(const QJsonArray& objectList) const
{
    AuxSendsParams result;

    for (const QJsonValue value : objectList) {
        AuxSendParams params = auxSendParamsFromJson(value.toObject());
        result.emplace_back(std::move(params));
    }

    return result;
}

AuxSendParams ProjectAudioSettings::auxSendParamsFromJson(const QJsonObject& object) const
{
    AuxSendParams result;
    result.signalAmount = object.value("signalAmount").toVariant().toFloat();
    result.active = object.value("active").toBool();

    return result;
}

AudioResourceMeta ProjectAudioSettings::resourceMetaFromJson(const QJsonObject& object) const
{
    AudioResourceMeta result;
    result.id = object.value("id").toString().toStdString();
    result.hasNativeEditorSupport = object.value("hasNativeEditorSupport").toBool();
    result.vendor = object.value("vendor").toString().toStdString();
    result.type = resourceTypeFromString(object.value("type").toString());
    result.attributes = attributesFromJson(object.value("attributes").toObject());

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

AudioResourceAttributes ProjectAudioSettings::attributesFromJson(const QJsonObject& object) const
{
    AudioResourceAttributes result;

    for (const QString& key : object.keys()) {
        result.emplace(String::fromQString(key), String::fromQString(object.value(key).toString()));
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

    if (!params.auxSends.empty()) {
        result.insert("auxSends", auxSendsToJson(params.auxSends));
    }

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

QJsonArray ProjectAudioSettings::auxSendsToJson(const audio::AuxSendsParams& auxSends) const
{
    QJsonArray result;

    for (const AuxSendParams& auxSend : auxSends) {
        result.push_back(auxSendParamsToJson(auxSend));
    }

    return result;
}

QJsonObject ProjectAudioSettings::auxSendParamsToJson(const audio::AuxSendParams& auxParams) const
{
    QJsonObject result;
    result.insert("active", auxParams.active);
    result.insert("signalAmount", auxParams.signalAmount);

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
    result.insert("attributes", attributesToJson(meta.attributes));

    return result;
}

QJsonObject ProjectAudioSettings::unitConfigToJson(const audio::AudioUnitConfig& config) const
{
    QJsonObject result;

    for (const auto& pair : config) {
        QByteArray byteArray = QByteArray::fromRawData(pair.second.c_str(), static_cast<int>(pair.second.size()));
        result.insert(QString::fromStdString(pair.first), QString(byteArray.toBase64()));
    }

    return result;
}

QJsonObject ProjectAudioSettings::attributesToJson(const audio::AudioResourceAttributes& attributes) const
{
    QJsonObject result;

    for (const auto& pair : attributes) {
        result.insert(pair.first.toQString(), pair.second.toQString());
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

QJsonObject ProjectAudioSettings::buildAuxObject(const audio::AudioOutputParams& params) const
{
    QJsonObject result;

    result.insert("out", outputParamsToJson(params));

    return result;
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

    auto soloMuteSearch = m_trackSoloMuteStatesMap.find(id);
    if (soloMuteSearch != m_trackSoloMuteStatesMap.end()) {
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

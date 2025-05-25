/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
using namespace muse;
using namespace muse::audio;
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

static void doCompatibilityConversions(AudioResourceMeta& meta)
{
    if (meta.type == AudioResourceType::MuseSamplerSoundPack) {
        // MS 4.5: resource name and category have been excluded from ID
        // Old format: category\\name\\uid
        if (meta.id.find("\\") == std::string::npos) {
            return;
        }

        const String& museUID = meta.attributeVal(u"museUID");
        if (!museUID.empty()) {
            meta.id = museUID.toStdString();
        }
    }
}

const AudioOutputParams& ProjectAudioSettings::masterAudioOutputParams() const
{
    return m_masterOutputParams;
}

void ProjectAudioSettings::setMasterAudioOutputParams(const AudioOutputParams& params)
{
    if (m_masterOutputParams == params) {
        return;
    }

    m_masterOutputParams = params;
    m_settingsChanged.notify();
}

bool ProjectAudioSettings::containsAuxOutputParams(aux_channel_idx_t index) const
{
    return muse::contains(m_auxOutputParams, index);
}

const AudioOutputParams& ProjectAudioSettings::auxOutputParams(aux_channel_idx_t index) const
{
    if (index < m_auxOutputParams.size()) {
        return m_auxOutputParams.at(index);
    }

    static const AudioOutputParams _dummy;
    return _dummy;
}

void ProjectAudioSettings::setAuxOutputParams(aux_channel_idx_t index, const AudioOutputParams& params)
{
    auto it = m_auxOutputParams.find(index);
    if (it != m_auxOutputParams.end() && it->second == params) {
        return;
    }

    m_auxOutputParams.insert_or_assign(index, params);
    m_settingsChanged.notify();
}

const AudioInputParams& ProjectAudioSettings::trackInputParams(const InstrumentTrackId& partId) const
{
    auto search = m_trackInputParamsMap.find(partId);

    if (search == m_trackInputParamsMap.end()) {
        static const AudioInputParams _dummy;
        return _dummy;
    }

    return search->second;
}

void ProjectAudioSettings::setTrackInputParams(const InstrumentTrackId& partId, const AudioInputParams& params)
{
    auto it = m_trackInputParamsMap.find(partId);
    if (it != m_trackInputParamsMap.end() && it->second == params) {
        return;
    }

    m_trackInputParamsMap.insert_or_assign(partId, params);
    m_trackInputParamsChanged.send(partId);
    m_settingsChanged.notify();
}

void ProjectAudioSettings::clearTrackInputParams()
{
    if (m_trackInputParamsMap.empty()) {
        return;
    }

    auto it = m_trackInputParamsMap.begin();
    while (it != m_trackInputParamsMap.end()) {
        InstrumentTrackId id = it->first;
        it = m_trackInputParamsMap.erase(it);
        m_trackInputParamsChanged.send(id);
    }

    m_settingsChanged.notify();
}

muse::async::Channel<mu::engraving::InstrumentTrackId> ProjectAudioSettings::trackInputParamsChanged() const
{
    return m_trackInputParamsChanged;
}

bool ProjectAudioSettings::trackHasExistingOutputParams(const InstrumentTrackId& partId) const
{
    return muse::contains(m_trackOutputParamsMap, partId);
}

const AudioOutputParams& ProjectAudioSettings::trackOutputParams(const InstrumentTrackId& partId) const
{
    auto search = m_trackOutputParamsMap.find(partId);

    if (search == m_trackOutputParamsMap.end()) {
        static const AudioOutputParams _dummy;
        return _dummy;
    }

    return search->second;
}

void ProjectAudioSettings::setTrackOutputParams(const InstrumentTrackId& partId, const AudioOutputParams& params)
{
    auto it = m_trackOutputParamsMap.find(partId);
    bool paramsChanged = it == m_trackOutputParamsMap.cend();

    if (!paramsChanged) {
        paramsChanged |= !muse::RealIsEqual(it->second.volume, params.volume);
        paramsChanged |= !muse::RealIsEqual(it->second.balance, params.balance);
        paramsChanged |= (it->second.fxChain != params.fxChain);
        paramsChanged |= (it->second.auxSends != params.auxSends);
    }

    m_trackOutputParamsMap.insert_or_assign(partId, params);

    if (paramsChanged) {
        m_settingsChanged.notify();
    }
}

const IProjectAudioSettings::SoloMuteState& ProjectAudioSettings::auxSoloMuteState(aux_channel_idx_t index) const
{
    auto it = m_auxSoloMuteStatesMap.find(index);
    if (it == m_auxSoloMuteStatesMap.end()) {
        static const IProjectAudioSettings::SoloMuteState _dummy;
        return _dummy;
    }

    return it->second;
}

void ProjectAudioSettings::setAuxSoloMuteState(aux_channel_idx_t index, const SoloMuteState& state)
{
    auto it = m_auxSoloMuteStatesMap.find(index);
    if (it != m_auxSoloMuteStatesMap.end() && it->second == state) {
        return;
    }

    m_auxSoloMuteStatesMap.insert_or_assign(index, state);
    m_auxSoloMuteStateChanged.send(index, state);
    m_settingsChanged.notify();
}

muse::async::Channel<aux_channel_idx_t, IProjectAudioSettings::SoloMuteState> ProjectAudioSettings::auxSoloMuteStateChanged() const
{
    return m_auxSoloMuteStateChanged;
}

void ProjectAudioSettings::removeTrackParams(const InstrumentTrackId& partId)
{
    auto inSearch = m_trackInputParamsMap.find(partId);
    if (inSearch != m_trackInputParamsMap.end()) {
        m_trackInputParamsMap.erase(inSearch);
        m_trackInputParamsChanged.send(partId);
        m_settingsChanged.notify();
    }

    auto outSearch = m_trackOutputParamsMap.find(partId);
    if (outSearch != m_trackOutputParamsMap.end()) {
        m_trackOutputParamsMap.erase(outSearch);
        m_settingsChanged.notify();
    }
}

const SoundProfileName& ProjectAudioSettings::activeSoundProfile() const
{
    return m_activeSoundProfileName;
}

void ProjectAudioSettings::setActiveSoundProfile(const playback::SoundProfileName& profileName)
{
    if (m_activeSoundProfileName == profileName) {
        return;
    }

    m_activeSoundProfileName = profileName;
    m_settingsChanged.notify();
}

muse::async::Notification ProjectAudioSettings::settingsChanged() const
{
    return m_settingsChanged;
}

Ret ProjectAudioSettings::read(const engraving::MscReader& reader)
{
    ByteArray json = reader.readAudioSettingsJsonFile();
    if (json.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();

    QJsonObject masterObj = rootObj.value("master").toObject();
    m_masterOutputParams = outputParamsFromJson(masterObj);

    QJsonArray auxArray = rootObj.value("aux").toArray();
    for (aux_channel_idx_t i = 0; i < static_cast<aux_channel_idx_t>(auxArray.size()); ++i) {
        QJsonObject auxObject = auxArray[i].toObject();

        AudioOutputParams outParams = outputParamsFromJson(auxObject.value("out").toObject());
        SoloMuteState soloMuteState = soloMuteStateFromJson(auxObject.value("soloMuteState").toObject());

        m_auxOutputParams.emplace(i, std::move(outParams));
        m_auxSoloMuteStatesMap.emplace(i, std::move(soloMuteState));
    }

    QJsonArray tracksArray = rootObj.value("tracks").toArray();

    for (const QJsonValue value : tracksArray) {
        QJsonObject trackObject = value.toObject();

        InstrumentTrackId id = {
            trackObject.value("partId").toString(),
            trackObject.value("instrumentId").toString()
        };

        AudioInputParams inParams = inputParamsFromJson(trackObject.value("in").toObject());
        AudioOutputParams outParams = outputParamsFromJson(trackObject.value("out").toObject());

        doCompatibilityConversions(inParams.resourceMeta);

        m_trackInputParamsMap.emplace(id, std::move(inParams));
        m_trackOutputParamsMap.emplace(id, std::move(outParams));
    }

    m_activeSoundProfileName = rootObj.value("activeSoundProfile").toString();
    if (m_activeSoundProfileName.empty()) {
        m_activeSoundProfileName = playbackConfig()->defaultProfileForNewProjects();
    } else if (m_activeSoundProfileName == playbackConfig()->compatMuseSoundsProfileName()) {
        m_activeSoundProfileName = playbackConfig()->museSoundsProfileName();
    }

    return make_ret(Ret::Code::Ok);
}

Ret ProjectAudioSettings::write(engraving::MscWriter& writer, notation::INotationSoloMuteStatePtr masterSoloMuteStatePtr)
{
    QJsonObject rootObj;
    rootObj["master"] = outputParamsToJson(m_masterOutputParams);

    QJsonArray auxArray;
    for (const auto& pair : m_auxOutputParams) {
        auxArray.append(buildAuxObject(pair.first, pair.second));
    }

    rootObj["aux"] = auxArray;

    QJsonArray tracksArray;
    for (const auto& pair : m_trackInputParamsMap) {
        tracksArray.append(buildTrackObject(masterSoloMuteStatePtr, pair.first));
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

QJsonObject ProjectAudioSettings::inputParamsToJson(const AudioInputParams& params) const
{
    QJsonObject result;
    result.insert("resourceMeta", resourceMetaToJson(params.resourceMeta));
    result.insert("unitConfiguration", unitConfigToJson(params.configuration));

    return result;
}

QJsonObject ProjectAudioSettings::outputParamsToJson(const AudioOutputParams& params) const
{
    QJsonObject result;
    result.insert("fxChain", fxChainToJson(params.fxChain));
    result.insert("balance", params.balance);
    result.insert("volumeDb", params.volume.raw());

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

QJsonObject ProjectAudioSettings::fxChainToJson(const AudioFxChain& fxChain) const
{
    QJsonObject result;

    for (const auto& pair : fxChain) {
        result.insert(QString::number(static_cast<int>(pair.first)), fxParamsToJson(pair.second));
    }

    return result;
}

QJsonArray ProjectAudioSettings::auxSendsToJson(const AuxSendsParams& auxSends) const
{
    QJsonArray result;

    for (const AuxSendParams& auxSend : auxSends) {
        result.push_back(auxSendParamsToJson(auxSend));
    }

    return result;
}

QJsonObject ProjectAudioSettings::auxSendParamsToJson(const AuxSendParams& auxParams) const
{
    QJsonObject result;
    result.insert("active", auxParams.active);
    result.insert("signalAmount", auxParams.signalAmount);

    return result;
}

QJsonObject ProjectAudioSettings::fxParamsToJson(const AudioFxParams& fxParams) const
{
    QJsonObject result;
    result.insert("active", fxParams.active);
    result.insert("chainOrder", static_cast<int>(fxParams.chainOrder));
    result.insert("resourceMeta", resourceMetaToJson(fxParams.resourceMeta));
    result.insert("unitConfiguration", unitConfigToJson(fxParams.configuration));

    return result;
}

QJsonObject ProjectAudioSettings::resourceMetaToJson(const AudioResourceMeta& meta) const
{
    QJsonObject result;
    result.insert("id", QString::fromStdString(meta.id));
    result.insert("hasNativeEditorSupport", meta.hasNativeEditorSupport);
    result.insert("vendor", QString::fromStdString(meta.vendor));
    result.insert("type", resourceTypeToString(meta.type));
    result.insert("attributes", attributesToJson(meta.attributes));

    return result;
}

QJsonObject ProjectAudioSettings::unitConfigToJson(const AudioUnitConfig& config) const
{
    QJsonObject result;

    for (const auto& pair : config) {
        QByteArray byteArray = QByteArray::fromRawData(pair.second.c_str(), static_cast<int>(pair.second.size()));
        result.insert(QString::fromStdString(pair.first), QString(byteArray.toBase64()));
    }

    return result;
}

QJsonObject ProjectAudioSettings::attributesToJson(const AudioResourceAttributes& attributes) const
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

QString ProjectAudioSettings::sourceTypeToString(const AudioSourceType& type) const
{
    auto search = SOURCE_TYPE_MAP.find(type);

    if (search != SOURCE_TYPE_MAP.end()) {
        return search->second;
    }

    return SOURCE_TYPE_MAP.at(AudioSourceType::Undefined);
}

QString ProjectAudioSettings::resourceTypeToString(const AudioResourceType& type) const
{
    auto search = RESOURCE_TYPE_MAP.find(type);

    if (search != RESOURCE_TYPE_MAP.end()) {
        return search->second;
    }

    return RESOURCE_TYPE_MAP.at(AudioResourceType::Undefined);
}

QJsonObject ProjectAudioSettings::buildAuxObject(aux_channel_idx_t index, const AudioOutputParams& params) const
{
    QJsonObject result;

    result.insert("out", outputParamsToJson(params));

    auto soloMuteSearch = m_auxSoloMuteStatesMap.find(index);
    if (soloMuteSearch != m_auxSoloMuteStatesMap.end()) {
        result.insert("soloMuteState", soloMuteStateToJson(soloMuteSearch->second));
    }

    return result;
}

QJsonObject ProjectAudioSettings::buildTrackObject(notation::INotationSoloMuteStatePtr masterSoloMuteStatePtr,
                                                   const InstrumentTrackId& id) const
{
    QJsonObject result;

    result.insert("partId", id.partId.toQString());
    result.insert("instrumentId", id.instrumentId.toQString());

    auto inputParamsSearch = m_trackInputParamsMap.find(id);
    if (inputParamsSearch != m_trackInputParamsMap.end()) {
        result.insert("in", inputParamsToJson(inputParamsSearch->second));
    }

    auto outputParamsSearch = m_trackOutputParamsMap.find(id);
    if (outputParamsSearch != m_trackOutputParamsMap.end()) {
        result.insert("out", outputParamsToJson(outputParamsSearch->second));
    }

    if (masterSoloMuteStatePtr && masterSoloMuteStatePtr->trackSoloMuteStateExists(id)) {
        SoloMuteState soloMuteState = masterSoloMuteStatePtr->trackSoloMuteState(id);
        result.insert("soloMuteState", soloMuteStateToJson(soloMuteState));
    }

    return result;
}

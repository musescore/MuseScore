/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "musesamplerresolver.h"

#include "types/version.h"

#include "musesamplerwrapper.h"

#include "serialization/json.h"

#include "log.h"

using namespace muse;
using namespace muse::async;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::musesampler;

static const std::unordered_map<String, ClefType> CLEF_NAME_LOWER_CASE_TO_TYPE {
    { u"none", ClefType::None },
    { u"treble", ClefType::Treble },
    { u"bass", ClefType::Bass },
    { u"alto", ClefType::Alto },
    { u"tenor", ClefType::Tenor },
    { u"percussion", ClefType::Percussion },
    { u"higheroctavetreble", ClefType::HigherOctaveTreble },
    { u"loweroctavetreble", ClefType::LowerOctaveTreble },
    { u"higheroctavebass", ClefType::HigherOctaveBass },
    { u"loweroctavebass", ClefType::LowerOctaveBass },
    { u"baritone", ClefType::Baritone },
    { u"mezzosoprano", ClefType::Mezzosoprano },
    { u"soprano", ClefType::Soprano },
    { u"frenchviolin", ClefType::FrenchViolin },
};

static const std::unordered_map<String, StaffType> STAFF_NAME_LOWER_CASE_TO_TYPE {
    { u"standard", StaffType::Standard },
    { u"grand", StaffType::Grand },
};

InstrumentInfo findInstrument(MuseSamplerLibHandlerPtr libHandler, const AudioResourceMeta& resourceMeta)
{
    if (!libHandler) {
        return InstrumentInfo();
    }

    auto instrumentList = libHandler->getInstrumentList();

    while (auto instrument = libHandler->getNextInstrument(instrumentList)) {
        int instrumentId = libHandler->getInstrumentId(instrument);

        if (resourceMeta.attributeVal(u"museUID") == String::number(instrumentId)) {
            return { instrumentId, instrument };
        }
    }

    return InstrumentInfo();
}

void MuseSamplerResolver::init()
{
    m_libHandler = std::make_shared<MuseSamplerLibHandler>(configuration()->libraryPath(), configuration()->useLegacyAudition());

    if (!m_libHandler->isValid()) {
        LOGE() << "Incompatible MuseSampler library, ignoring: " << configuration()->libraryPath();
        m_libHandler.reset();
        return;
    }

    if (!m_libHandler->init()) {
        LOGE() << "Could not init MuseSampler: " << configuration()->libraryPath();
        m_libHandler.reset();
        return;
    }

    LOGI() << "MuseSampler successfully inited: " << configuration()->libraryPath();
}

bool MuseSamplerResolver::reloadMuseSampler()
{
    if (!m_libHandler) {
        return false;
    }

    return m_libHandler->reloadAllInstruments() == ms_Result_OK;
}

ISynthesizerPtr MuseSamplerResolver::resolveSynth(const TrackId /*trackId*/, const AudioInputParams& params) const
{
    InstrumentInfo instrument = findInstrument(m_libHandler, params.resourceMeta);
    if (instrument.isValid()) {
        return std::make_shared<MuseSamplerWrapper>(m_libHandler, instrument, params, iocContext());
    }

    return nullptr;
}

bool MuseSamplerResolver::hasCompatibleResources(const PlaybackSetupData& setup) const
{
    UNUSED(setup);

    if (!m_libHandler) {
        return false;
    }

    return true;
}

AudioResourceMetaList MuseSamplerResolver::resolveResources() const
{
    AudioResourceMetaList result;

    if (!m_libHandler) {
        return result;
    }

    auto instrumentList = m_libHandler->getInstrumentList();
    while (auto instrument = m_libHandler->getNextInstrument(instrumentList)) {
        const int instrumentId = m_libHandler->getInstrumentId(instrument);
        const String internalName = String::fromUtf8(m_libHandler->getInstrumentName(instrument));
        const String internalCategory = String::fromUtf8(m_libHandler->getInstrumentCategory(instrument));
        const String instrumentSoundId = String::fromUtf8(m_libHandler->getMpeSoundId(instrument));
        const String vendorName = String::fromUtf8(m_libHandler->getInstrumentVendorName(instrument));
        const bool isOnline = m_libHandler->isOnlineInstrument(instrument);

        if (instrumentSoundId.empty()) {
            LOGE() << "MISSING INSTRUMENT ID for: " << internalName;
        }

        String instrumentPackName = String::fromUtf8(m_libHandler->getInstrumentPackName(instrument));
        if (instrumentPackName.empty()) {
            instrumentPackName = internalCategory;
        }

        AudioResourceMeta meta;
        meta.id = std::to_string(instrumentId);
        meta.type = AudioResourceType::MuseSamplerSoundPack;
        meta.vendor = "MuseSounds";
        meta.attributes = {
            { u"playbackSetupData", instrumentSoundId },
            { u"museCategory", internalCategory },
            { u"musePack", instrumentPackName },
            { u"museVendorName", vendorName },
            { u"museName", internalName },
            { u"museUID", String::fromStdString(std::to_string(instrumentId)) },
        };

        if (isOnline) {
            meta.attributes.insert(std::make_pair(u"isOnline", String::number(1)));
        }

        result.push_back(std::move(meta));
    }

    return result;
}

SoundPresetList MuseSamplerResolver::resolveSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    InstrumentInfo instrument = findInstrument(m_libHandler, resourceMeta);
    if (!instrument.msInstrument) {
        return SoundPresetList();
    }

    ms_PresetList presets = m_libHandler->getPresetList(instrument.msInstrument);
    SoundPresetList result;

    while (const char* presetCode = m_libHandler->getNextPreset(presets)) {
        SoundPreset soundPreset;
        soundPreset.code = presetCode;
        soundPreset.name = presetCode;
        loadSoundPresetAttributes(soundPreset.attributes, instrument.instrumentId, presetCode);

        result.emplace_back(std::move(soundPreset));
    }

    if (result.empty()) {
        // All instruments always have at least 1 preset (default)
        // If getPresetList returns an empty list, the default preset is implicitly defined as ""
        static const String DEFAULT_PRESET_CODE(u"Default");

        SoundPreset defaultPreset;
        defaultPreset.code = DEFAULT_PRESET_CODE;
        loadSoundPresetAttributes(defaultPreset.attributes, instrument.instrumentId, "");

        result.emplace_back(std::move(defaultPreset));
    }

    result.front().isDefault = true;

    return result;
}

void MuseSamplerResolver::refresh()
{
}

void MuseSamplerResolver::clearSources()
{
}

std::string MuseSamplerResolver::version() const
{
    if (!m_libHandler) {
        return std::string();
    }

    String ver = String::fromUtf8(m_libHandler->getVersionString());

    if (configuration()->shouldShowBuildNumber()) {
        ver += u"." + String::number(m_libHandler->getBuildNumber());
    }

    return ver.toStdString();
}

bool MuseSamplerResolver::isInstalled() const
{
    if (m_libHandler) {
        return true;
    }

    return false;
}

float MuseSamplerResolver::defaultReverbLevel(const String& instrumentSoundId) const
{
    if (!m_libHandler || !m_libHandler->getReverbLevel || instrumentSoundId.empty()) {
        return 0.f;
    }

    auto instrumentList = m_libHandler->getInstrumentList();
    while (auto instrument = m_libHandler->getNextInstrument(instrumentList)) {
        String soundId = String::fromUtf8(m_libHandler->getMpeSoundId(instrument));

        if (instrumentSoundId == soundId) {
            return m_libHandler->getReverbLevel(instrument) / 100.f;
        }
    }

    return 0.f;
}

ByteArray MuseSamplerResolver::drumMapping(int instrumentId) const
{
    if (!m_libHandler) {
        return ByteArray();
    }

    const char* mapping_cstr = m_libHandler->getDrumMapping(instrumentId);
    return mapping_cstr ? ByteArray(mapping_cstr) : ByteArray();
}

std::vector<Instrument> MuseSamplerResolver::instruments() const
{
    if (!m_libHandler) {
        return {};
    }

    std::vector<Instrument> result;

    auto instrumentList = m_libHandler->getInstrumentList();
    while (auto msInstrument = m_libHandler->getNextInstrument(instrumentList)) {
        const char* json_cstr = m_libHandler->getInstrumentInfoJson(msInstrument);
        if (!json_cstr) {
            continue;
        }

        ByteArray json(json_cstr);
        if (json.empty()) {
            continue;
        }

        std::string err;
        JsonDocument doc = JsonDocument::fromJson(json, &err);
        if (!err.empty()) {
            LOGE() << err;
            continue;
        }

        int id = m_libHandler->getInstrumentId(msInstrument);
        JsonObject obj = doc.rootObject();

        Instrument instrument;
        instrument.id = String::number(id);
        instrument.soundId = String::fromUtf8(m_libHandler->getMpeSoundId(msInstrument));
        instrument.musicXmlId = String::fromUtf8(m_libHandler->getMusicXmlSoundId(msInstrument));
        instrument.name = obj.value("FriendlyName").toString();
        instrument.abbreviation = obj.value("Abbreviation").toString();
        instrument.category = obj.value("Category").toString();
        instrument.vendor = obj.value("Vendor").toString();
        instrument.staffLines = obj.value("StaffLines", "5").toString().toInt();
        instrument.staffType = muse::value(STAFF_NAME_LOWER_CASE_TO_TYPE, obj.value(
                                               "DefaultStaffType").toString().toLower(), StaffType::Standard);
        instrument.clefType = muse::value(CLEF_NAME_LOWER_CASE_TO_TYPE, obj.value("DefaultClef").toString().toLower(), ClefType::Treble);

        result.emplace_back(std::move(instrument));
    }

    return result;
}

void MuseSamplerResolver::loadSoundPresetAttributes(SoundPresetAttributes& attributes, int instrumentId, const char* presetCode) const
{
    const char* articulations_cstr = m_libHandler->getTextArticulations(instrumentId, presetCode);
    if (articulations_cstr) {
        String articulation = String::fromAscii(articulations_cstr);

        if (!articulation.empty()) {
            attributes.emplace(PLAYING_TECHNIQUES_ATTRIBUTE, std::move(articulation));
        }
    }
}

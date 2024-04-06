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

#include "log.h"

using namespace mu;
using namespace mu::async;
using namespace muse::audio;
using namespace muse::audio::synth;
using namespace muse::musesampler;

InstrumentInfo findInstrument(MuseSamplerLibHandlerPtr libHandler, const AudioResourceMeta& resourceMeta)
{
    if (!libHandler) {
        return InstrumentInfo();
    }

    auto instrumentList = libHandler->getInstrumentList();

    while (auto instrument = libHandler->getNextInstrument(instrumentList)) {
        int instrumentId = libHandler->getInstrumentId(instrument);
        String internalName = String::fromUtf8(libHandler->getInstrumentName(instrument));
        String internalCategory = String::fromUtf8(libHandler->getInstrumentCategory(instrument));
        String instrumentSoundId = String::fromUtf8(libHandler->getMpeSoundId(instrument));

        if (resourceMeta.attributeVal(u"playbackSetupData") == instrumentSoundId
            && resourceMeta.attributeVal(u"museCategory") == internalCategory
            && resourceMeta.attributeVal(u"museName") == internalName
            && resourceMeta.attributeVal(u"museUID") == String::fromStdString(std::to_string(instrumentId))) {
            return { instrumentId, instrument };
        }
    }

    return InstrumentInfo();
}

void MuseSamplerResolver::init()
{
    io::path_t path = configuration()->userLibraryPath();
    m_libHandler = std::make_shared<MuseSamplerLibHandler>(path);
    if (checkLibrary()) {
        return;
    }

    // Use fallback
    path = configuration()->fallbackLibraryPath();
    m_libHandler = std::make_shared<MuseSamplerLibHandler>(path);
    if (!checkLibrary()) {
        m_libHandler.reset();
    }
}

ISynthesizerPtr MuseSamplerResolver::resolveSynth(const TrackId /*trackId*/, const AudioInputParams& params) const
{
    InstrumentInfo instrument = findInstrument(m_libHandler, params.resourceMeta);
    if (instrument.isValid()) {
        return std::make_shared<MuseSamplerWrapper>(m_libHandler, instrument, params);
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
    while (auto instrument = m_libHandler->getNextInstrument(instrumentList))
    {
        int instrumentId = m_libHandler->getInstrumentId(instrument);
        String internalName = String::fromUtf8(m_libHandler->getInstrumentName(instrument));
        String internalCategory = String::fromUtf8(m_libHandler->getInstrumentCategory(instrument));
        String instrumentPackName = String::fromUtf8(m_libHandler->getInstrumentPackName(instrument));
        String instrumentSoundId = String::fromUtf8(m_libHandler->getMpeSoundId(instrument));
        String vendorName = String::fromUtf8(m_libHandler->getInstrumentVendorName(instrument));

        if (instrumentSoundId.empty()) {
            LOGE() << "MISSING INSTRUMENT ID for: " << internalName;
        }

        if (instrumentPackName.empty()) {
            instrumentPackName = internalCategory;
        }

        AudioResourceMeta meta;
        meta.id = buildMuseInstrumentId(internalCategory, internalName, instrumentId).toStdString();
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

    return String::fromUtf8(m_libHandler->getVersionString()).toStdString();
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

String MuseSamplerResolver::drumMapping(int instrumentId) const
{
    if (!m_libHandler) {
        return String();
    }

    const char* mapping_cstr = m_libHandler->getDrumMapping(instrumentId);
    return mapping_cstr ? String::fromAscii(mapping_cstr) : String();
}

bool MuseSamplerResolver::checkLibrary() const
{
    if (!m_libHandler->isValid()) {
        LOGE() << "Incompatible MuseSampler library; ignoring";
        return false;
    }

    return true;
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

String MuseSamplerResolver::buildMuseInstrumentId(const String& category, const String& name, int uniqueId) const
{
    StringList list;
    list.append(category);
    list.append(name);
    list.append(String::fromStdString(std::to_string(uniqueId)));

    return list.join(u"\\");
}

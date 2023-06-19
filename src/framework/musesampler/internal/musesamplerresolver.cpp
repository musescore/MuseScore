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
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::musesampler;
using namespace mu::framework;

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

ISynthesizerPtr MuseSamplerResolver::resolveSynth(const audio::TrackId /*trackId*/, const audio::AudioInputParams& params) const
{
    if (!m_libHandler) {
        return nullptr;
    }

    auto instrumentList = m_libHandler->getInstrumentList();
    while (auto instrument = m_libHandler->getNextInstrument(instrumentList)) {
        String uniqueId = String::fromStdString(std::to_string(m_libHandler->getInstrumentId(instrument)));
        String internalName = String::fromUtf8(m_libHandler->getInstrumentName(instrument));
        String internalCategory = String::fromUtf8(m_libHandler->getInstrumentCategory(instrument));
        String instrumentSoundId = String::fromUtf8(m_libHandler->getMpeSoundId(instrument));

        if (params.resourceMeta.attributeVal(u"playbackSetupData") == instrumentSoundId
            && params.resourceMeta.attributeVal(u"museCategory") == internalCategory
            && params.resourceMeta.attributeVal(u"museName") == internalName
            && params.resourceMeta.attributeVal(u"museUID") == uniqueId) {
            return std::make_shared<MuseSamplerWrapper>(m_libHandler, params);
        }
    }

    return nullptr;
}

bool MuseSamplerResolver::hasCompatibleResources(const audio::PlaybackSetupData& setup) const
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
        int uniqueId = m_libHandler->getInstrumentId(instrument);
        String internalName = String::fromUtf8(m_libHandler->getInstrumentName(instrument));
        String internalCategory = String::fromUtf8(m_libHandler->getInstrumentCategory(instrument));
        String instrumentPack = String::fromUtf8(m_libHandler->getInstrumentPackage(instrument));
        String instrumentSoundId = String::fromUtf8(m_libHandler->getMpeSoundId(instrument));

        if (instrumentSoundId.empty()) {
            LOGE() << "MISSING INSTRUMENT ID for: " << internalName;
        }

        AudioResourceMeta meta;
        meta.id = buildMuseInstrumentId(internalCategory, internalName, uniqueId).toStdString();
        meta.type = AudioResourceType::MuseSamplerSoundPack;
        meta.vendor = instrumentPack.toStdString();
        meta.attributes = {
            { u"playbackSetupData", instrumentSoundId },
            { u"museCategory", internalCategory },
            { u"museName", internalName },
            { u"museUID", String::fromStdString(std::to_string(uniqueId)) },
        };

        result.push_back(std::move(meta));
    }

    return result;
}

void MuseSamplerResolver::refresh()
{
    NOT_SUPPORTED;
}

void MuseSamplerResolver::clearSources()
{
    NOT_SUPPORTED;
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

bool MuseSamplerResolver::checkLibrary() const
{
    if (!m_libHandler->isValid()) {
        LOGE() << "Incompatible MuseSampler library; ignoring";
        return false;
    }

    return true;
}

String MuseSamplerResolver::buildMuseInstrumentId(const String& category, const String& name, int uniqueId) const
{
    StringList list;
    list.append(category);
    list.append(name);
    list.append(String::fromStdString(std::to_string(uniqueId)));

    return list.join(u"\\");
}

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

#include "log.h"

#include "musesamplerwrapper.h"

using namespace mu;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::musesampler;

MuseSamplerResolver::MuseSamplerResolver()
{
    io::path_t path = configuration()->libraryPath();

    m_libHandler = std::make_shared<MuseSamplerLibHandler>(path);

    if (!m_libHandler->isValid()) {
        LOGE() << "Incompatible MuseSampler library; ignoring\n";
        m_libHandler.reset();
    }
}

ISynthesizerPtr MuseSamplerResolver::resolveSynth(const audio::TrackId /*trackId*/, const audio::AudioInputParams& params) const
{
    return std::make_shared<MuseSamplerWrapper>(m_libHandler, params);
}

bool MuseSamplerResolver::hasCompatibleResources(const audio::PlaybackSetupData& setup) const
{
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

String MuseSamplerResolver::buildMuseInstrumentId(const String& category, const String& name, int uniqueId) const
{
    StringList list;
    list.append(category);
    list.append(name);
    list.append(String::fromStdString(std::to_string(uniqueId)));

    return list.join(u"\\");
}

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

#include "musesamplerwrapper.h"

#include "log.h"

using namespace mu;
using namespace mu::async;
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::musesampler;

static std::array<int, 3> parseVersion(const std::string& versionString, bool& ok)
{
    std::array<int, 3> result { 0, 0, 0 };

    size_t componentIdx = 0;
    int curNum = 0;

    for (const char ch : versionString) {
        if (ch == '.' || ch == '\0') {
            result.at(componentIdx++) = curNum;
            curNum = 0;
        } else if ('0' <= ch && ch <= '9') {
            curNum = curNum * 10 + (ch - '0');
        } else {
            ok = false;
            return result;
        }
    }

    result.at(componentIdx) = curNum;

    ok = true;
    return result;
}

void MuseSamplerResolver::init()
{
    io::path_t path = configuration()->libraryPath();

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

bool MuseSamplerResolver::checkLibrary() const
{
    if (!m_libHandler->isValid()) {
        LOGE() << "Incompatible MuseSampler library; ignoring";
        return false;
    }

    if (!isVersionSupported()) {
        LOGE() << "MuseSampler " << version() << " is not supported; ignoring";
        return false;
    }

    return true;
}

bool MuseSamplerResolver::isVersionSupported() const
{
    IF_ASSERT_FAILED(m_libHandler) {
        return false;
    }

    return isVersionAboveMinSupported() && isVersionBelowMaxSupported();
}

bool MuseSamplerResolver::isVersionAboveMinSupported() const
{
    bool ok = true;
    std::array<int, 3> minimumSupported = parseVersion(configuration()->minimumSupportedVersion(), ok);
    if (!ok) {
        return false;
    }

    int currentMajorNum = m_libHandler->getVersionMajor();
    int currentMinorNum = m_libHandler->getVersionMinor();
    int currentRevisionNum = m_libHandler->getVersionRevision();

    if (currentMajorNum > minimumSupported.at(0)) {
        return true;
    } else if (currentMajorNum == minimumSupported.at(0)) {
        if (currentMinorNum > minimumSupported.at(1)) {
            return true;
        } else if (currentMinorNum == minimumSupported.at(1)) {
            if (currentRevisionNum > minimumSupported.at(2)) {
                return true;
            } else if (currentRevisionNum == minimumSupported.at(2)) {
                return true;
            }
        }
    }

    return false;
}

bool MuseSamplerResolver::isVersionBelowMaxSupported() const
{
    bool ok = true;
    std::array<int, 3> maxSupported = parseVersion(configuration()->maximumSupportedVersion(), ok);
    if (!ok) {
        return false;
    }

    int currentMajorNum = m_libHandler->getVersionMajor();
    int currentMinorNum = m_libHandler->getVersionMinor();
    int currentRevisionNum = m_libHandler->getVersionRevision();

    if (currentMajorNum < maxSupported.at(0)) {
        return true;
    } else if (currentMajorNum == maxSupported.at(0)) {
        if (currentMinorNum < maxSupported.at(1)) {
            return true;
        } else if (currentMinorNum == maxSupported.at(1)) {
            if (currentRevisionNum < maxSupported.at(2)) {
                return true;
            } else if (currentRevisionNum == maxSupported.at(2)) {
                return true;
            }
        }
    }

    return false;
}

String MuseSamplerResolver::buildMuseInstrumentId(const String& category, const String& name, int uniqueId) const
{
    StringList list;
    list.append(category);
    list.append(name);
    list.append(String::fromStdString(std::to_string(uniqueId)));

    return list.join(u"\\");
}

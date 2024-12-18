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
#include "instrumentsrepository.h"

#include "global/serialization/json.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/types/types.h"

#include "mpe/playbacksetupdata.h"

#include "log.h"
#include "translation.h"

using namespace muse;
using namespace muse::io;
using namespace mu::notation;

static InstrumentGroup* createAndAddOtherGroup()
{
    InstrumentGroup* otherGroup = new InstrumentGroup();
    otherGroup->id = "other_group";
    otherGroup->name = mtrc("instruments", "Other");
    mu::engraving::instrumentGroups.push_back(otherGroup);

    return otherGroup;
}

static mu::engraving::ClefType museSamplerClefTypeToEngravingClefType(musesampler::ClefType type)
{
    switch (type) {
    case musesampler::ClefType::Treble: return mu::engraving::ClefType::G;
    case musesampler::ClefType::Bass: return mu::engraving::ClefType::F;
    case musesampler::ClefType::Alto: return mu::engraving::ClefType::C3;
    case musesampler::ClefType::Tenor: return mu::engraving::ClefType::C4;
    case musesampler::ClefType::Percussion: return mu::engraving::ClefType::PERC;
    case musesampler::ClefType::HigherOctaveTreble: return mu::engraving::ClefType::G8_VA;
    case musesampler::ClefType::LowerOctaveTreble: return mu::engraving::ClefType::G8_VB;
    case musesampler::ClefType::HigherOctaveBass: return mu::engraving::ClefType::F_8VA;
    case musesampler::ClefType::LowerOctaveBass: return mu::engraving::ClefType::F8_VB;
    case musesampler::ClefType::Baritone: return mu::engraving::ClefType::C5;
    case musesampler::ClefType::Mezzosoprano: return mu::engraving::ClefType::C2;
    case musesampler::ClefType::Soprano: return mu::engraving::ClefType::C1;
    case musesampler::ClefType::FrenchViolin: return mu::engraving::ClefType::G_1;
    case musesampler::ClefType::None: break;
    }

    return mu::engraving::ClefType::G;
}

static size_t staffCount(musesampler::StaffType type)
{
    switch (type) {
    case musesampler::StaffType::Standard: return 1;
    case musesampler::StaffType::Grand: return 2;
    }

    return 1;
}

void InstrumentsRepository::init()
{
    configuration()->scoreOrderListPathsChanged().onNotify(this, [this]() {
        load();
    });

    load();
}

const InstrumentTemplateList& InstrumentsRepository::instrumentTemplates() const
{
    return m_instrumentTemplateList;
}

const InstrumentTemplate& InstrumentsRepository::instrumentTemplate(const String& instrumentId) const
{
    auto it = m_instrumentTemplateMap.find(instrumentId);
    if (it == m_instrumentTemplateMap.end()) {
        static const InstrumentTemplate dummy;
        return dummy;
    }

    return *it->second;
}

const ScoreOrderList& InstrumentsRepository::orders() const
{
    return mu::engraving::instrumentOrders;
}

const ScoreOrder& InstrumentsRepository::order(const String& orderId) const
{
    const ScoreOrderList& orders = mu::engraving::instrumentOrders;

    auto it = std::find_if(orders.begin(), orders.end(), [orderId](const ScoreOrder& order) {
        return order.id == orderId;
    });

    if (it == orders.cend()) {
        static ScoreOrder dummy;
        return dummy;
    }

    return *it;
}

const InstrumentGenreList& InstrumentsRepository::genres() const
{
    return mu::engraving::instrumentGenres;
}

const InstrumentGroupList& InstrumentsRepository::groups() const
{
    return mu::engraving::instrumentGroups;
}

const InstrumentStringTuningsMap& InstrumentsRepository::stringTuningsPresets() const
{
    return m_stringTuningsPresets;
}

void InstrumentsRepository::load()
{
    TRACEFUNC;

    m_instrumentTemplateList.clear();
    m_instrumentTemplateMap.clear();
    mu::engraving::clearInstrumentTemplates();

    path_t instrumentsPath = configuration()->instrumentListPath();
    if (!mu::engraving::loadInstrumentTemplates(instrumentsPath)) {
        LOGE() << "Could not load instruments from " << instrumentsPath << "!";
    }

    for (const path_t& ordersPath : configuration()->scoreOrderListPaths()) {
        if (!mu::engraving::loadInstrumentTemplates(ordersPath)) {
            LOGE() << "Could not load orders from " << ordersPath << "!";
        }
    }

    InstrumentTemplateMap instrumentByMusicXmlId;

    for (const InstrumentGroup* group : mu::engraving::instrumentGroups) {
        for (const InstrumentTemplate* templ : group->instrumentTemplates) {
            if (templ->trackName.isEmpty() || templ->longNames.empty()) {
                continue;
            }

            m_instrumentTemplateList.push_back(templ);
            m_instrumentTemplateMap.insert_or_assign(templ->id, templ);
            instrumentByMusicXmlId.insert_or_assign(templ->musicXmlId, templ);
        }
    }

    loadMuseInstruments(instrumentByMusicXmlId);

    path_t stringTuningsPresetsPath = configuration()->stringTuningsPresetsPath();
    if (!loadStringTuningsPresets(stringTuningsPresetsPath)) {
        LOGE() << "Could not load string tunings presets from " << stringTuningsPresetsPath << "!";
    }
}

bool InstrumentsRepository::loadStringTuningsPresets(const path_t& path)
{
    TRACEFUNC;

    Ret ret = fileSystem()->exists(path);
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    RetVal<ByteArray> retVal = fileSystem()->readFile(path);
    if (!retVal.ret) {
        LOGE() << retVal.ret.toString();
        return false;
    }

    std::string err;
    JsonArray arr = JsonDocument::fromJson(retVal.val, &err).rootArray();
    if (!err.empty()) {
        LOGE() << "failed parse string tunings presets, err: " << err;
        return false;
    }

    for (size_t i = 0; i < arr.size(); ++i) {
        const JsonValue& presetInfoVal = arr.at(i);
        JsonObject presetInfoObj = presetInfoVal.toObject();

        std::vector<StringTuningsInfo> strings;

        JsonArray stringsArr = presetInfoObj.value("strings").toArray();
        for (size_t j = 0; j < stringsArr.size(); ++j) {
            StringTuningsInfo info;

            const JsonValue& stringVal = stringsArr.at(j);
            JsonObject stringObj = stringVal.toObject();

            info.number = stringObj.value("number").toInt();

            JsonArray presetsArr = stringObj.value("presets").toArray();
            for (size_t k = 0; k < presetsArr.size(); ++k) {
                const JsonValue& presetVal = presetsArr.at(k);
                JsonObject presetObj = presetVal.toObject();

                StringTuningPreset preset;
                preset.name = muse::trc("instruments/stringTunings", presetObj.value("name").toStdString().c_str());

                JsonArray valuesArr = presetObj.value("value").toArray();
                for (size_t l = 0; l < valuesArr.size(); ++l) {
                    const JsonValue& valueVal = valuesArr.at(l);
                    preset.value.push_back(valueVal.toInt());
                }

                if (info.number != preset.value.size()) {
                    LOGE() << "Invalid preset " << preset.name;
                    continue;
                }

                if (presetObj.contains("useFlats")) {
                    preset.useFlats = presetObj.value("useFlats").toBool();
                }

                info.presets.emplace_back(std::move(preset));
            }

            strings.emplace_back(std::move(info));
        }

        std::string id = presetInfoObj.contains("familyId") ? presetInfoObj.value("familyId").toStdString()
                         : presetInfoObj.value("instrumentId").toStdString();

        m_stringTuningsPresets.emplace(std::move(id), std::move(strings));
    }

    return true;
}

void InstrumentsRepository::loadMuseInstruments(const InstrumentTemplateMap& standardInstrumentByMusicXmlId)
{
    TRACEFUNC;

    if (!museSampler()) {
        return;
    }

    InstrumentGroup* otherGroup = nullptr;
    std::vector<musesampler::Instrument> instruments = museSampler()->instruments();

    for (const musesampler::Instrument& instrument : instruments) {
        mpe::PlaybackSetupData sound = mpe::PlaybackSetupData::fromString(instrument.soundId);
        if (!sound.isValid()) {
            continue;
        }

        if (sound.isKnownSound()) {
            continue;
        }

        InstrumentTemplate* templ = new InstrumentTemplate();
        templ->id = instrument.id;
        templ->soundId = instrument.soundId;
        templ->musicXmlId = instrument.musicXmlId;
        templ->trackName = instrument.name;
        templ->longNames.emplace_back(StaffName(instrument.name));
        templ->shortNames.emplace_back(StaffName(instrument.abbreviation));
        templ->staffCount = staffCount(instrument.staffType);
        mu::engraving::ClefType clefType = museSamplerClefTypeToEngravingClefType(instrument.clefType);
        templ->clefTypes[0].concertClef = clefType;
        templ->clefTypes[0].transposingClef = clefType;

        if (instrument.staffType == musesampler::StaffType::Grand) {
            templ->bracketSpan[0] = static_cast<int>(templ->staffCount);
            templ->barlineSpan[0] = static_cast<int>(templ->staffCount);

            for (size_t i = 0; i < templ->staffCount; ++i) {
                templ->bracket[i] = mu::engraving::BracketType::BRACE;
            }

            templ->clefTypes[1].concertClef = mu::engraving::ClefType::F;
            templ->clefTypes[1].transposingClef = mu::engraving::ClefType::F;
        }

        for (int i = 0; i < MAX_STAVES; ++i) {
            templ->staffLines[i] = static_cast<int>(instrument.staffLines);
        }

        if (!instrument.musicXmlId.empty()) {
            const InstrumentTemplate* standardTempl = muse::value(standardInstrumentByMusicXmlId, instrument.musicXmlId, nullptr);
            if (standardTempl) {
                templ->family = standardTempl->family;
                templ->groupId = standardTempl->groupId;
                templ->genres = standardTempl->genres;
            }
        }

        if (templ->groupId.empty()) {
            if (!otherGroup) {
                otherGroup = createAndAddOtherGroup();
            }
            templ->groupId = otherGroup->id;
        }

        if (templ->genres.empty()) {
            const InstrumentGenre* commonGenre = mu::engraving::searchInstrumentGenre(COMMON_GENRE_ID);
            if (commonGenre) {
                templ->genres.push_back(commonGenre);
            }
        }

        m_instrumentTemplateList.push_back(templ);
        m_instrumentTemplateMap.insert_or_assign(templ->id, templ);

        mu::engraving::addTemplateToGroup(templ, templ->groupId);
    }
}

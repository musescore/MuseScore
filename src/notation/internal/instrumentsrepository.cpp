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
#include "instrumentsrepository.h"

#include "global/serialization/json.h"

#include "log.h"
#include "translation.h"

#include "engraving/dom/instrtemplate.h"

using namespace mu::notation;

void InstrumentsRepository::init()
{
    configuration()->scoreOrderListPathsChanged().onNotify(this, [this]() {
        load();
    });

    load();
}

const InstrumentTemplateList& InstrumentsRepository::instrumentTemplates() const
{
    return m_instrumentTemplates;
}

const InstrumentTemplate& InstrumentsRepository::instrumentTemplate(const std::string& instrumentId) const
{
    const InstrumentTemplateList& templates = m_instrumentTemplates;

    auto it = std::find_if(templates.begin(), templates.end(), [instrumentId](const InstrumentTemplate* templ) {
        return templ->id == instrumentId;
    });

    if (it == m_instrumentTemplates.cend()) {
        static InstrumentTemplate dummy;
        return dummy;
    }

    const InstrumentTemplate* templ = *it;
    return *templ;
}

const ScoreOrderList& InstrumentsRepository::orders() const
{
    return mu::engraving::instrumentOrders;
}

const ScoreOrder& InstrumentsRepository::order(const std::string& orderId) const
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
    return m_genres;
}

const InstrumentGroupList& InstrumentsRepository::groups() const
{
    return m_groups;
}

const InstrumentStringTuningsMap& InstrumentsRepository::stringTuningsPresets() const
{
    return m_stringTuningsPresets;
}

void InstrumentsRepository::load()
{
    TRACEFUNC;

    m_instrumentTemplates.clear();
    m_genres.clear();
    m_groups.clear();
    mu::engraving::clearInstrumentTemplates();

    io::path_t instrumentsPath = configuration()->instrumentListPath();
    if (!mu::engraving::loadInstrumentTemplates(instrumentsPath)) {
        LOGE() << "Could not load instruments from " << instrumentsPath << "!";
    }

    for (const io::path_t& ordersPath : configuration()->scoreOrderListPaths()) {
        if (!mu::engraving::loadInstrumentTemplates(ordersPath)) {
            LOGE() << "Could not load orders from " << ordersPath << "!";
        }
    }

    for (const InstrumentGenre* genre : mu::engraving::instrumentGenres) {
        m_genres << genre;
    }

    for (const InstrumentGroup* group : mu::engraving::instrumentGroups) {
        m_groups << group;

        for (InstrumentTemplate* templ : group->instrumentTemplates) {
            if (templ->trackName.isEmpty() || templ->longNames.empty()) {
                continue;
            }

            templ->groupId = group->id;
            m_instrumentTemplates << templ;
        }
    }

    io::path_t stringTuningsPresetsPath = configuration()->stringTuningsPresetsPath();
    if (!loadStringTuningsPresets(stringTuningsPresetsPath)) {
        LOGE() << "Could not load string tunings presets from " << stringTuningsPresetsPath << "!";
    }
}

bool InstrumentsRepository::loadStringTuningsPresets(const io::path_t& path)
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
                preset.name = trc("instruments/stringTunings", presetObj.value("name").toStdString().c_str());

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

        m_stringTuningsPresets.emplace(id, strings);
    }

    return true;
}

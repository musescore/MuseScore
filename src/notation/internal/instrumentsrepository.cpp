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
}

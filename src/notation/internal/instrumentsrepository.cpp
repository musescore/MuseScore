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

#include "libmscore/instrtemplate.h"

using namespace mu::notation;

void InstrumentsRepository::init()
{
    configuration()->instrumentListPathsChanged().onNotify(this, [this]() {
        load();
    });

    configuration()->scoreOrderListPathsChanged().onNotify(this, [this]() {
        load();
    });

    load();
}

const InstrumentTemplateList& InstrumentsRepository::instrumentTemplates() const
{
    return m_instrumentTemplates;
}

const InstrumentGenreList& InstrumentsRepository::genres() const
{
    return m_genres;
}

const InstrumentGroupList& InstrumentsRepository::groups() const
{
    return m_groups;
}

const ScoreOrderList& InstrumentsRepository::orders() const
{
    return Ms::instrumentOrders;
}

void InstrumentsRepository::load()
{
    TRACEFUNC;

    m_instrumentTemplates.clear();
    m_genres.clear();
    m_groups.clear();
    Ms::clearInstrumentTemplates();

    for (const io::path& filePath: configuration()->instrumentListPaths()) {
        if (!Ms::loadInstrumentTemplates(filePath.toQString())) {
            LOGE() << "Could not load instruments from " << filePath.toQString() << "!";
        }
    }

    for (const InstrumentGenre* genre : Ms::instrumentGenres) {
        m_genres << genre;
    }

    for (const InstrumentGroup* group : Ms::instrumentGroups) {
        m_groups << group;

        for (InstrumentTemplate* templ : group->instrumentTemplates) {
            if (templ->trackName.isEmpty() || templ->longNames.isEmpty()) {
                continue;
            }

            templ->groupId = group->id;
            m_instrumentTemplates << templ;
        }
    }
}

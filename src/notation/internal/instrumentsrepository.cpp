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

mu::RetValCh<InstrumentsMeta> InstrumentsRepository::instrumentsMeta()
{
    RetValCh<InstrumentsMeta> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = m_instrumentsMeta;
    result.ch = m_instrumentsMetaChannel;

    return result;
}

void InstrumentsRepository::load()
{
    TRACEFUNC;

    m_instrumentsMeta.clear();
    Ms::clearInstrumentTemplates();

    for (const io::path& filePath: configuration()->instrumentListPaths()) {
        if (!Ms::loadInstrumentTemplates(filePath.toQString())) {
            LOGE() << "Could not load instruments from " << filePath.toQString() << "!";
        }
    }

    fillInstrumentsMeta(m_instrumentsMeta);
    m_instrumentsMetaChannel.send(m_instrumentsMeta);
}

void InstrumentsRepository::fillInstrumentsMeta(InstrumentsMeta& meta)
{
    TRACEFUNC;

    meta.articulations = Ms::articulation;

    for (const InstrumentGenre* genre : Ms::instrumentGenres) {
        meta.genres << genre;
    }

    for (const InstrumentGroup* group : Ms::instrumentGroups) {
        meta.groups << group;

        for (InstrumentTemplate* templ : group->instrumentTemplates) {
            if (templ->trackName.isEmpty() || templ->longNames.isEmpty()) {
                continue;
            }

            templ->groupId = group->id;
            meta.instrumentTemplates << templ;
        }
    }

    for (const ScoreOrder* order : Ms::instrumentOrders) {
        meta.scoreOrders << order;
    }
}

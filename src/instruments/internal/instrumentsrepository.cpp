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
#include "libmscore/articulation.h"

#include "notation/internal/instrumentsconverter.h"

using namespace mu;
using namespace mu::instruments;
using namespace mu::extensions;
using namespace mu::framework;

void InstrumentsRepository::init()
{
    if (extensionsService()) {
        RetCh<Extension> extensionChanged = extensionsService()->extensionChanged();
        if (extensionChanged.ret) {
            extensionChanged.ch.onReceive(this, [this](const Extension& newExtension) {
                if (newExtension.types.testFlag(Extension::Instruments)) {
                    load();
                }
            });
        }
    }

    configuration()->instrumentListPathsChanged().onNotify(this, [this]() {
        load();
    });

    configuration()->scoreOrderListPathsChanged().onNotify(this, [this]() {
        load();
    });

    load();
}

RetValCh<InstrumentsMeta> InstrumentsRepository::instrumentsMeta()
{
    QMutexLocker locker(&m_instrumentsMutex);
    RetValCh<InstrumentsMeta> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = m_instrumentsMeta;
    result.ch = m_instrumentsMetaChannel;

    return result;
}

void InstrumentsRepository::load()
{
    TRACEFUNC;

    QMutexLocker locker(&m_instrumentsMutex);
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

    meta.clear();
    meta.articulations = Ms::articulation;

    for (const Ms::InstrumentGenre* msGenre : Ms::instrumentGenres) {
        InstrumentGenre genre;
        genre.id = msGenre->id;
        genre.name = msGenre->name;

        meta.genres << genre;
    }

    for (const Ms::InstrumentGroup* msGroup : Ms::instrumentGroups) {
        InstrumentGroup group;
        group.id = msGroup->id;
        group.name = msGroup->name;
        group.extended = msGroup->extended;

        meta.groups << group;

        for (const Ms::InstrumentTemplate* msTemplate : msGroup->instrumentTemplates) {
            if (msTemplate->trackName.isEmpty() || msTemplate->longNames.isEmpty()) {
                continue;
            }

            Instrument templ = notation::InstrumentsConverter::convertInstrument(*msTemplate);
            templ.groupId = msGroup->id;

            meta.instrumentTemplates << templ;
        }
    }

    for (const Ms::ScoreOrder& msOrder : Ms::instrumentOrders) {
        ScoreOrder order;
        order.id = msOrder.id;
        order.name = msOrder.name;
        order.instrumentMap = msOrder.instrumentMap;

        meta.scoreOrders << order;
    }

    ScoreOrder custom;
    custom.id = "custom";
    custom.name = qApp->translate("OrderXML", "Custom");
    meta.scoreOrders << custom;
}

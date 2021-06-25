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
    clear();

    for (const io::path& filePath: configuration()->instrumentListPaths()) {
        if (!Ms::loadInstrumentTemplates(filePath.toQString())) {
            LOGE() << "Could not load instruments from " << filePath.toQString() << "!";
        }
    }

    fillInstrumentsMeta();
}

void InstrumentsRepository::clear()
{
    Ms::clearInstrumentTemplates();

    m_instrumentsMeta.instrumentTemplates.clear();
    m_instrumentsMeta.genres.clear();
    m_instrumentsMeta.groups.clear();
    m_instrumentsMeta.articulations.clear();
    m_instrumentsMeta.scoreOrders.clear();
}

void InstrumentsRepository::fillInstrumentsMeta()
{
    InstrumentsMeta metaInstrument;
    metaInstrument.articulations = Ms::articulation;

    for (const Ms::InstrumentGenre* msGenre : Ms::instrumentGenres) {
        InstrumentGenre genre;
        genre.id = msGenre->id;
        genre.name = msGenre->name;

        metaInstrument.genres << genre;
    }

    for (const Ms::InstrumentGroup* msGroup : Ms::instrumentGroups) {
        InstrumentGroup group;
        group.id = msGroup->id;
        group.name = msGroup->name;
        group.extended = msGroup->extended;

        metaInstrument.groups << group;
    }

    for (const Ms::ScoreOrder& msOrder : Ms::instrumentOrders) {
        ScoreOrder order;
        order.id = msOrder.id;
        order.name = msOrder.name;
        order.instrumentMap = msOrder.instrumentMap;

        metaInstrument.scoreOrders << order;
    }

    /*
    const InstrumentTemplateMap& templates = metaInstrument.val.instrumentTemplates;
    for (auto it = templates.cbegin(); it != templates.cend(); ++it) {
        metaInstrument.instrumentTemplates.insert(it.key(), it.value());
    }
    */

    ScoreOrder custom;
    custom.id = "custom";
    custom.name = qApp->translate("OrderXML", "Custom");
    m_instrumentsMeta.scoreOrders << custom;

    m_instrumentsMetaChannel.send(m_instrumentsMeta);

}

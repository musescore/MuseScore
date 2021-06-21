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
    QMutexLocker locker(&m_instrumentsMutex);

    clear();

    io::paths instrumentsFiles = configuration()->instrumentListPaths();

    int globalGroupsSequenceOrder = 0;
    auto correctGroupSequenceOrder = [&globalGroupsSequenceOrder](const InstrumentGroup& group) {
        InstrumentGroup correctedGroup = group;
        correctedGroup.sequenceOrder += globalGroupsSequenceOrder;
        return correctedGroup;
    };

    for (const io::path& filePath: instrumentsFiles) {
        RetVal<InstrumentsMeta> metaInstrument = reader()->readMeta(filePath);

        if (!metaInstrument.ret) {
            LOGE() << metaInstrument.ret.toString();
            continue;
        }

        const InstrumentTemplateMap& templates = metaInstrument.val.instrumentTemplates;
        for (auto it = templates.cbegin(); it != templates.cend(); ++it) {
            m_instrumentsMeta.instrumentTemplates.insert(it.key(), it.value());
        }

        const MidiArticulationMap& acticulations = metaInstrument.val.articulations;
        for (auto it = acticulations.cbegin(); it != acticulations.cend(); ++it) {
            m_instrumentsMeta.articulations.insert(it.key(), it.value());
        }

        const InstrumentGenreMap& genres = metaInstrument.val.genres;
        for (auto it = genres.cbegin(); it != genres.cend(); ++it) {
            m_instrumentsMeta.genres.insert(it.key(), it.value());
        }

        const InstrumentGroupMap& groups = metaInstrument.val.groups;
        for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
            InstrumentGroup group = correctGroupSequenceOrder(it.value());
            m_instrumentsMeta.groups.insert(it.key(), group);
        }
        globalGroupsSequenceOrder += groups.size();

        const ScoreOrderMap& scoreOrders = metaInstrument.val.scoreOrders;
        for (auto it = scoreOrders.cbegin(); it != scoreOrders.cend(); ++it) {
            m_instrumentsMeta.scoreOrders.insert(it.key(), it.value());
        }

        Ms::loadInstrumentTemplates(filePath.toQString());
    }

    ScoreOrder custom;
    custom.index = m_instrumentsMeta.scoreOrders.size();
    custom.id = "custom";
    custom.name = qApp->translate("OrderXML", "Custom");
    m_instrumentsMeta.scoreOrders.insert(custom.id, custom);

    for (InstrumentTemplate& instrumentTemplate: m_instrumentsMeta.instrumentTemplates) {
        instrumentTemplate.transposition = transposition(instrumentTemplate.id);
    }

    m_instrumentsMetaChannel.send(m_instrumentsMeta);
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

Transposition InstrumentsRepository::transposition(const QString& instrumentTemplateId) const
{
    static Transposition transpositions[] = {
        { "a-", qtrc("instruments", "A") },
        { "ab-", qtrc("instruments", "A♭") },
        { "b-", qtrc("instruments", "B") },
        { "bb-", qtrc("instruments", "B♭") },
        { "c-", qtrc("instruments", "C") },
        { "d-", qtrc("instruments", "D") },
        { "db-", qtrc("instruments", "D♭") },
        { "e-", qtrc("instruments", "E") },
        { "eb-", qtrc("instruments", "E♭") },
        { "f-", qtrc("instruments", "F") },
        { "g-", qtrc("instruments", "G") }
    };

    for (const Transposition& transposition: transpositions) {
        if (instrumentTemplateId.startsWith(transposition.id)) {
            return transposition;
        }
    }

    return Transposition();
}

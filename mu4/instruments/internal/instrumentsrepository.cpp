//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "instrumentsrepository.h"

#include "log.h"
#include "translation.h"

using namespace mu;
using namespace mu::instruments;
using namespace mu::extensions;
using namespace mu::framework;

void InstrumentsRepository::init()
{
    RetCh<Extension> extensionChanged = extensionsController()->extensionChanged();
    if (extensionChanged.ret) {
        extensionChanged.ch.onReceive(this, [this](const Extension& newExtension) {
            if (newExtension.types.testFlag(Extension::Instruments)) {
                load();
            }
        });
    }

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

    std::vector<io::path> instrumentsPaths = configuration()->instrumentPaths();
    std::vector<io::path> instrumentsFiles;

    for (const io::path& path: instrumentsPaths) {
        RetVal<std::vector<io::path> > files = fileSystem()->scanFiles(path, { QString("*.xml") }, IFileSystem::ScanMode::IncludeSubdirs);
        if (!files.ret) {
            LOGW() << files.ret.code() << files.ret.text();
        }

        for (const io::path& file: files.val) {
            instrumentsFiles.push_back(file);
        }
    }

    for (const io::path& file: instrumentsFiles) {
        RetVal<InstrumentsMeta> metaInstrument = reader()->readMeta(file);

        if (!metaInstrument.ret) {
            continue;
        }

        m_instrumentsMeta.instrumentTemplates.unite(metaInstrument.val.instrumentTemplates);
        m_instrumentsMeta.articulations.unite(metaInstrument.val.articulations);
        m_instrumentsMeta.genres.unite(metaInstrument.val.genres);
        m_instrumentsMeta.groups.unite(metaInstrument.val.groups);
    }

    for (InstrumentTemplate& instrumentTemplate: m_instrumentsMeta.instrumentTemplates) {
        instrumentTemplate.transposition = transposition(instrumentTemplate.id);
    }

    m_instrumentsMetaChannel.send(m_instrumentsMeta);
}

void InstrumentsRepository::clear()
{
    m_instrumentsMeta.instrumentTemplates.clear();
    m_instrumentsMeta.genres.clear();
    m_instrumentsMeta.groups.clear();
    m_instrumentsMeta.articulations.clear();
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

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

#include "libmscore/instrtemplate.h"

using namespace mu;
using namespace mu::instruments;
using namespace mu::extensions;
using namespace mu::framework;

void InstrumentsRepository::init()
{
    RetCh<Extension> extensionChanged;
    if (extensionsService()) {
        extensionChanged = extensionsService()->extensionChanged();
    }
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

    io::paths instrumentsPaths = configuration()->instrumentPaths();
    io::paths instrumentsFiles;

    for (const io::path& path: instrumentsPaths) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, { QString("*.xml") });
        if (!files.ret) {
            LOGE() << files.ret.toString();
        }

        for (const io::path& file: files.val) {
            instrumentsFiles.push_back(file);
        }
    }

    int globalGroupsSequenceOrder = 0;

    for (const io::path& filePath: instrumentsFiles) {
        RetVal<InstrumentsMeta> metaInstrument = reader()->readMeta(filePath);

        if (!metaInstrument.ret) {
            LOGE() << metaInstrument.ret.toString();
            continue;
        }

        m_instrumentsMeta.templates << metaInstrument.val.templates;
        m_instrumentsMeta.articulations << metaInstrument.val.articulations;
        m_instrumentsMeta.genres << metaInstrument.val.genres;

        for (InstrumentGroup& group : metaInstrument.val.groups) {
            group.sequenceOrder += globalGroupsSequenceOrder;
            m_instrumentsMeta.groups << group;
        }

        globalGroupsSequenceOrder += metaInstrument.val.groups.size();

        Ms::loadInstrumentTemplates(filePath.toQString());
    }

    m_instrumentsMetaChannel.send(m_instrumentsMeta);
}

void InstrumentsRepository::clear()
{
    Ms::clearInstrumentTemplates();

    m_instrumentsMeta.templates.clear();
    m_instrumentsMeta.genres.clear();
    m_instrumentsMeta.groups.clear();
    m_instrumentsMeta.articulations.clear();
}

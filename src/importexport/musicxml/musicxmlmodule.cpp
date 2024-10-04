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
#include "musicxmlmodule.h"

#include "modularity/ioc.h"

#ifndef MUSICXML_NO_INTERNAL
#include "project/inotationreadersregister.h"
#include "internal/musicxmlreader.h"
#include "project/inotationwritersregister.h"
#include "internal/musicxmlwriter.h"
#include "internal/musicxmlwriter.h"
#include "internal/mxlwriter.h"
#include "internal/musicxmlconfiguration.h"
#endif

#include "log.h"

using namespace muse;
using namespace mu::iex::musicxml;

static void musicxml_init_qrc()
{
#ifndef MUSICXML_NO_INTERNAL
    Q_INIT_RESOURCE(musicxml);
#endif
}

std::string MusicXmlModule::moduleName() const
{
    return "iex_musicxml";
}

void MusicXmlModule::registerResources()
{
    musicxml_init_qrc();
}

void MusicXmlModule::registerExports()
{
#ifndef MUSICXML_NO_INTERNAL
    m_configuration = std::make_shared<MusicXmlConfiguration>();
    ioc()->registerExport<IMusicXmlConfiguration>(moduleName(), m_configuration);
#endif
}

void MusicXmlModule::resolveImports()
{
#ifndef MUSICXML_NO_INTERNAL
    using namespace mu::project;

    auto readers = ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "xml", "musicxml", "mxl" }, std::make_shared<MusicXmlReader>());
    }

    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "musicxml", "xml" }, std::make_shared<MusicXmlWriter>());
        writers->reg({ "mxl" }, std::make_shared<MxlWriter>());
    }
#endif
}

void MusicXmlModule::onInit(const IApplication::RunMode&)
{
#ifndef MUSICXML_NO_INTERNAL
    m_configuration->init();
#endif
}

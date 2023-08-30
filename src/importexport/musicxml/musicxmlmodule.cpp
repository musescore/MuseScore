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
#include "musicxmlmodule.h"

#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "internal/musicxmlreader.h"
#include "project/inotationwritersregister.h"
#include "internal/musicxmlwriter.h"
#include "internal/musicxmlwriter.h"
#include "internal/mxlwriter.h"

#include "internal/musicxmlconfiguration.h"

#include "log.h"

using namespace mu::iex::musicxml;
using namespace mu::project;

static void musicxml_init_qrc()
{
    Q_INIT_RESOURCE(musicxml);
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
    m_configuration = std::make_shared<MusicXmlConfiguration>();

    modularity::ioc()->registerExport<IMusicXmlConfiguration>(moduleName(), m_configuration);
}

void MusicXmlModule::resolveImports()
{
    auto readers = modularity::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "xml", "musicxml", "mxl" }, std::make_shared<MusicXmlReader>());
    }

    auto writers = modularity::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "musicxml", "xml" }, std::make_shared<MusicXmlWriter>());
        writers->reg({ "mxl" }, std::make_shared<MxlWriter>());
    }
}

void MusicXmlModule::onInit(const framework::IApplication::RunMode&)
{
    m_configuration->init();
}

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

#include "log.h"
#include "config.h"
#include "modularity/ioc.h"

#include "notation/inotationreadersregister.h"
#include "internal/musicxmlreader.h"
#include "notation/inotationwritersregister.h"
#include "internal/musicxmlwriter.h"
#include "internal/musicxmlwriter.h"
#include "internal/mxlwriter.h"

#include "internal/musicxmlconfiguration.h"

using namespace mu::iex::musicxml;
using namespace mu::notation;

static std::shared_ptr<MusicXmlConfiguration> s_configuration = std::make_shared<MusicXmlConfiguration>();

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
    modularity::ioc()->registerExport<IMusicXmlConfiguration>(moduleName(), s_configuration);
}

void MusicXmlModule::resolveImports()
{
    s_configuration->init();

    auto readers = modularity::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "xml", "musicxml", "mxl" }, std::make_shared<MusicXmlReader>());
    }

    auto writers = modularity::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "musicxml", "xml" }, std::make_shared<MusicXmlWriter>());
        writers->reg({ "mxl", "mxml" }, std::make_shared<MxlWriter>());
    }
}

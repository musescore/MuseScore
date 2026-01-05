/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "lyricsexportmodule.h"

#include "modularity/ioc.h"

#include "project/inotationwritersregister.h"

#include "internal/lrcwriter.h"
#include "internal/lyricsexportconfiguration.h"

using namespace muse;
using namespace mu::iex::lrcexport;
using namespace mu::project;

std::string LyricsExportModule::moduleName() const
{
    return "iex_lyricsexport";
}

void LyricsExportModule::registerExports()
{
    m_configuration = std::make_shared<LyricsExportConfiguration>();
    ioc()->registerExport<ILyricsExportConfiguration>(moduleName(), m_configuration);
}

void LyricsExportModule::resolveImports()
{
    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "lrc" }, std::make_shared<LRCWriter>(iocContext()));
    }
}

void LyricsExportModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

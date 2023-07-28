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
#include "guitarpromodule.h"

#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "internal/guitarproreader.h"
#include "internal/guitarproconfiguration.h"

#include "log.h"

using namespace mu::iex::guitarpro;
using namespace mu::project;

std::string GuitarProModule::moduleName() const
{
    return "iex_guitarpro";
}

void GuitarProModule::registerExports()
{
    m_configuration = std::make_shared<GuitarProConfiguration>();

    modularity::ioc()->registerExport<IGuitarProConfiguration>(moduleName(), m_configuration);
}

void GuitarProModule::resolveImports()
{
    auto readers = modularity::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "gtp", "gp3", "gp4", "gp5", "gpx", "gp", "ptb" }, std::make_shared<GuitarProReader>());
    }
}

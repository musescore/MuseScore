/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Wires the Encore module into the IoC: exports the configuration, registers the reader for "enc".

#include "enc-module.h"

#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "internal/notationencreader.h"
#include "internal/enc-importconfiguration.h"
#include "ienc-importconfiguration.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace mu::iex::enc;
using namespace mu::project;

std::string EncoreModule::moduleName() const
{
    return "iex_encore";
}

void EncoreModule::registerExports()
{
    m_configuration = std::make_shared<EncImportConfiguration>();
    globalIoc()->registerExport<IEncImportConfiguration>(moduleName(), m_configuration);
}

void EncoreModule::resolveImports()
{
    auto readers = globalIoc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "enc" }, std::make_shared<NotationEncoreReader>());
    }
}

void EncoreModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

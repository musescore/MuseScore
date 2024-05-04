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
#include "ovemodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "internal/overeader.h"

#include "internal/oveconfiguration.h"

using namespace muse;
using namespace mu::iex::ove;
using namespace mu::project;

std::string OveModule::moduleName() const
{
    return "iex_ove";
}

void OveModule::registerExports()
{
    m_configuration = std::make_shared<OveConfiguration>();

    ioc()->registerExport<IOveConfiguration>(moduleName(), m_configuration);
}

void OveModule::resolveImports()
{
    auto readers = ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "ove", "scw" }, std::make_shared<OveReader>());
    }
}

void OveModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
}

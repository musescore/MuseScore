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
#include "ovemodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationreadersregister.h"
#include "internal/overeader.h"

#include "internal/oveconfiguration.h"

using namespace mu::iex::ove;
using namespace mu::notation;

static std::shared_ptr<OveConfiguration> s_configuration = std::make_shared<OveConfiguration>();

std::string OveModule::moduleName() const
{
    return "iex_ove";
}

void OveModule::registerExports()
{
    modularity::ioc()->registerExport<IOveConfiguration>(moduleName(), s_configuration);
}

void OveModule::resolveImports()
{
    auto readers = modularity::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "ove", "scw" }, std::make_shared<OveReader>());
    }
}

void OveModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();
}

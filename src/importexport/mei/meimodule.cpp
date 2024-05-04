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
#include "meimodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "project/inotationwritersregister.h"
#include "internal/meireader.h"
#include "internal/meiwriter.h"

#include "internal/meiconfiguration.h"

using namespace muse;
using namespace mu::iex::mei;
using namespace mu::project;

std::string MeiModule::moduleName() const
{
    return "iex_mei";
}

void MeiModule::registerExports()
{
    m_configuration = std::make_shared<MeiConfiguration>();

    ioc()->registerExport<IMeiConfiguration>(moduleName(), m_configuration);
}

void MeiModule::resolveImports()
{
    auto readers = ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "mei" }, std::make_shared<MeiReader>());
    }

    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "mei" }, std::make_shared<MeiWriter>());
    }
}

void MeiModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
}

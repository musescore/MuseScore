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
#include "finalemodule.h"

#include "modularity/ioc.h"

#include "project/inotationreadersregister.h"
#include "internal/notationfinalereader.h"

using namespace muse::modularity;
using namespace mu::iex::finale;
using namespace mu::project;

std::string FinaleModule::moduleName() const
{
    return "iex_finale";
}

void FinaleModule::resolveImports()
{
    auto readers = ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "musx", "enigmaxml" }, std::make_shared<NotationFinaleReader>());
    }
}

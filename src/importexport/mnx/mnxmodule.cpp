/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "mnxmodule.h"

#include "project/inotationreadersregister.h"
#include "project/inotationwritersregister.h"
#include "internal/notationmnxreader.h"
#include "internal/notationmnxwriter.h"

using namespace muse::modularity;
using namespace mu::iex::mnxio;
using namespace mu::project;

std::string MnxModule::moduleName() const
{
    return "iex_mnx";
}

void MnxModule::resolveImports()
{
    auto readers = ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "mnx", "json" }, std::make_shared<NotationMnxReader>());
    }
    auto writers = ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "mnx" }, std::make_shared<NotationMnxWriter>());
    }
}

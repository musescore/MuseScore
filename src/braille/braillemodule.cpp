/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "braillemodule.h"

#include "modularity/ioc.h"
#include "project/inotationwritersregister.h"

#include "internal/braillewriter.h"

using namespace mu::engraving;
using namespace mu::project;

namespace mu::braille {
std::string BrailleModule::moduleName() const
{
    return "braille";
}

void BrailleModule::resolveImports()
{
    auto writers = modularity::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "brf" }, std::make_shared<BrailleWriter>());
    }
}
}

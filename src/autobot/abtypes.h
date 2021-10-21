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
#ifndef MU_AUTOBOT_ABTYPES_H
#define MU_AUTOBOT_ABTYPES_H

#include <string>
#include <vector>

#include "io/path.h"
#include "ret.h"

namespace mu::autobot {
struct File {
    io::path path;
    Ret completeRet; // if undefined - means not tested
};

using Files = std::vector<File>;

struct Script
{
    io::path path;
    QString title;
    QString description;
};

using Scripts = std::vector<Script>;
}

#endif // MU_AUTOBOT_AUTOBOTTYPES_H

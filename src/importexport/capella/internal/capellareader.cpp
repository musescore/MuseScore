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
#include "capellareader.h"

#include "io/path.h"

#include "libmscore/score.h"
#include "engraving/engravingerrors.h"

using namespace mu::iex::capella;
using namespace mu::engraving;

namespace mu::iex::capella {
extern Err importCapella(MasterScore*, const QString& name);
extern Err importCapXml(MasterScore*, const QString& name);
}

mu::Ret CapellaReader::read(MasterScore* score, const io::path_t& path, const Options&)
{
    Err err = Err::FileUnknownType;
    std::string suffix = mu::io::suffix(path);
    if (suffix == "cap") {
        err = importCapella(score, path.toQString());
    } else if (suffix == "capx") {
        err = importCapXml(score, path.toQString());
    }
    return make_ret(err, path);
}

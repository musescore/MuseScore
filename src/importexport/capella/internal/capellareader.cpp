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

#include "libmscore/masterscore.h"
#include "engraving/engravingerrors.h"

namespace mu::engraving {
extern Score::FileError importCapella(MasterScore*, const QString& name);
extern Score::FileError importCapXml(MasterScore*, const QString& name);
}

using namespace mu::iex::capella;

mu::Ret CapellaReader::read(mu::engraving::MasterScore* score, const io::path_t& path, const Options&)
{
    mu::engraving::Score::FileError err = mu::engraving::Score::FileError::FILE_UNKNOWN_TYPE;
    std::string suffix = mu::io::suffix(path);
    if (suffix == "cap") {
        err = mu::engraving::importCapella(score, path.toQString());
    } else if (suffix == "capx") {
        err = mu::engraving::importCapXml(score, path.toQString());
    }
    return mu::engraving::scoreFileErrorToRet(err, path);
}

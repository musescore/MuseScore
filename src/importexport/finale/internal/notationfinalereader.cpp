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
#include "notationfinalereader.h"
#include "importenigmaxml.h"

#include "engraving/dom/score.h"
#include "engraving/engravingerrors.h"

using namespace mu::iex::finale;
using namespace mu::engraving;

muse::Ret NotationFinaleReader::read(MasterScore* score, const muse::io::path_t& path, const Options&)
{
    Err err = Err::FileUnknownType;
    std::string suffix = muse::io::suffix(path);

    if (suffix == "enigmaxml") {
        err = importEnigmaXml(score, path.toString());
    } else if (suffix == "musx") {
        err = importMusx(score, path.toString());
    }
    return make_ret(err, path);
}

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/dom/score.h"
#include "engraving/engravingerrors.h"

using namespace mu::iex::capella;
using namespace mu::engraving;

namespace mu::iex::capella {
extern muse::Ret importCapella(MasterScore*, const QString& name, bool forceMode);
extern Err importCapXml(MasterScore*, const QString& name);
}

muse::Ret CapellaReader::read(MasterScore* score, const muse::io::path_t& path, const Options& options)
{
    std::string suffix = muse::io::suffix(path);
    if (suffix == "cap") {
        bool forceMode = muse::value(options, OptionKey::ForceMode, muse::Val(false)).toBool();
        return importCapella(score, path.toQString(), forceMode);
    }

    Err err = Err::FileUnknownType;
    if (suffix == "capx") {
        err = importCapXml(score, path.toQString());
    }
    return make_ret(err, path);
}

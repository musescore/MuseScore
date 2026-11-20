/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "engraving/dom/score.h"
#include "engraving/engravingerrors.h"

#include "importtef.h"
#include "tableditreader.h"

using namespace mu::iex::tabledit;
using namespace mu::engraving;

muse::Ret TablEditReader::read(MasterScore* score, const muse::io::path_t& path, const Options&)
{
    TablEdit tablEdit{ score };
    Err err = tablEdit.import(path);
    if (err == Err::FileBadFormat) {
        // Give a specific reason instead of the generic "Bad format" text.
        return make_ret(err, tablEdit.errorMessage());
    }

    return make_ret(err, path);
}

Err TablEditReader::import(MasterScore* score, const muse::io::path_t& path, const Options&)
{
    TablEdit tablEdit{ score };
    Err err = tablEdit.import(path);

    return err;
}

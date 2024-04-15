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
#include "notationbwwreader.h"

#include "engraving/dom/score.h"
#include "engraving/engravingerrors.h"

using namespace mu::iex::bww;
using namespace mu::engraving;

namespace mu::iex::bww {
extern Err importBww(MasterScore*, const QString& name);
}

muse::Ret NotationBwwReader::read(MasterScore* score, const muse::io::path_t& path, const Options&)
{
    Err err = importBww(score, path.toQString());
    return make_ret(err, path);
}

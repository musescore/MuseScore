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

#include <QFileInfo>

#include "musedatareader.h"

#include "musedata.h"

#include "engraving/dom/masterscore.h"
#include "engraving/engravingerrors.h"

using namespace mu::iex::musedata;
using namespace mu::engraving;

mu::Ret MuseDataReader::read(MasterScore* score, const io::path_t& path, const Options&)
{
    if (!QFileInfo::exists(path.toQString())) {
        return make_ret(Err::FileNotFound, path);
    }

    MuseData md(score);
    if (!md.read(path.toQString())) {
        return make_ret(Err::FileUnknownError, path);
    }

    md.convert();
    return make_ok();
}

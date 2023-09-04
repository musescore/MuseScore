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
#include "guitarproreader.h"

#include "io/file.h"

#include "engraving/dom/masterscore.h"
#include "engraving/engravingerrors.h"

namespace mu::iex::guitarpro {
extern mu::engraving::Err importGTP(mu::engraving::MasterScore*, mu::io::IODevice* io, bool createLinkedTabForce = false,
                                    bool experimental = false);

mu::Ret GuitarProReader::read(mu::engraving::MasterScore* score, const io::path_t& path, const Options&)
{
    mu::io::File file(path);
    mu::engraving::Err err = importGTP(score, &file, guitarProConfiguration()->linkedTabStaffCreated(),
                                       guitarProConfiguration()->experimental());
    return mu::engraving::make_ret(err, path);
}
} // namespace mu::iex::guitarpro

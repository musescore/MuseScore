/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Shared importer state: the BuildCtx threaded through every phase, the deferred "pending" element
// structs drained in the post-pass, and the emit-phase scratch tables.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_CTX_H
#define MU_IMPORTEXPORT_ENC_IMPORT_CTX_H

#include "import-options.h"

#include <array>
#include <map>
#include <set>
#include <memory>
#include <vector>

#include <QtGlobal>

#include "../parser/elem.h"
#include "../parser/readers.h"
#include "engraving/types/fraction.h"
#include "engraving/dom/masterscore.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// LINE staff-data entry for the given running staff index, or nullptr when absent/out of range.
// Centralizes the "lines non-empty + index in range" guard used by the part/staff routing.
inline const EncLineStaffData* lineStaffDataAt(const EncRoot& enc, int idx)
{
    if (enc.lines.empty() || idx < 0
        || idx >= static_cast<int>(enc.lines[0].staffData.size())) {
        return nullptr;
    }
    return &enc.lines[0].staffData[static_cast<size_t>(idx)];
}

struct BuildCtx
{
    mu::engraving::MasterScore* score;
    const EncRoot& enc;
    EncImportOptions opts;

    // Populated by buildParts():
    int totalStaves = 0;
    std::vector<int> staffPitchOffset {};
    std::vector<ClefType> staffTemplateConcertClef {};

};
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_CTX_H

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

// Page and printer geometry for the Encore importer: paper size/orientation/scale from the
// PREC block, margins from the WINI block, the SCO5 uniform-margin default, and the
// system-lock / page-break line layout derived from LINE blocks.

#pragma once

namespace mu::iex::enc {
struct BuildCtx;
struct EncPrintSetup;
struct EncRoot;


// Derive the display size index (1-4) for an instrument: per-instrument LINE staff-size hint
// when present, otherwise the global header score-size fallback. Used by applyStaffScale and
// the import debug summary.
int staffDisplaySize(const EncRoot& enc, int instrIdx);



} // namespace mu::iex::enc

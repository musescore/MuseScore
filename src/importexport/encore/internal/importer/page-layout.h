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

// Resolve the page size (inches) from the PREC (DEVMODE) block: dmPaperSize enum, falling back
// to dmPaperLength/Width for custom sizes, with the landscape swap applied. Returns false when
// PREC has no usable size. Exposed for the import debug summary; applyPageSetup uses it too.
bool precPageSizeInches(const EncPrintSetup& pr, double& wIn, double& hIn);

// Derive the display size index (1-4) for an instrument: per-instrument LINE staff-size hint
// when present, otherwise the global header score-size fallback. Used by applyStaffScale and
// the import debug summary.
int staffDisplaySize(const EncRoot& enc, int instrIdx);

// Resolve the WINI margin unit (units per inch) from the printable extent: (rightEdge + left)
// over the page width is ~72 when WINI is in typographic points and ~84 when it is screen
// pixels at the monitor DPI. The near-72 case snaps to exactly 72.
double winiUnitsPerInch(int rightEdge, int left, double pageWIn);

// Apply Encore's page geometry to the score in buildScore's layout phase: page
// size/orientation/scale (PREC), margins (WINI), the SCO5 uniform-margin default, and the
// system-lock / page-break line layout. Each part is gated by its matching import option.
void applyPageSetup(BuildCtx& ctx);

// When imported page breaks pushed a first-page system onto page 2, shrink the staff space
// (spatium) by the smallest step (up to 0.022 inch in 0.002 increments) that pulls the first
// break's measure back onto page 1. No-op unless page breaks were imported; run after full layout.
void fitFirstPageStaffSpace(BuildCtx& ctx);
} // namespace mu::iex::enc

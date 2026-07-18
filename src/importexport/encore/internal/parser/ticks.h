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

// Tick model for the Encore parser: face value to ticks and implied-tuplet detection, engraving-free.

#ifndef MU_IMPORTEXPORT_ENC_PARSER_TICKS_H
#define MU_IMPORTEXPORT_ENC_PARSER_TICKS_H

#include <QtGlobal>

namespace mu::iex::enc {
// Encore's internal tick resolution: 960 ticks per whole note (= 240 per quarter).
// This is fixed for all format versions and all time signatures.
inline constexpr int kEncWholeTicks = 960;

// Raw tick table: Encore face value (low nibble) to ticks. Pure integer model, no engraving
// types, so the parser can use it without depending on engraving/dom. The rendering decisions
// derived from these ticks (DurationType, dot count, tuplet shape) live in importer/durations.h.
int faceValue2ticks(quint8 fv);

// Inverse of faceValue2ticks: the face value whose base duration is the largest that fits in
// `ticks` (so a dotted/tuplet duration maps to its undotted base; dots come from realDuration).
// Used to give a face value to notes materialized from tab-only staves, whose source elements
// store no face value. Returns quarter (3) as a safe fallback.
quint8 ticks2faceValue(int ticks);

// Pure-integer implied-tuplet probe: returns the tuplet's actualN (with normalNotes set) when
// realDur is a 3:2 or 5:4 augmentation of the face value, else 0. Used by the v0xC2 parser pass
// and by the importer; carries no engraving dependency, so it stays in the parser layer.
int detectImpliedTuplet(qint16 realDur, quint8 fv, int& normalNotes);
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_PARSER_TICKS_H

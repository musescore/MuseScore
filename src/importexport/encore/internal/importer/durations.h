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
#pragma once

// Rendering decisions derived from Encore's raw ticks: MuseScore DurationType, dot count and
// tuplet shape. These are importer concerns (they translate the parser's raw tick/face-value
// model into engraving types), so they live in the importer layer and keep the engraving/dom
// dependency out of the parser. The parser exposes only the raw tick table (faceValue2ticks)
// and the pure-integer implied-tuplet probe (detectImpliedTuplet) in parser/ticks.h.

#include <QtGlobal>

#include "engraving/dom/durationtype.h"

namespace mu::iex::enc {
mu::engraving::DurationType faceValue2DurationType(quint8 fv);
mu::engraving::DurationType realDuration2DurationType(qint16 realDur, quint8 fv);
int calcDots(qint16 realDur, quint8 fv);
int calcDotsSnap(qint16 dur, quint8 fv);
mu::engraving::Fraction dottedAdvance(mu::engraving::DurationType durationType, int dots);

// Dot-count computation for note/rest handlers. See ENCORE_FORMAT.md §Note element (dotControl).
// When useBit0Fallback=true (notes only), bit 0 of dotControl is Encore's dotted flag.
int computeDotCount(quint8 dotControl, qint16 realDuration, quint8 faceValue, bool useBit0Fallback = false);

// Standard Encore tuplet ratios (3:2, 4:3, 5:4, 6:4).
// Other actualN:normalN pairs are MIDI timing noise; caller should zero them.
bool isStandardExplicitTuplet(int actualN, int normalN);

// True when a measure's beat is a dotted quarter (compound feel): either the raw beatTicks is the
// explicit 360, or the time signature is a compound x/8 (6/8, 9/8, 12/8, ...) whose legacy files
// still store beatTicks=240. Used for tempo-mark beat-unit display. See ENCORE_IMPORTER.md.
bool isCompoundBeat(quint16 rawBeatTicks, mu::engraving::Fraction timesig);
} // namespace mu::iex::enc

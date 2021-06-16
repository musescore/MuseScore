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
#ifndef IMPORTMIDI_TUPLET_FILTER_H
#define IMPORTMIDI_TUPLET_FILTER_H

#include <vector>

namespace Ms {
class ReducedFraction;

namespace MidiTuplet {
struct TupletInfo;

void filterTuplets(std::vector<TupletInfo>& tuplets, const ReducedFraction& basicQuant);
} // namespace MidiTuplet
} // namespace Ms

#endif // IMPORTMIDI_TUPLET_FILTER_H

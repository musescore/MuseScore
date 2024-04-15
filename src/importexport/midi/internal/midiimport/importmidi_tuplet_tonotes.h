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
#ifndef IMPORTMIDI_TUPLET_TONOTES_H
#define IMPORTMIDI_TUPLET_TONOTES_H

#include <map>

namespace mu::engraving {
class DurationElement;
class Staff;
}

namespace mu::iex::midi {
class ReducedFraction;

namespace MidiTuplet {
struct TupletData;

void addElementToTuplet(int voice, const ReducedFraction& onTime, const ReducedFraction& len, engraving::DurationElement* el,
                        std::multimap<ReducedFraction, TupletData>& tuplets);

void createTupletNotes(engraving::Staff* staff, const std::multimap<ReducedFraction, TupletData>& tuplets);

#ifdef QT_DEBUG
bool haveTupletsEnoughElements(const engraving::Staff* staff);
#endif
} // namespace MidiTuplet
} // namespace mu::iex::midi

#endif // IMPORTMIDI_TUPLET_TONOTES_H

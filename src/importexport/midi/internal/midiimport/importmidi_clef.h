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
#ifndef IMPORTMIDI_CLEF_H
#define IMPORTMIDI_CLEF_H

#include "engraving/types/types.h"

namespace mu::engraving {
class Staff;
class InstrumentTemplate;
}

namespace mu::iex::midi {
namespace MidiClef {
bool hasGFclefs(const engraving::InstrumentTemplate* templ);
void createClefs(engraving::Staff* staff, int indexOfOperation, bool isDrumTrack);
engraving::ClefType clefTypeFromAveragePitch(int averagePitch);
} // namespace MidiClef
} // namespace mu::iex::midi

#endif // IMPORTMIDI_CLEF_H

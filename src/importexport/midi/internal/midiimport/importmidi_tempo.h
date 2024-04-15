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
#ifndef IMPORTMIDI_TEMPO_H
#define IMPORTMIDI_TEMPO_H

#include <map>

namespace mu::engraving {
class Score;
}

namespace mu::iex::midi {
class MTrack;
class ReducedFraction;

namespace MidiTempo {
ReducedFraction time2Tick(double time, double ticksPerSec);
double findBasicTempo(const std::multimap<int, MTrack>& tracks, bool isHumanPerformance);
void setTempo(const std::multimap<int, MTrack>& tracks, engraving::Score* score);
} // namespace MidiTempo
} // namespace mu::iex::midi

#endif // IMPORTMIDI_TEMPO_H

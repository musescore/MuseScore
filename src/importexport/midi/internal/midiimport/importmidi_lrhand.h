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
#pragma once

#include <map>

#include "internal/midishared/generalmidi.h"

namespace mu::iex::midi {
class MTrack;
class MidiChord;
class ReducedFraction;

namespace LRHand {
bool needToSplit(const std::multimap<ReducedFraction, MidiChord>& chords, GM1Program, bool isDrumTrack);

void splitIntoLeftRightHands(std::multimap<int, MTrack>& tracks);
} // namespace LRHand
} // namespace mu::iex::midi

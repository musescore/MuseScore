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
#ifndef IMPORTMIDI_DRUM_H
#define IMPORTMIDI_DRUM_H

#include <map>
#include <QList>

namespace Ms {
class MTrack;
class TimeSigMap;

namespace MidiDrum {
void splitDrumVoices(std::multimap<int, MTrack>& tracks);
void splitDrumTracks(std::multimap<int, MTrack>& tracks);
void setStaffBracketForDrums(QList<MTrack>& tracks);
} // namespace MidiDrum
} // namespace Ms

#endif // IMPORTMIDI_DRUM_H

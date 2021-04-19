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
#ifndef IMPORTMIDI_INSTRUMENT_H
#define IMPORTMIDI_INSTRUMENT_H

#include "framework/midi_old/midifile.h"

class QString;

namespace Ms {
class Score;
class MTrack;

namespace MidiInstr {
QString instrumentName(MidiType type, int program, bool isDrumTrack);
QString msInstrName(int trackIndex);
QString concatenateWithComma(const QString& left, const QString& right);
bool isGrandStaff(const MTrack& t1, const MTrack& t2);
void setGrandStaffProgram(QList<MTrack>& tracks);
void findInstrumentsForAllTracks(const QList<MTrack>& tracks, bool forceReload = false);
void createInstruments(Score* score, QList<MTrack>& tracks);

extern void instrumentTemplatesChanged();
} // namespace MidiInstr
} // namespace Ms

#endif // IMPORTMIDI_INSTRUMENT_H

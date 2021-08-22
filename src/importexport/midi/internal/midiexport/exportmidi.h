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

#ifndef EXPORTMIDI_H
#define EXPORTMIDI_H

#include <QFile>

#include "engraving/compat/midi/midifile.h"

namespace Ms {
class Score;
class TempoMap;
class SynthesizerState;

//---------------------------------------------------------
//   ExportMidi
//---------------------------------------------------------

class ExportMidi
{
public:
    ExportMidi(Score* s) { m_score = s; }
    bool write(const QString& name, bool midiExpandRepeats, bool exportRPNs);
    bool write(QIODevice* device, bool midiExpandRepeats, bool exportRPNs);
    bool write(const QString& name, bool midiExpandRepeats, bool exportRPNs, const SynthesizerState& synthState);
    bool write(QIODevice* device, bool midiExpandRepeats, bool exportRPNs, const SynthesizerState& synthState);

private:
    //---------------------------------------------------
    //   PauseMap
    //    MIDI files cannot contain pauses so need to insert
    //    extra ticks extra ticks and tempo changes instead.
    //---------------------------------------------------
    class PauseMap : std::map<int, int>
    {
        int offsetAtUTick(int utick) const;

    public:
        TempoMap* tempomapWithPauses = nullptr;

        void calculate(const Score* s);
        inline int addPauseTicks(int utick) const { return utick + this->offsetAtUTick(utick); }
    };

    void writeHeader();

    QFile m_file;
    MidiFile m_midiFile;
    Score* m_score = nullptr;
    PauseMap m_pauseMap;
};
}
#endif // EXPORTMIDI_H

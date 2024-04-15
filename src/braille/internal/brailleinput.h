/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#ifndef MU_BRAILLE_BRAILLEINPUT_H
#define MU_BRAILLE_BRAILLEINPUT_H

#include "notation/notationtypes.h"

#include "braillecode.h"
#include "brailleinputparser.h"
#include "brailletypes.h"

namespace mu::engraving {
enum class NoteGroup
{
    Undefined = 0,
    Group1, // whole, half, quarter, 8th
    Group2, // 16th, 32nd, 64th, 128th
    Group3  // 256th, 512th, 1024th, 2048th
};

enum class IntervalDirection
{
    Up      = static_cast<char>(braille::BrailleIntervalDirection::Up),
    Down    = static_cast<char>(braille::BrailleIntervalDirection::Down),
};

class BrailleInputState
{
public:
    BrailleInputState();
    ~BrailleInputState();

    void initialize();
    void reset();
    void resetBuffer();
    void insertToBuffer(const QString);
    QString buffer();

    BieSequencePatternType parseBraille(IntervalDirection direction);

    AccidentalType accidental();
    notation::NoteName noteName();
    notation::NoteName chordBaseNoteName();
    NoteGroup noteGroup();

    DurationType currentDuration();
    std::vector<DurationType> noteDurations();
    bool isDurationMatch();
    DurationType getCloseDuration();

    notation::SymbolId articulation();
    int octave();
    int dots();
    int addedOctave();
    int chordBaseNoteOctave();
    voice_idx_t voice();
    bool noteSlur();
    bool longSlurStart();
    bool longSlurStop();
    bool tie();

    void setAccidental(const AccidentalType accidental);
    void setNoteName(const notation::NoteName notename, const bool chord_base = true);
    void setCurrentDuration(const DurationType duration);
    void setNoteDurations(const std::vector<DurationType> durations);
    void setArticulation(const notation::SymbolId articulation);
    void setOctave(const int octave, const bool chord_base = false);
    void setDots(const int dots);
    void setAddedOctave(const int octave);
    void setVoice(const voice_idx_t voice);

    void setNoteGroup(const NoteGroup g);

    std::vector<int> intervals();
    void clearIntervals();
    void removeLastInterval();
    int addInterval(int interval);

    void setTie(const bool s);
    Note* tieStartNote();
    void setTieStartNote(Note*);
    void clearTie();

    void setNoteSlur(const bool s);
    void setLongSlurStart(const bool s);
    void setLongSlurStop(const bool s);
    Note* slurStartNote();
    Note* longSlurStartNote();
    void setSlurStartNote(Note*);
    void setLongSlurStartNote(Note*);
    void clearSlur();
    void clearLongSlur();

    bool accord();
    void setAccord(const bool val);

    int tupletNumber();
    void setTupletNumber(const int num);
    notation::Duration tupletDuration();
    void setTupletDuration(const notation::Duration d);
    void clearTuplet();
    bool tupletIndicator();
    void setTupletIndicator(bool val);
private:
    AccidentalType _accidental = AccidentalType::NONE;
    notation::NoteName _note_name = notation::NoteName::C;
    notation::NoteName _chordbase_note_name = notation::NoteName::C;
    notation::SymbolId _articulation = notation::SymbolId::noSym;
    int _octave = 4;
    int _chordbase_note_octave = 4;
    int _added_octave = -1;
    int _dots = 0;
    voice_idx_t _voice = 0;
    QString _input_buffer = QString();
    int _code_num = 0;

    DurationType _current_duration;
    std::vector<DurationType> _note_durations;
    NoteGroup _note_group = NoteGroup::Undefined;

    std::vector<int> _intervals;

    bool _tie;
    Note* _tie_start_note =  NULL;

    bool _note_slur, _long_slur_start, _long_slur_stop;
    Note* _slur_start_note =  NULL;
    Note* _long_slur_start_note =  NULL;

    bool _accord;
    int _tuplet_number = -1;
    notation::Duration _tuplet_duration;
    bool _tuplet_indicator = false;
};

QString parseBrailleKeyInput(QString keys);

notation::NoteName getNoteName(const braille_code* code);
std::vector<DurationType> getNoteDurations(const braille_code* code);
std::vector<DurationType> getRestDurations(const braille_code* code);
int getInterval(const braille_code* code);
bool isNoteName(const braille_code* code);
QString fromNoteName(notation::NoteName);
AccidentalType getAccidentalType(const braille_code* code);
notation::SymbolId getArticulation(const braille_code* code);
int getOctave(const braille_code* code);
int getOctaveDiff(notation::NoteName source, notation::NoteName note);
std::pair<notation::NoteName, int> applyInterval(notation::NoteName source, int interval, IntervalDirection direction);
}
#endif // MU_BRAILLE_BRAILLEINPUT_H

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

#include "brailleinput.h"

using namespace mu::notation;

namespace mu::engraving {
BrailleInputState brailleInputState;

QString parseBrailleKeyInput(QString keys)
{
    static const std::map<std::string, std::string > braille_input_keys = {
        { "S", "3" }, { "D", "2" }, { "F", "1" },
        { "J", "4" }, { "K", "5" }, { "L", "6" }
    };

    QStringList lst = keys.split(QString::fromStdString("+"));

    std::vector<std::string> nlst;
    for (int i = 0; i < lst.size(); i++) {
        std::string key = lst.at(i).toStdString();

        auto it = braille_input_keys.find(key);
        if (it != braille_input_keys.end()) {
            nlst.push_back(it->second);
        }
    }
    std::sort(nlst.begin(), nlst.end());

    QString buff = QString();
    for (auto n : nlst) {
        buff.append(QString::fromStdString(n));
    }

    return buff;
}

NoteName getNoteName(const braille_code* code)
{
    char c = code->tag[0];
    switch (c) {
    case 'A': case 'a':
        return NoteName::A;
    case 'B': case 'b':
        return NoteName::B;
    case 'C': case 'c':
        return NoteName::C;
    case 'D': case 'd':
        return NoteName::D;
    case 'E': case 'e':
        return NoteName::E;
    case 'F': case 'f':
        return NoteName::F;
    case 'G': case 'g':
        return NoteName::G;
    }
    return NoteName::C;
}

std::vector<DurationType> getNoteDurations(const braille_code* code)
{
    for (int i=0; i < 7; i++) {
        if (code->code == Braille_wholeNotes[i]->code) {
            //return {DurationType::V_WHOLE, DurationType::V_16TH, DurationType::V_256TH};
            return { DurationType::V_WHOLE, DurationType::V_16TH };
        }
    }

    for (int i=0; i < 7; i++) {
        if (code->code == Braille_halfNotes[i]->code) {
            //return {DurationType::V_HALF, DurationType::V_32ND, DurationType::V_512TH};
            return { DurationType::V_HALF, DurationType::V_32ND };
        }
    }

    for (int i=0; i < 7; i++) {
        if (code->code == Braille_quarterNotes[i]->code) {
            //return {DurationType::V_QUARTER, DurationType::V_64TH, DurationType::V_1024TH};
            return { DurationType::V_QUARTER, DurationType::V_64TH };
        }
    }

    for (int i=0; i < 7; i++) {
        if (code->code == Braille_8thNotes[i]->code) {
            return { DurationType::V_EIGHTH, DurationType::V_128TH };
        }
    }

    return {};
}

std::vector<DurationType> getRestDurations(const braille_code* code)
{
    if (code->code == Braille_RestWhole.code) {
        return { DurationType::V_WHOLE, DurationType::V_16TH };
    }

    if (code->code == Braille_RestHalf.code) {
        return { DurationType::V_HALF, DurationType::V_32ND };
    }

    if (code->code == Braille_RestQuarter.code) {
        return { DurationType::V_QUARTER, DurationType::V_64TH };
    }

    if (code->code == Braille_Rest8th.code) {
        return { DurationType::V_EIGHTH, DurationType::V_128TH };
    }

    return {};
}

int getInterval(const braille_code* code)
{
    char c = code->tag.back();

    IF_ASSERT_FAILED(c >= '2' && c <= '8') {
        return -1;
    }

    return c - '0';
}

bool isNoteName(const braille_code* code)
{
    static const std::vector<std::string> note_names = {
        "aMaxima", "aLonga", "aBreve", "aWhole", "aHalf",
        "aQuarter", "a8th", "a16th", "a32nd", "a64th", "a128th",
        "a256th", "a512th", "a128th", "a2048th", "aBreveAlt",

        "bMaxima", "bLonga", "bBreve", "bWhole", "bHalf",
        "bQuarter", "b8th", "b16th", "b32nd", "b64th", "b128th",
        "b256th", "b512th", "b128th", "b2048th", "bBreveAlt",

        "cMaxima", "cLonga", "cBreve", "cWhole", "cHalf",
        "cQuarter", "c8th", "c16th", "c32nd", "c64th", "c128th",
        "c256th", "c512th", "c128th", "c2048th", "cBreveAlt",

        "dMaxima", "dLonga", "dBreve", "dWhole", "dHalf",
        "dQuarter", "d8th", "d16th", "d32nd", "d64th", "d128th",
        "d256th", "d512th", "d128th", "d2048th", "dBreveAlt",

        "eMaxima", "eLonga", "eBreve", "eWhole", "eHalf",
        "eQuarter", "e8th", "e16th", "e32nd", "e64th", "e128th",
        "e256th", "e512th", "e128th", "e2048th", "eBreveAlt",

        "fMaxima", "fLonga", "fBreve", "fWhole", "fHalf",
        "fQuarter", "f8th", "f16th", "f32nd", "f64th", "f128th",
        "f256th", "f512th", "f128th", "f2048th", "fBreveAlt",

        "gMaxima", "gLonga", "gBreve", "gWhole", "gHalf",
        "gQuarter", "g8th", "g16th", "g32nd", "g64th", "g128th",
        "g256th", "g512th", "g128th", "g2048th", "gBreveAlt",
    };

    return std::find(note_names.begin(), note_names.end(), code->tag) != note_names.end();
}

QString fromNoteName(NoteName notename)
{
    switch (notename) {
    case NoteName::A:
        return "A";
    case NoteName::B:
        return "B";
    case NoteName::C:
        return "C";
    case NoteName::D:
        return "D";
    case NoteName::E:
        return "E";
    case NoteName::F:
        return "F";
    case NoteName::G:
        return "G";
    default:
        return "";
    }
}

AccidentalType getAccidentalType(const braille_code* code)
{
    if (code->tag == "NaturalAccidental") {
        return AccidentalType::NATURAL;
    }

    if (code->tag == "SharpAccidental") {
        return AccidentalType::SHARP;
    }

    if (code->tag == "FlatAccidental") {
        return AccidentalType::FLAT;
    }

    return AccidentalType::NONE;
}

SymbolId getArticulation(const braille_code* code)
{
    static const std::map<std::string, SymbolId> articulations = {
        { "Finger0", SymbolId::fingering0 },
        { "Finger1", SymbolId::fingering1 },
        { "Finger2", SymbolId::fingering2 },
        { "Finger3", SymbolId::fingering3 },
        { "Finger4", SymbolId::fingering4 },
        { "Finger5", SymbolId::fingering5 },
    };

    auto it = articulations.find(code->tag);
    if (it != articulations.end()) {
        return it->second;
    }

    return SymbolId::noSym;
}

int getOctave(const braille_code* code)
{
    static const std::map<std::string, int> octaves = {
        { "Octave1", 1 }, { "Octave2", 2 }, { "Octave3", 3 }, { "Octave4", 4 },
        { "Octave5", 5 }, { "Octave6", 6 }, { "Octave7", 7 },
    };

    auto it = octaves.find(code->tag);
    if (it != octaves.end()) {
        return it->second;
    }
    return -1;
}

// Given a note (dest) that lacks an octave mark, compute its octave relative
// to a previous note (source) for which the octave is already known.
int getOctaveDiff(NoteName source, NoteName dest)
{
    // Calculate interval between notes assuming they are in the same octave.
    int interval = std::abs(static_cast<int>(dest) - static_cast<int>(source));
    ++interval; // so unison == 1

    if (interval >= 6) {
        // Impossible because notes separated by a 6th or more require octave
        // marks. Therefore, the dest note must be in a neighboring octave.
        // actual_interval = 8 - interval;
        return (source > dest) ? 1 : -1; // octave above or below
    }

    return 0; // same octave
}

// Given a starting note (source), and an interval and direction, compute the
// resulting note and its octave relative to the octave of the starting note.
std::pair<NoteName, int> applyInterval(NoteName source, int interval, IntervalDirection direction)
{
    IF_ASSERT_FAILED(interval >= 1) {
        return std::make_pair(source, 0);
    }

    --interval; // so unison == 0

    int pitch = static_cast<int>(source);
    int octave_diff = 0;

    switch (direction) {
    case IntervalDirection::Up:
        pitch += interval;
        octave_diff = pitch / 7;
        break;
    case IntervalDirection::Down:
        pitch -= interval;
        while (pitch < 0) {
            pitch += 7;
            --octave_diff;
        }
        break;
    }

    return std::make_pair(static_cast<NoteName>(pitch % 7), octave_diff);
}

BrailleInputState::BrailleInputState()
{
    initialize();
}

BrailleInputState::~BrailleInputState()
{
    _input_buffer.clear();
}

void BrailleInputState::initialize()
{
    _accidental = AccidentalType::NONE;
    _note_name = NoteName::C;
    _articulation = SymbolId::noSym;
    _octave = 4;
    _voice = 0;
    _dots = 0;
    _input_buffer.clear();
    _code_num = 0;
    _note_slur = _long_slur_start = _long_slur_stop = _tie = false;
    _note_group = NoteGroup::Group1;
    _intervals.clear();
    _accord = false;
    _tuplet_number = -1;
}

void BrailleInputState::reset()
{
    _input_buffer = QString();
    _accidental = AccidentalType::NONE;
    _articulation = SymbolId::noSym;
    _dots = 0;
    _code_num = 0;
    _note_slur = _long_slur_start = _long_slur_stop = _tie = false;
    _added_octave = -1;
    _accord = false;
    _tuplet_indicator = false;
}

void BrailleInputState::resetBuffer()
{
    _input_buffer = QString();
    _code_num = 0;
}

void BrailleInputState::insertToBuffer(const QString code)
{
    if (_input_buffer.isEmpty()) {
        _input_buffer = code;
        _code_num = 0;
    } else {
        _input_buffer.append("-").append(code);
        _code_num++;
    }
}

BieSequencePatternType BrailleInputState::parseBraille(IntervalDirection direction)
{
    std::string braille = translate2Braille(_input_buffer.toStdString());
    BieSequencePattern* pattern = BieRecognize(braille, tupletIndicator());

    if (pattern == NULL) {
        return BieSequencePatternType::Undefined;
    }

    switch (pattern->type()) {
    case BieSequencePatternType::Note: {
        clearIntervals();

        braille_code* code = pattern->res("note");

        NoteName note_name = getNoteName(code);

        setNoteDurations(getNoteDurations(code));

        code = pattern->res("octave");
        if (code != NULL) {
            setAddedOctave(getOctave(code));
        } else if (chordBaseNoteOctave() != -1) {
            int octave_diff = getOctaveDiff(chordBaseNoteName(), note_name);
            setAddedOctave(chordBaseNoteOctave() + octave_diff);
        } else {
            setAddedOctave(-1); // unknown octave
        }

        setNoteName(note_name, true); // do this after determining the octave

        code = pattern->res("accidental");
        if (code != NULL) {
            setAccidental(getAccidentalType(code));
        }

        code = pattern->res("long-slur-start");
        if (code != NULL) {
            setLongSlurStart(true);
        }

        code = pattern->res("accord");
        if (code != NULL) {
            setAccord(true);
        }
        break;
    }
    case BieSequencePatternType::Rest: {
        braille_code* code = pattern->res("rest");
        setNoteDurations(getRestDurations(code));

        code = pattern->res("accord");
        if (code != NULL) {
            setAccord(true);
        }
        break;
    }
    case BieSequencePatternType::Interval: {
        braille_code* code = pattern->res("interval");
        int interval = getInterval(code);
        interval = addInterval(interval);
        std::pair<NoteName, int> applied = applyInterval(chordBaseNoteName(), interval, direction);

        setNoteName(applied.first, false);

        code = pattern->res("octave");
        if (code != NULL) {
            setAddedOctave(getOctave(code));
        } else {
            setAddedOctave(chordBaseNoteOctave() + applied.second);
        }

        code = pattern->res("accidental");
        if (code != NULL) {
            setAccidental(getAccidentalType(code));
        }

        break;
    }
    case BieSequencePatternType::Tuplet3: {
        setTupletNumber(3);
        setTupletDuration(Duration(DurationType::V_EIGHTH));
        break;
    }
    case BieSequencePatternType::Tuplet: {
        braille_code* code = pattern->res("tuplet-number");
        setTupletNumber(code->tag.back() - '0');

        code = pattern->res("c-note");

        std::string stateTuplet;
        stateTuplet = "Tuplet " + std::to_string(tupletNumber()) + " " + code->tag;
        LOGD() << stateTuplet;
        if (code->tag == "cWhole") {
            setTupletDuration(Duration(DurationType::V_WHOLE));
        } else if (code->tag == "cHalf") {
            setTupletDuration(Duration(DurationType::V_HALF));
        } else if (code->tag == "cQuarter") {
            setTupletDuration(Duration(DurationType::V_QUARTER));
        } else if (code->tag == "c8th") {
            setTupletDuration(Duration(DurationType::V_EIGHTH));
        } else {
            setTupletDuration(Duration(DurationType::V_INVALID));
        }
        break;
    }
    case BieSequencePatternType::Tie: {
        braille_code* code = pattern->res("tie");
        if (code != NULL) {
            setTie(true);
        }
        break;
    }
    case BieSequencePatternType::NoteSlur: {
        braille_code* code = pattern->res("note-slur");
        if (code != NULL) {
            setNoteSlur(true);
        }
        break;
    }
    case BieSequencePatternType::LongSlurStop: {
        braille_code* code = pattern->res("long-slur-stop");
        if (code != NULL) {
            setLongSlurStop(true);
        }
        break;
    }
    case BieSequencePatternType::Dot: {
        braille_code* code = pattern->res("dot");
        if (code != NULL) {
            setDots(1);
        }
        break;
    }
    default: {
        break;
    }
    }

    return pattern->type();
}

QString BrailleInputState::buffer()
{
    return _input_buffer;
}

AccidentalType BrailleInputState::accidental()
{
    return _accidental;
}

NoteName BrailleInputState::noteName()
{
    return _note_name;
}

NoteName BrailleInputState::chordBaseNoteName()
{
    return _chordbase_note_name;
}

NoteGroup BrailleInputState::noteGroup()
{
    return _note_group;
}

DurationType BrailleInputState::currentDuration()
{
    return _current_duration;
}

std::vector<DurationType> BrailleInputState::noteDurations()
{
    return _note_durations;
}

bool BrailleInputState::isDurationMatch()
{
    return std::find(_note_durations.begin(), _note_durations.end(), _current_duration) != _note_durations.end();
}

DurationType BrailleInputState::getCloseDuration()
{
    switch (_note_group) {
    case NoteGroup::Group1: {
        return _note_durations[0];
        break;
    }
    case NoteGroup::Group2: {
        return _note_durations[1];
        break;
    }
    case NoteGroup::Group3: {
        return _note_durations[2];
        break;
    }
    default: {
        if (isDurationMatch()) {
            return _current_duration;
        } else {
            for (auto d : _note_durations) {
                if (d > _current_duration) {
                    return d;
                }
            }
        }
    }
    }
    return _note_durations.back();
}

SymbolId BrailleInputState::articulation()
{
    return _articulation;
}

int BrailleInputState::octave()
{
    return _octave;
}

int BrailleInputState::dots()
{
    return _dots;
}

int BrailleInputState::addedOctave()
{
    return _added_octave;
}

int BrailleInputState::chordBaseNoteOctave()
{
    return _chordbase_note_octave;
}

voice_idx_t BrailleInputState::voice()
{
    return _voice;
}

void BrailleInputState::setAccidental(const AccidentalType accidental)
{
    _accidental = accidental;
}

void BrailleInputState::setNoteName(const NoteName notename, const bool chord_base)
{
    _note_name = notename;
    if (chord_base) {
        _chordbase_note_name = notename;
    }
}

void BrailleInputState::setNoteGroup(const NoteGroup notegroup)
{
    LOGD() << (int)notegroup;
    _note_group = notegroup;
}

void BrailleInputState::setCurrentDuration(const DurationType duration)
{
    _current_duration = duration;
}

void BrailleInputState::setNoteDurations(const std::vector<DurationType> durations)
{
    _note_durations = durations;
}

void BrailleInputState::setArticulation(const SymbolId articulation)
{
    _articulation = articulation;
}

void BrailleInputState::setOctave(const int octave, const bool chord_base)
{
    _octave = octave;

    if (chord_base) {
        _chordbase_note_octave = octave;
    }
}

void BrailleInputState::setDots(const int dots)
{
    _dots = dots;
}

void BrailleInputState::setAddedOctave(const int octave)
{
    LOGD() << octave;
    _added_octave = octave;
}

void BrailleInputState::setVoice(const voice_idx_t voice)
{
    _voice = voice;
}

std::vector<int> BrailleInputState::intervals()
{
    return _intervals;
}

void BrailleInputState::removeLastInterval()
{
    if (!_intervals.empty()) {
        _intervals.pop_back();
    }
}

void BrailleInputState::clearIntervals()
{
    _intervals.clear();
}

int BrailleInputState::addInterval(int interval)
{
    if (!intervals().empty()) {
        int last = _intervals.back();
        while (interval < last) {
            interval += 7;
        }
    }

    _intervals.push_back(interval);
    return interval;
}

bool BrailleInputState::tie()
{
    return _tie;
}

void BrailleInputState::setTie(const bool s)
{
    _tie = s;
}

Note* BrailleInputState::tieStartNote()
{
    return _tie_start_note;
}

void BrailleInputState::setTieStartNote(Note* note)
{
    _tie_start_note = note;
}

void BrailleInputState::clearTie()
{
    _tie_start_note = NULL;
}

void BrailleInputState::setNoteSlur(const bool s)
{
    _note_slur = s;
}

void BrailleInputState::setLongSlurStart(const bool s)
{
    _long_slur_start = s;
}

void BrailleInputState::setLongSlurStop(const bool s)
{
    _long_slur_stop = s;
}

bool BrailleInputState::noteSlur()
{
    return _note_slur;
}

bool BrailleInputState::longSlurStart()
{
    return _long_slur_start;
}

bool BrailleInputState::longSlurStop()
{
    return _long_slur_stop;
}

Note* BrailleInputState::slurStartNote()
{
    return _slur_start_note;
}

Note* BrailleInputState::longSlurStartNote()
{
    return _long_slur_start_note;
}

void BrailleInputState::setSlurStartNote(Note* note)
{
    _slur_start_note = note;
}

void BrailleInputState::setLongSlurStartNote(Note* note)
{
    _long_slur_start_note = note;
}

void BrailleInputState::clearSlur()
{
    _slur_start_note = NULL;
}

void BrailleInputState::clearLongSlur()
{
    _long_slur_start_note = NULL;
}

bool BrailleInputState::accord()
{
    return _accord;
}

void BrailleInputState::setAccord(const bool val)
{
    _accord = val;
}

int BrailleInputState::tupletNumber()
{
    return _tuplet_number;
}

void BrailleInputState::setTupletNumber(const int num)
{
    _tuplet_number = num;
}

Duration BrailleInputState::tupletDuration()
{
    return _tuplet_duration;
}

void BrailleInputState::setTupletDuration(const Duration d)
{
    switch (noteGroup()) {
    case NoteGroup::Group2: {
        switch (d.type()) {
        case DurationType::V_WHOLE: {
            _tuplet_duration = Duration(DurationType::V_16TH);
            break;
        }
        case DurationType::V_HALF: {
            _tuplet_duration = Duration(DurationType::V_32ND);
            break;
        }
        case DurationType::V_QUARTER: {
            _tuplet_duration = Duration(DurationType::V_64TH);
            break;
        }
        case DurationType::V_EIGHTH: {
            _tuplet_duration = Duration(DurationType::V_128TH);
            break;
        }
        default: {
            _tuplet_duration = Duration(DurationType::V_INVALID);
            break;
        }
        }
        break;
    }
    case NoteGroup::Group3: {
        switch (d.type()) {
        case DurationType::V_WHOLE: {
            _tuplet_duration = Duration(DurationType::V_256TH);
            break;
        }
        case DurationType::V_HALF: {
            _tuplet_duration = Duration(DurationType::V_512TH);
            break;
        }
        case DurationType::V_QUARTER: {
            _tuplet_duration = Duration(DurationType::V_1024TH);
            break;
        }
        case DurationType::V_EIGHTH: {
            _tuplet_duration = Duration(DurationType::V_INVALID);
            break;
        }
        default: {
            _tuplet_duration = Duration(DurationType::V_INVALID);
            break;
        }
        }
        break;
    }
    default: {
        _tuplet_duration = d;
        break;
    }
    }
}

void BrailleInputState::clearTuplet()
{
    _tuplet_number = -1;
    _tuplet_duration = Duration(DurationType::V_INVALID);
}

bool BrailleInputState::tupletIndicator()
{
    return _tuplet_indicator;
}

void BrailleInputState::setTupletIndicator(bool val)
{
    _tuplet_indicator = val;
}
}

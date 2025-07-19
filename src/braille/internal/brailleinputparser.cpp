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

#include "brailleinputparser.h"

#include <QString>

#include "braillecode.h"

namespace mu::engraving {
static BiePattern pattern_accord = { "accord", { &Braille_FullMeasureAccord } };

static BiePattern pattern_accidentals = { "accidental",
                                          { &Braille_NaturalAccidental, &Braille_SharpAccidental, &Braille_FlatAccidental } };

static BiePattern pattern_octaves = { "octave",
                                      { &Braille_Octave0, &Braille_Octave1, &Braille_Octave2,
                                        &Braille_Octave3, &Braille_Octave4, &Braille_Octave5,
                                        &Braille_Octave6, &Braille_Octave7, &Braille_Octave8 } };

static BiePattern pattern_notes ={ "note",
                                   { &Braille_aWhole, &Braille_aHalf, &Braille_aQuarter, &Braille_a8th,
                                     &Braille_bWhole, &Braille_bHalf, &Braille_bQuarter, &Braille_b8th,
                                     &Braille_cWhole, &Braille_cHalf, &Braille_cQuarter, &Braille_c8th,
                                     &Braille_dWhole, &Braille_dHalf, &Braille_dQuarter, &Braille_d8th,
                                     &Braille_eWhole, &Braille_eHalf, &Braille_eQuarter, &Braille_f8th,
                                     &Braille_fWhole, &Braille_fHalf, &Braille_fQuarter, &Braille_e8th,
                                     &Braille_gWhole, &Braille_gHalf, &Braille_gQuarter, &Braille_g8th } };

static BiePattern pattern_c_notes ={ "c-note",
                                     { &Braille_cWhole, &Braille_cHalf, &Braille_cQuarter, &Braille_c8th } };

static BiePattern pattern_dot = { "dot", { &Braille_Dot } };
static BiePattern pattern_dot_2 = { "dot-2", { &Braille_Dot } };
static BiePattern pattern_dot_3 = { "dot-3", { &Braille_Dot } };
static BiePattern pattern_tie = { "tie", { &Braille_NoteTie } };
static BiePattern pattern_note_slur = { "note-slur", { &Braille_NoteSlur } };
static BiePattern pattern_slur_start = { "long-slur-start", { &Braille_LongSlurOpenBracket } };
static BiePattern pattern_slur_stop = { "long-slur-stop", { &Braille_LongSlurCloseBracket } };

static BiePattern pattern_rests ={ "rest",
                                   { &Braille_RestWhole, &Braille_RestHalf, &Braille_RestQuarter, &Braille_Rest8th } };

static BiePattern pattern_intervals ={ "interval",
                                       { &Braille_Interval2, &Braille_Interval3, &Braille_Interval4, &Braille_Interval5,
                                         &Braille_Interval6, &Braille_Interval7, &Braille_Interval8 } };

static BiePattern pattern_fingerings ={ "fingering",
                                        { &Braille_Finger0, &Braille_Finger1, &Braille_Finger2,
                                          &Braille_Finger3, &Braille_Finger4, &Braille_Finger5 } };

static BiePattern pattern_tuplet_indicator ={ "tuplet-indicator", { &Braille_Tuplet3 } };
static BiePattern pattern_tuplet3 ={ "tuplet3", { &Braille_Tuplet3 } };
static BiePattern pattern_tuplet_prefix ={ "tuplet-prefix", { &Braille_TupletPrefix } };
static BiePattern pattern_tuplet_suffix ={ "tuplet-suffix", { &Braille_Dot } };

static BiePattern pattern_tuplet_numbers ={ "tuplet-number",
                                            { &Braille_Lower2, &Braille_Lower3, &Braille_Lower5, &Braille_Lower6,
                                              &Braille_Lower7, &Braille_Lower8, &Braille_Lower9 } };

static int maxPatternLength(BiePattern* pattern)
{
    int max = 0;
    for (auto code : pattern->codes) {
        if (code->num_cells > max) {
            max = code->num_cells;
        }
    }
    return max;
}

BieSequencePattern::BieSequencePattern(BieSequencePatternType t, std::string sequence)
{
    _type = t;
    patterns.clear();
    max_cell_length = 0;

    size_t len = sequence.length();
    QString key;
    bool mandatory = false;
    char openning = ' ';
    for (size_t i = 0; i < len; ++i) {
        char c = sequence.at(i);
        //LOGD() << i << " c=" << c << " open=" << openning;
        switch (c) {
        case '(': {
            if (openning != ' ') {
                _valid = false;
                return;
            }
            mandatory = true;
            openning = '(';
            break;
        }
        case ')': {
            if (openning != '(') {
                _valid = false;
                return;
            }
            openning = ' ';
            break;
        }
        case '[': {
            if (openning != ' ') {
                _valid = false;
                return;
            }
            mandatory = false;
            openning = '[';
            break;
        }
        case ']': {
            if (openning != '[') {
                _valid = false;
                return;
            }
            openning = ' ';
            break;
        }
        case '{': {
            // ignore
            break;
        }
        case '}': {
            // ignore
            break;
        }
        default: {
            key.push_back(sequence.at(i));
            break;
        }
        }

        if (openning == ' ' && !key.isEmpty()) {
            //LOGD() << key << " " << mandatory;
            BiePattern* pattern = NULL;
            if (key == "accord") {
                pattern = &pattern_accord;
            } else if (key == "accord") {
                pattern = &pattern_accord;
            } else if (key == "accidental") {
                pattern = &pattern_accidentals;
            } else if (key == "octave") {
                pattern = &pattern_octaves;
            } else if (key == "note") {
                pattern = &pattern_notes;
            } else if (key == "dot") {
                pattern = &pattern_dot;
            } else if (key == "dot-2") {
                pattern = &pattern_dot_2;
            } else if (key == "dot-3") {
                pattern = &pattern_dot_3;
            } else if (key == "tie") {
                pattern = &pattern_tie;
            } else if (key == "note-slur") {
                pattern = &pattern_note_slur;
            } else if (key == "long-slur-start") {
                pattern = &pattern_slur_start;
            } else if (key == "long-slur-stop") {
                pattern = &pattern_slur_stop;
            } else if (key == "rest") {
                pattern = &pattern_rests;
            } else if (key == "interval") {
                pattern = &pattern_intervals;
            } else if (key == "fingering") {
                pattern = &pattern_fingerings;
            } else if (key == "tuplet-indicator") {
                pattern = &pattern_tuplet_indicator;
            } else if (key == "tuplet3") {
                pattern = &pattern_tuplet3;
            } else if (key == "tuplet-prefix") {
                pattern = &pattern_tuplet_prefix;
            } else if (key == "tuplet-number") {
                pattern = &pattern_tuplet_numbers;
            } else if (key == "tuplet-suffix") {
                pattern = &pattern_tuplet_suffix;
            } else if (key == "c-note") {
                pattern = &pattern_c_notes;
            }
            if (pattern != NULL) {
                patterns.push_back({ pattern->name, pattern->codes, mandatory });
                int n = maxPatternLength(pattern);
                if (max_cell_length < n) {
                    max_cell_length = n;
                }
                if (mandatory) {
                    _mandatories++;
                }
            }
            mandatory = false;
            key = QString();
        }
    }
    _valid = true;
}

BieSequencePattern::~BieSequencePattern()
{
    // TODO
}

BieSequencePatternType BieSequencePattern::type() const
{
    return _type;
}

bool BieSequencePattern::recognize(std::string braille)
{
    _res.clear();

    size_t pos = 0;
    size_t cursor = 0;

    int mandatory_matches = 0;
    while (cursor < braille.length() && pos < patterns.size()) {
        //LOGD() << "cursor: " << cursor;
        bool match = false;
        for (auto code : patterns[pos].codes) {
            int len = code->num_cells;
            if (cursor + len <= braille.length()) {
                std::string bxt = braille.substr(cursor, len);
                if (bxt == code->braille) {
                    //code->print();
                    match = true;
                    _res[patterns[pos].name] = code;
                    if (patterns[pos].mandatory) {
                        mandatory_matches++;
                    }
                    pos++;
                    cursor += len;
                    break;
                }
            }
        }
        if (!match) {
            if (patterns[pos].mandatory) {
                return false;
            } else {
                pos++;
            }
        }
    }
    //for(std::map<std::string, braille_code *>::iterator it = _res.begin(); it != _res.end(); ++it) {
    //    LOGD() << "Key: " << it->first << " value: " << it->second->tag;
    //}
    return mandatory_matches == _mandatories && cursor >= braille.length();
}

const std::map<std::string, braille_code*>& BieSequencePattern::res() const
{
    return _res;
}

braille_code* BieSequencePattern::res(std::string key)
{
    return _res[key];
}

bool BieSequencePattern::valid() const
{
    return _valid;
}

BieSequencePattern* BieRecognize(std::string braille, bool tuplet_indicator)
{
    //static const std::string note_input_seq = "{[accord][long-slur-start][accidental][octave](note)[dot][dot-2][dot-3][fingering][note-slur][long-slur-stop][tie]}";
    static const std::string note_input_seq = "{[accord][long-slur-start][accidental][octave](note)}";
    static BieSequencePattern bie_note_input(BieSequencePatternType::Note, note_input_seq);

    //static const std::string rest_input_seq = "{[accord](rest)[dot][dot-2][dot-3][slur]}";
    static const std::string rest_input_seq = "{[accord](rest)}";
    static BieSequencePattern bie_rest_input(BieSequencePatternType::Rest, rest_input_seq);

    static const std::string interval_input_seq = "{[accidental][octave](interval)}";
    static BieSequencePattern bie_interval_input(BieSequencePatternType::Interval, interval_input_seq);

    static const std::string tuplet3_seq = "{(tuplet3)}";
    static BieSequencePattern bie_tuplet3(BieSequencePatternType::Tuplet3, tuplet3_seq);

    static const std::string tuplet_seq = "{(tuplet-prefix)(tuplet-number)(c-note)(tuplet-suffix)}";
    static BieSequencePattern bie_tuplet(BieSequencePatternType::Tuplet, tuplet_seq);

    static const std::string dot_seq = "{(dot)}";
    static BieSequencePattern bie_dot(BieSequencePatternType::Dot, dot_seq);

    static const std::string tie_seq = "{(tie)}";
    static BieSequencePattern bie_tie(BieSequencePatternType::Tie, tie_seq);

    static const std::string noteslur_seq = "{(note-slur)}";
    static BieSequencePattern bie_noteslur(BieSequencePatternType::NoteSlur, noteslur_seq);

    static const std::string longslur_seq = "{(long-slur-stop)}";
    static BieSequencePattern bie_longslur(BieSequencePatternType::LongSlurStop, longslur_seq);

    BieSequencePattern* res = NULL;

    if (!tuplet_indicator) { // check note input before tuplet
        if (bie_note_input.valid() && bie_note_input.recognize(braille)) {
            res = &bie_note_input;
        } else if (bie_rest_input.valid() && bie_rest_input.recognize(braille)) {
            res = &bie_rest_input;
        } else if (bie_interval_input.valid() && bie_interval_input.recognize(braille)) {
            res = &bie_interval_input;
        } else if (bie_tuplet3.valid() && bie_tuplet3.recognize(braille)) {
            res = &bie_tuplet3;
            //} else if(bie_tuplet.valid() && bie_tuplet.recognize(braille)) {
            //    res = &bie_tuplet;
        } else if (bie_tie.valid() && bie_tie.recognize(braille)) {
            res = &bie_tie;
        } else if (bie_noteslur.valid() && bie_noteslur.recognize(braille)) {
            res = &bie_noteslur;
        } else if (bie_longslur.valid() && bie_longslur.recognize(braille)) {
            res = &bie_longslur;
        } else if (bie_dot.valid() && bie_dot.recognize(braille)) {
            res = &bie_dot;
        }
    } else { // check tuplet input
        if (bie_tuplet.valid() && bie_tuplet.recognize(braille)) {
            res = &bie_tuplet;
        }
    }

    return res;
}
}

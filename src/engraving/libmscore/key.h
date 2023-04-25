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

#ifndef __KEY__H__
#define __KEY__H__

#include <vector>

#include "types/types.h"

namespace mu::engraving {
class Score;

enum class AccidentalVal : signed char;

//---------------------------------------------------------
//   KeySym
//    position of one symbol in KeySig
//---------------------------------------------------------

struct KeySym {
    SymId sym;
    int line;       // relative line position (first staffline: line == 0, first gap: line == 1, ...)
    double xPos;    // x position in staff spatium units
};

//---------------------------------------------------------
//   CustDef
//    definition of one symbol in Custom KeySig
//---------------------------------------------------------

struct CustDef {
    int degree;             // scale degree
    SymId sym;
    double xAlt { 0.0 };    // x position alteration in spatium units (default symbol position is based on index)
    int octAlt { 0 };       // octave alteration
};

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent
{
    Key _key            { Key::INVALID };                // -7 -> +7
    KeyMode _mode       { KeyMode::UNKNOWN };
    bool _custom        { false };
    bool _forInstrumentChange{ false };
    std::vector<CustDef> _customKeyDefs;
    std::vector<KeySym> _keySymbols;

    void enforceLimits();

public:
    KeySigEvent() = default;

    bool operator==(const KeySigEvent& e) const;
    bool operator!=(const KeySigEvent& e) const { return !(*this == e); }

    void setKey(Key v);
    void print() const;

    Key key() const { return _key; }
    KeyMode mode() const { return _mode; }
    void setMode(KeyMode m) { _mode = m; }
    bool custom() const { return _custom; }
    void setCustom(bool val) { _custom = val; _key = (_key == Key::INVALID ? Key::C : _key); }
    bool isValid() const { return _key != Key::INVALID; }
    bool isAtonal() const { return _mode == KeyMode::NONE; }
    void setForInstrumentChange(bool forInstrumentChange) { _forInstrumentChange = forInstrumentChange; }
    bool forInstrumentChange() const { return _forInstrumentChange; }
    void initFromSubtype(int);      // for backward compatibility
    int degInKey(int degree) const; // return "absolute degree"
    SymId symInKey(SymId sym, int degree) const;
    std::vector<KeySym>& keySymbols() { return _keySymbols; }
    const std::vector<KeySym>& keySymbols() const { return _keySymbols; }
    std::vector<CustDef>& customKeyDefs() { return _customKeyDefs; }
    const std::vector<CustDef>& customKeyDefs() const { return _customKeyDefs; }
};

//---------------------------------------------------------
//   AccidentalState
///   Contains a state for every absolute staff line.
//---------------------------------------------------------

static const int TIE_CONTEXT = 0x10;
static const int MIN_ACC_STATE = 0;
static const int MAX_ACC_STATE = 75;

class AccidentalState
{
    uint8_t state[MAX_ACC_STATE] = {};      // (0 -- 4) | TIE_CONTEXT

public:
    AccidentalState() {}
    void init(Key key);
    void init(const KeySigEvent&);
    AccidentalVal accidentalVal(int line, bool& error) const;
    AccidentalVal accidentalVal(int line) const;
    bool tieContext(int line) const;
    void setAccidentalVal(int line, AccidentalVal val, bool tieContext = false);
};

struct Interval;

enum class PreferSharpFlat : char;
extern Key transposeKey(Key oldKey, const Interval&, PreferSharpFlat prefer = PreferSharpFlat(0));
extern Interval calculateInterval(Key key1, Key key2);
} // namespace mu::engraving
#endif

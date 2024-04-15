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

#ifndef MU_ENGRAVING_KEY_H
#define MU_ENGRAVING_KEY_H

#include <vector>
#include <array>

#include "../types/types.h"

namespace mu::engraving {
class Score;

enum class AccidentalVal : signed char;

//---------------------------------------------------------
//   KeySym
//    position of one symbol in KeySig
//---------------------------------------------------------

struct KeySym {
    SymId sym = SymId::noSym;
    int line = 0;       // relative line position (first staffline: line == 0, first gap: line == 1, ...)
    double xPos = 0.0;    // x position in staff spatium units
};

//---------------------------------------------------------
//   CustDef
//    definition of one symbol in Custom KeySig
//---------------------------------------------------------

struct CustDef {
    int degree = 0;             // scale degree
    SymId sym = SymId::noSym;
    double xAlt = 0.0;    // x position alteration in spatium units (default symbol position is based on index)
    int octAlt = 0;       // octave alteration
};

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent
{
public:
    KeySigEvent() = default;

    bool operator==(const KeySigEvent& e) const;
    bool operator!=(const KeySigEvent& e) const { return !(*this == e); }

    void setConcertKey(Key v);
    void setKey(Key v);
    void setCustom(bool val);
    void print() const;

    Key concertKey() const { return m_concertKey; }
    Key key() const { return m_key; }
    KeyMode mode() const { return m_mode; }
    void setMode(KeyMode m) { m_mode = m; }
    bool custom() const { return m_custom; }
    bool isValid() const { return m_key != Key::INVALID; }
    bool isAtonal() const { return m_mode == KeyMode::NONE; }
    void setForInstrumentChange(bool forInstrumentChange) { m_forInstrumentChange = forInstrumentChange; }
    bool forInstrumentChange() const { return m_forInstrumentChange; }
    void initFromSubtype(int);      // for backward compatibility
    int degInKey(int degree) const; // return "absolute degree"
    SymId symInKey(SymId sym, int degree) const;
    std::vector<CustDef>& customKeyDefs() { return m_customKeyDefs; }
    const std::vector<CustDef>& customKeyDefs() const { return m_customKeyDefs; }

private:

    void enforceLimits(bool transposing = false);       // if true, enforce only trnasposing, otherways both

    Key m_concertKey = Key::INVALID;               // -7 -> +7
    Key m_key = Key::INVALID;               // actual key, depends on staff transposition
    KeyMode m_mode = KeyMode::UNKNOWN;
    bool m_custom = false;
    bool m_forInstrumentChange = false;
    std::vector<CustDef> m_customKeyDefs;
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
public:
    AccidentalState() {}
    void init(Key key);
    void init(const KeySigEvent&);
    AccidentalVal accidentalVal(int line, bool& error) const;
    AccidentalVal accidentalVal(int line) const;
    bool forceRestateAccidental(int line) const;
    bool tieContext(int line) const;
    void setAccidentalVal(int line, AccidentalVal val, bool tieContext = false);
    void setForceRestateAccidental(int line, bool forceRestate);

private:

    uint8_t m_state[MAX_ACC_STATE] = {};      // (0 -- 4) | TIE_CONTEXT
    std::array<bool, MAX_ACC_STATE> m_forceRestateAccidental;
};

struct Interval;

enum class PreferSharpFlat : char;
extern Key transposeKey(Key oldKey, const Interval&, PreferSharpFlat prefer = PreferSharpFlat(0));
extern Interval calculateInterval(Key key1, Key key2);
} // namespace mu::engraving
#endif

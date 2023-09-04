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

#ifndef __MCURSOR_H__
#define __MCURSOR_H__

#include "types/string.h"
#include "types/fraction.h"

namespace mu::engraving {
class MasterScore;
class TDuration;
class TimeSig;
class Chord;
enum class Key;

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

class MCursor
{
    MasterScore* _score;
    Fraction _tick;
    int _track;
    Fraction _sig;

    void createMeasures();

public:
    MCursor(MasterScore* s = 0);
    void createScore(const String& s);

    void addPart(const String& instrument);
    Chord* addChord(int pitch, const TDuration& duration);
    void addKeySig(Key);
    TimeSig* addTimeSig(const Fraction&);

    void move(int track, const Fraction& tick);
    MasterScore* score() const { return _score; }
    void setScore(MasterScore* s) { _score = s; }
    void setTimeSig(Fraction f) { _sig = f; }
};
} // namespace mu::engraving
#endif

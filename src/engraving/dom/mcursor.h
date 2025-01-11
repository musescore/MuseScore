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

#ifndef MU_ENGRAVING_MCURSOR_H
#define MU_ENGRAVING_MCURSOR_H

#include "types/string.h"
#include "../types/fraction.h"

#include "modularity/ioc.h"

namespace mu::engraving {
class MasterScore;
class TDuration;
class TimeSig;
class Chord;
enum class Key : signed char;

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

class MCursor
{
public:
    MCursor(MasterScore* s = 0);
    void createScore(const muse::modularity::ContextPtr& iocCtx, const String& s);

    void addPart(const String& instrument);
    Chord* addChord(int pitch, const TDuration& duration);
    void addKeySig(Key);
    TimeSig* addTimeSig(const Fraction&);

    void move(int track, const Fraction& tick);
    MasterScore* score() const { return m_score; }
    void setScore(MasterScore* s) { m_score = s; }
    void setTimeSig(Fraction f) { m_sig = f; }

private:

    void createMeasures();

    MasterScore* m_score = nullptr;
    Fraction m_tick;
    int m_track = 0;
    Fraction m_sig;
};
} // namespace mu::engraving
#endif

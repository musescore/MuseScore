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

#ifndef __POS_H__
#define __POS_H__

#include "global/allocator.h"
#include "types/string.h"

namespace mu::engraving {
class SigEvent;
class TempoMap;
class TimeSigMap;
class XmlReader;
class XmlWriter;

enum class TType : char {
    TICKS, FRAMES
};

//---------------------------------------------------------
//   Pos
//    depending on type _tick or _frame is a cached
//    value. When the tempomap changes, all cached values
//    are invalid. Sn is used to check for tempomap
//    changes.
//---------------------------------------------------------

class Pos
{
    TType _type;
    bool _valid;
    mutable int sn;
    mutable unsigned _tick;
    mutable unsigned _frame;

protected:
    TempoMap* tempo;
    TimeSigMap* sig;

public:
    Pos();
    Pos(TempoMap*, TimeSigMap*);
    Pos(TempoMap*, TimeSigMap*, int measure, int beat, int tick);
    Pos(TempoMap*, TimeSigMap*, int minute, int sec, int frame, int subframe);
    Pos(TempoMap*, TimeSigMap*, unsigned, TType type = TType::TICKS);
    Pos(TempoMap*, TimeSigMap*, const String&);

    void setContext(TempoMap* t, TimeSigMap* s) { tempo = t; sig = s; }
    void dump(int n = 0) const;

    unsigned time(TType t) const { return t == TType::TICKS ? tick() : frame(); }
    void mbt(int* measure, int* beat, int* tick) const;
    void msf(int* minute, int* sec, int* frame, int* subframe) const;
    SigEvent timesig() const;
    void snap(int);
    void upSnap(int);
    void downSnap(int);
    Pos snapped(int) const;
    Pos upSnapped(int) const;
    Pos downSnapped(int) const;

    void invalidSn() { sn = -1; }

    TType  type() const { return _type; }
    void   setType(TType t);

    Pos& operator+=(const Pos& a);
    Pos& operator+=(int a);
    Pos& operator-=(const Pos& a);
    Pos& operator-=(int a);

    bool operator>=(const Pos& s) const;
    bool operator>(const Pos& s) const;
    bool operator<(const Pos& s) const;
    bool operator<=(const Pos& s) const;
    bool operator==(const Pos& s) const;
    bool operator!=(const Pos& s) const;

    friend Pos operator+(const Pos& a, const Pos& b);
    friend Pos operator-(const Pos& a, const Pos& b);
    friend Pos operator+(const Pos& a, int b);
    friend Pos operator-(const Pos& a, int b);

    unsigned tick() const;
    unsigned frame() const;
    void setTick(unsigned);
    void setFrame(unsigned);

    bool valid() const { return _valid && tempo && sig; }
    void setInvalid() { _valid = false; }
};

//---------------------------------------------------------
//   PosLen
//---------------------------------------------------------

class PosLen : public Pos
{
    OBJECT_ALLOCATOR(engraving, PosLen)

    mutable unsigned _lenTick;
    mutable unsigned _lenFrame;
    mutable int sn;

public:
    PosLen(TempoMap*, TimeSigMap*);
    PosLen(const PosLen&);
    void dump(int n = 0) const;

    void setLenTick(unsigned);
    void setLenFrame(unsigned);
    unsigned lenTick() const;
    unsigned lenFrame() const;
    Pos end() const;
    unsigned endTick() const { return end().tick(); }
    unsigned endFrame() const { return end().frame(); }
    void setPos(const Pos&);

    bool operator==(const PosLen& s) const;
};
} // namespace mu::engraving
#endif

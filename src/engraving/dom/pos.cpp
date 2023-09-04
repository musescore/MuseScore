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

#include "pos.h"

#include <cmath>

#include "mscore.h"

#include "sig.h"
#include "tempo.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   Pos
//---------------------------------------------------------

Pos::Pos()
{
    tempo  = 0;
    sig    = 0;
    _type  = TType::TICKS;
    _tick  = 0;
    _frame = 0;
    sn     = -1;
    _valid = false;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl)
{
    tempo  = tl;
    sig    = sl;
    _type   = TType::TICKS;
    _tick   = 0;
    _frame  = 0;
    sn      = -1;
    _valid  = false;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, unsigned t, TType timeType)
{
    tempo  = tl;
    sig    = sl;
    _type = timeType;
    if (_type == TType::TICKS) {
        _tick   = t;
    } else {
        _frame = t;
    }
    sn = -1;
    _valid = true;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, const String& s)
{
    tempo  = tl;
    sig    = sl;
    int m, b, t;
    sscanf(s.toAscii().constChar(), "%04d.%02d.%03d", &m, &b, &t);
    _tick  = sig->bar2tick(m, b) + t;
    _type  = TType::TICKS;
    _frame = 0;
    sn     = -1;
    _valid = true;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, int measure, int beat, int tick)
{
    tempo  = tl;
    sig    = sl;
    _tick  = sig->bar2tick(measure, beat) + tick;
    _type  = TType::TICKS;
    _frame = 0;
    sn     = -1;
    _valid = true;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, int min, int sec, int frame, int subframe)
{
    tempo  = tl;
    sig    = sl;
    double time = min * 60.0 + sec;

    double f = frame + subframe / 100.0;
    switch (MScore::mtcType) {
    case 0:             // 24 frames sec
        time += f * 1.0 / 24.0;
        break;
    case 1:             // 25
        time += f * 1.0 / 25.0;
        break;
    case 2:             // 30 drop frame
        time += f * 1.0 / 30.0;
        break;
    case 3:             // 30 non drop frame
        time += f * 1.0 / 30.0;
        break;
    }
    _type  = TType::FRAMES;
    _tick  = 0;
    _frame = lrint(time * MScore::sampleRate);
    sn     = -1;
    _valid = true;
}

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void Pos::setType(TType t)
{
    if (t == _type) {
        return;
    }

    if (_type == TType::TICKS) {
        // convert from ticks to frames
        _frame = tempo->tick2time(_tick, _frame, &sn) * MScore::sampleRate;
    } else {
        // convert from frames to ticks
        _tick = tempo->time2tick(_frame / MScore::sampleRate, _tick, &sn);
    }
    _type = t;
}

//---------------------------------------------------------
//   operator+=
//---------------------------------------------------------

Pos& Pos::operator+=(const Pos& a)
{
    if (_type == TType::FRAMES) {
        _frame += a.frame();
    } else {
        _tick += a.tick();
    }
    sn = -1;            // invalidate cached values
    return *this;
}

//---------------------------------------------------------
//   operator-=
//---------------------------------------------------------

Pos& Pos::operator-=(const Pos& a)
{
    if (_type == TType::FRAMES) {
        _frame -= a.frame();
    } else {
        _tick -= a.tick();
    }
    sn = -1;            // invalidate cached values
    return *this;
}

//---------------------------------------------------------
//   operator+=
//---------------------------------------------------------

Pos& Pos::operator+=(int a)
{
    if (_type == TType::FRAMES) {
        _frame += a;
    } else {
        _tick += a;
    }
    sn = -1;            // invalidate cached values
    return *this;
}

//---------------------------------------------------------
//   operator-=
//---------------------------------------------------------

Pos& Pos::operator-=(int a)
{
    if (_type == TType::FRAMES) {
        _frame -= a;
    } else {
        _tick -= a;
    }
    sn = -1;            // invalidate cached values
    return *this;
}

Pos operator+(const Pos& a, int b)
{
    Pos c(a);
    return c += b;
}

Pos operator-(const Pos& a, int b)
{
    Pos c(a);
    return c -= b;
}

Pos operator+(const Pos& a, const Pos& b)
{
    Pos c(a);
    return c += b;
}

Pos operator-(const Pos& a, const Pos& b)
{
    Pos c(a);
    return c -= b;
}

bool Pos::operator>=(const Pos& s) const
{
    if (_type == TType::FRAMES) {
        return _frame >= s.frame();
    } else {
        return _tick >= s.tick();
    }
}

bool Pos::operator>(const Pos& s) const
{
    if (_type == TType::FRAMES) {
        return _frame > s.frame();
    } else {
        return _tick > s.tick();
    }
}

bool Pos::operator<(const Pos& s) const
{
    if (_type == TType::FRAMES) {
        return _frame < s.frame();
    } else {
        return _tick < s.tick();
    }
}

bool Pos::operator<=(const Pos& s) const
{
    if (_type == TType::FRAMES) {
        return _frame <= s.frame();
    } else {
        return _tick <= s.tick();
    }
}

bool Pos::operator==(const Pos& s) const
{
    if (_type == TType::FRAMES) {
        return _frame == s.frame();
    } else {
        return _tick == s.tick();
    }
}

bool Pos::operator!=(const Pos& s) const
{
    if (_type == TType::FRAMES) {
        return _frame != s.frame();
    } else {
        return _tick != s.tick();
    }
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

unsigned Pos::tick() const
{
    if (_type == TType::FRAMES) {
        _tick = tempo->time2tick(_frame / MScore::sampleRate, _tick, &sn);
    }
    return _tick;
}

//---------------------------------------------------------
//   frame
//---------------------------------------------------------

unsigned Pos::frame() const
{
    if (_type == TType::TICKS) {
        // double time = _frame / MScore::sampleRate;
        // _frame = tempo->tick2time(_tick, time, &sn) * MScore::sampleRate;
        _frame = tempo->tick2time(_tick) * MScore::sampleRate;
    }
    return _frame;
}

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Pos::setTick(unsigned pos)
{
    _tick = pos;
    sn    = -1;
    if (_type == TType::FRAMES) {
        _frame = tempo->tick2time(pos, &sn) * MScore::sampleRate;
    }
    _valid = true;
}

//---------------------------------------------------------
//   setFrame
//---------------------------------------------------------

void Pos::setFrame(unsigned pos)
{
    _frame = pos;
    sn     = -1;
    if (_type == TType::TICKS) {
        _tick = tempo->time2tick(pos / MScore::sampleRate, &sn);
    }
    _valid = true;
}

//---------------------------------------------------------
//   PosLen
//---------------------------------------------------------

PosLen::PosLen(TempoMap* tl, TimeSigMap* sl)
    : Pos(tl, sl)
{
    _lenTick  = 0;
    _lenFrame = 0;
    sn        = -1;
}

PosLen::PosLen(const PosLen& p)
    : Pos(p)
{
    _lenTick  = p._lenTick;
    _lenFrame = p._lenFrame;
    sn = -1;
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void PosLen::dump(int n) const
{
    Pos::dump(n);
    LOGD("  Len(");
    switch (type()) {
    case TType::FRAMES:
        LOGD("samples=%d)", _lenFrame);
        break;
    case TType::TICKS:
        LOGD("ticks=%d)", _lenTick);
        break;
    }
}

void Pos::dump(int /*n*/) const
{
    LOGD("Pos(%s, sn=%d, ", type() == TType::FRAMES ? "Frames" : "Ticks", sn);
    switch (type()) {
    case TType::FRAMES:
        LOGD("samples=%d)", _frame);
        break;
    case TType::TICKS:
        LOGD("ticks=%d)", _tick);
        break;
    }
}

//---------------------------------------------------------
//   setLenTick
//---------------------------------------------------------

void PosLen::setLenTick(unsigned len)
{
    _lenTick = len;
    sn       = -1;
    if (type() == TType::FRAMES) {
        _lenFrame = tempo->tick2time(len, &sn) * MScore::sampleRate;
    } else {
        _lenTick = len;
    }
}

//---------------------------------------------------------
//   setLenFrame
//---------------------------------------------------------

void PosLen::setLenFrame(unsigned len)
{
    sn      = -1;
    if (type() == TType::TICKS) {
        _lenTick = tempo->time2tick(len / MScore::sampleRate, &sn);
    } else {
        _lenFrame = len;
    }
}

//---------------------------------------------------------
//   lenTick
//---------------------------------------------------------

unsigned PosLen::lenTick() const
{
    if (type() == TType::FRAMES) {
        _lenTick = tempo->time2tick(_lenFrame / MScore::sampleRate, _lenTick, &sn);
    }
    return _lenTick;
}

//---------------------------------------------------------
//   lenFrame
//---------------------------------------------------------

unsigned PosLen::lenFrame() const
{
    if (type() == TType::TICKS) {
        _lenFrame = tempo->tick2time(_lenTick, _lenFrame, &sn) * MScore::sampleRate;
    }
    return _lenFrame;
}

//---------------------------------------------------------
//   end
//---------------------------------------------------------

Pos PosLen::end() const
{
    Pos pos(*this);
    pos.invalidSn();
    switch (type()) {
    case TType::FRAMES:
        pos.setFrame(pos.frame() + _lenFrame);
        break;
    case TType::TICKS:
        pos.setTick(pos.tick() + _lenTick);
        break;
    }
    return pos;
}

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void PosLen::setPos(const Pos& pos)
{
    switch (pos.type()) {
    case TType::FRAMES:
        setFrame(pos.frame());
        break;
    case TType::TICKS:
        setTick(pos.tick());
        break;
    }
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool PosLen::operator==(const PosLen& pl) const
{
    if (type() == TType::TICKS) {
        return _lenTick == pl._lenTick && Pos::operator==((const Pos&)pl);
    } else {
        return _lenFrame == pl._lenFrame && Pos::operator==((const Pos&)pl);
    }
}

//---------------------------------------------------------
//   mbt
//---------------------------------------------------------

void Pos::mbt(int* bar, int* beat, int* tk) const
{
    sig->tickValues(tick(), bar, beat, tk);
}

//---------------------------------------------------------
//   msf
//---------------------------------------------------------

void Pos::msf(int* min, int* sec, int* fr, int* subFrame) const
{
    // for further testing:

    double time = double(frame()) / double(MScore::sampleRate);
    *min        = int(time) / 60;
    *sec        = int(time) % 60;
    double rest = time - ((*min) * 60 + (*sec));
    switch (MScore::mtcType) {
    case 0:             // 24 frames sec
        rest *= 24;
        break;
    case 1:             // 25
        rest *= 25;
        break;
    case 2:             // 30 drop frame
        rest *= 30;
        break;
    case 3:             // 30 non drop frame
        rest *= 30;
        break;
    }
    *fr       = lrint(rest);
    *subFrame = lrint((rest - (*fr)) * 100.0);
}

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

SigEvent Pos::timesig() const
{
    return sig->timesig(tick());
}

//---------------------------------------------------------
//   snap
//    raster = 1  no snap
//    raster = 0  snap to measure
//    all other raster values snap to raster tick
//---------------------------------------------------------

void Pos::snap(int raster)
{
    setTick(sig->raster(tick(), raster));
}

void Pos::upSnap(int raster)
{
    setTick(sig->raster2(tick(), raster));
}

void Pos::downSnap(int raster)
{
    setTick(sig->raster1(tick(), raster));
}

Pos Pos::snapped(int raster) const
{
    return Pos(tempo, sig, sig->raster(tick(), raster));
}

Pos Pos::upSnapped(int raster) const
{
    return Pos(tempo, sig, sig->raster2(tick(), raster));
}

Pos Pos::downSnapped(int raster) const
{
    return Pos(tempo, sig, sig->raster1(tick(), raster));
}
}

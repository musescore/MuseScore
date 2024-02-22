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
    m_tempo  = 0;
    m_sig    = 0;
    m_type  = TType::TICKS;
    m_tick  = 0;
    m_frame = 0;
    m_sn     = -1;
    m_valid = false;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl)
{
    m_tempo  = tl;
    m_sig    = sl;
    m_type   = TType::TICKS;
    m_tick   = 0;
    m_frame  = 0;
    m_sn      = -1;
    m_valid  = false;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, unsigned t, TType timeType)
{
    m_tempo  = tl;
    m_sig    = sl;
    m_type = timeType;
    if (m_type == TType::TICKS) {
        m_tick   = t;
    } else {
        m_frame = t;
    }
    m_sn = -1;
    m_valid = true;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, const String& s)
{
    m_tempo  = tl;
    m_sig    = sl;
    int m, b, t;
    sscanf(s.toAscii().constChar(), "%04d.%02d.%03d", &m, &b, &t);
    m_tick  = m_sig->bar2tick(m, b) + t;
    m_type  = TType::TICKS;
    m_frame = 0;
    m_sn     = -1;
    m_valid = true;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, int measure, int beat, int tick)
{
    m_tempo  = tl;
    m_sig    = sl;
    m_tick  = m_sig->bar2tick(measure, beat) + tick;
    m_type  = TType::TICKS;
    m_frame = 0;
    m_sn     = -1;
    m_valid = true;
}

Pos::Pos(TempoMap* tl, TimeSigMap* sl, int min, int sec, int frame, int subframe)
{
    m_tempo  = tl;
    m_sig    = sl;
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
    m_type  = TType::FRAMES;
    m_tick  = 0;
    m_frame = lrint(time * MScore::sampleRate);
    m_sn     = -1;
    m_valid = true;
}

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void Pos::setType(TType t)
{
    if (t == m_type) {
        return;
    }

    if (m_type == TType::TICKS) {
        // convert from ticks to frames
        m_frame = m_tempo->tick2time(m_tick, m_frame, &m_sn) * MScore::sampleRate;
    } else {
        // convert from frames to ticks
        m_tick = m_tempo->time2tick(m_frame / MScore::sampleRate, m_tick, &m_sn);
    }
    m_type = t;
}

//---------------------------------------------------------
//   operator+=
//---------------------------------------------------------

Pos& Pos::operator+=(const Pos& a)
{
    if (m_type == TType::FRAMES) {
        m_frame += a.frame();
    } else {
        m_tick += a.tick();
    }
    m_sn = -1;            // invalidate cached values
    return *this;
}

//---------------------------------------------------------
//   operator-=
//---------------------------------------------------------

Pos& Pos::operator-=(const Pos& a)
{
    if (m_type == TType::FRAMES) {
        m_frame -= a.frame();
    } else {
        m_tick -= a.tick();
    }
    m_sn = -1;            // invalidate cached values
    return *this;
}

//---------------------------------------------------------
//   operator+=
//---------------------------------------------------------

Pos& Pos::operator+=(int a)
{
    if (m_type == TType::FRAMES) {
        m_frame += a;
    } else {
        m_tick += a;
    }
    m_sn = -1;            // invalidate cached values
    return *this;
}

//---------------------------------------------------------
//   operator-=
//---------------------------------------------------------

Pos& Pos::operator-=(int a)
{
    if (m_type == TType::FRAMES) {
        m_frame -= a;
    } else {
        m_tick -= a;
    }
    m_sn = -1;            // invalidate cached values
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
    if (m_type == TType::FRAMES) {
        return m_frame >= s.frame();
    } else {
        return m_tick >= s.tick();
    }
}

bool Pos::operator>(const Pos& s) const
{
    if (m_type == TType::FRAMES) {
        return m_frame > s.frame();
    } else {
        return m_tick > s.tick();
    }
}

bool Pos::operator<(const Pos& s) const
{
    if (m_type == TType::FRAMES) {
        return m_frame < s.frame();
    } else {
        return m_tick < s.tick();
    }
}

bool Pos::operator<=(const Pos& s) const
{
    if (m_type == TType::FRAMES) {
        return m_frame <= s.frame();
    } else {
        return m_tick <= s.tick();
    }
}

bool Pos::operator==(const Pos& s) const
{
    if (m_type == TType::FRAMES) {
        return m_frame == s.frame();
    } else {
        return m_tick == s.tick();
    }
}

bool Pos::operator!=(const Pos& s) const
{
    if (m_type == TType::FRAMES) {
        return m_frame != s.frame();
    } else {
        return m_tick != s.tick();
    }
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

unsigned Pos::tick() const
{
    if (m_type == TType::FRAMES) {
        m_tick = m_tempo->time2tick(m_frame / MScore::sampleRate, m_tick, &m_sn);
    }
    return m_tick;
}

//---------------------------------------------------------
//   frame
//---------------------------------------------------------

unsigned Pos::frame() const
{
    if (m_type == TType::TICKS) {
        // double time = _frame / MScore::sampleRate;
        // _frame = tempo->tick2time(_tick, time, &sn) * MScore::sampleRate;
        m_frame = m_tempo->tick2time(m_tick) * MScore::sampleRate;
    }
    return m_frame;
}

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Pos::setTick(unsigned pos)
{
    m_tick = pos;
    m_sn    = -1;
    if (m_type == TType::FRAMES) {
        m_frame = m_tempo->tick2time(pos, &m_sn) * MScore::sampleRate;
    }
    m_valid = true;
}

//---------------------------------------------------------
//   setFrame
//---------------------------------------------------------

void Pos::setFrame(unsigned pos)
{
    m_frame = pos;
    m_sn     = -1;
    if (m_type == TType::TICKS) {
        m_tick = m_tempo->time2tick(pos / MScore::sampleRate, &m_sn);
    }
    m_valid = true;
}

//---------------------------------------------------------
//   PosLen
//---------------------------------------------------------

PosLen::PosLen(TempoMap* tl, TimeSigMap* sl)
    : Pos(tl, sl)
{
    m_lenTick  = 0;
    m_lenFrame = 0;
    m_sn        = -1;
}

PosLen::PosLen(const PosLen& p)
    : Pos(p)
{
    m_lenTick  = p.m_lenTick;
    m_lenFrame = p.m_lenFrame;
    m_sn = -1;
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
        LOGD("samples=%d)", m_lenFrame);
        break;
    case TType::TICKS:
        LOGD("ticks=%d)", m_lenTick);
        break;
    }
}

void Pos::dump(int /*n*/) const
{
    LOGD("Pos(%s, sn=%d, ", type() == TType::FRAMES ? "Frames" : "Ticks", m_sn);
    switch (type()) {
    case TType::FRAMES:
        LOGD("samples=%d)", m_frame);
        break;
    case TType::TICKS:
        LOGD("ticks=%d)", m_tick);
        break;
    }
}

//---------------------------------------------------------
//   setLenTick
//---------------------------------------------------------

void PosLen::setLenTick(unsigned len)
{
    m_lenTick = len;
    m_sn       = -1;
    if (type() == TType::FRAMES) {
        m_lenFrame = m_tempo->tick2time(len, &m_sn) * MScore::sampleRate;
    } else {
        m_lenTick = len;
    }
}

//---------------------------------------------------------
//   setLenFrame
//---------------------------------------------------------

void PosLen::setLenFrame(unsigned len)
{
    m_sn      = -1;
    if (type() == TType::TICKS) {
        m_lenTick = m_tempo->time2tick(len / MScore::sampleRate, &m_sn);
    } else {
        m_lenFrame = len;
    }
}

//---------------------------------------------------------
//   lenTick
//---------------------------------------------------------

unsigned PosLen::lenTick() const
{
    if (type() == TType::FRAMES) {
        m_lenTick = m_tempo->time2tick(m_lenFrame / MScore::sampleRate, m_lenTick, &m_sn);
    }
    return m_lenTick;
}

//---------------------------------------------------------
//   lenFrame
//---------------------------------------------------------

unsigned PosLen::lenFrame() const
{
    if (type() == TType::TICKS) {
        m_lenFrame = m_tempo->tick2time(m_lenTick, m_lenFrame, &m_sn) * MScore::sampleRate;
    }
    return m_lenFrame;
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
        pos.setFrame(pos.frame() + m_lenFrame);
        break;
    case TType::TICKS:
        pos.setTick(pos.tick() + m_lenTick);
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
        return m_lenTick == pl.m_lenTick && Pos::operator==((const Pos&)pl);
    } else {
        return m_lenFrame == pl.m_lenFrame && Pos::operator==((const Pos&)pl);
    }
}

//---------------------------------------------------------
//   mbt
//---------------------------------------------------------

void Pos::mbt(int* bar, int* beat, int* tk) const
{
    m_sig->tickValues(tick(), bar, beat, tk);
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
    return m_sig->timesig(tick());
}

//---------------------------------------------------------
//   snap
//    raster = 1  no snap
//    raster = 0  snap to measure
//    all other raster values snap to raster tick
//---------------------------------------------------------

void Pos::snap(int raster)
{
    setTick(m_sig->raster(tick(), raster));
}

void Pos::upSnap(int raster)
{
    setTick(m_sig->raster2(tick(), raster));
}

void Pos::downSnap(int raster)
{
    setTick(m_sig->raster1(tick(), raster));
}

Pos Pos::snapped(int raster) const
{
    return Pos(m_tempo, m_sig, m_sig->raster(tick(), raster));
}

Pos Pos::upSnapped(int raster) const
{
    return Pos(m_tempo, m_sig, m_sig->raster2(tick(), raster));
}

Pos Pos::downSnapped(int raster) const
{
    return Pos(m_tempo, m_sig, m_sig->raster1(tick(), raster));
}
}

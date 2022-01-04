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

#include "tempo.h"

#include <cmath>

#include "rw/xml.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   TEvent
//---------------------------------------------------------

TEvent::TEvent()
{
    type     = TempoType::INVALID;
    tempo    = 0.0;
    pause    = 0.0;
    time     = 0.0;
}

TEvent::TEvent(const TEvent& e)
{
    type  = e.type;
    tempo = e.tempo;
    pause = e.pause;
    time  = e.time;
}

TEvent::TEvent(BeatsPerSecond t, qreal p, TempoType tp)
{
    type  = tp;
    tempo = t;
    pause = p;
    time  = 0.0;
}

bool TEvent::valid() const
{
    return !(!type);
}

//---------------------------------------------------------
//   TempoMap
//---------------------------------------------------------

TempoMap::TempoMap()
{
    _tempo    = 2.0;          // default fixed tempo in beat per second
    _tempoSN  = 1;
    _relTempo = 1.0;
}

//---------------------------------------------------------
//   setPause
//---------------------------------------------------------

void TempoMap::setPause(int tick, qreal pause)
{
    auto e = find(tick);
    if (e != end()) {
        e->second.pause = pause;
        e->second.type |= TempoType::PAUSE;
    } else {
        BeatsPerSecond t = tempo(tick);
        insert(std::pair<const int, TEvent>(tick, TEvent(t, pause, TempoType::PAUSE)));
    }
    normalize();
}

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void TempoMap::setTempo(int tick, BeatsPerSecond tempo)
{
    auto e = find(tick);
    if (e != end()) {
        e->second.tempo = tempo;
        e->second.type |= TempoType::FIX;
    } else {
        insert(std::pair<const int, TEvent>(tick, TEvent(tempo, 0.0, TempoType::FIX)));
    }
    normalize();
}

//---------------------------------------------------------
//   TempoMap::normalize
//---------------------------------------------------------

void TempoMap::normalize()
{
    qreal time  = 0;
    int tick    = 0;
    BeatsPerSecond tempo = 2.0;
    for (auto e = begin(); e != end(); ++e) {
        // entries that represent a pause *only* (not tempo change also)
        // need to be corrected to continue previous tempo
        if (!(e->second.type & (TempoType::FIX | TempoType::RAMP))) {
            e->second.tempo = tempo;
        }
        int delta = e->first - tick;
        time += qreal(delta) / (Constant::division * tempo.val * _relTempo.val);
        time += e->second.pause;
        e->second.time = time;
        tick  = e->first;
        tempo = e->second.tempo.val;
    }
    ++_tempoSN;
}

//---------------------------------------------------------
//   TempoMap::dump
//---------------------------------------------------------

void TempoMap::dump() const
{
    qDebug("\nTempoMap:");
    for (auto i = begin(); i != end(); ++i) {
        qDebug("%6d type: %2d tempo: %f pause: %f time: %f",
               i->first, static_cast<int>(i->second.type), i->second.tempo.val, i->second.pause, i->second.time);
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void TempoMap::clear()
{
    std::map<int, TEvent>::clear();
    ++_tempoSN;
}

//---------------------------------------------------------
//   clearRange
//    Clears the given range, start tick included, end tick
//    excluded.
//---------------------------------------------------------

void TempoMap::clearRange(int tick1, int tick2)
{
    iterator first = lower_bound(tick1);
    iterator last = lower_bound(tick2);
    if (first == last) {
        return;
    }
    erase(first, last);
    ++_tempoSN;
}

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

BeatsPerSecond TempoMap::tempo(int tick) const
{
    if (empty()) {
        return 2.0;
    }
    auto i = lower_bound(tick);
    if (i == end()) {
        --i;
        return i->second.tempo;
    }
    if (i->first == tick) {
        return i->second.tempo;
    }
    if (i == begin()) {
        return 2.0;
    }
    --i;
    return i->second.tempo;
}

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TempoMap::del(int tick)
{
    auto e = find(tick);
    if (e == end()) {
        qDebug("TempoMap::del event at (%d): not found", tick);
        // abort();
        return;
    }
    // don't delete event if still being used for pause
    if (e->second.type & TempoType::PAUSE) {
        e->second.type = TempoType::PAUSE;
    } else {
        erase(e);
    }
    normalize();
}

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void TempoMap::setRelTempo(BeatsPerSecond val)
{
    _relTempo = val;
    normalize();
}

//---------------------------------------------------------
//   delTempo
//---------------------------------------------------------

void TempoMap::delTempo(int tick)
{
    del(tick);
    ++_tempoSN;
}

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

qreal TempoMap::tick2time(int tick, qreal time, int* sn) const
{
    return (*sn == _tempoSN) ? time : tick2time(tick, sn);
}

//---------------------------------------------------------
//   time2tick
//    return cached value t if list did not change
//---------------------------------------------------------

int TempoMap::time2tick(qreal time, int t, int* sn) const
{
    return (*sn == _tempoSN) ? t : time2tick(time, sn);
}

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

qreal TempoMap::tick2time(int tick, int* sn) const
{
    qreal time  = 0.0;
    qreal delta = qreal(tick);
    BeatsPerSecond tempo = 2.0;

    if (!empty()) {
        int ptick  = 0;
        auto e = lower_bound(tick);
        if (e == end()) {
            auto pe = e;
            --pe;
            ptick = pe->first;
            tempo = pe->second.tempo;
            time  = pe->second.time;
        } else if (e->first == tick) {
            ptick = tick;
            tempo = e->second.tempo;
            time  = e->second.time;
        } else if (e != begin()) {
            auto pe = e;
            --pe;
            ptick = pe->first;
            tempo = pe->second.tempo;
            time  = pe->second.time;
        }
        delta = qreal(tick - ptick);
    } else {
        qDebug("TempoMap: empty");
    }
    if (sn) {
        *sn = _tempoSN;
    }
    time += delta / (Constant::division * tempo.val * _relTempo.val);
    return time;
}

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

int TempoMap::time2tick(qreal time, int* sn) const
{
    int tick     = 0;
    qreal delta = time;
    BeatsPerSecond tempo = _tempo;

    delta = 0.0;
    tempo = 2.0;
    for (auto e = begin(); e != end(); ++e) {
        // if in a pause period, wait on previous tick
        if ((time <= e->second.time) && (time > e->second.time - e->second.pause)) {
            delta = (time - (e->second.time - e->second.pause) + delta);
            break;
        }
        if (e->second.time >= time) {
            break;
        }
        delta = e->second.time;
        tick  = e->first;
        tempo = e->second.tempo;
    }
    delta = time - delta;
    tick += lrint(delta * _relTempo.val * Constant::division * tempo.val);
    if (sn) {
        *sn = _tempoSN;
    }
    return tick;
}
}

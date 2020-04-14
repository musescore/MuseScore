//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2007 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "sig.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   ticks_beat
//---------------------------------------------------------

int ticks_beat(int n)
      {
      int m = (MScore::division * 4) / n;
      if ((MScore::division * 4) % n) {
            qFatal("Mscore: ticks_beat(): bad divisor %d", n);
            }
      return m;
      }

//---------------------------------------------------------
//   ticks_measure
//---------------------------------------------------------

static int ticks_measure(const Fraction& f)
      {
      return (MScore::division * 4 * f.numerator()) / f.denominator();
      }

//---------------------------------------------------------
//   rtick2beatType
//    caller must adjust rtick as appropriate if the measure's
//    actual timeSig is different from the nominal timeSig.
//---------------------------------------------------------

BeatType TimeSigFrac::rtick2beatType(int rtick) const
      {
      if (rtick == 0)
            return BeatType::DOWNBEAT; // note: only return DOWNBEAT for rtick = 0, not for rtick = measureTicks,

      if (rtick % dUnitTicks() != 0)
            return BeatType::SUBBEAT;

      if (isCompound()) {
            if (rtick % beatTicks() != 0)
                  return BeatType::COMPOUND_SUBBEAT;
            }

      const int beatNum = rtick / beatTicks();

      int stressBeat = 0;

      if (isTriple())
            stressBeat = 3;
      else if (isDuple())
            stressBeat = 2;
      else
            stressBeat = (numerator() + 1) / 2; // Assumes 5/4 timeSig = (3+2)/4. (The same assumption is used for beaming)

      if (stressBeat && beatNum % stressBeat == 0)
            return isCompound() ? BeatType::COMPOUND_STRESSED : BeatType::SIMPLE_STRESSED;

      return isCompound() ? BeatType::COMPOUND_UNSTRESSED : BeatType::SIMPLE_UNSTRESSED;
      }

//---------------------------------------------------------
//   strongestBeatInRange
//    dUnitsCrossed - pointer to store number crossed
//    subbeatTick - pointer to store tick of strongest beat
//    saveLast - which tick to store if strongest type is
//                crossed more than once
//
//    caller must adjust rticks as appropriate if the measure's
//    actual timeSig is different from the nominal timeSig.
//---------------------------------------------------------

BeatType TimeSigFrac::strongestBeatInRange(int rtick1, int rtick2, int* dUnitsCrossed, int* subbeatTick, bool saveLast) const
      {
      Q_ASSERT(rtick2 > rtick1);

      BeatType strongest = BeatType::SUBBEAT;

      for (int rtick = rtick1 + ticksToNextDUnit(rtick1); rtick < rtick2; rtick += dUnitTicks()) {
            if (dUnitsCrossed)
                  (*dUnitsCrossed)++;
            BeatType type = rtick2beatType(rtick);
            if (static_cast<int>(type) < static_cast<int>(strongest) + saveLast) { // "<" behaves like "<=" if saveLast is true
                  strongest = type;
                  if (subbeatTick)
                        (*subbeatTick) = rtick;
                  }
            }

      return strongest;
      }

//---------------------------------------------------------
//   subbeatTicks
//    divides dUnitTicks() by 2 once for each level.
//---------------------------------------------------------

int TimeSigFrac::subbeatTicks(int level) const
      {
      Q_ASSERT(level <= maxSubbeatLevel());
      int subbeatTicks = dUnitTicks();
      while (level > 0) {
            subbeatTicks /= 2;
            level--;
            }
      return subbeatTicks;
      }

//---------------------------------------------------------
//   maxSubbeatLevel
//    subdivision beyond this level would result in rounding errors
//---------------------------------------------------------

int TimeSigFrac::maxSubbeatLevel() const
      {
      int level = 0;
      int subbeatTicks = dUnitTicks();
      while (subbeatTicks % 2 == 0) {
            subbeatTicks /= 2;
            level++;
            }
      return level;
      }

//---------------------------------------------------------
//   rtick2subbeatLevel
//    returns 0 if rtick is on a beat or denominator unit.
//    returns 1 if rtick lies halfway between dUnits
//    returns 2 if rtick lies on a multiple of  1/4  of dUnit
//            3                                 1/8
//            4                                 1/16
//            n                                 1/(2**n)
//    returns -(n+1) if max n is reached and rtick still not found.
//
//    Caller must adjust rtick as appropriate if the measure's
//    actual timeSig is different from the nominal timeSig.
//---------------------------------------------------------

int TimeSigFrac::rtick2subbeatLevel(int rtick) const
      {
      int level = 0;
      int subbeatTicks = dUnitTicks();
      int remainder = rtick % subbeatTicks;
      while (remainder != 0) {
            level++;
            if (subbeatTicks % 2 != 0)
                  return -level; // further sub-division would split measure into chunks of unequal length.
            subbeatTicks /= 2;
            remainder %= subbeatTicks;
            }
      return level;
      }

//---------------------------------------------------------
//   strongestSubbeatLevelInRange
//    Return value is negative if none are found.
//
//    Caller must adjust rtick as appropriate if the measure's
//    actual timeSig is different from the nominal timeSig.
//---------------------------------------------------------

int TimeSigFrac::strongestSubbeatLevelInRange(int rtick1, int rtick2, int* subbeatTick) const
      {
      Q_ASSERT(rtick2 > rtick1);

      for (int level = 0, subbeatTicks = dUnitTicks();;) {
            int n = rtick1 / subbeatTicks;
            int m = (rtick2 - 1) / subbeatTicks; // -1 to make the range exclusive
            if (m > n) {
                  if (subbeatTick)
                        (*subbeatTick) = m * subbeatTicks;
                  return level;
                  }
            level++;
            if (subbeatTicks % 2 != 0)
                  return -level; // further sub-division would split measure into chunks of unequal length.
            subbeatTicks /= 2;
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SigEvent::operator==(const SigEvent& e) const
      {
      return (_timesig.identical(e._timesig));
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TimeSigMap::add(int tick, const Fraction& f)
      {
      if (!f.isValid()) {
            qDebug("illegal signature %d/%d", f.numerator(), f.denominator());
            }
      (*this)[tick] = SigEvent(f);
      normalize();
      }

void TimeSigMap::add(int tick, const SigEvent& ev)
      {
      (*this)[tick] = ev;
      normalize();
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TimeSigMap::del(int tick)
      {
      erase(tick);
      normalize();
      }

//---------------------------------------------------------
//   clearRange
//    Clears the given range, start tick included, end tick
//    excluded.
//---------------------------------------------------------

void TimeSigMap::clearRange(int tick1, int tick2)
      {
      iterator first = lower_bound(tick1);
      iterator last = lower_bound(tick2);
      if (first == last)
            return;
      erase(first, last);
      normalize();
      }

//---------------------------------------------------------
//   TimeSigMap::normalize
//---------------------------------------------------------

void TimeSigMap::normalize()
      {
      int z    = 4;
      int n    = 4;
      int tick = 0;
      TimeSigFrac bar;
      int tm   = ticks_measure(TimeSigFrac(z, n));

      for (auto i = begin(); i != end(); ++i) {
            SigEvent& e  = i->second;
            bar += TimeSigFrac(i->first - tick, tm).reduced();
            e.setBar(bar.numerator() / bar.denominator());
            tick = i->first;
            tm   = ticks_measure(e.timesig());
            }
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

const SigEvent& TimeSigMap::timesig(int tick) const
      {
      static const SigEvent ev(TimeSigFrac(4, 4));
      if (empty())
            return ev;
      auto i = upper_bound(tick);
      if (i != begin())
            --i;
      return i->second;
      }

//---------------------------------------------------------
//   tickValues
//    t - some time moment on timeline (in ticks)
//
//    Result - values computed for this time moment:
//    bar - index of bar containing time moment t
//    beat - index of beat in bar containing t
//    tick - position of t in beat (in ticks)
//---------------------------------------------------------

void TimeSigMap::tickValues(int t, int* bar, int* beat, int* tick) const
      {
      if (empty()) {
            *bar  = 0;
            *beat = 0;
            *tick = 0;
            return;
            }
      auto e = upper_bound(t);
      if (empty() || e == begin()) {
            qFatal("tickValue(0x%x) not found", t);
            }
      --e;
      int delta  = t - e->first;
      int ticksB = ticks_beat(e->second.timesig().denominator()); // ticks in beat
      int ticksM = ticksB * e->second.timesig().numerator();      // ticks in measure (bar)
      if (ticksM == 0) {
            qDebug("TimeSigMap::tickValues: at %d %s", t, qPrintable(e->second.timesig().print()));
            *bar  = 0;
            *beat = 0;
            *tick = 0;
            return;
            }
      *bar       = e->second.bar() + delta / ticksM;
      int rest   = delta % ticksM;
      *beat      = rest / ticksB;
      *tick      = rest % ticksB;
      }

//---------------------------------------------------------
//   pos
//    Return string representation of tick position.
//    This is not reentrant and only for debugging!
//---------------------------------------------------------

QString TimeSigMap::pos(int t) const
      {
      int bar, beat, tick;
      tickValues(t, &bar, &beat, &tick);
      return QString("%1:%2:%3").arg(bar+1).arg(beat).arg(tick);
      }

//---------------------------------------------------------
//   bar2tick
//    Returns the absolute start time (in ticks)
//    of beat in bar
//---------------------------------------------------------

int TimeSigMap::bar2tick(int bar, int beat) const
      {
      // bar - index of current bar (terminology: bar == measure)
      // beat - index of beat in current bar
      auto e = begin();

      for (; e != end(); ++e) {
            if (bar < e->second.bar())
                  break;
            }
      if (empty() || e == begin()) {
            qDebug("TimeSigMap::bar2tick(): not found(%d,%d) not found", bar, beat);
            if (empty())
                  qDebug("   list is empty");
            return 0;
            }
      --e; // current TimeSigMap value
      int ticksB = ticks_beat(e->second.timesig().denominator()); // ticks per beat
      int ticksM = ticksB * e->second.timesig().numerator();      // bar length in ticks
      return e->first + (bar - e->second.bar()) * ticksM + ticksB * beat;
      }

//---------------------------------------------------------
//   TimeSigMap::write
//---------------------------------------------------------

void TimeSigMap::write(XmlWriter& xml) const
      {
      xml.stag("siglist");
      for (auto i = begin(); i != end(); ++i)
            i->second.write(xml, i->first);
      xml.etag();
      }

//---------------------------------------------------------
//   TimeSigMap::read
//---------------------------------------------------------

void TimeSigMap::read(XmlReader& e, int fileDivision)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "sig") {
                  SigEvent t;
                  int tick = t.read(e, fileDivision);
                  (*this)[tick] = t;
                  }
            else
                  e.unknown();
            }
      normalize();
      }

//---------------------------------------------------------
//   SigEvent
//---------------------------------------------------------

SigEvent::SigEvent(const SigEvent& e)
      {
      _timesig = e._timesig;
      _nominal = e._nominal;
      _bar     = e._bar;
      }

//---------------------------------------------------------
//   SigEvent::write
//---------------------------------------------------------

void SigEvent::write(XmlWriter& xml, int tick) const
      {
      xml.stag(QString("sig tick=\"%1\"").arg(tick));
      xml.tag("nom",   _timesig.numerator());
      xml.tag("denom", _timesig.denominator());
      xml.etag();
      }

//---------------------------------------------------------
//   SigEvent::read
//---------------------------------------------------------

int SigEvent::read(XmlReader& e, int fileDivision)
      {
      int tick  = e.intAttribute("tick", 0);
      tick      = tick * MScore::division / fileDivision;

      int numerator = 1;
      int denominator = 1;
      int denominator2 = -1;
      int numerator2   = -1;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "nom")
                  numerator = e.readInt();
            else if (tag == "denom")
                  denominator = e.readInt();
            else if (tag == "nom2")
                  numerator2 = e.readInt();
            else if (tag == "denom2")
                  denominator2 = e.readInt();
            else
                  e.unknown();
            }
      if ((numerator2 == -1) || (denominator2 == -1)) {
            numerator2   = numerator;
            denominator2 = denominator;
            }
      _timesig = TimeSigFrac(numerator, denominator);
      _nominal = TimeSigFrac(numerator2, denominator2);
      return tick;
      }

//---------------------------------------------------------
//   ticksPerMeasure
//---------------------------------------------------------

int ticksPerMeasure(int numerator, int denominator)
      {
      return ticks_beat(denominator) * numerator;
      }

//---------------------------------------------------------
//   rasterEval
//---------------------------------------------------------

unsigned rasterEval(unsigned t, int raster, int startTick,
         int numerator, int denominator, int addition)
      {
      int delta  = t - startTick;
      int ticksM = ticksPerMeasure(numerator, denominator);
      if (raster == 0)
            raster = ticksM;
      int rest   = delta % ticksM;
      int bb     = (delta / ticksM) * ticksM;
      return startTick + bb + ((rest + addition) / raster) * raster;
      }

//---------------------------------------------------------
//   raster
//---------------------------------------------------------

unsigned TimeSigMap::raster(unsigned t, int raster) const
      {
      if (raster == 1)
            return t;
      auto e = upper_bound(t);
      if (e == end()) {
            qDebug("TimeSigMap::raster(%x,)", t);
            return t;
            }
      auto timesig = e->second.timesig();
      return rasterEval(t, raster, e->first, timesig.numerator(),
                        timesig.denominator(), raster / 2);
      }

//---------------------------------------------------------
//   raster1
//    round down
//---------------------------------------------------------

unsigned TimeSigMap::raster1(unsigned t, int raster) const
      {
      if (raster == 1)
            return t;
      auto e = upper_bound(t);
      auto timesig = e->second.timesig();
      return rasterEval(t, raster, e->first, timesig.numerator(),
                        timesig.denominator(), 0);
      }

//---------------------------------------------------------
//   raster2
//    round up
//---------------------------------------------------------

unsigned TimeSigMap::raster2(unsigned t, int raster) const
      {
      if (raster == 1)
            return t;
      auto e = upper_bound(t);
      auto timesig = e->second.timesig();
      return rasterEval(t, raster, e->first, timesig.numerator(),
                        timesig.denominator(), raster - 1);
      }

//---------------------------------------------------------
//   rasterStep
//---------------------------------------------------------

int TimeSigMap::rasterStep(unsigned t, int raster) const
      {
      if (raster == 0) {
            auto timesig = upper_bound(t)->second.timesig();
            return ticksPerMeasure(timesig.denominator(), timesig.numerator());
            }
      return raster;
      }

//---------------------------------------------------------
//   TimeSigMap::dump
//---------------------------------------------------------

void TimeSigMap::dump() const
      {
      qDebug("TimeSigMap:");
      for (auto i = begin(); i != end(); ++i)
            qDebug("%6d timesig: %s measure: %d",
               i->first, qPrintable(i->second.timesig().print()), i->second.bar());
      }

//---------------------------------------------------------
//   dUnitTicks
//---------------------------------------------------------

int TimeSigFrac::dUnitTicks() const
      {
      return (4 * MScore::division) / denominator();
      }

}


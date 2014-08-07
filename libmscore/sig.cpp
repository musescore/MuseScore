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

static int ticks_beat(int n)
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
//   operator==
//---------------------------------------------------------

bool SigEvent::operator==(const SigEvent& e) const
      {
      return (_timesig.identical(e._timesig));
      }

//---------------------------------------------------------
//   TimeSigMap
//---------------------------------------------------------

TimeSigMap::TimeSigMap()
      {
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
//   TimeSigMap::normalize
//---------------------------------------------------------

void TimeSigMap::normalize()
      {
      int z    = 4;
      int n    = 4;
      int tick = 0;
      Fraction bar;
      int tm   = ticks_measure(Fraction(z, n));

      for (auto i = begin(); i != end(); ++i) {
            SigEvent& e  = i->second;
            bar += Fraction(i->first - tick, tm).reduced();
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
      static const SigEvent ev(Fraction(4, 4));
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

void TimeSigMap::write(Xml& xml) const
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

void SigEvent::write(Xml& xml, int tick) const
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
      _timesig = Fraction(numerator, denominator);
      _nominal = Fraction(numerator2, denominator2);
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


}


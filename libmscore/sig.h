//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2009 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __AL_SIG_H__
#define __AL_SIG_H__

#include "fraction.h"
#include "mscore.h"

namespace Ms {

class Xml;
class XmlReader;

//-------------------------------------------------------------------
//   BeatType
//-------------------------------------------------------------------

enum class BeatType : char {
      DOWNBEAT,               // 1st beat of measure (rtick == 0)
      SIMPLE_STRESSED,        // e.g. beat 3 in 4/4
      SIMPLE_UNSTRESSED,      // "offbeat" e.g. beat 2 and 4 in 4/4 (i.e. the denominator unit)
      COMPOUND_STRESSED,      // e.g. eighth-note number 7 in 12/8
      COMPOUND_UNSTRESSED,    // e.g. eighth-note numbers 4 or 10 in 12/8
      COMPOUND_SUBBEAT,       // e.g. any other eighth-note in 12/8 (i.e. the denominator unit)
      SUBBEAT                 // does not fall on a beat
      };

//-------------------------------------------------------------------
//   Time Signature Fraction   (n/d - numerator/denominator)
//-------------------------------------------------------------------

class TimeSigFrac : public Fraction {

   public:
      using Fraction::Fraction;
      constexpr TimeSigFrac(int n = 0, int d = 1) : Fraction(n, d) {}
      TimeSigFrac(const Fraction& f) : TimeSigFrac(f.numerator(), f.denominator()) {}
      TimeSigFrac(const TimeSigFrac& f) : TimeSigFrac(f.numerator(), f.denominator()) {}

      // isCompound? Note: 3/8, 3/16, ... are NOT considered compound.
      bool isCompound() { return numerator() > 3 /*&& denominator() >= 8*/ && numerator() % 3 == 0; }

      // isBeatedCompound? Note: Conductors will beat the simple unit at slow tempos (<60 compound units per minute)
      // However, the meter is still considered to be compound (at least for our purposes).
      bool isBeatedCompound(qreal tempo) { return tempo2beatsPerMinute(tempo) >= 60.0; }

      int dUnitTicks()        { return (4 * MScore::division) / denominator(); }
      int ticksPerMeasure()   { return numerator() * dUnitTicks(); }

      int dUnitsPerBeat()     { return isCompound() ? 3 : 1; }
      int beatTicks()         { return dUnitTicks() * dUnitsPerBeat(); }
      int beatsPerMeasure()   { return numerator() / dUnitsPerBeat(); }

      bool isTriple()   { return beatsPerMeasure() % 3 == 0; }
      bool isDuple()    { Q_ASSERT(!isTriple()); return beatsPerMeasure() % 2 == 0; } // note: always test isTriple() first

      // MuseScore stores tempos in quarter-notes-per-second, so conversions to conventional beats-per-minute format are provided here:
      qreal tempo2beatsPerMinute(qreal tempo)   { return tempo * denominator() * 15.0 / dUnitsPerBeat(); }
      qreal beatsPerMinute2tempo(qreal bpm)     { return bpm * dUnitsPerBeat() / (15.0 * denominator()); }

      BeatType rtick2beatType(int rtick);

      };

//-------------------------------------------------------------------
//   Time Signature Event
//    Incomplete measures as for example pickup measures have
//    a nominal duration different from actual duration.
//-------------------------------------------------------------------

class SigEvent {
      TimeSigFrac _timesig;
      TimeSigFrac _nominal;
      int _bar;               ///< precomputed value

   public:
      int read(XmlReader&, int fileDivision);
      void write(Xml&, int) const;

      constexpr SigEvent() : _bar(0) {}       ///< default SigEvent is invalid
      SigEvent(const Fraction& s, int bar = 0)
         : _timesig(s), _nominal(s), _bar(bar) {}
      SigEvent(const Fraction& s, const Fraction& ss, int bar = 0)
         : _timesig(s), _nominal(ss), _bar(bar) {}
      SigEvent(const SigEvent& e);

      bool operator==(const SigEvent& e) const;
      bool valid() const       { return _timesig.isValid(); }
      QString print() const    { return _timesig.print();  }
      TimeSigFrac timesig() const { return _timesig;          }
      TimeSigFrac nominal() const { return _nominal;          }
      void setNominal(const Fraction& f) { _nominal = f;  }
      int bar() const          { return _bar;              }
      void setBar(int val)     { _bar = val;               }
      };

//---------------------------------------------------------
//   SigList
//---------------------------------------------------------

class TimeSigMap : public std::map<int, SigEvent > {
      void normalize();

   public:
      TimeSigMap() {}

      void add(int tick, const Fraction&);
      void add(int tick, const SigEvent& ev);

      void del(int tick);

      void read(XmlReader&, int fileDiv);
      void write(Xml&) const;
      void dump() const;

      const SigEvent& timesig(int tick) const;

      void tickValues(int t, int* bar, int* beat, int* tick) const;
      int bar2tick(int bar, int beat) const;
      QString pos(int t) const;

      unsigned raster(unsigned tick, int raster) const;
      unsigned raster1(unsigned tick, int raster) const;    // round down
      unsigned raster2(unsigned tick, int raster) const;    // round up
      int rasterStep(unsigned tick, int raster) const;
      };

}     // namespace Ms
#endif

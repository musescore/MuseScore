//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "durationtype.h"
#include "mscore.h"
#include "note.h"

namespace Ms {

//---------------------------------------------------------
//   dots
//---------------------------------------------------------

static int getDots(int base, int rest, int* dots)
      {
      *dots = 0;
      if (rest >= base / 2) {
            *dots = *dots + 1;
            rest -= base / 2;
            }
      if (rest >= base / 4) {
            *dots = *dots + 1;
            rest -= base / 4;
            }
      if (*dots > MAX_DOTS)
            *dots = MAX_DOTS;
      return rest;
      }

//---------------------------------------------------------
//   setDots
//---------------------------------------------------------

void TDuration::setDots(int v)
      {
      if (v > MAX_DOTS)
            v = MAX_DOTS;
      _dots = v;
      }

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

void TDuration::setVal(int ticks)
      {
      if (ticks == 0)
            _val = DurationType::V_MEASURE;
      else {
            TDuration dt;
            for (int i = 0; i < int(TDuration::DurationType::V_ZERO); ++i) {
                  dt.setType(TDuration::DurationType(i));
                  int t = dt.ticks();
                  if (ticks / t) {
                        int remain = ticks % t;
                        if ((t - remain) < (t/4)) {
                              _val = DurationType(i - 1);
                              return;
                              }
                        _val = DurationType(i);
                        getDots(t, remain, &_dots);
                        return;
                        }
                  }
            qDebug("2: no duration type for ticks %d", ticks);
            _val = DurationType::V_QUARTER;       // fallback default value
            }
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int TDuration::ticks() const
      {
      int t;
      switch(_val) {
            case DurationType::V_QUARTER:   t = MScore::division;        break;
            case DurationType::V_1024TH:    t = MScore::division / 256;  break;
            case DurationType::V_512TH:     t = MScore::division / 128;  break;
            case DurationType::V_256TH:     t = MScore::division / 64;   break;
            case DurationType::V_128TH:     t = MScore::division / 32;   break;
            case DurationType::V_64TH:      t = MScore::division / 16;   break;
            case DurationType::V_32ND:      t = MScore::division / 8;    break;
            case DurationType::V_16TH:      t = MScore::division / 4;    break;
            case DurationType::V_EIGHTH:    t = MScore::division / 2;    break;
            case DurationType::V_HALF:      t = MScore::division * 2;    break;
            case DurationType::V_WHOLE:     t = MScore::division * 4;    break;
            case DurationType::V_BREVE:     t = MScore::division * 8;    break;
            case DurationType::V_LONG:      t = MScore::division * 16;   break;
            case DurationType::V_ZERO:
            case DurationType::V_MEASURE:
                  return 0;
            default:
            case DurationType::V_INVALID:
                  return -1;
            }
      int tmp = t;
      for (int i = 0; i < _dots; ++i)
            tmp += (t >> (i+1));
      return tmp;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString TDuration::name() const
      {
      switch(_val) {
            case DurationType::V_QUARTER:   return "quarter";
            case DurationType::V_EIGHTH:    return "eighth";
            case DurationType::V_1024TH:    return "1024th";
            case DurationType::V_512TH:     return "512th";
            case DurationType::V_256TH:     return "256th";
            case DurationType::V_128TH:     return "128th";
            case DurationType::V_64TH:      return "64th";
            case DurationType::V_32ND:      return "32nd";
            case DurationType::V_16TH:      return "16th";
            case DurationType::V_HALF:      return "half";
            case DurationType::V_WHOLE:     return "whole";
            case DurationType::V_MEASURE:   return "measure";
            case DurationType::V_BREVE:     return "breve";
            case DurationType::V_LONG:      return "long";
            default:
qDebug("TDuration::name(): invalid duration type %hhd", _val);
            case DurationType::V_ZERO:
            case DurationType::V_INVALID:   return "";
            }
      }

//---------------------------------------------------------
//   headType
//---------------------------------------------------------

NoteHead::Type TDuration::headType() const
      {
      NoteHead::Type headType = NoteHead::Type::HEAD_WHOLE;
      switch(_val) {
            case DurationType::V_1024TH:
            case DurationType::V_512TH:
            case DurationType::V_256TH:
            case DurationType::V_128TH:
            case DurationType::V_64TH:
            case DurationType::V_32ND:
            case DurationType::V_16TH:
            case DurationType::V_EIGHTH:
            case DurationType::V_QUARTER:
                  headType = NoteHead::Type::HEAD_QUARTER;
                  break;
            case DurationType::V_HALF:
                  headType = NoteHead::Type::HEAD_HALF;
                  break;
            case DurationType::V_MEASURE:
            case DurationType::V_WHOLE:
                  headType = NoteHead::Type::HEAD_WHOLE;
                  break;
            case DurationType::V_BREVE:
                  headType = NoteHead::Type::HEAD_BREVIS;
                  break;
            case DurationType::V_LONG:
                  headType = NoteHead::Type::HEAD_BREVIS;
                  break;
            default:
            case DurationType::V_INVALID:
            case DurationType::V_ZERO:
                  headType = NoteHead::Type::HEAD_QUARTER;
                  break;
            }
      return headType;
      }

//---------------------------------------------------------
//   hooks
//---------------------------------------------------------

int TDuration::hooks() const
      {
      static const int table[] = {
         // V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHTH, V_16TH,
            0,      0,       0,       0,      0,         1,        2,
         // V_32ND, V_64TH, V_128TH, V_256TH, V_512TH, V_1024TH,
            3,      4,       5,       6,      7,       8,
         // V_ZERO, V_MEASURE, V_INVALID
            0,      0,       0
            };
      return table[int(_val)];
      }

//---------------------------------------------------------
//   hasStem
//---------------------------------------------------------

bool TDuration::hasStem() const
      {
      switch(_val) {
            case DurationType::V_1024TH:
            case DurationType::V_512TH:
            case DurationType::V_256TH:
            case DurationType::V_128TH:
            case DurationType::V_64TH:
            case DurationType::V_32ND:
            case DurationType::V_16TH:
            case DurationType::V_EIGHTH:
            case DurationType::V_QUARTER:
            case DurationType::V_HALF:
            case DurationType::V_LONG:
                  return true;
            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

TDuration::TDuration(const QString& s)
      {
      setType(s);
      _dots = 0;
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void TDuration::setType(const QString& s)
      {
      if (s == "quarter")
            _val = DurationType::V_QUARTER;
      else if (s == "eighth")
            _val = DurationType::V_EIGHTH;
      else if (s == "1024th")
            _val = DurationType::V_1024TH;
      else if (s == "512th")
            _val = DurationType::V_512TH;
      else if (s == "256th")
            _val = DurationType::V_256TH;
      else if (s == "128th")
            _val = DurationType::V_128TH;
      else if (s == "64th")
            _val = DurationType::V_64TH;
      else if (s == "32nd")
            _val = DurationType::V_32ND;
      else if (s == "16th")
            _val = DurationType::V_16TH;
      else if (s == "half")
            _val = DurationType::V_HALF;
      else if (s == "whole")
            _val = DurationType::V_WHOLE;
      else if (s == "breve")
            _val = DurationType::V_BREVE;
      else if (s == "long")
            _val = DurationType::V_LONG;
      else if (s == "measure")
            _val = DurationType::V_MEASURE;
      else {
            // _val = V_INVALID;
            _val = DurationType::V_QUARTER;
            qDebug("TDuration::setType(%s): unknown, assume \"quarter\"", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   shift
//    this discardes any dots
//---------------------------------------------------------

TDuration TDuration::shift(int v) const
      {
      if (_val == DurationType::V_MEASURE || _val == DurationType::V_INVALID || _val == DurationType::V_ZERO)
            return TDuration();
      int newValue = int(_val) + v;
      if ((newValue < int(DurationType::V_LONG)) || (newValue > int(DurationType::V_256TH)))
            return TDuration();
      return TDuration(DurationType(newValue));
      }

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool TDuration::operator<(const TDuration& t) const
      {
      if (t._val < _val)
            return true;
      if (t._val == _val) {
            if (_dots < t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator>=
//---------------------------------------------------------

bool TDuration::operator>=(const TDuration& t) const
      {
      if (t._val > _val)
            return true;
      if (t._val == _val) {
            if (_dots >= t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator<=
//---------------------------------------------------------

bool TDuration::operator<=(const TDuration& t) const
      {
      if (t._val < _val)
            return true;
      if (t._val == _val) {
            if (_dots <= t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator>
//---------------------------------------------------------

bool TDuration::operator>(const TDuration& t) const
      {
      if (t._val > _val)
            return true;
      if (t._val == _val) {
            if (_dots > t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   fraction
//---------------------------------------------------------

Fraction TDuration::fraction() const
      {
      int z = 1;
      unsigned n;
      switch(_val) {
            case DurationType::V_1024TH:    n = 1024;     break;
            case DurationType::V_512TH:     n = 512;      break;
            case DurationType::V_256TH:     n = 256;      break;
            case DurationType::V_128TH:     n = 128;      break;
            case DurationType::V_64TH:      n = 64;       break;
            case DurationType::V_32ND:      n = 32;       break;
            case DurationType::V_16TH:      n = 16;       break;
            case DurationType::V_EIGHTH:    n = 8;        break;
            case DurationType::V_QUARTER:   n = 4;        break;
            case DurationType::V_HALF:      n = 2;        break;
            case DurationType::V_WHOLE:     n = 1;        break;
            case DurationType::V_BREVE:     z = 2; n = 1; break;
            case DurationType::V_LONG:      z = 4; n = 1; break;
            case DurationType::V_ZERO:      z = 0; n = 1; break;
            default:          z = 0; n = 0; break;    // zero+invalid fraction
            }
      Fraction a(z, n);
      if (a.isValid()) {
            for (int i = 0; i < _dots; ++i) {
                  n *= 2;
                  a += Fraction(z, n);
                  }
            }
      return a;
      }

TDuration::TDuration(const Fraction& _f)
      {
      Fraction f(_f.reduced());
      _dots = 0;
      if (f.numerator() == 0) {
            _val  = DurationType::V_ZERO;
            _dots = 0;
            return;
            }
      switch(f.denominator()) {
            case 1:     _val = DurationType::V_WHOLE; break;
            case 2:     _val = DurationType::V_HALF; break;
            case 4:     _val = DurationType::V_QUARTER; break;
            case 8:     _val = DurationType::V_EIGHTH; break;
            case 16:    _val = DurationType::V_16TH; break;
            case 32:    _val = DurationType::V_32ND; break;
            case 64:    _val = DurationType::V_64TH; break;
            case 128:   _val = DurationType::V_128TH; break;
            case 256:   _val = DurationType::V_256TH; break;
            case 512:   _val = DurationType::V_512TH; break;
            case 1024:  _val = DurationType::V_1024TH; break;
            default:    _val = DurationType::V_INVALID; break;
            }

      if (f.denominator() != 0) {
            int v = f.numerator() / f.denominator();
            if(v == 4) {
                  _val = DurationType::V_LONG;
                  return;
                  }
            else if (v == 2) {
                  _val = DurationType::V_BREVE;
                  return;
                  }
            }

      if (f.numerator() != 1) {
            switch(f.numerator()) {
                  case 3:
                        _val = DurationType(int(_val) - 1);
                        _dots = 1;
                        break;
                  case 7:
                        _val = DurationType(int(_val) - 2);
                        _dots = 2;
                        break;
                  default:
                        qDebug("TDuration(%d/%d): not implemented", f.numerator(), f.denominator());
// abort();
                        _val = DurationType::V_INVALID;
                        _dots = 0;
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   operator -=
//---------------------------------------------------------

TDuration& TDuration::operator-=(const TDuration& t)
      {
      Fraction f1 = fraction() - t.fraction();
      TDuration d(f1);
      _val  = d._val;
      _dots = d._dots;
      return *this;
      }

//---------------------------------------------------------
//   operator +=
//---------------------------------------------------------

TDuration& TDuration::operator+=(const TDuration& t)
      {
      Fraction f1 = fraction() + t.fraction();
      TDuration d(f1);
      _val  = d._val;
      _dots = d._dots;
      return *this;
      }

//---------------------------------------------------------
//   toDurationList
//---------------------------------------------------------

QList<TDuration> toDurationList(Fraction l, bool useDots, int maxDots, bool printRestRemains)
      {
      QList<TDuration> dList;
      if (useDots) {
            for (TDuration d = TDuration(TDuration::DurationType::V_LONG); d.isValid() && (l.numerator() != 0);) {
                  int dots = maxDots;
                  for ( ; dots > 0; --dots) {
                        d.setDots(dots);
                        Fraction ff = l - d.fraction();
                        if (ff.numerator() >= 0) {
                              dList.append(d);
                              l -= d.fraction();
                              break;
                              }
                        }
                  if (dots > 0)
                        continue;
                  d.setDots(0);
                  Fraction ff = l - d.fraction();
                  if (ff.numerator() < 0) {
                        d = d.shift(1);
                        }
                  else {
                        dList.append(d);
                        l -= d.fraction();
                        }
                  }
            }
      else {
            for (TDuration d = TDuration(TDuration::DurationType::V_LONG); d.isValid() && (l.numerator() != 0);) {
                  Fraction ff(l - d.fraction());
                  if (ff.numerator() < 0) {
                        d = d.shift(1);
                        continue;
                        }
                  l -= d.fraction();
                  dList.append(d);
                  }
            }
      if (printRestRemains && l != Fraction())
            qDebug("toDurationList:: rest remains %d/%d", l.numerator(), l.denominator());

      return dList;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

QString TDuration::durationTypeUserName() const
      {
      QString s = QObject::tr("Custom");
      switch(_val) {
            case DurationType::V_LONG:      s = QObject::tr("Long"   ); break;
            case DurationType::V_BREVE:     s = QObject::tr("Breve"  ); break;
            case DurationType::V_WHOLE:     s = QObject::tr("Whole"  ); break;
            case DurationType::V_HALF:      s = QObject::tr("Half"   ); break;
            case DurationType::V_QUARTER:   s = QObject::tr("Quarter"); break;
            case DurationType::V_EIGHTH:    s = QObject::tr("Eighth" ); break;
            case DurationType::V_16TH:      s = QObject::tr("16th"   ); break;
            case DurationType::V_32ND:      s = QObject::tr("32nd"   ); break;
            case DurationType::V_64TH:      s = QObject::tr("64th"   ); break;
            case DurationType::V_128TH:     s = QObject::tr("128th"  ); break;
            case DurationType::V_256TH:     s = QObject::tr("256th"  ); break;
            case DurationType::V_512TH:     s = QObject::tr("512th"  ); break;
            case DurationType::V_1024TH:    s = QObject::tr("1024th" ); break;
            case DurationType::V_ZERO:      s = QObject::tr("Zero"   ); break;
            case DurationType::V_MEASURE:   s = QObject::tr("Measure"); break;
            case DurationType::V_INVALID:   s = QObject::tr("Invalid"); break;
            };
      return s;
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void TDuration::setType(DurationType t)
      {
      _val = t;
      if (_val == DurationType::V_MEASURE)
            _dots = 0;
      }
}


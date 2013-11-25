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
      return rest;
      }

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

void TDuration::setVal(int ticks)
      {
      if (ticks == 0)
            _val = V_MEASURE;
      else {
            TDuration dt;
            for (int i = 0; i < TDuration::V_ZERO; ++i) {
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
            _val = V_QUARTER;       // fallback default value
            }
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int TDuration::ticks() const
      {
      int t;
      switch(_val) {
            case V_QUARTER:   t = MScore::division;        break;
            case V_1024TH:    t = MScore::division / 256;  break;
            case V_512TH:     t = MScore::division / 128;  break;
            case V_256TH:     t = MScore::division / 64;   break;
            case V_128TH:     t = MScore::division / 32;   break;
            case V_64TH:      t = MScore::division / 16;   break;
            case V_32ND:      t = MScore::division / 8;    break;
            case V_16TH:      t = MScore::division / 4;    break;
            case V_EIGHT:     t = MScore::division / 2;    break;
            case V_HALF:      t = MScore::division * 2;    break;
            case V_WHOLE:     t = MScore::division * 4;    break;
            case V_BREVE:     t = MScore::division * 8;    break;
            case V_LONG:      t = MScore::division * 16;   break;
            case V_ZERO:
            case V_MEASURE:
                  return 0;
            default:
            case V_INVALID:
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
            case V_QUARTER:   return "quarter";
            case V_EIGHT:     return "eighth";
            case V_1024TH:    return "1024th";
            case V_512TH:     return "512th";
            case V_256TH:     return "256th";
            case V_128TH:     return "128th";
            case V_64TH:      return "64th";
            case V_32ND:      return "32nd";
            case V_16TH:      return "16th";
            case V_HALF:      return "half";
            case V_WHOLE:     return "whole";
            case V_MEASURE:   return "measure";
            case V_BREVE:     return "breve";
            case V_LONG:      return "long";
            default:
qDebug("TDuration::name(): invalid duration type %d", _val);
            case V_ZERO:
            case V_INVALID:   return "";
            }
      }

//---------------------------------------------------------
//   headType
//---------------------------------------------------------

NoteHeadType TDuration::headType() const
      {
      NoteHeadType headType = NoteHeadType::HEAD_WHOLE;
      switch(_val) {
            case V_1024TH:
            case V_512TH:
            case V_256TH:
            case V_128TH:
            case V_64TH:
            case V_32ND:
            case V_16TH:
            case V_EIGHT:
            case V_QUARTER:
                  headType = NoteHeadType::HEAD_QUARTER;
                  break;
            case V_HALF:
                  headType = NoteHeadType::HEAD_HALF;
                  break;
            case V_MEASURE:
            case V_WHOLE:
                  headType = NoteHeadType::HEAD_WHOLE;
                  break;
            case V_BREVE:
                  headType = NoteHeadType::HEAD_BREVIS;
                  break;
            case V_LONG:
                  headType = NoteHeadType::HEAD_BREVIS;
                  break;
            default:
            case V_INVALID:
            case V_ZERO:
                  headType = NoteHeadType::HEAD_QUARTER;
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
         // V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHT, V_16TH,
            0,      0,       0,       0,      0,         1,       2,
         // V_32ND, V_64TH, V_128TH, V_256TH, V_512TH, V_1024TH,
            3,      4,       5,       6,      7,       8,
         // V_ZERO, V_MEASURE, V_INVALID
            0,      0,       0
            };
      return table[_val];
      }

//---------------------------------------------------------
//   hasStem
//---------------------------------------------------------

bool TDuration::hasStem() const
      {
      switch(_val) {
            case V_1024TH:
            case V_512TH:
            case V_256TH:
            case V_128TH:
            case V_64TH:
            case V_32ND:
            case V_16TH:
            case V_EIGHT:
            case V_QUARTER:
            case V_HALF:
            case V_LONG:
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
            _val = V_QUARTER;
      else if (s == "eighth")
            _val = V_EIGHT;
      else if (s == "1024th")
            _val = V_1024TH;
      else if (s == "512th")
            _val = V_512TH;
      else if (s == "256th")
            _val = V_256TH;
      else if (s == "128th")
            _val = V_128TH;
      else if (s == "64th")
            _val = V_64TH;
      else if (s == "32nd")
            _val = V_32ND;
      else if (s == "16th")
            _val = V_16TH;
      else if (s == "half")
            _val = V_HALF;
      else if (s == "whole")
            _val = V_WHOLE;
      else if (s == "breve")
            _val = V_BREVE;
      else if (s == "long")
            _val = V_LONG;
      else if (s == "measure")
            _val = V_MEASURE;
      else {
            // _val = V_INVALID;
            _val = V_QUARTER;
            qDebug("TDuration::setType(%s): unknown, assume \"quarter\"", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   shift
//    this discardes any dots
//---------------------------------------------------------

TDuration TDuration::shift(int v) const
      {
      if (_val == V_MEASURE || _val == V_INVALID || _val == V_ZERO)
            return TDuration();
      int newValue = _val + v;
      if ((newValue < 0) || (newValue > V_256TH))
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
            case V_1024TH:    n = 1024;     break;
            case V_512TH:     n = 512;      break;
            case V_256TH:     n = 256;      break;
            case V_128TH:     n = 128;      break;
            case V_64TH:      n = 64;       break;
            case V_32ND:      n = 32;       break;
            case V_16TH:      n = 16;       break;
            case V_EIGHT:     n = 8;        break;
            case V_QUARTER:   n = 4;        break;
            case V_HALF:      n = 2;        break;
            case V_WHOLE:     n = 1;        break;
            case V_BREVE:     z = 2; n = 1; break;
            case V_LONG:      z = 4; n = 1; break;
            case V_ZERO:      z = 0; n = 1; break;
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
            _val  = V_ZERO;
            _dots = 0;
            return;
            }
      switch(f.denominator()) {
            case 1:     _val = V_WHOLE; break;
            case 2:     _val = V_HALF; break;
            case 4:     _val = V_QUARTER; break;
            case 8:     _val = V_EIGHT; break;
            case 16:    _val = V_16TH; break;
            case 32:    _val = V_32ND; break;
            case 64:    _val = V_64TH; break;
            case 128:   _val = V_128TH; break;
            case 256:   _val = V_256TH; break;
            case 512:   _val = V_512TH; break;
            case 1024:  _val = V_1024TH; break;
            default:    _val = V_INVALID; break;
            }

      if (f.denominator() != 0) {
            int v = f.numerator() / f.denominator();
            if(v == 4) {
                  _val = V_LONG;
                  return;
                  }
            else if (v == 2) {
                  _val = V_BREVE;
                  return;
                  }
            }

      if (f.numerator() != 1) {
            switch(f.numerator()) {
                  case 3:
                        _val = DurationType(_val - 1);
                        _dots = 1;
                        break;
                  case 7:
                        _val = DurationType(_val - 2);
                        _dots = 2;
                        break;
                  default:
                        qDebug("TDuration(%d/%d): not implemented", f.numerator(), f.denominator());
// abort();
                        _val = V_INVALID;
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

QList<TDuration> toDurationList(Fraction l, bool useDots, int maxDots)
      {
      QList<TDuration> dList;
      if (useDots) {
            for (TDuration d = TDuration(TDuration::V_LONG); d.isValid() && (l.numerator() != 0);) {
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
            for (TDuration d = TDuration(TDuration::V_LONG); d.isValid() && (l.numerator() != 0);) {
                  Fraction ff(l - d.fraction());
                  if (ff.numerator() < 0) {
                        d = d.shift(1);
                        continue;
                        }
                  l -= d.fraction();
                  dList.append(d);
                  }
            }
      if (l != Fraction())
            qDebug("toDurationList:: rest remains %d/%d", l.numerator(), l.denominator());
      return dList;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void TDuration::print() const
      {
      qDebug("TDuration(");
      const char* s = "?";
      switch(_val) {
            case V_LONG:      s = "Long";    break;
            case V_BREVE:     s = "Breve";   break;
            case V_WHOLE:     s = "Whole";   break;
            case V_HALF:      s = "Half";    break;
            case V_QUARTER:   s = "Quarter"; break;
            case V_EIGHT:     s = "Eighth";   break;
            case V_16TH:      s = "16th";    break;
            case V_32ND:      s = "32nd";    break;
            case V_64TH:      s = "64th";    break;
            case V_128TH:     s = "128th";   break;
            case V_256TH:     s = "256th";   break;
            case V_512TH:     s = "512th";   break;
            case V_1024TH:    s = "1024th";  break;
            case V_ZERO:      s = "Zero";    break;
            case V_MEASURE:   s = "Measure"; break;
            case V_INVALID:   s = "Invalid"; break;
            };
      qDebug("   %s,dots=%d)", s, _dots);
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void TDuration::setType(DurationType t)
      {
      _val = t;
      if (_val == V_MEASURE)
            _dots = 0;
      }
}


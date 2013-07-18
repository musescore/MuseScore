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

#include "timesig.h"
#include "xml.h"
#include "score.h"
#include "style.h"
#include "sym.h"
#include "symbol.h"
#include "staff.h"
#include "stafftype.h"

namespace Ms {

//---------------------------------------------------------
//   TimeSig
//    Constructs an invalid time signature element.
//    After construction first call setTrack() then
//    call setSig().
//    Layout() is static and called in setSig().
//---------------------------------------------------------

TimeSig::TimeSig(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      _showCourtesySig = true;
      customText = false;
      _stretch.set(1, 1);
      _sig.set(0, 1);               // initialize to invalid
      _timeSigType   = TSIG_NORMAL;
      customText = false;
      _needLayout = true;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal TimeSig::mag() const
      {
      return staff() ? staff()->mag() : 1.0;
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void TimeSig::setSig(const Fraction& f, TimeSigType st)
      {
      if (_sig != f) {
//            customText = false;
            _sig       = f;
            }
      if (st == TSIG_FOUR_FOUR || st == TSIG_ALLA_BREVE)
            customText = false;
      _timeSigType = st;
      _needLayout = true;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == TIMESIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TimeSig::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == TIMESIG) {
            // change timesig applies to all staves, can't simply set subtype
            // for this one only
            // ownership of e is transferred to cmdAddTimeSig
            score()->cmdAddTimeSig(measure(), staffIdx(), static_cast<TimeSig*>(e), false);
            return 0;
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   setNumeratorString
//---------------------------------------------------------

void TimeSig::setNumeratorString(const QString& a)
      {
      _numeratorString = a;
      // text is custom if only one string is present or if either string is not the default string
      customText = _numeratorString.isEmpty() != _denominatorString.isEmpty()
            || ( !_numeratorString.isEmpty() && _numeratorString != QString::number(_sig.numerator()) )
            || ( !_denominatorString.isEmpty() && _denominatorString != QString::number(_sig.denominator()) );
      _needLayout = true;
      }

//---------------------------------------------------------
//   setDenominatorString
//---------------------------------------------------------

void TimeSig::setDenominatorString(const QString& a)
      {
      _denominatorString = a;
      // text is custom if only one string is present or if either string is not the default string
      customText = _numeratorString.isEmpty() != _denominatorString.isEmpty()
            || ( !_numeratorString.isEmpty() && _numeratorString != QString::number(_sig.numerator()) )
            || ( !_denominatorString.isEmpty() && _denominatorString != QString::number(_sig.denominator()) );
      _needLayout = true;
      }

//---------------------------------------------------------
//   write TimeSig
//---------------------------------------------------------

void TimeSig::write(Xml& xml) const
      {
      xml.stag("TimeSig");
      if (timeSigType() != TSIG_NORMAL)
            xml.tag("subtype", timeSigType());
      Element::writeProperties(xml);

      xml.tag("sigN",  _sig.numerator());
      xml.tag("sigD",  _sig.denominator());
      if (stretch() != Fraction(1,1)) {
            xml.tag("stretchN", stretch().numerator());
            xml.tag("stretchD", stretch().denominator());
            }
      if (customText) {
            xml.tag("textN", _numeratorString);
            xml.tag("textD", _denominatorString);
            }
      if (!_groups.empty())
            _groups.write(xml);
      xml.tag("showCourtesySig", _showCourtesySig);
      xml.etag();
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(XmlReader& e)
      {
      int n=0, z1=0, z2=0, z3=0, z4=0;
      bool old = false;

      customText = false;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "den") {
                  old = true;
                  n = e.readInt();
                  }
            else if (tag == "nom1") {
                  old = true;
                  z1 = e.readInt();
                  }
            else if (tag == "nom2") {
                  old = true;
                  z2 = e.readInt();
                  }
            else if (tag == "nom3") {
                  old = true;
                  z3 = e.readInt();
                  }
            else if (tag == "nom4") {
                  old = true;
                  z4 = e.readInt();
                  }
            else if (tag == "subtype") {
                  TimeSigType i = TimeSigType(e.readInt());
                  if (score()->mscVersion() < 122 && score()->mscVersion() > 114) {
                        setSig(Fraction(
                             ((i >> 24) & 0x3f)
                           + ((i >> 18) & 0x3f)
                           + ((i >> 12) & 0x3f)
                           + ((i >>  6) & 0x3f), i & 0x3f), TSIG_NORMAL);
                        }
                  else
                        _timeSigType = i;
                  }
            else if (tag == "showCourtesySig")
                  _showCourtesySig = e.readInt();
            else if (tag == "sigN")
                  _sig.setNumerator(e.readInt());
            else if (tag == "sigD")
                  _sig.setDenominator(e.readInt());
            else if (tag == "stretchN")
                  _stretch.setNumerator(e.readInt());
            else if (tag == "stretchD")
                  _stretch.setDenominator(e.readInt());
            else if (tag == "textN")
                  setNumeratorString(e.readElementText());
            else if (tag == "textD")
                  setDenominatorString(e.readElementText());
            else if (tag == "Groups")
                  _groups.read(e);
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      if (old) {
            _sig.set(z1+z2+z3+z4, n);
            customText = false;
            if (timeSigType() == 0x40000104)
                  _timeSigType = TSIG_FOUR_FOUR;
            else if (timeSigType() == 0x40002084)
                  _timeSigType = TSIG_ALLA_BREVE;
            else
                  _timeSigType = TSIG_NORMAL;
            }
      _needLayout = true;
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TimeSig::layout1()
      {
      qreal _spatium = spatium();

      setbbox(QRectF());                  // prepare for an empty time signature
      pz = QPointF();
      pn = QPointF();

      qreal lineDist      = 1.0;          // assume dimensions a standard staff
      int   numOfLines    = 5;
      TimeSigType sigType = timeSigType();
      Staff* _staff       = staff();

      if (_staff) {
            // if staff is without time sig, format as if no text at all
            if (!_staff->staffType()->genTimesig() ) {
                  // reset position and box sizes to 0
                  pn.rx() = 0.0;
                  pz.rx() = 0.0;
                  setbbox(QRectF());
                  // leave everything else as it is:
                  // draw() will anyway skip any drawing if staff type has no time sigs
//                sigType = TSIG_NORMAL;
//                _numeratorString.clear();
//                _denominatorString.clear();
                  return;
                  }
            // update to real staff values
            numOfLines  = _staff->lines();
            lineDist    = _staff->lineDistance();
            }

      // if some symbol
      // compute vert. displacement to center in the staff height
      // determine middle staff position:

      qreal yoff = _spatium * (numOfLines-1) *.5 * lineDist;
      qreal mag  = magS();

      // C and Ccut are placed at the middle of the staff: use yoff directly
      if (sigType ==  TSIG_FOUR_FOUR) {
            pz = QPointF(0.0, yoff);
            Sym& sym = symbols[score()->symIdx()][fourfourmeterSym];
            setbbox(sym.bbox(mag).translated(pz));
            _numeratorString = sym.toString();
            _denominatorString.clear();
            }
      else if (sigType == TSIG_ALLA_BREVE) {
            pz = QPointF(0.0, yoff);
            Sym& sym = symbols[score()->symIdx()][allabreveSym];
            setbbox(sym.bbox(mag).translated(pz));
            _numeratorString = sym.toString();
            _denominatorString.clear();
            }
      else {
            if (!customText) {
                  _numeratorString   = QString("%1").arg(_sig.numerator());   // build numerator string
                  _denominatorString = QString("%1").arg(_sig.denominator()); // build denominator string
                  }
            QFont font = fontId2font(symIdx2fontId(score()->symIdx()));
            font.setPixelSize(lrint(20.0 * MScore::DPI/PPI));
            QFontMetricsF fm(font);
            QRectF numRect = fm.tightBoundingRect(_numeratorString);          // get 'tight' bounding boxes for strings
            QRectF denRect = fm.tightBoundingRect(_denominatorString);

            // position numerator and denominator; vertical displacement:
            // number of lines is odd: 0.0 (strings are directly above and below the middle line)
            // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

            qreal displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

            pz = QPointF(0.0, yoff - displ);
            // denom. horiz. posit.: centred around centre of numerator
            // vert. position:       base line is lowered by displ and by the whole height of a digit

            qreal spatium2 = _spatium * 2.0;
            pn = QPointF((numRect.width() - denRect.width())*.5, yoff + displ + spatium2);

            setbbox(numRect.translated(pz));   // translate bounding boxes to actual string positions
            addbbox(denRect.translated(pn));
            }
      qreal im = (MScore::DPI * SPATIUM20) / _spatium;

      pz *= im;                           // convert positions to raster units
      pn *= im;
      _needLayout = false;
      // adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(QPainter* painter) const
      {
      if (staff() && !staff()->staffType()->genTimesig())
            return;
      painter->setPen(curColor());
      QFont font = fontId2font(symIdx2fontId(score()->symIdx()));
      font.setPixelSize(lrint(20.0 * MScore::DPI/PPI));
      painter->setFont(font);
      qreal mag  = spatium() / (MScore::DPI * SPATIUM20);
      qreal imag = 1.0 / mag;

      painter->scale(mag, mag);
      painter->drawText(pz, _numeratorString);    // use positions and strings computed in layout()
      painter->drawText(pn, _denominatorString);
      painter->scale(imag, imag);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space TimeSig::space() const
      {
      return Space(point(score()->styleS(ST_timesigLeftMargin)), width());
      }

//---------------------------------------------------------
//   setFrom
//---------------------------------------------------------

void TimeSig::setFrom(const TimeSig* ts)
      {
      _timeSigType       = ts->timeSigType();
      _numeratorString   = ts->_numeratorString;
      _denominatorString = ts->_denominatorString;
      _sig               = ts->_sig;
      _stretch           = ts->_stretch;
      customText         = ts->customText;
      layout1();
      }

//---------------------------------------------------------
//   ssig
//---------------------------------------------------------

QString TimeSig::ssig() const
      {
      return QString("%1/%2").arg(_sig.numerator()).arg(_sig.denominator());
      }

//---------------------------------------------------------
//   setSSig
//---------------------------------------------------------

void TimeSig::setSSig(const QString& s)
      {
      QStringList sl = s.split("/");
      if (sl.size() == 2) {
            _sig.setNumerator(sl[0].toInt());
            _sig.setDenominator(sl[1].toInt());
            }
      }

//---------------------------------------------------------
//   undoSetShowCourtesySig
//---------------------------------------------------------

void TimeSig::undoSetShowCourtesySig(bool v)
      {
      score()->undoChangeProperty(this, P_SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   undoSetNumeratorString
//---------------------------------------------------------

void TimeSig::undoSetNumeratorString(const QString& s)
      {
      score()->undoChangeProperty(this, P_NUMERATOR_STRING, s);
      }

//---------------------------------------------------------
//   undoSetDenominatorString
//---------------------------------------------------------

void TimeSig::undoSetDenominatorString(const QString& s)
      {
      score()->undoChangeProperty(this, P_DENOMINATOR_STRING, s);
      }

//---------------------------------------------------------
//   undoSetGroups
//---------------------------------------------------------

void TimeSig::undoSetGroups(const Groups& g)
      {
      score()->undoChangeProperty(this, P_GROUPS, QVariant::fromValue(g));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TimeSig::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SHOW_COURTESY:      return int(showCourtesySig());
            case P_NUMERATOR_STRING:   return numeratorString();
            case P_DENOMINATOR_STRING: return denominatorString();
            case P_GROUPS:             return QVariant::fromValue(groups());
            case P_TIMESIG:            return QVariant::fromValue(_sig);
            case P_TIMESIG_GLOBAL:     return QVariant::fromValue(globalSig());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TimeSig::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SHOW_COURTESY:
                  setShowCourtesySig(v.toBool());
                  break;
            case P_NUMERATOR_STRING:
                  setNumeratorString(v.toString());
                  break;
            case P_DENOMINATOR_STRING:
                  setDenominatorString(v.toString());
                  break;
            case P_GROUPS:
                  setGroups(v.value<Groups>());
                  break;
            case P_TIMESIG:
                  setSig(v.value<Fraction>());
                  break;
            case P_TIMESIG_GLOBAL:
                  setGlobalSig(v.value<Fraction>());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      _needLayout = true;
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TimeSig::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_SHOW_COURTESY:      return true;
            case P_NUMERATOR_STRING:   return QString();
            case P_DENOMINATOR_STRING: return QString();
            case P_TIMESIG:            return QVariant::fromValue(Fraction(4,4));
            case P_TIMESIG_GLOBAL:     return QVariant::fromValue(Fraction(1,1));
            default:                   return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TimeSig::spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/)
      {
      _needLayout = true;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
      {
      if (_needLayout)
            layout1();
      }

}


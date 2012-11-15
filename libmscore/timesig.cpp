//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: timesig.cpp 5568 2012-04-22 10:08:43Z wschweer $
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
      _subtype   = TSIG_NORMAL;
      customText = false;
      needLayout = true;
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void TimeSig::setSig(const Fraction& f, TimeSigType st)
      {
      if (_sig != f) {
            customText = false;
            _sig       = f;
            }
      if (st == TSIG_FOUR_FOUR) {
            _sig.set(4, 4);
            customText = false;
            }
      else if (st == TSIG_ALLA_BREVE) {
            _sig.set(2, 2);
            customText = false;
            }
      _subtype = st;
      layout1();
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
      customText = !(_numeratorString.isEmpty() && _denominatorString.isEmpty());
      }

//---------------------------------------------------------
//   setDenominatorString
//---------------------------------------------------------

void TimeSig::setDenominatorString(const QString& a)
      {
      _denominatorString = a;
      customText = !(_numeratorString.isEmpty() && _denominatorString.isEmpty());
      }

//---------------------------------------------------------
//   write TimeSig
//---------------------------------------------------------

void TimeSig::write(Xml& xml) const
      {
      xml.stag("TimeSig");
      if (subtype() != TSIG_NORMAL)
            xml.tag("subtype", subtype());
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
      xml.tag("showCourtesySig", _showCourtesySig);
      xml.etag();
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(const QDomElement& de)
      {
      int n=0, z1=0, z2=0, z3=0, z4=0;
      bool old = false;

      customText = false;

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());

            if (tag == "den") {
                  old = true;
                  n = val.toInt();
                  }
            else if (tag == "nom1") {
                  old = true;
                  z1 = val.toInt();
                  }
            else if (tag == "nom2") {
                  old = true;
                  z2 = val.toInt();
                  }
            else if (tag == "nom3") {
                  old = true;
                  z3 = val.toInt();
                  }
            else if (tag == "nom4") {
                  old = true;
                  z4 = val.toInt();
                  }
            else if (tag == "subtype") {
                  TimeSigType i = TimeSigType(val.toInt());
                  if (score()->mscVersion() < 122) {
                        setSig(Fraction(
                             ((i >> 24) & 0x3f)
                           + ((i >> 18) & 0x3f)
                           + ((i >> 12) & 0x3f)
                           + ((i >>  6) & 0x3f), i & 0x3f), TSIG_NORMAL);
                        }
                  else
                        _subtype = i;
                  }
            else if (tag == "showCourtesySig")
                  _showCourtesySig = val.toInt();
            else if (tag == "sigN")
                  _sig.setNumerator(val.toInt());
            else if (tag == "sigD")
                  _sig.setDenominator(val.toInt());
            else if (tag == "stretchN")
                  _stretch.setNumerator(val.toInt());
            else if (tag == "stretchD")
                  _stretch.setDenominator(val.toInt());
            else if (tag == "textN")
                  setNumeratorString(val);
            else if (tag == "textD")
                  setDenominatorString(val);
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (old) {
            _sig.set(z1+z2+z3+z4, n);
            customText = false;
            if (subtype() == 0x40000104)
                  _subtype = TSIG_FOUR_FOUR;
            else if (subtype() == 0x40002084)
                  _subtype = TSIG_ALLA_BREVE;
            else
                  _subtype = TSIG_NORMAL;
            }
      needLayout = true;
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TimeSig::layout1()
      {
      qreal _spatium = spatium();

      setbbox(QRectF());                 // prepare for an empty time signature
      pz = QPointF();
      pn = QPointF();

      qreal lineDist    = 1.0;            // assume dimensions a standard staff
      int    numOfLines = 5;
      TimeSigType st    = subtype();

      if (staff()) {
            StaffType* staffType = staff()->staffType();
            numOfLines  = staff()->staffType()->lines();
            lineDist    = staff()->staffType()->lineDistance().val();

            // if tablature, but without time sig, set empty symbol
            if ((staffType->group() == TAB_STAFF) &&
               !(static_cast<StaffTypeTablature*>(staffType)->genTimesig())) {
                  st = TSIG_NORMAL;
                  }
            }

      // if some symbol
      // compute vert. displacement to center in the staff height
      // determine middle staff position:

      qreal yoff = _spatium * (numOfLines-1) *.5 * lineDist;
      qreal mag  = magS();

      // C and Ccut are placed at the middle of the staff: use yoff directly
      if (st ==  TSIG_FOUR_FOUR) {
            pz = QPointF(0.0, yoff);
            Sym& sym = symbols[score()->symIdx()][fourfourmeterSym];
            setbbox(sym.bbox(mag).translated(pz));
            _numeratorString = sym.toString();
            _denominatorString.clear();
            }
      else if (st == TSIG_ALLA_BREVE) {
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
            QFontMetricsF fm(fontId2font(symIdx2fontId(score()->symIdx())));
            QRectF rz = fm.tightBoundingRect(_numeratorString);   // get 'tight' bounding boxes for strings
            QRectF rn = fm.tightBoundingRect(_denominatorString);

            // scale bounding boxes to mag
            qreal spatium2 = _spatium * 2.0;
            rz = QRectF(rz.x() * mag, -spatium2, rz.width() * mag, spatium2);
            rn = QRectF(rn.x() * mag, -spatium2, rn.width() * mag, spatium2);

            // position numerator and denominator; vertical displacement:
            // number of lines is odd: 0.0 (strings are directly above and below the middle line)
            // number of lines even:   0.5 (strings are moved up/down to leave 1 line dist. between them)

            qreal displ = (numOfLines & 1) ? 0.0 : (0.5 * lineDist * _spatium);

            pz = QPointF(0.0, yoff - displ);
            // denom. horiz. posit.: centred around centre of numerator
            // vert. position:       base line is lowered by displ and by the whole height of a digit
            pn = QPointF((rz.width() - rn.width())*.5, yoff + displ + spatium2);

            setbbox(rz.translated(pz));   // translate bounding boxes to actual string positions
            addbbox(rn.translated(pn));
            }
      qreal im = (MScore::DPI * SPATIUM20) / _spatium;

      pz *= im;                           // convert positions to raster units
      pn *= im;
      needLayout = false;
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
      _subtype           = ts->subtype();
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
//   getProperty
//---------------------------------------------------------

QVariant TimeSig::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: return int(showCourtesySig());
            case P_NUMERATOR_STRING:  return numeratorString();
            case P_DENOMINATOR_STRING: return denominatorString();
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
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
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
            default:                   return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TimeSig::spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/)
      {
      needLayout = true;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
      {
      if (needLayout)
            layout1();
      }


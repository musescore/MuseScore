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
      setFlags(ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _showCourtesySig = true;
      customText = false;
      _stretch.set(1, 1);
      _sig.set(0, 1);               // initialize to invalid
      _timeSigType   = TimeSigType::NORMAL;
      customText = false;
      _needLayout = true;
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

TimeSig* TimeSig::clone() const
      {
      TimeSig* ts = new TimeSig(*this);
      ts->setNeedLayout(true); // relayout of the ts is needed in parts #24571
      return ts;
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
      if (_sig != f)
            _sig = f;
      if (st == TimeSigType::FOUR_FOUR || st == TimeSigType::ALLA_BREVE)
            customText = false;
      _timeSigType = st;
      _needLayout = true;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == Element::Type::TIMESIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TimeSig::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == Element::Type::TIMESIG) {
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
      if (timeSigType() != TimeSigType::NORMAL)
            xml.tag("subtype", int(timeSigType()));
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
                  int i = e.readInt();
                  if (score()->mscVersion() < 122 && score()->mscVersion() > 114) {
                        setSig(Fraction(
                             ((i >> 24) & 0x3f)
                           + ((i >> 18) & 0x3f)
                           + ((i >> 12) & 0x3f)
                           + ((i >>  6) & 0x3f), i & 0x3f), TimeSigType::NORMAL);
                        }
                  else if (score()->mscVersion() <= 114) {
                        if (i == 0x40000104)
                              _timeSigType = TimeSigType::FOUR_FOUR;
                        else if (i == 0x40002084)
                              _timeSigType = TimeSigType::ALLA_BREVE;
                        else
                              _timeSigType = TimeSigType::NORMAL;
                        }
                  else
                        _timeSigType = TimeSigType(i);
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
            }
      _stretch.reduce();
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

      // C and Ccut are placed at the middle of the staff: use yoff directly
      if (sigType ==  TimeSigType::FOUR_FOUR) {
            pz = QPointF(0.0, yoff);
            setbbox(symBbox(SymId::timeSigCommon).translated(pz));
            _numeratorString = score()->scoreFont()->toString(SymId::timeSigCommon);
            _denominatorString.clear();
            }
      else if (sigType == TimeSigType::ALLA_BREVE) {
            pz = QPointF(0.0, yoff);
            setbbox(symBbox(SymId::timeSigCutCommon).translated(pz));
            _numeratorString = score()->scoreFont()->toString(SymId::timeSigCutCommon);
            _denominatorString.clear();
            }
      else {
            if (!customText) {
                  _numeratorString   = QString("%1").arg(_sig.numerator());   // build numerator string
                  _denominatorString = QString("%1").arg(_sig.denominator()); // build denominator string
                  }
            QString ns = toTimeSigString(_numeratorString);
            QString ds = toTimeSigString(_denominatorString);
            QFont font = score()->scoreFont()->font();
            QFontMetricsF fm(font);
            qreal mag = magS();
            QRectF numRect = fm.tightBoundingRect(ns);          // get 'tight' bounding boxes for strings
            QRectF denRect = fm.tightBoundingRect(ds);
            // really Qt does not provide a QRectF::scale() method?!!
            numRect = QRect(numRect.x()*mag, numRect.y()*mag, fm.width(ns)*mag, numRect.height()*mag);
            denRect = QRect(denRect.x()*mag, denRect.y()*mag, fm.width(ds)*mag, denRect.height()*mag);

            // position numerator and denominator; vertical displacement:
            // number of lines is odd: 0.0 (strings are directly above and below the middle line)
            // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

            qreal displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

            //align on the wider
            if (numRect.width() >= denRect.width()) {
                  // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
                  pz = QPointF(0.0, yoff - ((denRect.width() < 0.01) ? 0.0 : (displ + _spatium)) );

                  // denominator: horiz: centred around centre of numerator | vert: one space below centre line
                  pn = QPointF((numRect.width() - denRect.width())*.5, yoff + displ + _spatium);
                  }
            else {
                  // denominator: horiz: centred around centre of numerator | vert: one space below centre line
                  pn = QPointF(0.0, yoff + displ + _spatium);

                  // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
                  pz = QPointF((denRect.width() - numRect.width())*.5, yoff - ((denRect.width() < 0.01) ? 0.0 : (displ + _spatium)) );
                  }

            setbbox(numRect.translated(pz));   // translate bounding boxes to actual string positions
            addbbox(denRect.translated(pn));
            }

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
      QString ns = toTimeSigString(_numeratorString);
      QString ds = toTimeSigString(_denominatorString);

      drawSymbols(ns, painter, pz);
      drawSymbols(ds, painter, pn);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space TimeSig::space() const
      {
      return Space(point(score()->styleS(StyleIdx::timesigLeftMargin)), width());
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
      _needLayout        = true;
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
      score()->undoChangeProperty(this, P_ID::SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   undoSetNumeratorString
//---------------------------------------------------------

void TimeSig::undoSetNumeratorString(const QString& s)
      {
      score()->undoChangeProperty(this, P_ID::NUMERATOR_STRING, s);
      }

//---------------------------------------------------------
//   undoSetDenominatorString
//---------------------------------------------------------

void TimeSig::undoSetDenominatorString(const QString& s)
      {
      score()->undoChangeProperty(this, P_ID::DENOMINATOR_STRING, s);
      }

//---------------------------------------------------------
//   undoSetGroups
//---------------------------------------------------------

void TimeSig::undoSetGroups(const Groups& g)
      {
      score()->undoChangeProperty(this, P_ID::GROUPS, QVariant::fromValue(g));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TimeSig::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::SHOW_COURTESY:      return int(showCourtesySig());
            case P_ID::NUMERATOR_STRING:   return numeratorString();
            case P_ID::DENOMINATOR_STRING: return denominatorString();
            case P_ID::GROUPS:             return QVariant::fromValue(groups());
            case P_ID::TIMESIG:            return QVariant::fromValue(_sig);
            case P_ID::TIMESIG_GLOBAL:     return QVariant::fromValue(globalSig());
            case P_ID::TIMESIG_TYPE: return QVariant::fromValue(int(_timeSigType));
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
            case P_ID::SHOW_COURTESY:
                  setShowCourtesySig(v.toBool());
                  break;
            case P_ID::NUMERATOR_STRING:
                  setNumeratorString(v.toString());
                  break;
            case P_ID::DENOMINATOR_STRING:
                  setDenominatorString(v.toString());
                  break;
            case P_ID::GROUPS:
                  setGroups(v.value<Groups>());
                  break;
            case P_ID::TIMESIG:
                  setSig(v.value<Fraction>());
                  break;
            case P_ID::TIMESIG_GLOBAL:
                  setGlobalSig(v.value<Fraction>());
                  break;
            case P_ID::TIMESIG_TYPE:
                  _timeSigType = (TimeSigType)(v.toInt());
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
            case P_ID::SHOW_COURTESY:      return true;
            case P_ID::NUMERATOR_STRING:   return QString();
            case P_ID::DENOMINATOR_STRING: return QString();
            case P_ID::TIMESIG:            return QVariant::fromValue(Fraction(4,4));
            case P_ID::TIMESIG_GLOBAL:     return QVariant::fromValue(Fraction(1,1));
            case P_ID::TIMESIG_TYPE:       return int(TimeSigType::NORMAL);
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


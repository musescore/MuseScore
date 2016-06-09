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
      customText       = false;
      _stretch.set(1, 1);
      _sig.set(0, 1);               // initialize to invalid
      _timeSigType      = TimeSigType::NORMAL;
      _largeParentheses = false;
      _needLayout       = true;
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
//    custom text has to be set after setSig()
//---------------------------------------------------------

void TimeSig::setSig(const Fraction& f, TimeSigType st)
      {
      if (!_sig.identical(f)) {
            _sig = f;
            _needLayout = true;
            }
      if (_timeSigType != st) {
            _timeSigType = st;
            _needLayout = true;
            }
      customText = false;
      _largeParentheses = false;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(const DropData& data) const
      {
      return data.element->type() == Element::Type::TIMESIG;
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
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setNumeratorString(const QString& a)
      {
      if (_timeSigType ==  TimeSigType::NORMAL) {
            _numeratorString = a;
            customText = (_denominatorString != QString::number(_sig.denominator()))
               || (_numeratorString != QString::number(_sig.numerator()));
            _needLayout = true;
            }
      }

//---------------------------------------------------------
//   setDenominatorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setDenominatorString(const QString& a)
      {
      if (_timeSigType ==  TimeSigType::NORMAL) {
            _denominatorString = a;
            customText = (_denominatorString != QString::number(_sig.denominator()))
               || (_numeratorString != QString::number(_sig.numerator()));
            _needLayout = true;
            }
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
                  if (score()->mscVersion() <= 114) {
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
//   layout
//---------------------------------------------------------

void TimeSig::layout()
      {
      if (_needLayout)
            layout1();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TimeSig::layout1()
      {
      setPos(0.0, 0.0);
      qreal _spatium = spatium();

      setbbox(QRectF());                  // prepare for an empty time signature
      pointLargeLeftParen = QPointF();
      pz = QPointF();
      pn = QPointF();
      pointLargeRightParen = QPointF();

      qreal lineDist;
      int   numOfLines;
      TimeSigType sigType = timeSigType();
      Staff* _staff       = staff();

      if (_staff) {
            // if staff is without time sig, format as if no text at all
            if (!_staff->staffType()->genTimesig() ) {
                  // reset position and box sizes to 0
                  pointLargeLeftParen.rx() = 0.0;
                  pn.rx() = 0.0;
                  pz.rx() = 0.0;
                  pointLargeRightParen.rx() = 0.0;
                  setbbox(QRectF());
                  // leave everything else as it is:
                  // draw() will anyway skip any drawing if staff type has no time sigs
                  return;
                  }
            numOfLines  = _staff->lines();
            lineDist    = _staff->lineDistance();
            }
      else {
            // assume dimensions of a standard staff
            lineDist = 1.0;
            numOfLines = 5;
            }

      // if some symbol
      // compute vert. displacement to center in the staff height
      // determine middle staff position:

      qreal yoff = _spatium * (numOfLines-1) *.5 * lineDist;

      // C and Ccut are placed at the middle of the staff: use yoff directly
      if (sigType ==  TimeSigType::FOUR_FOUR) {
            pz = QPointF(0.0, yoff);
            setbbox(symBbox(SymId::timeSigCommon).translated(pz));
            _numeratorString = "C";
            _denominatorString.clear();
            }
      else if (sigType == TimeSigType::ALLA_BREVE) {
            pz = QPointF(0.0, yoff);
            setbbox(symBbox(SymId::timeSigCutCommon).translated(pz));
            _numeratorString = '\xA2';
            _denominatorString.clear();
            }
      else {
            if (!customText) {
                  _numeratorString   = QString("%1").arg(_sig.numerator());   // build numerator string
                  _denominatorString = QString("%1").arg(_sig.denominator()); // build denominator string
                  }
            std::vector<SymId> ns = toTimeSigString(_numeratorString);
            std::vector<SymId> ds = toTimeSigString(_denominatorString);

            ScoreFont* font = score()->scoreFont();
            qreal mag = magS();
            QRectF numRect = font->bbox(ns, mag);
            QRectF denRect = font->bbox(ds, mag);

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

            // centering of parenthesis so the middle of the parenthesis is at the divisor marking level
            int centerY = yoff/2 + _spatium;
            int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
            pointLargeLeftParen = QPointF(-_spatium, centerY);
            pointLargeRightParen = QPointF(widestPortion + _spatium, centerY);

            setbbox(numRect.translated(pz));   // translate bounding boxes to actual string positions
            addbbox(denRect.translated(pn));
            if (_largeParentheses) {
                  addbbox(QRect(pointLargeLeftParen.x(), pointLargeLeftParen.y() - denRect.height(), _spatium / 2, numRect.height() + denRect.height()));
                  addbbox(QRect(pointLargeRightParen.x(), pointLargeRightParen.y() - denRect.height(),  _spatium / 2, numRect.height() + denRect.height()));
                  }
            }

      _needLayout = false;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(QPainter* painter) const
      {
      if (staff() && !staff()->staffType()->genTimesig())
            return;
      painter->setPen(curColor());
      std::vector<SymId> ns = toTimeSigString(_numeratorString);
      std::vector<SymId> ds = toTimeSigString(_denominatorString);

      drawSymbols(ns, painter, pz);
      drawSymbols(ds, painter, pn);
      if (_largeParentheses) {
            drawSymbol(SymId::timeSigParensLeft, painter, pointLargeLeftParen);
            drawSymbol(SymId::timeSigParensRight, painter, pointLargeRightParen);
            }
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
      undoChangeProperty(P_ID::SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   undoSetNumeratorString
//---------------------------------------------------------

void TimeSig::undoSetNumeratorString(const QString& s)
      {
      undoChangeProperty(P_ID::NUMERATOR_STRING, s);
      }

//---------------------------------------------------------
//   undoSetDenominatorString
//---------------------------------------------------------

void TimeSig::undoSetDenominatorString(const QString& s)
      {
      undoChangeProperty(P_ID::DENOMINATOR_STRING, s);
      }

//---------------------------------------------------------
//   undoSetGroups
//---------------------------------------------------------

void TimeSig::undoSetGroups(const Groups& g)
      {
      undoChangeProperty(P_ID::GROUPS, QVariant::fromValue(g));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TimeSig::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::SHOW_COURTESY:      return int(showCourtesySig());
            case P_ID::NUMERATOR_STRING:   return numeratorString();
            case P_ID::DENOMINATOR_STRING: return denominatorString();
            case P_ID::GROUPS:             return QVariant::fromValue(groups());
            case P_ID::TIMESIG:            return QVariant::fromValue(_sig);
            case P_ID::TIMESIG_GLOBAL:     return QVariant::fromValue(globalSig());
            case P_ID::TIMESIG_STRETCH:    return QVariant::fromValue(stretch());
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
      switch (propertyId) {
            case P_ID::SHOW_COURTESY:
                  if (generated())
                        return false;
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
            case P_ID::TIMESIG_STRETCH:
                  setStretch(v.value<Fraction>());
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
      score()->setLayoutAll();
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
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TimeSig::spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/)
      {
      _needLayout = true;
      }

void TimeSig::localSpatiumChanged(qreal /*oldValue*/, qreal /*newValue*/)
      {
      _needLayout = true;
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* TimeSig::nextElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* TimeSig::prevElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString TimeSig::accessibleInfo() const
      {
      QString timeSigString;
      switch (timeSigType()) {
            case TimeSigType::FOUR_FOUR:
                  timeSigString = tr("Common time");
            case TimeSigType::ALLA_BREVE:
                  timeSigString = tr("Cut time");
            default:
                  timeSigString = tr("%1/%2 time").arg(QString::number(numerator())).arg(QString::number(denominator()));
            }
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(timeSigString);
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool TimeSig::operator==(const TimeSig& ts) const
      {
      return (timeSigType() == ts.timeSigType())
         && (sig().identical(ts.sig()))
         && (stretch() == ts.stretch())
         && (groups() == ts.groups())
         && (customText == ts.customText)
         && (!customText || (_numeratorString == ts._numeratorString && _denominatorString == ts._denominatorString))
         ;
      }

}


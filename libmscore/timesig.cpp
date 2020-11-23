//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
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
#include "segment.h"
#include "utils.h"

namespace Ms {

static const ElementStyle timesigStyle {
      { Sid::timesigScale,                       Pid::SCALE                   },
      };

//---------------------------------------------------------
//   TimeSig
//    Constructs an invalid time signature element.
//    After construction first call setTrack() then
//    call setSig().
//    Layout() is static and called in setSig().
//---------------------------------------------------------

TimeSig::TimeSig(Score* s)
  : Element(s, ElementFlag::ON_STAFF | ElementFlag::MOVABLE)
      {
      initElementStyle(&timesigStyle);

      _showCourtesySig = true;
      _stretch.set(1, 1);
      _sig.set(0, 1);               // initialize to invalid
      _timeSigType      = TimeSigType::NORMAL;
      _largeParentheses = false;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal TimeSig::mag() const
      {
      return staff() ? staff()->mag(tick()) : 1.0;
      }

//---------------------------------------------------------
//   setSig
//    custom text has to be set after setSig()
//---------------------------------------------------------

void TimeSig::setSig(const Fraction& f, TimeSigType st)
      {
      _sig              = f;
      _timeSigType      = st;
      _largeParentheses = false;
      _numeratorString.clear();
      _denominatorString.clear();
      _parserString.clear();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(EditData& data) const
      {
      return data.dropElement->isTimeSig();
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TimeSig::drop(EditData& data)
      {
      Element* e = data.dropElement;
      if (e->isTimeSig()) {
            // change timesig applies to all staves, can't simply set subtype
            // for this one only
            // ownership of e is transferred to cmdAddTimeSig
            score()->cmdAddTimeSig(measure(), staffIdx(), toTimeSig(e), false);
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
      if (_timeSigType == TimeSigType::NORMAL)
            _numeratorString = a;
      }

//---------------------------------------------------------
//   setDenominatorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setDenominatorString(const QString& a)
      {
      if (_timeSigType ==  TimeSigType::NORMAL)
            _denominatorString = a;
      }

//   setParserString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setParserString(const QString& a)
      {
      if (_timeSigType ==  TimeSigType::NORMAL)
            _parserString = a;
      }

//---------------------------------------------------------
//   write TimeSig
//---------------------------------------------------------

void TimeSig::write(XmlWriter& xml) const
      {
      xml.stag(this);
      writeProperty(xml, Pid::TIMESIG_TYPE);
      Element::writeProperties(xml);

      xml.tag("sigN",  _sig.numerator());
      xml.tag("sigD",  _sig.denominator());
      if (stretch() != Fraction(1,1)) {
            xml.tag("stretchN", stretch().numerator());
            xml.tag("stretchD", stretch().denominator());
            }
      if (!_parserString.isEmpty()) {
            writeProperty(xml, Pid::PARSER_STRING);
            }
      writeProperty(xml, Pid::NUMERATOR_STRING);
      writeProperty(xml, Pid::DENOMINATOR_STRING);
      if (!_groups.empty())
            _groups.write(xml);
      writeProperty(xml, Pid::SHOW_COURTESY);
      writeProperty(xml, Pid::SCALE);

      xml.etag();
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(XmlReader& e)
      {
      int n=0, z1=0, z2=0, z3=0, z4=0;
      bool old = false;

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
            else if (tag == "textP")
                  setParserString(e.readElementText());
            else if (tag == "Groups")
                  _groups.read(e);
            else if (readStyledProperty(e, tag))
                  ;
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      if (old) {
            _sig.set(z1+z2+z3+z4, n);
            }
      _stretch.reduce();
      
      // Read and Write num/den strings into parserString for backwards compatibility
      if ( _timeSigType == TimeSigType::NORMAL && parserString().isEmpty() 
          && (!numeratorString().isEmpty() || !denominatorString().isEmpty())) {
            setParserString(numeratorString());
            if (!denominatorString().isEmpty())
                  setParserString(parserString() + "/" + denominatorString());
            }

      // HACK: handle time signatures from scores before 3.5 differently on some special occasions.
      // See https://musescore.org/node/308139.
      QString version = masterScore()->mscoreVersion();
      if (!version.isEmpty() && (version >= "3.0") && (version < "3.5")) {
            if ((_timeSigType == TimeSigType::NORMAL) && !_numeratorString.isEmpty() && _denominatorString.isEmpty()) {
                  if (_numeratorString == QString::number(_sig.numerator()))
                        _numeratorString.clear();
                  else
                        setDenominatorString(QString::number(_sig.denominator()));
                  }
            }
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid TimeSig::propertyId(const QStringRef& name) const
      {
      if (name == "subtype")
            return Pid::TIMESIG_TYPE;
      if (name == "sigN" || name == "sigD")
            return Pid::TIMESIG;
      if (name == "stretchN" || name == "stretchD")
            return Pid::TIMESIG_STRETCH;
      if (name == "Groups")
            return Pid::GROUPS;
      return Element::propertyId(name);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
      {
      setPos(0.0, 0.0);
      qreal _spatium = spatium();

      setbbox(QRectF());                  // prepare for an empty time signature

      qreal lineDist;
      int   numOfLines;
      TimeSigType sigType = timeSigType();
      const Staff* _staff       = staff();

      if (_staff) {
            // if staff is without time sig, format as if no text at all
            if (!_staff->staffTypeForElement(this)->genTimesig()) {
                  setbbox(QRectF());
                  // leave everything else as it is:
                  // draw() will anyway skip any drawing if staff type has no time sigs
                  return;
                  }
            numOfLines  = _staff->lines(tick());
            lineDist    = _staff->lineDistance(tick());
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
            pns = { QPoint(0.0, yoff) };
            pds = pns;
            setbbox(symBbox(SymId::timeSigCommon).translated(pns[0]));
            ns = { {SymId::timeSigCommon} };
            ds = { {} };
            }
      else if (sigType == TimeSigType::ALLA_BREVE) {
            pns = { QPoint(0.0, yoff) };
            pds = pns;
            setbbox(symBbox(SymId::timeSigCutCommon).translated(pns[0]));
            ns = { {SymId::timeSigCutCommon} };
            ds = { {} };
            }
      else if (sigType == TimeSigType::CUT_BACH) {
            pns = { QPoint(0.0, yoff) };
            pds = pns;
            setbbox(symBbox(SymId::timeSigCut2).translated(pns[0]));
            ns = { {SymId::timeSigCut2} };
            ds = { {} };
            }
      else if (sigType == TimeSigType::CUT_TRIPLE) {
            pns = { QPoint(0.0, yoff) };
            pds = pns;
            setbbox(symBbox(SymId::timeSigCut3).translated(pns[0]));
            ns = { {SymId::timeSigCut3} };
            ds = { {} };
            }
      else  {
            if (!_parserString.isEmpty()) {
                  QString pString = _parserString;
                  // Replace bracketed [strings] with the respective symbols ('w h q e' will convert to note glyphs)
                  pString.replace("[1/4]","\uE097");
                  pString.replace("[1/2]","\uE098");
                  pString.replace("[3/4]","\uE099");
                  pString.replace("[1/3]","\uE09A");
                  pString.replace("[2/3]","\uE09B");
                  pString.replace("[1]","w");
                  pString.replace("[1.]","w.");
                  pString.replace("[2]","h");
                  pString.replace("[2.]","h.");
                  pString.replace("[4]","q");
                  pString.replace("[4.]","q.");
                  pString.replace("[8]","e");
                  pString.replace("[8.]","e.");
                  pString.replace("[16]","s");
                  pString.replace("[16.]","s.");
                  pString.remove(QRegExp("\\[([^]]+)\\]")); // eliminate unidentified bracketed [strings]

                  // Parse time signature text:
                  // Use '|' to separate timesig blocks of 'numerator/denominator'
                  // (if a block's denominator is missing, block will be drawn centered to the staff. This is also the case for large parenthesis and '+' )

                  pString.replace(QRegExp("(\\/[0-9whqe.]+)"),"\\1|");      // insert separator after denominators
                  pString.replace(QRegExp("\\)([ +=x\\*\\-]+)"),")|\\1");   // insert separator between ')' and '+x=-'
                  pString.replace(QRegExp("(\\|[ +=x\\*\\-]+)"),"\\1|");    // then after isolated '+x-='
                  pString.replace(" ","| |");                       // split spaces
                  // add separator after '(' with no matching ')' inside numerators (which include numbers, '+' and fractions) 
                  pString.replace(QRegExp("\\(([0-9+\\xE097\\xE098\\xE099\\xE09A\\xE09B]+[\\/])+"), "(|\\1");

                  // Create list of time signature blocks ('numerator/denominator', or just 'numerator' for center drawn glyphs like 'C', '+' or '(' )
                  QStringList sList = pString.split("|", QString::SkipEmptyParts);

                  // LOOP thru the sList and create the numerator and denominator strings and positions 
                  ns.clear();
                  ds.clear();
                  pns.clear();
                  pds.clear();

                  for (int i=0; i < sList.size(); i++) {
                        QStringList numDem = sList[i].split("/",QString::SkipEmptyParts);
                        pns.push_back(QPointF());
                        pds.push_back(QPointF());
                        if (numDem.size() == 1) {
                              ns.push_back(toTimeSigString(numDem[0], true)); // if no denominator, use large '()' and '+' (true)
                              ds.push_back(toTimeSigString("", false));       // empty denominator timeSigString
                              }
                        else if (numDem.size() >= 2) {
                              ns.push_back(toTimeSigString(numDem[0], false));
                              ds.push_back(toTimeSigString(numDem[1], false));
                              }
                        else
                              qDebug("no time signature strings detected");
                        }

                  // Convert first block to old num/den strings for backwards compatibility
                  if (sList.size() > 0) {
                        QStringList numDem = sList[0].split("/",QString::KeepEmptyParts);
                        if (numDem.size() >= 1)
                              setNumeratorString(numDem[0]);
                        if (numDem.size() >= 2)
                              setDenominatorString(numDem[1]);
                        else
                              setDenominatorString(QString());
                        }
                  }
            else {
                  // Use actual values
                  pns = { QPointF() };
                  pds = { QPointF() };
                  ns = { toTimeSigString(QString::number(_sig.numerator()), false) };
                  ds = { toTimeSigString(QString::number(_sig.denominator()), false) };
                  }

            ScoreFont* font = score()->scoreFont();
            QSizeF mag(magS() * _scale);

            // position numerator and denominator; vertical displacement:
            // number of lines is odd: 0.0 (strings are directly above and below the middle line)
            // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

            qreal displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

            // loop thru num/dem blocks to lay out positions and bounding boxes
            setbbox(QRectF());
            qreal xpos = 0.0;
            for (int i = 0; i < ns.size(); i++) {
                  QRectF nRect = font->bbox(ns[i], mag);
                  QRectF dRect = font->bbox(ds[i], mag);
                  // Handle special cases here:
                  // SymId::space doesn't have a bbox, so add ::space _advance value (10.0) to the rect width
                  if (ns[i].size() && ns[i][0] == SymId::space)
                        nRect.adjust(0, 0, 10.0 * mag.width(), 0);  
                  // else if ... (other special cases):
                  //   TO DO:... "Emmentaler" parentheses have bad bboxes
                  //   TO DO:... "Gonville" doesn't have big parentheses. 
                  //   TO DO: * Both cases are problems with the font. It's better to fix the glyphs there
                  // Adjust note symbols' bboxes in denominator so the glyphs are draw at a correct vertical pos. in the staff
                  if (ds[i].size() && (ds[i][0] == SymId::metNoteQuarterDown || ds[i][0] == SymId::metNote8thDown 
                      || ds[i][0] == SymId::metNoteHalfDown || ds[i][0] == SymId::metNote16thDown)) {
                        dRect.adjust(0, 53 * mag.height(), 0, 0);
                        if (ds[i].size() > 1 || ds[i][1] == SymId::metAugmentationDot)
                              dRect.adjust(10 * mag.width(), 0, 0, 0);
                        }
                  // If denominator is absent, draw numerator vertically centered to staff, otherwise, above center line
                  qreal pzY = yoff - (dRect.width() < 0.01 ? 0.0 : (displ + nRect.height() * .5));
                  qreal pnY = yoff + displ + dRect.height() * .5;  //() missing?
                  // Horizontally align numerator and denominator, centered on the wider:
                  if (nRect.width() >= dRect.width()) {
                        pns[i] = QPointF(0.0, pzY);
                        pds[i] = QPointF((nRect.width() - dRect.width()) * .5, pnY);
                        }
                  else {
                        pns[i] = QPointF((dRect.width() - nRect.width()) * .5, pzY);
                        pds[i] = QPointF(0.0, pnY);
                        }
                  // add to bbox and advance xpos for the next block
                  pns[i].rx() += xpos;
                  pds[i].rx() += xpos;
                  addbbox(nRect.translated(pns[i]));
                  addbbox(dRect.translated(pds[i]));
                  xpos += qMax(nRect.width(), dRect.width());
                  }
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape TimeSig::shape() const
      {
      QRectF box(bbox());
      const Staff* st = staff();
      if (st && addToSkyline()) {
            // Extend time signature shape up and down to
            // the first ledger line height to ensure that
            // no notes will be too close to the timesig.
            const qreal sp = spatium();
            const qreal y = pos().y();
            box.setTop(std::min(-sp - y, box.top()));
            box.setBottom(std::max(st->height() - y + sp, box.bottom()));
            }
      return Shape(box);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(QPainter* painter) const
      {
      if (staff() && !const_cast<const Staff*>(staff())->staffType(tick())->genTimesig())
            return;
      painter->setPen(curColor());

      // loop and draw t/s blocks 
      for (int i = 0; i < ns.size(); i++) {
            drawSymbols(ns[i], painter, pns[i], _scale);
            drawSymbols(ds[i], painter, pds[i], _scale);
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
      _parserString      = ts->_parserString;
      _sig               = ts->_sig;
      _stretch           = ts->_stretch;
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
//   getProperty
//---------------------------------------------------------

QVariant TimeSig::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SHOW_COURTESY:
                  return int(showCourtesySig());
            case Pid::NUMERATOR_STRING:
                  return numeratorString();
            case Pid::DENOMINATOR_STRING:
                  return denominatorString();
            case Pid::PARSER_STRING:
                  return parserString();
            case Pid::GROUPS:
                  return QVariant::fromValue(groups());
            case Pid::TIMESIG:
                  return QVariant::fromValue(_sig);
            case Pid::TIMESIG_GLOBAL:
                  return QVariant::fromValue(globalSig());
            case Pid::TIMESIG_STRETCH:
                  return QVariant::fromValue(stretch());
            case Pid::TIMESIG_TYPE:
                  return int(_timeSigType);
            case Pid::SCALE:
                  return _scale;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TimeSig::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::SHOW_COURTESY:
                  if (generated())
                        return false;
                  setShowCourtesySig(v.toBool());
                  break;
            case Pid::NUMERATOR_STRING:
                  setNumeratorString(v.toString());
                  break;
            case Pid::DENOMINATOR_STRING:
                  setDenominatorString(v.toString());
                  break;
            case Pid::PARSER_STRING:
                  setParserString(v.toString());
                  break;
            case Pid::GROUPS:
                  setGroups(v.value<Groups>());
                  break;
            case Pid::TIMESIG:
                  setSig(v.value<Fraction>());
                  break;
            case Pid::TIMESIG_GLOBAL:
                  setGlobalSig(v.value<Fraction>());
                  break;
            case Pid::TIMESIG_STRETCH:
                  setStretch(v.value<Fraction>());
                  break;
            case Pid::TIMESIG_TYPE:
                  _timeSigType = (TimeSigType)(v.toInt());
                  break;
            case Pid::SCALE:
                  _scale = v.toSizeF();
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayoutAll();      // TODO
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TimeSig::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SHOW_COURTESY:
                  return true;
            case Pid::NUMERATOR_STRING:
                  return QString();
            case Pid::DENOMINATOR_STRING:
                  return QString();
            case Pid::TIMESIG:
                  return QVariant::fromValue(Fraction(4,4));
            case Pid::TIMESIG_GLOBAL:
                  return QVariant::fromValue(Fraction(1,1));
            case Pid::TIMESIG_TYPE:
                  return int(TimeSigType::NORMAL);
            case Pid::SCALE:
                  return score()->styleV(Sid::timesigScale);
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* TimeSig::nextSegmentElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* TimeSig::prevSegmentElement()
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
                  timeSigString = qApp->translate("symUserNames", "Common time");
                  break;
            case TimeSigType::ALLA_BREVE:
                  timeSigString = qApp->translate("symUserNames", "Cut time");
                  break;
            case TimeSigType::CUT_BACH:
                  timeSigString = qApp->translate("symUserNames", "Cut time (Bach)");
                  break;
            case TimeSigType::CUT_TRIPLE:
                  timeSigString = qApp->translate("symUserNames", "Cut triple time (9/8)");
                  break;
            default:
                  timeSigString = QObject::tr("%1/%2 time").arg(QString::number(numerator()), QString::number(denominator()));
            }
      return QString("%1: %2").arg(Element::accessibleInfo(), timeSigString);
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
         && (_numeratorString == ts._numeratorString)
         && (_denominatorString == ts._denominatorString)
         && (_parserString == ts._parserString)
         ;
      }

}


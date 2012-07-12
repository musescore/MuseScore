//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: stafftype.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stafftype.h"
#include "staff.h"
#include "xml.h"
#include "mscore.h"

QList<StaffType*> staffTypes;

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

void initStaffTypes()
      {
      StaffTypePitched* st = new StaffTypePitched("Pitched 5 lines");
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(true);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);

      StaffTypeTablature* stab = new StaffTypeTablature("Tab");
      staffTypes.append(stab);

      StaffTypePercussion* sp = new StaffTypePercussion("Percussion 5 lines");
      sp->setLines(5);
      sp->setLineDistance(Spatium(1.0));
      sp->setGenClef(true);
      sp->setGenKeysig(false);
      sp->setSlashStyle(false);
      sp->setShowBarlines(true);
      sp->setShowLedgerLines(true);
      staffTypes.append(sp);
      }

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

StaffType::StaffType()
      {
      _lines           = 5;
      _stepOffset      = 0;
      _lineDistance    = Spatium(1);

      _genClef         = true;      // create clef at beginning of system
      _showBarlines    = true;
      _slashStyle      = false;     // do not show stems
      _genTimesig      = true;      // whether time signature is shown or not
      }

StaffType::StaffType(const QString& s)
      {
      _name            = s;
      _lines           = 5;
      _stepOffset      = 0;
      _lineDistance    = Spatium(1);

      _genClef         = true;      // create clef at beginning of system
      _showBarlines    = true;
      _slashStyle      = false;     // do not show stems
      _genTimesig      = true;      // whether time signature is shown or not
      }

//---------------------------------------------------------
//   isEqual
//---------------------------------------------------------

bool StaffType::isEqual(const StaffType& st) const
      {
      return st.group() == group()
         && st._name         == _name
         && st._lines        == _lines
         && st._stepOffset   == _stepOffset
         && st._lineDistance == _lineDistance
         && st._genClef      == _genClef
         && st._showBarlines == _showBarlines
         && st._slashStyle   == _slashStyle
         && st._genTimesig   == _genTimesig
         ;
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void StaffType::setLines(int val)
      {
      _lines = val;
      switch(_lines) {
            case 1:
                  _stepOffset = 0;
                  break;
            case 2:
            case 3:
                  _stepOffset = -2;
                  break;
            default:
                  _stepOffset = 0;
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffType::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void StaffType::writeProperties(Xml& xml) const
      {
      xml.tag("name", name());
      // unconditionally save number of lines
      // as different staff types have different default
//      if (lines() != 5)
            xml.tag("lines", lines());
      if (lineDistance().val() != 1.0)
            xml.tag("lineDistance", lineDistance().val());
      if (!genClef())
            xml.tag("clef", genClef());
      if (slashStyle())
            xml.tag("slashStyle", slashStyle());
      if (!showBarlines())
            xml.tag("barlines", showBarlines());
      if(!genTimesig())
            xml.tag("timesig", genTimesig());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffType::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if(!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool StaffType::readProperties(const QDomElement& e)
      {
      const QString& tag(e.tagName());
      int v = e.text().toInt();
      if (tag == "name")
            setName(e.text());
      else if (tag == "lines")
            setLines(v);
      else if (tag == "lineDistance")
            setLineDistance(Spatium(e.text().toDouble()));
      else if (tag == "clef")
            setGenClef(v);
      else if (tag == "slashStyle")
            setSlashStyle(v);
      else if (tag == "barlines")
            setShowBarlines(v);
      else if (tag == "timesig")
            setGenTimesig(v);
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   StaffTypePitched
//---------------------------------------------------------

StaffTypePitched::StaffTypePitched()
   : StaffType()
      {
      _genKeysig       = true;      // create key signature at beginning of system
      _showLedgerLines = true;
      }

//---------------------------------------------------------
//   isEqual
//---------------------------------------------------------

bool StaffTypePitched::isEqual(const StaffType& st) const
      {
      return StaffType::isEqual(st)
         && static_cast<const StaffTypePitched&>(st)._genKeysig       == _genKeysig
         && static_cast<const StaffTypePitched&>(st)._showLedgerLines == _showLedgerLines
         ;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypePitched::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      StaffType::writeProperties(xml);

      if (!genKeysig())
            xml.tag("keysig", genKeysig());
      if (!showLedgerLines())
            xml.tag("ledgerlines", showLedgerLines());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypePitched::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "keysig")
                  setGenKeysig(e.text().toInt());
            else if (tag == "ledgerlines")
                  setShowLedgerLines(e.text().toInt());
            else {
                  if (!StaffType::readProperties(e))
                        domError(e);
                  }
            }
      }

//---------------------------------------------------------
//   StaffTypePercussion
//---------------------------------------------------------

StaffTypePercussion::StaffTypePercussion()
   : StaffType()
      {
      _genKeysig       = true;      // create key signature at beginning of system
      _showLedgerLines = true;
      }

//---------------------------------------------------------
//   isEqual
//---------------------------------------------------------

bool StaffTypePercussion::isEqual(const StaffType& st) const
      {
      return StaffType::isEqual(st)
         && static_cast<const StaffTypePercussion&>(st)._genKeysig       == _genKeysig
         && static_cast<const StaffTypePercussion&>(st)._showLedgerLines == _showLedgerLines
         ;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypePercussion::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      StaffType::writeProperties(xml);

      if (!genKeysig())
            xml.tag("keysig", genKeysig());
      if (!showLedgerLines())
            xml.tag("ledgerlines", showLedgerLines());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypePercussion::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "keysig")
                  setGenKeysig(e.text().toInt());
            else if (tag == "ledgerlines")
                  setShowLedgerLines(e.text().toInt());
            else {
                  if (!StaffType::readProperties(e))
                        domError(e);
                  }
            }
      }

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

#define TAB_DEFAULT_DUR_YOFFS	(-1.75)
#define TAB_DEFAULT_LINE_SP	(1.5)

void StaffTypeTablature::init()
      {
      // set reasonable defaults for inherited members
      setLines(6);
      setLineDistance(Spatium(TAB_DEFAULT_LINE_SP));
      setGenClef(true);
      setSlashStyle(false);
      setShowBarlines(true);
      setGenTimesig(false);
      // for specific members
      setDurationFontName("MScoreTabulatureModern");
      setDurationFontSize(15.0);
      setDurationFontUserY(0.0);
      setFretFontName("MScoreTabulatureModern");
      setFretFontSize(10.0);
      setFretFontUserY(0.0);
      setGenDurations(false);
      setLinesThrough(false);
      setOnLines(true);
      setUpsideDown(false);
      setUseNumbers(true);
      // internal
      _durationMetricsValid = _fretMetricsValid = false;
      _durationBoxH = _durationBoxY = _durationYOffset = _fretBoxH = _fretBoxY = _fretYOffset = _refDPI = 0.0;
      }

//---------------------------------------------------------
//   isEqual
//---------------------------------------------------------

bool StaffTypeTablature::isEqual(const StaffType& st) const
      {
      return StaffType::isEqual(st)
         && static_cast<const StaffTypeTablature&>(st)._durationFontName   == _durationFontName
         && static_cast<const StaffTypeTablature&>(st)._durationFontSize   == _durationFontSize
         && static_cast<const StaffTypeTablature&>(st)._durationFontUserY  == _durationFontUserY
         && static_cast<const StaffTypeTablature&>(st)._fretFontName       == _fretFontName
         && static_cast<const StaffTypeTablature&>(st)._fretFontSize       == _fretFontSize
         && static_cast<const StaffTypeTablature&>(st)._fretFontUserY      == _fretFontUserY
         && static_cast<const StaffTypeTablature&>(st)._genDurations       == _genDurations
         && static_cast<const StaffTypeTablature&>(st)._linesThrough       == _linesThrough
         && static_cast<const StaffTypeTablature&>(st)._onLines            == _onLines
         && static_cast<const StaffTypeTablature&>(st)._upsideDown         == _upsideDown
         && static_cast<const StaffTypeTablature&>(st)._useNumbers         == _useNumbers
         ;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypeTablature::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());

            if (tag == "durations")
                  setGenDurations(val.toInt() != 0);
            else if (tag == "durationFontName")
                  setDurationFontName(e.text());
            else if (tag == "durationFontSize")
                  setDurationFontSize(val.toDouble());
            else if (tag == "durationFontY")
                  setDurationFontUserY(val.toDouble());
            else if (tag == "fretFontName")
                  setFretFontName(e.text());
            else if (tag == "fretFontSize")
                  setFretFontSize(val.toDouble());
            else if (tag == "fretFontY")
                  setFretFontUserY(val.toDouble());
            else if (tag == "linesThrough")
                  setLinesThrough(val.toInt() != 0);
            else if (tag == "onLines")
                  setOnLines(val.toInt() != 0);
            else if (tag == "upsideDown")
                  setUpsideDown(val.toInt() != 0);
            else if (tag == "useNumbers")
                  setUseNumbers(val.toInt() != 0);
            else
                  if(!StaffType::readProperties(e))
                        domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypeTablature::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      StaffType::writeProperties(xml);
      xml.tag("durations",        _genDurations);
      xml.tag("durationFontName", _durationFontName);
      xml.tag("durationFontSize", _durationFontSize);
      xml.tag("durationFontY",    _durationFontUserY);
      xml.tag("fretFontName",     _fretFontName);
      xml.tag("fretFontSize",     _fretFontSize);
      xml.tag("fretFontY",        _fretFontUserY);
      xml.tag("linesThrough",     _linesThrough);
      xml.tag("onLines",          _onLines);
      xml.tag("upsideDown",       _upsideDown);
      xml.tag("useNumbers",       _useNumbers);
      xml.etag();
      }

//---------------------------------------------------------
//   setOnLines
//---------------------------------------------------------

void StaffTypeTablature::setOnLines(bool val)
      {
      _onLines = val;
      _durationMetricsValid = _fretMetricsValid = false;
      }

//---------------------------------------------------------
//   set metrics
//    checks whether the internally computed metrics are is still valid and re-computes them, if not
//---------------------------------------------------------

static QString    g_strNumbers("0123456789");
static QString    g_strLetters("abcdefghiklmnopq");
// used both to generate duration symbols and to compute duration metrics:
static QChar g_cDurationChars[] = { 0xE0FF, 0xE100, 0xE101, 0xE102, 0xE103, 0xE104,
//                                   Longa  Brevis   Whole   Half   Quarter  1/8
                                    0xE105, 0xE106, 0xE107, 0xE108, 0xE109, 0xE10B, ' ', ' '};
//                                   1\16    1\32    1\64    1\128   1\256   dot
#define STAFFTYPETAB_NUMOFDURCHARS  12    /* how many used chars there are in g_cDurationChar[] */
#define STAFFTYPETAB_IDXOFDOTCHAR   11    /* the offset of the dot char in g_cDurationChars[] */


void StaffTypeTablature::setDurationMetrics()
{
      if (_durationMetricsValid && _refDPI == MScore::DPI)           // metrics are still valid
            return;

      QFontMetricsF fm(durationFont());
      QRectF bb( fm.tightBoundingRect(QString(g_cDurationChars, STAFFTYPETAB_NUMOFDURCHARS)) );
      // move symbols so that the lowest margin 'sits' on the base line:
      // move down by the whole part above (negative) the base line
      // ( -bb.y() ) then up by the whole height ( -bb.height()/2 )
      _durationYOffset = -bb.y() - bb.height()
      // then move up by a default margin and, if marks are above lines, by half the line distance
      // (converted from spatium units to raster units)
            + ( TAB_DEFAULT_DUR_YOFFS - (_onLines ? 0.0 : lineDistance().val()/2.0) ) * MScore::DPI*SPATIUM20;
      _durationBoxH = bb.height();
      _durationBoxY = bb.y()  + _durationYOffset;
      // keep track of the conditions under which metrics have been computed
      _refDPI = MScore::DPI;
      _durationMetricsValid = true;
}

void StaffTypeTablature::setFretMetrics()
{
      if(_fretMetricsValid && _refDPI == MScore::DPI)
            return;

      QFontMetricsF fm(fretFont());
      // compute total height of used characters
      QRectF bb(fm.tightBoundingRect(_useNumbers ? g_strNumbers : g_strLetters));
      // compute vertical displacement
      if(_useNumbers) {
            // for numbers: centre on '0': move down by the whole part above (negative)
            // the base line ( -bb.y() ) then up by half the whole height ( -bb.height()/2 )
            QRectF bx( fm.tightBoundingRect("0") );
            _fretYOffset = -(bx.y() + bx.height()/2.0);
            // _fretYOffset = -(bb.y() + bb.height()/2.0);  // <- using bbox of all chars
            }
      else {
            // for letters: centre on the 'a' ascender, by moving down half of the part above the base line in bx
            QRectF bx( fm.tightBoundingRect("a") );
            _fretYOffset = -bx.y() / 2.0;
            }
      // if on string, we are done; if between strings, raise by half line distance
      if(!_onLines)
            _fretYOffset -= lineDistance().val()*MScore::DPI*SPATIUM20 / 2.0;

      // from _fretYOffset, compute _charBoxH and _charBoxY
      _fretBoxH = bb.height();
      _fretBoxY = bb.y()  + _fretYOffset;

      // keep track of the conditions under which metrics have been computed
      _refDPI = MScore::DPI;
      _fretMetricsValid = true;
}

//---------------------------------------------------------
//   setDurationFontSize
//---------------------------------------------------------

void StaffTypeTablature::setDurationFontSize(qreal val)
      {
      _durationFontSize = val;
//    _durationFont.setPointSizeF(val);
      _durationFont.setPixelSize( lrint(val * MScore::DPI / PPI) );
      _durationMetricsValid = false;
      }

//---------------------------------------------------------
//   setFretFontSize
//---------------------------------------------------------

void StaffTypeTablature::setFretFontSize(qreal val)
      {
      _fretFontSize = val;
//    _fretFont.setPointSizeF(val);
      _fretFont.setPixelSize( lrint(val * MScore::DPI / PPI) );
      _fretMetricsValid = false;
      }

//---------------------------------------------------------
//   TabDurationSymbol
//---------------------------------------------------------

TabDurationSymbol::TabDurationSymbol(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setGenerated(true);
      _tab  = 0;
      _text = QString();
      }

TabDurationSymbol::TabDurationSymbol(Score* s, StaffTypeTablature * tab, TDuration::DurationType type, int dots)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setGenerated(true);
      _tab  = tab;
      buildText(type, dots);
      }

TabDurationSymbol::TabDurationSymbol(const TabDurationSymbol& e)
   : Element(e)
      {
      _tab = e._tab;
      _text = e._text;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------
/*
void TabDurationSymbol::layout()
      {
      QFontMetricsF fm(_tab->durationFont());
      qreal mags = magS();
      qreal w = fm.width(_text);
      _bbox = QRectF(0.0, _tab->durationBoxY() * mags, w * mags, _tab->durationBoxH() * mags);
      }
*/
//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TabDurationSymbol::draw(QPainter* painter) const
      {
      if(!_tab)
            return;
      qreal mag = magS();
      qreal imag = 1.0 / mag;

      painter->setPen(curColor());
      painter->scale(mag, mag);
      painter->setFont(_tab->durationFont());
      painter->drawText(QPointF(0.0, _tab->durationFontYOffset()), _text);
      painter->scale(imag, imag);
      }

//---------------------------------------------------------
//   buildText
//---------------------------------------------------------

void TabDurationSymbol::buildText(TDuration::DurationType type, int dots)
      {
      // text string is a main symbol plus as many dots as required by chord duration
      _text = QString(g_cDurationChars[type]);
      for(int count=0; count < dots; count++)
            _text.append(g_cDurationChars[STAFFTYPETAB_IDXOFDOTCHAR]);
      }

//---------------------------------------------------------
//   doty1
//    get y dot position of first repeat barline dot
//---------------------------------------------------------

qreal StaffType::doty1() const
      {
      switch(_lines) {
            case 1:
                  return -_lineDistance.val() * .5;
            case 2:
                  return -_lineDistance.val() * .5;
            case 3:
                  return _lineDistance.val() * .5;
            case 4:
                  return _lineDistance.val() * .5;
            case 5:
                  return _lineDistance.val() * 1.5;
            case 6:
                  return _lineDistance.val() * 1.5;
            default:
                  qDebug("StaffType::doty1(): lines %d unsupported\n", _lines);
                  break;
            }
      return 0.0;
      }

//---------------------------------------------------------
//   doty2
//    get y dot position of second repeat barline dot
//---------------------------------------------------------

qreal StaffType::doty2() const
      {
      switch(_lines) {
            case 1:
                  return _lineDistance.val() * .5;
            case 2:
                  return _lineDistance.val() * 1.5;
            case 3:
                  return _lineDistance.val() * 1.5;
            case 4:
                  return _lineDistance.val() * 2.5;
            case 5:
                  return _lineDistance.val() * 2.5;
            case 6:
                  return _lineDistance.val() * 3.5;
            default:
                  qDebug("StaffType::doty2(): lines %d unsupported\n", _lines);
                  break;
            }
      return 0.0;
      }

//---------------------------------------------------------
//   durationBoxH
//---------------------------------------------------------

qreal StaffTypeTablature::durationBoxH()
      {
      if (!_genDurations && !_slashStyle)
            return 0.0;
      setDurationMetrics();
      return _durationBoxH;
      }

//---------------------------------------------------------
//   durationBoxY
//---------------------------------------------------------

qreal StaffTypeTablature::durationBoxY()
      {
      if(!_genDurations && !_slashStyle)
            return 0.0;
      setDurationMetrics();
      return _durationBoxY + _durationFontUserY * MScore::MScore::DPI * SPATIUM20;
      }


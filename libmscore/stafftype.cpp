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

#define TAB_DEFAULT_LINE_SP   (1.5)
#define TAB_RESTSYMBDISPL     2.0

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
      return isSameStructure(st)
         && st._name == _name;
      }

//---------------------------------------------------------
//   isSameStructure
//
//    same as isEqual, but ignores name
//---------------------------------------------------------

bool StaffType::isSameStructure(const StaffType& st) const
      {
      return st.group() == group()
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
      // uncontionally write properties: staff types are read back over a copy of the built-in types
      // and properties may be different across types => each might need to be properly (re-)set
//      if (lines() != 5)
            xml.tag("lines", lines());
//      if (lineDistance().val() != 1.0)
            xml.tag("lineDistance", lineDistance().val());
//      if (!genClef())
            xml.tag("clef", genClef());
//      if (slashStyle())
            xml.tag("slashStyle", slashStyle());
//      if (!showBarlines())
            xml.tag("barlines", showBarlines());
//      if(!genTimesig())
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

QList<TablatureFretFont>     StaffTypeTablature::_fretFonts      = QList<TablatureFretFont>();
QList<TablatureDurationFont> StaffTypeTablature::_durationFonts  = QList<TablatureDurationFont>();

void StaffTypeTablature::init()
      {
      // set reasonable defaults for type-specific members */
      setDurationFontName(_durationFonts[0].displayName);
      setDurationFontSize(15.0);
      setDurationFontUserY(0.0);
      setFretFontName(_fretFonts[0].displayName);
      setFretFontSize(10.0);
      setFretFontUserY(0.0);
      setGenDurations(false);
      setGenTimesig(false);
      setLineDistance(Spatium(TAB_DEFAULT_LINE_SP));
      setLines(6);
      setLinesThrough(false);
      setMinimStyle(TAB_MINIM_NONE);
      setOnLines(true);
      setShowRests(false);
      setStemsDown(true);
      setStemsThrough(true);
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
         && static_cast<const StaffTypeTablature&>(st)._durationFontIdx       == _durationFontIdx
         && static_cast<const StaffTypeTablature&>(st)._durationFontSize      == _durationFontSize
         && static_cast<const StaffTypeTablature&>(st)._durationFontUserY     == _durationFontUserY
         && static_cast<const StaffTypeTablature&>(st)._fretFontIdx           == _fretFontIdx
         && static_cast<const StaffTypeTablature&>(st)._fretFontSize          == _fretFontSize
         && static_cast<const StaffTypeTablature&>(st)._fretFontUserY         == _fretFontUserY
         && static_cast<const StaffTypeTablature&>(st)._genDurations          == _genDurations
         && static_cast<const StaffTypeTablature&>(st)._linesThrough          == _linesThrough
         && static_cast<const StaffTypeTablature&>(st)._minimStyle            == _minimStyle
         && static_cast<const StaffTypeTablature&>(st)._onLines               == _onLines
         && static_cast<const StaffTypeTablature&>(st)._showRests             == _showRests
         && static_cast<const StaffTypeTablature&>(st)._stemsDown             == _stemsDown
         && static_cast<const StaffTypeTablature&>(st)._stemsThrough          == _stemsThrough
         && static_cast<const StaffTypeTablature&>(st)._upsideDown            == _upsideDown
         && static_cast<const StaffTypeTablature&>(st)._useNumbers            == _useNumbers
         ;
      }

//---------------------------------------------------------
//   isSameStructure
//
//    same as isEqual, but ignores name and font data
//---------------------------------------------------------

bool StaffTypeTablature::isSameStructure(const StaffTypeTablature& stt) const
      {
      return StaffType::isSameStructure(static_cast<const StaffType&>(stt))
         && stt._genDurations == _genDurations
         && stt._linesThrough == _linesThrough
         && stt._minimStyle   == _minimStyle
         && stt._onLines      == _onLines
         && stt._showRests    == _showRests
         && stt._stemsDown    == _stemsDown
         && stt._stemsThrough == _stemsThrough
         && stt._upsideDown   == _upsideDown
         && stt._useNumbers   == _useNumbers
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
            else if (tag == "minimStyle")
                  setMinimStyle( (TablatureMinimStyle) val.toInt() );
            else if (tag == "onLines")
                  setOnLines(val.toInt() != 0);
            else if (tag == "showRests")
                  setShowRests(val.toInt() != 0);
            else if (tag == "stemsDown")
                  setStemsDown(val.toInt() != 0);
            else if (tag == "stemsThrough")
                  setStemsThrough(val.toInt() != 0);
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
      xml.tag("durations",          _genDurations);
      xml.tag("durationFontName",   _durationFonts[_durationFontIdx].displayName);
      xml.tag("durationFontSize",   _durationFontSize);
      xml.tag("durationFontY",      _durationFontUserY);
      xml.tag("fretFontName",       _fretFonts[_fretFontIdx].displayName);
      xml.tag("fretFontSize",       _fretFontSize);
      xml.tag("fretFontY",          _fretFontUserY);
      xml.tag("linesThrough",       _linesThrough);
      xml.tag("minimStyle",         _minimStyle);
      xml.tag("onLines",            _onLines);
      xml.tag("showRests",          _showRests);
      xml.tag("stemsDown",          _stemsDown);
      xml.tag("stemsThrough",       _stemsThrough);
      xml.tag("upsideDown",         _upsideDown);
      xml.tag("useNumbers",         _useNumbers);
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
//   setDurationFontName / setFretFontName
//---------------------------------------------------------

void StaffTypeTablature::setDurationFontName(QString name)
      {
      int   idx;
      for(idx=0; idx < _durationFonts.size(); idx++)
            if(_durationFonts[idx].displayName == name)
                  break;
      if(idx >= _durationFonts.size())    idx = 0;          // if name not found, use first font
      _durationFont.setFamily(_durationFonts[idx].family);
      _durationFontIdx = idx;
      _durationMetricsValid = false;
      }

void StaffTypeTablature::setFretFontName(QString name)
      {
      int   idx;
      for(idx=0; idx < _fretFonts.size(); idx++)
            if(_fretFonts[idx].displayName == name)
                  break;
      if(idx >= _fretFonts.size())        idx = 0;          // if name not found, use first font
      _fretFont.setFamily(_fretFonts[idx].family);
      _fretFontIdx = idx;
      _fretMetricsValid = false;
      }

//---------------------------------------------------------
//   durationBoxH / durationBoxY
//---------------------------------------------------------

qreal StaffTypeTablature::durationBoxH()
      {
      if (!_genDurations && !_slashStyle)
            return 0.0;
      setDurationMetrics();
      return _durationBoxH;
      }

qreal StaffTypeTablature::durationBoxY()
      {
      if(!_genDurations && !_slashStyle)
            return 0.0;
      setDurationMetrics();
      return _durationBoxY + _durationFontUserY * MScore::MScore::DPI * SPATIUM20;
      }

//---------------------------------------------------------
//   setDurationFontSize / setFretFontSize
//---------------------------------------------------------

void StaffTypeTablature::setDurationFontSize(qreal val)
      {
      _durationFontSize = val;
      _durationFont.setPixelSize( lrint(val * MScore::DPI / PPI) );
      _durationMetricsValid = false;
      }

void StaffTypeTablature::setFretFontSize(qreal val)
      {
      _fretFontSize = val;
      _fretFont.setPixelSize( lrint(val * MScore::DPI / PPI) );
      _fretMetricsValid = false;
      }

//---------------------------------------------------------
//   fretString / durationString
//
//    construct the text string for a given fret / duration
//---------------------------------------------------------

QString StaffTypeTablature::fretString(int fret, bool ghost) const
      {
      QString s = ghost ? _fretFonts[_fretFontIdx].ghostChar :
            ( _useNumbers ?   _fretFonts[_fretFontIdx].displayDigit[fret] :
                              _fretFonts[_fretFontIdx]. displayLetter[fret]
            );
      return s;
      }

QString StaffTypeTablature::durationString(TDuration::DurationType type, int dots) const
{
      QString s = _durationFonts[_durationFontIdx].displayValue[type];
      for(int count=0; count < dots; count++)
            s.append(_durationFonts[_durationFontIdx].displayDot);
      return s;
}

//---------------------------------------------------------
//   physStringToVisual / VisualStringToPhys
//
//    returns the string ordinal in visual order (top to down) from a string ordinal in physical order
//    or viceversa: manage upsideDown
//
//    (The 2 functions are at the moment almost identical; support for unfrettted strings will
//    introduce more differences)
//---------------------------------------------------------

int StaffTypeTablature::physStringToVisual(int strg) const
{
      if(strg <= STRING_NONE || strg >= _lines)             // if no physical string, return topmost visual string
            return 0;
      // if TAB upside down, reverse string number
      return (_upsideDown ? _lines - 1 - strg : strg);
}

int StaffTypeTablature::VisualStringToPhys(int strg) const
{
      if(strg <= VISUAL_STRING_NONE || strg >= _lines)      // if no visual string, return topmost physical string
            return 0;
      // if TAB upside down, reverse string number
      return (_upsideDown ? _lines - 1 - strg : strg);
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

void TabDurationSymbol::layout()
      {
      QFontMetricsF fm(_tab->durationFont());
      qreal mags = magS();
      qreal w = fm.width(_text);
      qreal y = _tab->durationBoxY();
      // with rests, move symbol down by half its displacement from staff
      if(parent() && parent()->type() == REST)
            y += TAB_RESTSYMBDISPL * spatium();
      setbbox(QRectF(0.0, y * mags, w * mags, _tab->durationBoxH() * mags) );
      }

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
      qreal y = _tab->durationFontYOffset();
      if(parent() && parent()->type() == REST)
            y += TAB_RESTSYMBDISPL * spatium();
      painter->drawText(QPointF(0.0, y), _text);
      painter->scale(imag, imag);
      }

//---------------------------------------------------------
//   buildText
//---------------------------------------------------------
/*
void TabDurationSymbol::buildText(TDuration::DurationType type, int dots)
      {
      // text string is a main symbol plus as many dots as required by chord duration
      _text = QString(g_cDurationChars[type]);
      for(int count=0; count < dots; count++)
            _text.append(g_cDurationChars[STAFFTYPETAB_IDXOFDOTCHAR]);
      }
*/
//---------------------------------------------------------
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

bool TablatureFretFont::read(const QDomElement &de)
{
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString&    tag(e.tagName());
            const QString&    txt(e.text());
            if(txt.size() < 1)
                  return false;
            int               val = e.attribute("value").toInt();

            if (tag == "family")
                  family = txt;
            else if(tag == "displayName")
                  displayName = txt;
            else if(tag == "defaultPitch")
                  defPitch = txt.toDouble();
            else if(tag == "mark") {
                  QString val = e.attribute("value");
                  if(val.size() < 1)
                        return false;
                  if(val == "x")
                        xChar = txt[0];
                  else if(val == "ghost")
                        ghostChar = txt[0];
                  }
            else if(tag == "fret") {
                  bool bLetter = e.attribute("letter").toInt();
                  if(bLetter) {
                        if(val >= 0 && val < NUM_OF_LETTERFRETS)
                              displayLetter[val] = txt[0];
                        }
                  else {
                        if(val >= 0 && val < NUM_OF_DIGITFRETS)
                              displayDigit[val] = txt;
                        }
                  }
            else {
                  domError(e);
                  return false;
                  }
            }
      return true;
}

bool TablatureDurationFont::read(const QDomElement &de)
{
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString&    tag(e.tagName());
            const QString&    txt(e.text());
            if(txt.size() < 1)
                  return false;
            QChar             chr = txt[0];

            if (tag == "family")
                  family = txt;
            else if(tag == "displayName")
                  displayName = txt;
            else if(tag == "defaultPitch")
                  defPitch = txt.toDouble();
            else if(tag == "duration") {
                  QString val = e.attribute("value");
                  if(val.size() < 1)
                        return false;
                  if(val == "longa")
                        displayValue[TAB_VAL_LONGA] = chr;
                  else if(val == "brevis")
                        displayValue[TAB_VAL_BREVIS] = chr;
                  else if(val == "semibrevis")
                        displayValue[TAB_VAL_SEMIBREVIS] = chr;
                  else if(val == "minima")
                        displayValue[TAB_VAL_MINIMA] = chr;
                  else if(val == "semiminima")
                        displayValue[TAB_VAL_SEMIMINIMA] = chr;
                  else if(val == "fusa")
                        displayValue[TAB_VAL_FUSA] = chr;
                  else if(val == "semifusa")
                        displayValue[TAB_VAL_SEMIFUSA] = chr;
                  else if(val == "32")
                        displayValue[TAB_VAL_32] = chr;
                  else if(val == "64")
                        displayValue[TAB_VAL_64] = chr;
                  else if(val == "128")
                        displayValue[TAB_VAL_128] = chr;
                  else if(val == "256")
                        displayValue[TAB_VAL_256] = chr;
                  else if(val == "dot")
                        displayDot = chr;
                  }
            else {
                  domError(e);
                  return false;
                  }
            }
      return true;
}

//---------------------------------------------------------
//   Read Configuration File
//
//    reads a configuration and appends read data to g_TABFonts
//    resets everythings and reads the built-in config file if fileName is null or empty
//---------------------------------------------------------

bool StaffTypeTablature::readConfigFile(const QString& fileName)
{
      QString     path;

      if(fileName == 0 || fileName.isEmpty()) {       // defaults to built-in xml
#ifdef Q_WS_IOS
            {
            extern QString resourcePath();
            QString rpath = resourcePath();
            path = rpath + QString("/fonts_tablature.xml");
            }
#else
            path = ":/fonts/fonts_tablature.xml";
#endif
            _durationFonts.clear();
            _fretFonts.clear();
            }
      else
            path = fileName;

      QFileInfo fi(path);
      QFile f(path);

      if (!fi.exists() || !f.open(QIODevice::ReadOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "cannot open tablature font description:\n%1\n%2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
qDebug("StaffTypeTablature::readConfigFile failed: <%s>\n", qPrintable(path));
            return false;
            }
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error reading tablature font description %1 at line %2 column %3: %4\n");
            MScore::lastError = s.arg(f.fileName()).arg(line).arg(column).arg(err);
            return false;
            }
      docName = f.fileName();

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement de = e.firstChildElement(); !de.isNull();  de = de.nextSiblingElement()) {
                        const QString& tag(de.tagName());
                        if (tag == "fretFont") {
                              TablatureFretFont f;
                              if(f.read(de))
                                    _fretFonts.append(f);
                              else
                                    continue;
                              }
                        else if (tag == "durationFont") {
                              TablatureDurationFont f;
                              if(f.read(de))
                                    _durationFonts.append(f);
                              else
                                    continue;
                              }
                        else
                              domError(de);
                        }
                  return true;
                  }
            }
      return false;
}

//---------------------------------------------------------
//   Get Font Names
//
//    returns a list of display names for the fonts  configured to work with Tablatures;
//    the index of a name in the list can be used to retrieve the font data with fontData()
//---------------------------------------------------------

QList<QString> StaffTypeTablature::fontNames(bool bDuration)
      {
      QList<QString> names;
      if(bDuration)
            foreach(const TablatureDurationFont& f, _durationFonts)
                  names.append(f.displayName);
      else
            foreach(const TablatureFretFont& f, _fretFonts)
                  names.append(f.displayName);
      return names;
      }

//---------------------------------------------------------
//   Get Font Data
//
//    retrieves data about a Tablature font.
//    returns: true if idx is valid | false if it is not
// any of the pointer parameter can be null, if that datum is not needed
//---------------------------------------------------------

bool StaffTypeTablature::fontData(bool bDuration, int nIdx, QString * pFamily, QString * pDisplayName,
            qreal * pSize)
      {
      if(bDuration) {
            if(nIdx >= 0 && nIdx < _durationFonts.size()) {
                  TablatureDurationFont f = _durationFonts.at(nIdx);
                  if(pFamily)       *pFamily          = f.family;
                  if(pDisplayName)  *pDisplayName     = f.displayName;
                  if(pSize)         *pSize            = f.defPitch;
                  return true;
                  }
            }
      else {
            if(nIdx >= 0 && nIdx < _fretFonts.size()) {
                  TablatureFretFont f = _fretFonts.at(nIdx);
                  if(pFamily)       *pFamily          = f.family;
                  if(pDisplayName)  *pDisplayName     = f.displayName;
                  if(pSize)         *pSize            = f.defPitch;
                  return true;
                  }
            }
      return false;
      }


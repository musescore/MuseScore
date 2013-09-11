//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "chord.h"
#include <array>

#define TAB_DEFAULT_LINE_SP   (1.5)
#define TAB_RESTSYMBDISPL     2.0

namespace Ms {

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

StaffType::StaffType()
      {
      _builtin         = false;
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
//    same as isEqual(), but ignores name
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
      xml.tag("lines", lines());
      xml.tag("lineDistance", lineDistance().val());
      xml.tag("clef", genClef());
      xml.tag("slashStyle", slashStyle());
      xml.tag("barlines", showBarlines());
      xml.tag("timesig", genTimesig());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffType::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool StaffType::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "name")
            setName(e.readElementText());
      else if (tag == "lines")
            setLines(e.readInt());
      else if (tag == "lineDistance")
            setLineDistance(Spatium(e.readDouble()));
      else if (tag == "clef")
            setGenClef(e.readInt());
      else if (tag == "slashStyle")
            setSlashStyle(e.readInt());
      else if (tag == "barlines")
            setShowBarlines(e.readInt());
      else if (tag == "timesig")
            setGenTimesig(e.readInt());
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
//   isSameStructure
//
//    same as isEqual(), but ignores name
//---------------------------------------------------------

bool StaffTypePitched::isSameStructure(const StaffType& st) const
{    return StaffType::isSameStructure(st)
        && static_cast<const StaffTypePitched&>(st)._genKeysig       == _genKeysig
        && static_cast<const StaffTypePitched&>(st)._showLedgerLines == _showLedgerLines;
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

void StaffTypePitched::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "keysig")
                  setGenKeysig(e.readInt());
            else if (tag == "ledgerlines")
                  setShowLedgerLines(e.readInt());
            else {
                  if (!StaffType::readProperties(e))
                        e.unknown();
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
//   isSameStructure
//
//    same as isEqual(), but ignores name
//---------------------------------------------------------

bool StaffTypePercussion::isSameStructure(const StaffType& st) const
{    return StaffType::isSameStructure(st)
        && static_cast<const StaffTypePercussion&>(st)._genKeysig       == _genKeysig
        && static_cast<const StaffTypePercussion&>(st)._showLedgerLines == _showLedgerLines;
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

void StaffTypePercussion::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "keysig")
                  setGenKeysig(e.readInt());
            else if (tag == "ledgerlines")
                  setShowLedgerLines(e.readInt());
            else {
                  if (!StaffType::readProperties(e))
                        e.unknown();
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
//    same as isEqual(), but ignores name and font data
//---------------------------------------------------------

bool StaffTypeTablature::isSameStructure(const StaffType &st) const
      {
      return StaffType::isSameStructure(st)
         && static_cast<const StaffTypeTablature&>(st)._genDurations == _genDurations
         && static_cast<const StaffTypeTablature&>(st)._linesThrough == _linesThrough
         && static_cast<const StaffTypeTablature&>(st)._minimStyle   == _minimStyle
         && static_cast<const StaffTypeTablature&>(st)._onLines      == _onLines
         && static_cast<const StaffTypeTablature&>(st)._showRests    == _showRests
         && static_cast<const StaffTypeTablature&>(st)._stemsDown    == _stemsDown
         && static_cast<const StaffTypeTablature&>(st)._stemsThrough == _stemsThrough
         && static_cast<const StaffTypeTablature&>(st)._upsideDown   == _upsideDown
         && static_cast<const StaffTypeTablature&>(st)._useNumbers   == _useNumbers;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypeTablature::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "durations")
                  setGenDurations(e.readInt() != 0);
            else if (tag == "durationFontName")
                  setDurationFontName(e.readElementText());
            else if (tag == "durationFontSize")
                  setDurationFontSize(e.readDouble());
            else if (tag == "durationFontY")
                  setDurationFontUserY(e.readDouble());
            else if (tag == "fretFontName")
                  setFretFontName(e.readElementText());
            else if (tag == "fretFontSize")
                  setFretFontSize(e.readDouble());
            else if (tag == "fretFontY")
                  setFretFontUserY(e.readDouble());
            else if (tag == "linesThrough")
                  setLinesThrough(e.readInt() != 0);
            else if (tag == "minimStyle")
                  setMinimStyle( (TablatureMinimStyle) e.readInt() );
            else if (tag == "onLines")
                  setOnLines(e.readInt() != 0);
            else if (tag == "showRests")
                  setShowRests(e.readInt() != 0);
            else if (tag == "stemsDown")
                  setStemsDown(e.readInt() != 0);
            else if (tag == "stemsThrough")
                  setStemsThrough(e.readInt() != 0);
            else if (tag == "upsideDown")
                  setUpsideDown(e.readInt() != 0);
            else if (tag == "useNumbers")
                  setUseNumbers(e.readInt() != 0);
            else
                  if (!StaffType::readProperties(e))
                        e.unknown();
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

void StaffTypeTablature::setDurationMetrics()
{
      if (_durationMetricsValid && _refDPI == MScore::DPI)           // metrics are still valid
            return;

      QFontMetricsF fm(durationFont());
      QString txt(_durationFonts[_durationFontIdx].displayValue, NUM_OF_TAB_VALS);
      QRectF bb( fm.tightBoundingRect(txt) );
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
      QRectF bb;
      // compute vertical displacement
      if(_useNumbers) {
            // compute total height of used characters
            QString txt = QString();
            for (int idx = 0; idx < 10; idx++)  // use only first 10 digits
                  txt.append(_fretFonts[_fretFontIdx].displayDigit[idx]);
            bb = fm.tightBoundingRect(txt);
            // for numbers: centre on '0': move down by the whole part above (negative)
            // the base line ( -bb.y() ) then up by half the whole height ( -bb.height()/2 )
            QRectF bx( fm.tightBoundingRect(_fretFonts[_fretFontIdx].displayDigit[0]) );
            _fretYOffset = -(bx.y() + bx.height()/2.0);
            // _fretYOffset = -(bb.y() + bb.height()/2.0);  // <- using bbox of all chars
            }
      else {
            // compute total height of used characters
            QString txt(_fretFonts[_fretFontIdx].displayLetter, NUM_OF_LETTERFRETS);
            bb = fm.tightBoundingRect(txt);
            // for letters: centre on the 'a' ascender, by moving down half of the part above the base line in bx
            QRectF bx( fm.tightBoundingRect(_fretFonts[_fretFontIdx].displayLetter[0]) );
            _fretYOffset = -bx.y() / 2.0;
            }
      // if on string, we are done; if between strings, raise by half line distance
      if(!_onLines)
            _fretYOffset -= lineDistance().val()*MScore::DPI*SPATIUM20 / 2.0;

      // from _fretYOffset, compute _fretBoxH and _fretBoxY
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
//   chordRestStemPosY / chordStemPos / chordStemPosBeam / chordStemLength
//
//    computes the stem data for the given chord, according to TAB settings
//    NOTE: unit: spatium, position: relative to chord (DIFFERENT from Chord own functions)
//
//   chordRestStemPosY
//          returns the vertical position of stem start point
//---------------------------------------------------------

qreal StaffTypeTablature::chordRestStemPosY(const ChordRest *chordRest) const
      {
      if (stemThrough())            // does not make sense for "stems through staves" setting; just return top line vert. position
            return 0.0;

      // if stems beside staff, position are fixed, but take into account delta for half notes
      qreal delta =                             // displacement for half note stems (if used)
            // if half notes have not a short stem OR not a half note => 0
            (minimStyle() != TAB_MINIM_SHORTER || chordRest->durationType().type() != TDuration::V_HALF) ?
                  0.0 :
                  // if stem is up, displace of half stem length down (positive)
                  // if stem is down, displace of half stem length up (negative)
                  (chordRest->up() ?
                        -STAFFTYPE_TAB_DEFAULTSTEMLEN_UP : STAFFTYPE_TAB_DEFAULTSTEMLEN_DN) * 0.5;
      // if fret marks above lines and chordRest is up, move half a line distance up
      if (!onLines() && chordRest->up())
            delta -= _lineDistance.val() *0.5;
      qreal y = (chordRest->up() ? STAFFTYPE_TAB_DEFAULTSTEMPOSY_UP : (_lines-1)*_lineDistance.val() + STAFFTYPE_TAB_DEFAULTSTEMPOSY_DN)
            + delta;
      return y;
      }

//---------------------------------------------------------
//   chordStemPos
//          return position of note at other side of beam
//---------------------------------------------------------

QPointF StaffTypeTablature::chordStemPos(const Chord *chord) const
      {
      qreal y;
      if (stemThrough())
            // if stems are through staff, stem goes from fartest note string
            y = (chord->up() ? chord->downString() : chord->upString()) * _lineDistance.val();
      else
            // if stems are beside staff, stem start point has a fixed vertical position,
            // according to TAB parameters and stem up/down
            y = chordRestStemPosY(chord);
      return QPointF(chordStemPosX(chord), y);
      }

//---------------------------------------------------------
//   chordStemPosBeam
//          return position of note at beam side of stem
//---------------------------------------------------------

QPointF StaffTypeTablature::chordStemPosBeam(const Chord *chord) const
      {
      qreal y = ( stemsDown() ? chord->downString() : chord->upString() ) * _lineDistance.val();

      return QPointF(chordStemPosX(chord), y);
      }

//---------------------------------------------------------
//   chordStemLength
//          return length of stem
//---------------------------------------------------------

qreal StaffTypeTablature::chordStemLength(const Chord *chord) const
      {
      qreal    stemLen;
      // if stems are through staff, length should be computed by relevant chord algorithm;
      // here, just return default length (= 1 'octave' = 3.5 line spaces)
      if (stemThrough())
            return STAFFTYPE_TAB_DEFAULTSTEMLEN_THRU * _lineDistance.val();
      // if stems beside staff, length is fixed, but take into account shorter half note stems
      else {
            bool shrt = (minimStyle() == TAB_MINIM_SHORTER) && (chord->durationType().type() == TDuration::V_HALF);
            stemLen = (stemsDown() ? STAFFTYPE_TAB_DEFAULTSTEMLEN_DN : STAFFTYPE_TAB_DEFAULTSTEMLEN_UP)
                        * (shrt ? STAFFTYPE_TAB_SHORTSTEMRATIO : 1.0);
            }
      return stemLen;
      }

//---------------------------------------------------------
//   fretString / durationString
//
//    construct the text string for a given fret / duration
//---------------------------------------------------------

static const QString unknownFret = QString("?");

QString StaffTypeTablature::fretString(int fret, bool ghost) const
      {
      if (fret == FRET_NONE)
            return unknownFret;
      if (ghost)
            return _fretFonts[_fretFontIdx].ghostChar;
      else {
            if (_useNumbers) {
                  if(fret >= NUM_OF_DIGITFRETS)
                        return unknownFret;
                  else
                        return _fretFonts[_fretFontIdx].displayDigit[fret];
                  }
           else {
                  if(fret >= NUM_OF_LETTERFRETS)
                        return unknownFret;
                  else
                        return _fretFonts[_fretFontIdx].displayLetter[fret];
                  }
           }
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
      setDuration(type, dots, tab);
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
      if(!_tab) {
            setbbox(QRectF());
            return;
            }
      QFontMetricsF fm(_tab->durationFont());
      qreal mags = magS();
      qreal w = fm.width(_text);
      qreal y = _tab->durationBoxY();
      // with rests, move symbol down by half its displacement from staff
      if(parent() && parent()->type() == REST)
            y += TAB_RESTSYMBDISPL * spatium();
      bbox().setRect(0.0, y * mags, w * mags, _tab->durationBoxH() * mags);
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
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

bool TablatureFretFont::read(XmlReader& e)
      {
      defPitch = 9.0;
      defYOffset = 0.0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            int val = e.intAttribute("value");

            if (tag == "family")
                  family = e.readElementText();
            else if (tag == "displayName")
                  displayName = e.readElementText();
            else if (tag == "defaultPitch")
                  defPitch = e.readDouble();
            else if (tag == "defaultYOffset")
                  defYOffset = e.readDouble();
            else if (tag == "mark") {
                  QString val = e.attribute("value");
                  QString txt(e.readElementText());
                  if (val.size() < 1)
                        return false;
                  if (val == "x")
                        xChar = txt[0];
                  else if (val == "ghost")
                        ghostChar = txt[0];
                  }
            else if (tag == "fret") {
                  bool bLetter = e.intAttribute("letter");
                  QString txt(e.readElementText());
                  if (bLetter) {
                        if (val >= 0 && val < NUM_OF_LETTERFRETS)
                              displayLetter[val] = txt[0];
                        }
                  else {
                        if (val >= 0 && val < NUM_OF_DIGITFRETS)
                              displayDigit[val] = txt;
                        }
                  }
            else {
                  e.unknown();
                  return false;
                  }
            }
      return true;
      }

bool TablatureDurationFont::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "family")
                  family = e.readElementText();
            else if (tag == "displayName")
                  displayName = e.readElementText();
            else if (tag == "defaultPitch")
                  defPitch = e.readDouble();
            else if (tag == "defaultYOffset")
                  defYOffset = e.readDouble();
            else if (tag == "duration") {
                  QString val = e.attribute("value");
                  QString txt(e.readElementText());
                  QChar chr = txt[0];
                  if (val == "longa")
                        displayValue[TAB_VAL_LONGA] = chr;
                  else if (val == "brevis")
                        displayValue[TAB_VAL_BREVIS] = chr;
                  else if (val == "semibrevis")
                        displayValue[TAB_VAL_SEMIBREVIS] = chr;
                  else if (val == "minima")
                        displayValue[TAB_VAL_MINIMA] = chr;
                  else if (val == "semiminima")
                        displayValue[TAB_VAL_SEMIMINIMA] = chr;
                  else if (val == "fusa")
                        displayValue[TAB_VAL_FUSA] = chr;
                  else if (val == "semifusa")
                        displayValue[TAB_VAL_SEMIFUSA] = chr;
                  else if (val == "32")
                        displayValue[TAB_VAL_32] = chr;
                  else if (val == "64")
                        displayValue[TAB_VAL_64] = chr;
                  else if (val == "128")
                        displayValue[TAB_VAL_128] = chr;
                  else if (val == "256")
                        displayValue[TAB_VAL_256] = chr;
                  else if (val == "dot")
                        displayDot = chr;
                  else
                        e.unknown();
                  }
            else {
                  e.unknown();
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
#ifdef Q_OS_IOS
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

      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "fretFont") {
                              TablatureFretFont f;
                              if (f.read(e))
                                    _fretFonts.append(f);
                              else
                                    continue;
                              }
                        else if (tag == "durationFont") {
                              TablatureDurationFont f;
                              if (f.read(e))
                                    _durationFonts.append(f);
                              else
                                    continue;
                              }
                        else
                              e.unknown();
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
            qreal * pSize, qreal* pYOff)
      {
      if (bDuration) {
            if (nIdx >= 0 && nIdx < _durationFonts.size()) {
                  TablatureDurationFont f = _durationFonts.at(nIdx);
                  if (pFamily)      *pFamily          = f.family;
                  if (pDisplayName) *pDisplayName     = f.displayName;
                  if (pSize)        *pSize            = f.defPitch;
                  if (pYOff)        *pYOff            = f.defYOffset;
                  return true;
                  }
            }
      else {
            if (nIdx >= 0 && nIdx < _fretFonts.size()) {
                  TablatureFretFont f = _fretFonts.at(nIdx);
                  if (pFamily)      *pFamily          = f.family;
                  if (pDisplayName) *pDisplayName     = f.displayName;
                  if (pSize)        *pSize            = f.defPitch;
                  if (pYOff)        *pYOff            = f.defYOffset;
                  return true;
                  }
            }
      return false;
      }

//=========================================================
//
//   BUILT-IN STAFF TYPES and STAFF TYPE PRESETS
//
//=========================================================

QList<StaffType*> staffTypes;

struct StaffTypePreset {
      QString     xmlName;                      // the name used to reference this preset in intruments.xml
      StaffType * staffType;                    // the actual StaffType settings
};

std::array<StaffTypePreset, STAFF_TYPES> _presets;

static const int _defaultPreset[STAFF_GROUP_MAX] =
      { 0,              // default pitched preset is "stdNormal"
        3,              // default percussion preset is "perc5lines"
        5               // default tab preset is "tab6StrCommon"
      };

static const QString _emptyString = QString();

//---------------------------------------------------------
//   Static functions for StaffType presets
//---------------------------------------------------------

size_t StaffType::numOfPresets()
      {
      return _presets.size();
      }

const StaffType* StaffType::preset(int idx)
      {
      if (idx < 0 || idx >= (int)_presets.size())
            return _presets[0].staffType;
      return _presets[idx].staffType;
      }

const StaffType* StaffType::presetFromXmlName(QString& xmlName, int* idx)
{
      for (int i = 0; i < (int)_presets.size(); ++i)
            if (_presets[i].xmlName == xmlName) {
                  if (idx)
                        *idx = i;
                  return _presets[i].staffType;
                  }
      return 0;
}

const StaffType* StaffType::presetFromName(QString& name, int* idx)
{
      for (int i = 0; i < (int)_presets.size(); ++i)
            if (_presets[i].staffType->name() == name) {
                  if (idx)
                        *idx = i;
                  return _presets[i].staffType;
                  }
      return 0;
}

const QString& StaffType::presetXmlName(int idx)
      {
      if (idx < 0 || idx >= (int)_presets.size())
            return _emptyString;
      return _presets[idx].xmlName;
      }

const QString& StaffType::presetName(int idx)
      {
      if (idx < 0 || idx >= (int)_presets.size())
            return _emptyString;
      return _presets[idx].staffType->_name;
      }

const StaffType* StaffType::getDefaultPreset(StaffGroup grp, int *idx)
      {
      int _idx = _defaultPreset[(int)grp];
      if (idx)
            *idx = _idx;
      return _presets[_idx].staffType;
      }

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

void initStaffTypes()
      {
      // init staff type presets
//                                                                                 human readable name  lin dst clef  bars stmless time  key  ledger
      _presets[STANDARD_STAFF_TYPE].staffType     = new StaffTypePitched   (QObject::tr("Standard"),      5, 1, true, true, false, true, true, true);
      _presets[PERC_1LINE_STAFF_TYPE].staffType   = new StaffTypePercussion(QObject::tr("Perc. 1 lines"), 1, 1, true, true, false, true, false, true);
      _presets[PERC_3LINE_STAFF_TYPE].staffType   = new StaffTypePercussion(QObject::tr("Perc. 3 lines"), 3, 1, true, true, false, true, false, true);
      _presets[PERC_5LINE_STAFF_TYPE].staffType   = new StaffTypePercussion(QObject::tr("Perc. 5 lines"), 5, 1, true, true, false, true, false, true);
//                                                                                     human-readable name  lin dist  clef   bars stemless time  duration font                 size off genDur fret font                       size off  thru  minim style       onLin  rests  stmDn  stmThr upsDn  nums
      _presets[TAB_6SIMPLE_STAFF_TYPE].staffType  = new StaffTypeTablature(QObject::tr("Tab. 6-str simple"), 6, 1.5, true,  true, true,  false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Sans"),     9, 0, false, TAB_MINIM_NONE,   true,  false, true,  false, false, true);
      _presets[TAB_6COMMON_STAFF_TYPE].staffType  = new StaffTypeTablature(QObject::tr("Tab. 6-str common"), 6, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),    9, 0, false, TAB_MINIM_SHORTER,true,  false, true,  false, false, true);
      _presets[TAB_6FULL_STAFF_TYPE].staffType    = new StaffTypeTablature(QObject::tr("Tab. 6-str full"),   6, 1.5, true,  true, false, true,  QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),    9, 0, false, TAB_MINIM_SLASHED,true,  true,  true,  true,  false, true);
      _presets[TAB_4SIMPLE_STAFF_TYPE].staffType  = new StaffTypeTablature(QObject::tr("Tab. 4-str simple"), 4, 1.5, true,  true, true,  false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Sans"),     9, 0, false, TAB_MINIM_NONE,   true,  false, true,  false, false, true);
      _presets[TAB_4COMMON_STAFF_TYPE].staffType  = new StaffTypeTablature(QObject::tr("Tab. 4-str common"), 4, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),    9, 0, false, TAB_MINIM_SHORTER,true,  false, true,  false, false, true);
      _presets[TAB_4FULL_STAFF_TYPE].staffType    = new StaffTypeTablature(QObject::tr("Tab. 4-str full"),   4, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),    9, 0, false, TAB_MINIM_SLASHED,true,  true,  true,  true,  false, true);
      _presets[TAB_UKULELE_STAFF_TYPE].staffType  = new StaffTypeTablature(QObject::tr("Tab. ukulele"),      4, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),    9, 0, false, TAB_MINIM_SHORTER,true,  true,  true,  false, false, true);
      _presets[TAB_BALALAJKA_STAFF_TYPE].staffType= new StaffTypeTablature(QObject::tr("Tab. balalajka"),    3, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),    9, 0, false, TAB_MINIM_SHORTER,true,  true,  true,  false, false, true);
//                                                                        (QObject::tr("Tab. bandurria"),    6, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Serif"),   10, 0, false, TAB_MINIM_SLASHED,true,  true,  true,  true,  false, true);
      _presets[TAB_ITALIAN_STAFF_TYPE].staffType  = new StaffTypeTablature(QObject::tr("Tab. 6-str Italian"),6, 1.5, false, true, true,  true,  QString("MuseScore Tab Italian"),15, 0, true,  QString("MuseScore Tab Renaiss"), 10, 0, true,  TAB_MINIM_NONE,   true,  true,  false, false, true,  true);
      _presets[TAB_FRENCH_STAFF_TYPE].staffType   = new StaffTypeTablature(QObject::tr("Tab. 6-str French"), 6, 1.5, false, true, true,  true,  QString("MuseScore Tab French"), 15, 0, true,  QString("MuseScore Tab Renaiss"), 10, 0, true,  TAB_MINIM_NONE,   false, false, false, false, false, false);

      _presets[STANDARD_STAFF_TYPE].xmlName     = QString("stdNormal");
      _presets[PERC_1LINE_STAFF_TYPE].xmlName   = QString("perc1Line");
      _presets[PERC_3LINE_STAFF_TYPE].xmlName   = QString("perc3Line");
      _presets[PERC_5LINE_STAFF_TYPE].xmlName   = QString("perc5Line");
      _presets[TAB_6SIMPLE_STAFF_TYPE].xmlName  = QString("tab6StrSimple");
      _presets[TAB_6COMMON_STAFF_TYPE].xmlName  = QString("tab6StrCommon");
      _presets[TAB_6FULL_STAFF_TYPE].xmlName    = QString("tab6StrFull");
      _presets[TAB_4SIMPLE_STAFF_TYPE].xmlName  = QString("tab4StrSimple");
      _presets[TAB_4COMMON_STAFF_TYPE].xmlName  = QString("tab4StrCommon");
      _presets[TAB_4FULL_STAFF_TYPE].xmlName    = QString("tab4StrFull");
      _presets[TAB_UKULELE_STAFF_TYPE].xmlName  = QString("tabUkulele");
      _presets[TAB_BALALAJKA_STAFF_TYPE].xmlName= QString("tabBalajka");
      _presets[TAB_ITALIAN_STAFF_TYPE].xmlName  = QString("tab6StrItalian");
      _presets[TAB_FRENCH_STAFF_TYPE].xmlName   = QString("tab6StrFrench");
      }

}                 // namespace Ms


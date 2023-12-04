//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/fraction.h"

#include "importmxmllogger.h"
#include "importmxmlnoteduration.h"

namespace Ms {

//---------------------------------------------------------
//   noteTypeToFraction
//---------------------------------------------------------

/**
 Convert MusicXML note type to fraction.
 */

static Fraction noteTypeToFraction(const QString& type)
      {
      if (type == "1024th")
            return Fraction(1, 1024);
      else if (type == "512th")
            return Fraction(1, 512);
      else if (type == "256th")
            return Fraction(1, 256);
      else if (type == "128th")
            return Fraction(1, 128);
      else if (type == "64th")
            return Fraction(1, 64);
      else if (type == "32nd")
            return Fraction(1, 32);
      else if (type == "16th")
            return Fraction(1, 16);
      else if (type == "eighth")
            return Fraction(1, 8);
      else if (type == "quarter")
            return Fraction(1, 4);
      else if (type == "half")
            return Fraction(1, 2);
      else if (type == "whole")
            return Fraction(1, 1);
      else if (type == "breve")
            return Fraction(2, 1);
      else if (type == "long")
            return Fraction(4, 1);
      else if (type == "maxima")
            return Fraction(8, 1);
      else
            return Fraction(0, 0);
      }

//---------------------------------------------------------
//   calculateFraction
//---------------------------------------------------------

/**
 Convert note type, number of dots and actual and normal notes into a duration
 */

static Fraction calculateFraction(const QString& type, const int dots, const Fraction timeMod)
      {
      // type
      Fraction f = noteTypeToFraction(type);
      if (f.isValid()) {
            // dot(s)
            Fraction ff = f;
            for (int i = 0; i < dots; ++i) {
                  ff = ff * Fraction(1,2);
                  f += ff;
                  }
            // tuplet
            if (timeMod.isValid())
                  f *= timeMod;
            // clean up (just in case)
            f.reduce();
            }
      return f;
      }

//---------------------------------------------------------
//   checkTiming
//---------------------------------------------------------

/**
 Do timing error checks.
 Return empty string if OK, message in case of error.
 */

QString mxmlNoteDuration::checkTiming(const QString& type, const bool rest, const bool grace)
      {
      //qDebug("type %s rest %d grace %d", qPrintable(type), rest, grace);
      QString errorStr;

      // normalize duration
      if (_specDura.isValid())
            _specDura.reduce();

      // default: use calculated duration
      _calcDura = calculateFraction(type, _dots, _timeMod);
      if (_calcDura.isValid()) {
            _dura = _calcDura;
            }

      if (_specDura.isValid() && _calcDura.isValid()) {
            if (_specDura != _calcDura) {
                  errorStr = QString("calculated duration (%1) not equal to specified duration (%2)")
                        .arg(_calcDura.print(), _specDura.print());
                  //qDebug("rest %d type '%s' timemod %s", rest, qPrintable(type), qPrintable(_timeMod.print()));

                  if (rest && type == "whole" && _specDura.isValid()) {
                        // Sibelius whole measure rest (not an error)
                        errorStr = "";
                        _dura = _specDura;
                        }
                  else if (grace && _specDura == Fraction(0, 1)) {
                        // grace note (not an error)
                        errorStr = "";
                        _dura = _specDura;
                        }
                  else {
                        if (qAbs(_calcDura.ticks() - _specDura.ticks()) <= _pass1->maxDiff()) {
                              errorStr += " -> assuming rounding error";
                              _pass1->insertAdjustedDuration(_dura, _calcDura);
                              _dura = _calcDura;
                              _specDura = _calcDura; // prevent changing off time
                              }
                        else
                              errorStr += " -> using calculated duration";
                        }

                  // Special case:
                  // Encore generates rests in tuplets w/o <tuplet> or <time-modification>.
                  // Detect this by comparing the actual duration with the expected duration
                  // based on note type. If actual is 2/3 of expected, the rest is part
                  // of a tuplet.
                  if (rest) {
                        if (2 * _calcDura.ticks() == 3 * _specDura.ticks()) {
                              _timeMod = Fraction(2, 3);
                              errorStr += errorStr.isEmpty() ? " ->" : ",";
                              errorStr += " assuming triplet";
                              _dura = _specDura;
                              }
                        }
                  }
            }
      else if (_specDura.isValid() && !_calcDura.isValid()) {
            // do not report an error for typeless (whole measure) rests
            if (!(rest && type == "")) {
                  errorStr = "calculated duration invalid, using specified duration";
                  }
            _dura = _specDura;
            }
      else if (!_specDura.isValid() && _calcDura.isValid()) {
            if (!grace) {
                  errorStr = "specified duration invalid, using calculated duration";
                  }
            }
      else {
            errorStr = "calculated and specified duration invalid, using 4/4";
            _dura = Fraction(4, 4);
            }

      _pass1->insertSeenDenominator(_dura.reduced().denominator());
      return errorStr;
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void mxmlNoteDuration::duration(QXmlStreamReader& e)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::duration", &e);

      _specDura.set(0, 0);        // invalid unless set correctly
      int intDura = e.readElementText().toInt();
      _specDura = _pass1->calcTicks(intDura, _divs, &e); // Duration reading (and rounding) code consolidated to pass1
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

/**
 Handle selected child elements of the /score-partwise/part/measure/note/duration node.
 Return true if handled.
 */

bool mxmlNoteDuration::readProperties(QXmlStreamReader& e)
      {
      const QStringRef& tag(e.name());
      //qDebug("tag %s", qPrintable(tag.toString()));
      if (tag == "dot") {
            _dots++;
            e.skipCurrentElement();  // skip but don't log
            return true;
            }
      else if (tag == "duration") {
            duration(e);
            return true;
            }
      else if (tag == "time-modification") {
            timeModification(e);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   timeModification
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/time-modification node.
 */

void mxmlNoteDuration::timeModification(QXmlStreamReader& e)
      {
      _logger->logDebugTrace("MusicXMLParserPass1::timeModification", &e);

      int intActual = 0;
      int intNormal = 0;
      QString strActual;
      QString strNormal;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "actual-notes")
                  strActual = e.readElementText();
            else if (tag == "normal-notes")
                  strNormal = e.readElementText();
            else if (tag == "normal-type") {
                  // "measure" is not a valid normal-type,
                  // but would be accepted by setType()
                  QString strNormalType = e.readElementText();
                  if (strNormalType != "measure")
                        _normalType.setType(strNormalType);
                  }
            else {
                  _logger->logDebugInfo(QString("skipping '%1'").arg(e.name().toString()), &e);
                  e.skipCurrentElement();
                  }
            }

      intActual = strActual.toInt();
      intNormal = strNormal.toInt();
      if (intActual > 0 && intNormal > 0)
            _timeMod.set(intNormal, intActual);
      else {
            _timeMod.set(1, 1);
            _logger->logError(QString("illegal time-modification: actual-notes %1 normal-notes %2")
                              .arg(strActual, strNormal), &e);
            }
      }

}

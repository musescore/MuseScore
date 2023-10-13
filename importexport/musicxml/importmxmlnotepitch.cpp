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

#include "importmxmllogger.h"
#include "importmxmlnotepitch.h"
#include "musicxmlsupport.h"

namespace Ms {

//---------------------------------------------------------
//   accidental
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/accidental node.
 Return the result as an Accidental.
 */

// TODO: split in reading parameters versus creation

static Accidental* accidental(QXmlStreamReader& e, Score* score)
      {
      const bool cautionary = e.attributes().value("cautionary") == "yes";
      const bool editorial = e.attributes().value("editorial") == "yes";
      const bool parentheses = e.attributes().value("parentheses") == "yes";
      const bool bracket = e.attributes().value("bracket") == "yes";
      const QColor accColor = e.attributes().value("color").toString();
      QString smufl = e.attributes().value("smufl").toString();

      const auto s = e.readElementText();
      const auto type = mxmlString2accidentalType(s, smufl);

      if (type != AccidentalType::NONE) {
            auto a = new Accidental(score);
            a->setAccidentalType(type);
            if (cautionary || parentheses) {
                  a->setBracket(AccidentalBracket(AccidentalBracket::PARENTHESIS));
                  a->setRole(AccidentalRole::USER);
                  }
            else if (editorial || bracket) {
                  a->setBracket(AccidentalBracket(AccidentalBracket::BRACKET));
                  a->setRole(AccidentalRole::USER);
                  }
            if (accColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  a->setColor(accColor);
            return a;
            }

      return 0;
      }

//---------------------------------------------------------
//   displayStepOctave
//---------------------------------------------------------

/**
 Handle <display-step> and <display-octave> for <rest> and <unpitched>
 */

void mxmlNotePitch::displayStepOctave(QXmlStreamReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "display-step") {
                  const auto step = e.readElementText();
                  int pos = QString("CDEFGAB").indexOf(step);
                  if (step.size() == 1 && pos >=0 && pos < 7)
                        _displayStep = pos;
                  else
                        //logError(QString("invalid step '%1'").arg(strStep));
                        qDebug("invalid step '%s'", qPrintable(step));        // TODO
                  }
            else if (e.name() == "display-octave") {
                  const auto oct = e.readElementText();
                  bool ok;
                  _displayOctave = oct.toInt(&ok);
                  if (!ok || _displayOctave < 0 || _displayOctave > 9) {
                        //logError(QString("invalid octave '%1'").arg(strOct));
                        qDebug("invalid octave '%s'", qPrintable(oct));       // TODO
                        _displayOctave = -1;
                        }
                  }
            else
                  e.skipCurrentElement();                   // TODO log
            }
      }

//---------------------------------------------------------
//   pitch
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/pitch node.
 */

void mxmlNotePitch::pitch(QXmlStreamReader& e)
      {
      // defaults
      _step = -1;
      _alter = 0;
      _octave = -1;

      while (e.readNextStartElement()) {
            if (e.name() == "alter") {
                  const auto alter = e.readElementText();
                  bool ok;
                  _alter = MxmlSupport::stringToInt(alter, &ok);       // fractions not supported by mscore
                  if (!ok || _alter < -2 || _alter > 2) {
                        _logger->logError(QString("invalid alter '%1'").arg(alter), &e);
                        bool ok2;
                        const auto altervalue = alter.toDouble(&ok2);
                        if (ok2 && (qAbs(altervalue) < 2.0) && (_accType == AccidentalType::NONE)) {
                              // try to see if a microtonal accidental is needed
                              _accType = microtonalGuess(altervalue);
                              }
                        _alter = 0;
                        }
                  }
            else if (e.name() == "octave") {
                  const auto oct = e.readElementText();
                  bool ok;
                  _octave = oct.toInt(&ok);
                  if (!ok || _octave < 0 || _octave > 9) {
                        _logger->logError(QString("invalid octave '%1'").arg(oct), &e);
                        _octave = -1;
                        }
                  }
            else if (e.name() == "step") {
                  const auto step = e.readElementText();
                  const auto pos = QString("CDEFGAB").indexOf(step);
                  if (step.size() == 1 && pos >=0 && pos < 7)
                        _step = pos;
                  else
                        _logger->logError(QString("invalid step '%1'").arg(step), &e);
                  }
            else {
                  ;       // TODO skipLogCurrElem();
                  }
            }
      //qDebug("pitch step %d alter %d oct %d accid %hhd", step, alter, oct, accid);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

/**
 Handle selected child elements of the /score-partwise/part/measure/note node.
 Return true if handled.
 */

bool mxmlNotePitch::readProperties(QXmlStreamReader& e, Score* score)
      {
      const QStringRef& tag(e.name());

      if (tag == "accidental") {
            _acc = accidental(e, score);
            return true;
            }
      else if (tag == "unpitched") {
            _unpitched = true;
            displayStepOctave(e);
            return true;
            }
      else if (tag == "pitch") {
            pitch(e);
            return true;
            }
      return false;
      }

}

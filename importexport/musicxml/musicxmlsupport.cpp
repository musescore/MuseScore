//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

/**
 MusicXML support.
 */

#include "musicxmlsupport.h"

#include "libmscore/sym.h"
#include "libmscore/accidental.h"

namespace Ms {

NoteList::NoteList()
      {
      for (int i = 0; i < MAX_STAVES; ++i)
            _staffNoteLists << StartStopList();
      }

void NoteList::addNote(const int startTick, const int endTick, const int staff)
      {
      if (staff >= 0 && staff < _staffNoteLists.size())
            _staffNoteLists[staff] << StartStop(startTick, endTick);
      }

void NoteList::dump(const QString& voice) const
      {
      // dump contents
      for (int i = 0; i < MAX_STAVES; ++i) {
            printf("voice %s staff %d:", qPrintable(voice), i);
            for (int j = 0; j < _staffNoteLists.at(i).size(); ++j)
                  printf(" %d-%d", _staffNoteLists.at(i).at(j).first, _staffNoteLists.at(i).at(j).second);
            printf("\n");
            }
      // show overlap
      printf("overlap voice %s:", qPrintable(voice));
      for (int i = 0; i < MAX_STAVES - 1; ++i)
            for (int j = i + 1; j < MAX_STAVES; ++j)
                  stavesOverlap(i, j);
      printf("\n");
      }

/**
 Determine if notes n1 and n2 overlap.
 This is NOT the case if
 - n1 starts when or after n2 stops
 - or n2 starts when or after n1 stops
 */

static bool notesOverlap(const StartStop& n1, const StartStop& n2)
      {
      return !(n1.first >= n2.second || n1.second <= n2.first);
      }

/**
 Determine if any note in staff1 and staff2 overlaps.
 */

bool NoteList::stavesOverlap(const int staff1, const int staff2) const
      {
      for (int i = 0; i < _staffNoteLists.at(staff1).size(); ++i)
            for (int j = 0; j < _staffNoteLists.at(staff2).size(); ++j)
                  if (notesOverlap(_staffNoteLists.at(staff1).at(i), _staffNoteLists.at(staff2).at(j))) {
//printf(" %d-%d", staff1, staff2);
                        return true;
                        }
      return false;
      }

/**
 Determine if any note in any staff overlaps.
 */

bool NoteList::anyStaffOverlaps() const
      {
      for (int i = 0; i < MAX_STAVES - 1; ++i)
            for (int j = i + 1; j < MAX_STAVES; ++j)
                  if (stavesOverlap(i, j))
                        return true;
      return false;
      }

VoiceOverlapDetector::VoiceOverlapDetector()
      {
      // qDebug("VoiceOverlapDetector::VoiceOverlapDetector(staves %d)", MAX_STAVES);
      }

void VoiceOverlapDetector::addNote(const int startTick, const int endTick, const QString& voice, const int staff)
      {
      // if necessary, create the note list for voice
      if (!_noteLists.contains(voice))
            _noteLists.insert(voice, NoteList());
      _noteLists[voice].addNote(startTick, endTick, staff);
      }

void VoiceOverlapDetector::dump() const
      {
      // qDebug("VoiceOverlapDetector::dump()");
      QMapIterator<QString, NoteList> i(_noteLists);
      while (i.hasNext()) {
            i.next();
            i.value().dump(i.key());
            }
      }

void VoiceOverlapDetector::newMeasure()
      {
      // qDebug("VoiceOverlapDetector::newMeasure()");
      _noteLists.clear();
      }

bool VoiceOverlapDetector::stavesOverlap(const QString& voice) const
      {
      if (_noteLists.contains(voice))
            return _noteLists.value(voice).anyStaffOverlaps();
      else
            return false;
      }

QString MusicXMLDrumInstrument::toString() const
      {
      return QString("chan %1 prog %2 vol %3 pan %4 pitch %5 name '%6' sound '%7' head %8 line %9 stemDir %10")
             .arg(midiChannel)
             .arg(midiProgram)
             .arg(midiVolume)
             .arg(midiPan)
             .arg(pitch)
             .arg(name)
             .arg(sound)
             .arg(int(notehead))
             .arg(line)
             .arg(int(stemDirection));
      }

void ValidatorMessageHandler::handleMessage(QtMsgType type, const QString& description,
                                            const QUrl& /* identifier */, const QSourceLocation& sourceLocation)
      {
      // convert description from html to text
      QDomDocument desc;
      QString contentError;
      int contentLine;
      int contentColumn;
      if (!desc.setContent(description, false, &contentError, &contentLine,
                           &contentColumn)) {
            qDebug("ValidatorMessageHandler: could not parse validation error line %d column %d: %s",
                   contentLine, contentColumn, qPrintable(contentError));
            return;
            }
      QDomElement e = desc.documentElement();
      if (e.tagName() != "html") {
            qDebug("ValidatorMessageHandler: description is not html");
            return;
            }
      QString descText = e.text();

      QString strType;
      switch (type) {
            case 0:  strType = tr("Debug"); break;
            case 1:  strType = tr("Warning"); break;
            case 2:  strType = tr("Critical"); break;
            case 3:  strType = tr("Fatal"); break;
            default: strType = tr("Unknown"); break;
            }

      QString errorStr = QString(tr("%1 error: line %2 column %3 %4"))
            .arg(strType)
            .arg(sourceLocation.line())
            .arg(sourceLocation.column())
            .arg(descText);

      // append error, separated by newline if necessary
      if (errors != "")
            errors += "\n";
      errors += errorStr;
      }

//---------------------------------------------------------
//   printDomElementPath
//---------------------------------------------------------

static QString domElementPath(const QDomElement& e)
      {
      QString s;
      QDomNode dn(e);
      while (!dn.parentNode().isNull()) {
            dn = dn.parentNode();
            const QDomElement& de = dn.toElement();
            const QString k(de.tagName());
            if (!s.isEmpty())
                  s += ":";
            s += k;
            }
      return s;
      }

//---------------------------------------------------------
//   domError
//---------------------------------------------------------

void domError(const QDomElement& e)
      {
      QString m;
      QString s = domElementPath(e);
//      if (!docName.isEmpty())
//            m = QString("<%1>:").arg(docName);
      int ln = e.lineNumber();
      if (ln != -1)
            m += QString("line:%1 ").arg(ln);
      int col = e.columnNumber();
      if (col != -1)
            m += QString("col:%1 ").arg(col);
      m += QString("%1: Unknown Node <%2>, type %3").arg(s).arg(e.tagName()).arg(e.nodeType());
      if (e.isText())
            m += QString("  text node <%1>").arg(e.toText().data());
      qDebug("%s", qPrintable(m));
      }

//---------------------------------------------------------
//   domNotImplemented
//---------------------------------------------------------

void domNotImplemented(const QDomElement& e)
      {
      if (!MScore::debugMode)
            return;
      QString s = domElementPath(e);
//      if (!docName.isEmpty())
//            qDebug("<%s>:", qPrintable(docName));
      qDebug("%s: Node not implemented: <%s>, type %d",
             qPrintable(s), qPrintable(e.tagName()), e.nodeType());
      if (e.isText())
            qDebug("  text node <%s>", qPrintable(e.toText().data()));
      }


//---------------------------------------------------------
//   stringToInt
//---------------------------------------------------------

/**
 Convert a string in \a s into an int. Set *ok to true iff conversion was
 successful. \a s may end with ".0", as is generated by Audiveris 3.2 and up,
 in elements <divisions>, <duration>, <alter> and <sound> attributes
 dynamics and tempo.
 In case of error val return a default value of 0.
 Note that non-integer values cannot be handled by mscore.
 */

int MxmlSupport::stringToInt(const QString& s, bool* ok)
      {
      int res = 0;
      QString str = s;
      if (s.endsWith(".0"))
            str = s.left(s.size() - 2);
      res = str.toInt(ok);
      return res;
      }

//---------------------------------------------------------
//   durationAsFraction
//---------------------------------------------------------

/**
 Return duration specified in the element e as Fraction.
 Caller must ensure divisions is valid.
 */

Fraction MxmlSupport::durationAsFraction(const int divisions, const QDomElement e)
      {
      Fraction f;
      if (e.tagName() == "duration") {
            bool ok;
            int val = MxmlSupport::stringToInt(e.text(), &ok);
            f = Fraction(val, 4 * divisions); // note divisions = ticks / quarter note
            f.reduce();
            }
      else {
            qDebug() << "durationAsFraction tagname error" << f.print();
            }
      return f;
      }

//---------------------------------------------------------
//   noteTypeToFraction
//---------------------------------------------------------

/**
 Convert MusicXML note type to fraction.
 */

Fraction MxmlSupport::noteTypeToFraction(QString type)
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

Fraction MxmlSupport::calculateFraction(QString type, int dots, int normalNotes, int actualNotes)
      {
      // type
      Fraction f = MxmlSupport::noteTypeToFraction(type);
      if (f.isValid()) {
            // dot(s)
            Fraction f_no_dots = f;
            for (int i = 0; i < dots; ++i)
                  f += (f_no_dots / Fraction(2 << i, 1));
            // tuplet
            if (actualNotes > 0 && normalNotes > 0) {
                  f *= normalNotes;
                  f /= Fraction(actualNotes,1);
                  }
            // clean up (just in case)
            f.reduce();
            }
      return f;
      }

//---------------------------------------------------------
//   accSymId2MxmlString
//---------------------------------------------------------

QString accSymId2MxmlString(const SymId id)
      {
      QString s;
      switch (id) {
            case SymId::accidentalSharp:                 s = "sharp";                break;
            case SymId::accidentalNatural:               s = "natural";              break;
            case SymId::accidentalFlat:                  s = "flat";                 break;
            case SymId::accidentalDoubleSharp:           s = "double-sharp";         break;
            //case SymId::accidentalDoubleSharp:           s = "sharp-sharp";          break; // see abobe
            //case SymId::accidentalDoubleFlat:            s = "double-flat";          break; // doesn't exist in MusicXML, but see below
            case SymId::accidentalDoubleFlat:            s = "flat-flat";            break;
            case SymId::accidentalNaturalSharp:          s = "natural-sharp";        break;
            case SymId::accidentalNaturalFlat:           s = "natural-flat";         break;

            case SymId::accidentalQuarterToneFlatStein:  s = "quarter-flat";         break;
            case SymId::accidentalQuarterToneSharpStein: s = "quarter-sharp";        break;
            case SymId::accidentalThreeQuarterTonesFlatZimmermann: s = "three-quarters-flat";  break;
            //case SymId::noSym:                                     s = "three-quarters-flat";  break; // AccidentalType::FLAT_FLAT_SLASH, MuseScore 1?
            case SymId::accidentalThreeQuarterTonesSharpStein:     s = "three-quarters-sharp"; break;
            case SymId::accidentalQuarterToneSharpArrowDown:       s = "sharp-down";           break;
            case SymId::accidentalThreeQuarterTonesSharpArrowUp:   s = "sharp-up";             break;
            case SymId::accidentalQuarterToneFlatNaturalArrowDown: s = "natural-down";         break;
            case SymId::accidentalQuarterToneSharpNaturalArrowUp:  s = "natural-up";           break;
            case SymId::accidentalThreeQuarterTonesFlatArrowDown:  s = "flat-down";            break;
            case SymId::accidentalQuarterToneFlatArrowUp:          s = "flat-up";              break;
            case SymId::accidentalThreeQuarterTonesSharpArrowDown: s = "double-sharp-down";    break;
            case SymId::accidentalFiveQuarterTonesSharpArrowUp:    s = "double-sharp-up";      break;
            case SymId::accidentalFiveQuarterTonesFlatArrowDown:   s = "flat-flat-down";       break;
            case SymId::accidentalThreeQuarterTonesFlatArrowUp:    s = "flat-flat-up";         break;

            case SymId::accidentalArrowDown:             s = "arrow-down";           break;
            case SymId::accidentalArrowUp:               s = "arrow-up";             break;

            //case SymId::accidentalTripleSharp:           s = "triple-sharp";         break;
            //case SymId::accidentalTripleFlat:            s = "triple-flat";          break;

            case SymId::accidentalKucukMucennebSharp:    s = "slash-quarter-sharp";  break;
            case SymId::accidentalBuyukMucennebSharp:    s = "slash-sharp";          break;
            case SymId::accidentalBakiyeFlat:            s = "slash-flat";           break;
            case SymId::accidentalBuyukMucennebFlat:     s = "double-slash-flat";    break;

            //case SymId::noSym:                           s = "sharp1";               break;
            //case SymId::noSym:                           s = "sharp2";               break;
            //case SymId::noSym:                           s = "sharp3";               break;
            //case SymId::noSym:                           s = "sharp4";               break;
            //case SymId::noSym:                           s = "flat1";                break;
            //case SymId::noSym:                           s = "flat2";                break;
            //case SymId::noSym:                           s = "flat3";                break;
            //case SymId::noSym:                           s = "flat4";                break;

            case SymId::accidentalSori:                  s = "sori";                 break;
            case SymId::accidentalKoron:                 s = "koron";                break;
            default:
                  //s = "other"; // actually pick up the SMuFL name or SymId
                  qDebug("accSymId2MxmlString: unknown accidental %d", static_cast<int>(id));
            }
      return s;
      }

//---------------------------------------------------------
//   mxmlString2accSymId
// see https://github.com/w3c/musicxml/blob/6e3a667b85855b04d7e4548ea508b537bc29fc52/schema/musicxml.xsd#L1392-L1439
//---------------------------------------------------------

SymId mxmlString2accSymId(const QString mxmlName)
      {
      QMap<QString, SymId> map; // map MusicXML accidental name to MuseScore enum SymId
      map["sharp"] = SymId::accidentalSharp;
      map["natural"] = SymId::accidentalNatural;
      map["flat"] = SymId::accidentalFlat;
      map["double-sharp"] = SymId::accidentalDoubleSharp;
      map["sharp-sharp"] = SymId::accidentalDoubleSharp;
      //map["double-flat"] = SymId::accidentalDoubleFlat; // shouldn't harm, but doesn't exist in MusicXML
      map["flat-flat"] = SymId::accidentalDoubleFlat;
      map["natural-sharp"] = SymId::accidentalNaturalSharp;
      map["natural-flat"] = SymId::accidentalNaturalFlat;

      map["quarter-flat"] = SymId::accidentalQuarterToneFlatStein;
      map["quarter-sharp"] = SymId::accidentalQuarterToneSharpStein;
      map["three-quarters-flat"] = SymId::accidentalThreeQuarterTonesFlatZimmermann;
      map["three-quarters-sharp"] = SymId::accidentalThreeQuarterTonesSharpStein;

      map["sharp-down"] = SymId::accidentalQuarterToneSharpArrowDown;
      map["sharp-up"] = SymId::accidentalThreeQuarterTonesSharpArrowUp;
      map["natural-down"] = SymId::accidentalQuarterToneFlatNaturalArrowDown;
      map["natural-up"] = SymId::accidentalQuarterToneSharpNaturalArrowUp;
      map["flat-down"] = SymId::accidentalThreeQuarterTonesFlatArrowDown;
      map["flat-up"] = SymId::accidentalQuarterToneFlatArrowUp;
      map["double-sharp-down"] = SymId::accidentalThreeQuarterTonesSharpArrowDown;
      map["double-sharp-up"] = SymId::accidentalFiveQuarterTonesSharpArrowUp;
      map["flat-flat-down"] = SymId::accidentalFiveQuarterTonesFlatArrowDown;
      map["flat-flat-up"] = SymId::accidentalThreeQuarterTonesFlatArrowUp;

      map["arrow-down"] = SymId::accidentalArrowDown;
      map["arrow-up"] = SymId::accidentalArrowUp;

      //map["triple-sharp"] = SymId::accidentalTripleSharp;
      //map["triple-flat"] = SymId::accidentalTripleFlat;

      map["slash-quarter-sharp"] = SymId::accidentalKucukMucennebSharp;
      map["slash-sharp"] = SymId::accidentalBuyukMucennebSharp;
      map["slash-flat"] = SymId::accidentalBakiyeFlat;
      map["double-slash-flat"] = SymId::accidentalBuyukMucennebFlat;

      //map["sharp1"] = SymId::noSym;
      //map["sharp2"] = SymId::noSym;
      //map["sharp3"] = SymId::noSym;
      //map["sharp4"] = SymId::noSym;
      //map["flat1"] = SymId::noSym;
      //map["flat2"] = SymId::noSym;
      //map["flat3"] = SymId::noSym;
      //map["flat3"] = SymId::noSym;

      map["sori"] = SymId::accidentalSori;
      map["koron"] = SymId::accidentalKoron;

      //map["other"] = SymId::noSym; // actually pick up the SMuFL name or SymId

      if (map.contains(mxmlName))
            return map.value(mxmlName);
      else
            qDebug("mxmlString2accSymId: unknown accidental '%s'", qPrintable(mxmlName));

      // default
      return SymId::noSym;
      }

//---------------------------------------------------------
//   accidentalType2MxmlString
//---------------------------------------------------------

QString accidentalType2MxmlString(const AccidentalType type)
      {
      QString s;
      switch (type) {
            case AccidentalType::SHARP:              s = "sharp";                break;
            case AccidentalType::NATURAL:            s = "natural";              break;
            case AccidentalType::FLAT:               s = "flat";                 break;
            case AccidentalType::SHARP2:             s = "double-sharp";         break;
            //case AccidentalType::SHARP2:             s = "sharp-sharp";          break; // see abobe
            //case AccidentalType::FLAT2:              s = "double-flat";          break; // doesn't exist in MusicXML, but see below
            case AccidentalType::FLAT2:              s = "flat-flat";            break;
            case AccidentalType::NATURAL_SHARP:      s = "natural-sharp";        break;
            case AccidentalType::NATURAL_FLAT:       s = "natural-flat";         break;
            case AccidentalType::SHARP_ARROW_UP:     s = "sharp-up";             break;

            case AccidentalType::MIRRORED_FLAT:      s = "quarter-flat";         break;
            case AccidentalType::SHARP_SLASH:        s = "quarter-sharp";        break;
            case AccidentalType::MIRRORED_FLAT2:     s = "three-quarters-flat";  break;
            //case AccidentalType::FLAT_FLAT_SLASH:    s = "three-quarters-flat";  break; // MuseScore 1?
            case AccidentalType::SHARP_SLASH4:       s = "three-quarters-sharp"; break;
            case AccidentalType::SHARP_ARROW_DOWN:   s = "sharp-down";           break;
            case AccidentalType::NATURAL_ARROW_UP:   s = "natural-up";           break;
            case AccidentalType::NATURAL_ARROW_DOWN: s = "natural-down";         break;
            case AccidentalType::FLAT_ARROW_DOWN:    s = "flat-down";            break;
            case AccidentalType::FLAT_ARROW_UP:      s = "flat-up";              break;
            case AccidentalType::SHARP2_ARROW_DOWN:  s = "double-sharp-down";    break;
            case AccidentalType::SHARP2_ARROW_UP:    s = "double-sharp-up";      break;
            case AccidentalType::FLAT2_ARROW_DOWN:   s = "flat-flat-down";       break;
            case AccidentalType::FLAT2_ARROW_UP:     s = "flat-flat-up";         break;

            case AccidentalType::ARROW_DOWN:         s = "arrow-down";           break;
            case AccidentalType::ARROW_UP:           s = "arrow-up";             break;

            //case AccidentalType::SHARP3:             s = "triple-sharp";         break;
            //case AccidentalType::FLAT3:              s = "triple-flat";          break;

            case AccidentalType::SHARP_SLASH3:       s = "slash-quarter-sharp";  break;
            case AccidentalType::SHARP_SLASH2:       s = "slash-sharp";          break;
            case AccidentalType::FLAT_SLASH:         s = "slash-flat";           break;
            case AccidentalType::FLAT_SLASH2:        s = "double-slash-flat";    break;

            //case AccidentalType::NONE:               s = "sharp1";               break;
            //case AccidentalType::NONE:               s = "sharp2";               break;
            //case AccidentalType::NONE:               s = "sharp3";               break;
            //case AccidentalType::NONE:               s = "sharp4";               break;
            //case AccidentalType::NONE:               s = "flat1";                break;
            //case AccidentalType::NONE:               s = "flat2";                break;
            //case AccidentalType::NONE:               s = "flat3";                break;
            //case AccidentalType::NONE:               s = "flat3";                break;

            case AccidentalType::SORI:               s = "sori";                 break;
            case AccidentalType::KORON:              s = "koron";                break;
            default:
                  //s = "other"; // actually pick up the SMuFL name or SymId
                  qDebug("accidentalType2MxmlString: unknown accidental %d", static_cast<int>(type));
            }
      return s;
      }

//---------------------------------------------------------
//   mxmlString2accidentalType
//---------------------------------------------------------

/**
 Convert a MusicXML accidental name to a MuseScore enum AccidentalType.
 see https://github.com/w3c/musicxml/blob/6e3a667b85855b04d7e4548ea508b537bc29fc52/schema/musicxml.xsd#L1392-L1439
 */

AccidentalType mxmlString2accidentalType(const QString mxmlName)
      {
      QMap<QString, AccidentalType> map; // map MusicXML accidental name to MuseScore enum AccidentalType
      map["sharp"] = AccidentalType::SHARP;
      map["natural"] = AccidentalType::NATURAL;
      map["flat"] = AccidentalType::FLAT;
      map["double-sharp"] = AccidentalType::SHARP2;
      map["sharp-sharp"] = AccidentalType::SHARP2;
      //map["double-flat"] = AccidentalType::FLAT2; // shouldn't harm, but doesn't exist in MusicXML
      map["flat-flat"] = AccidentalType::FLAT2;
      map["natural-sharp"] = AccidentalType::SHARP;
      map["natural-flat"] = AccidentalType::FLAT;

      map["quarter-flat"] = AccidentalType::MIRRORED_FLAT;
      map["quarter-sharp"] = AccidentalType::SHARP_SLASH;
      map["three-quarters-flat"] = AccidentalType::MIRRORED_FLAT2;
      map["three-quarters-sharp"] = AccidentalType::SHARP_SLASH4;

      map["sharp-up"] = AccidentalType::SHARP_ARROW_UP;
      map["natural-down"] = AccidentalType::NATURAL_ARROW_DOWN;
      map["natural-up"] = AccidentalType::NATURAL_ARROW_UP;
      map["sharp-down"] = AccidentalType::SHARP_ARROW_DOWN;
      map["flat-down"] = AccidentalType::FLAT_ARROW_DOWN;
      map["flat-up"] = AccidentalType::FLAT_ARROW_UP;
      map["double-sharp-down"] = AccidentalType::SHARP2_ARROW_DOWN;
      map["double-sharp-up"] = AccidentalType::SHARP2_ARROW_UP;
      map["flat-flat-down"] = AccidentalType::FLAT2_ARROW_DOWN;
      map["flat-flat-up"] = AccidentalType::FLAT2_ARROW_UP;

      map["arrow-down"] = AccidentalType::ARROW_DOWN;
      map["arrow-up"] = AccidentalType::ARROW_UP;

      //map["triple-sharp"] = AccidentalType::SHARP3;
      //map["triple-flat"] = AccidentalType::FLAT3;

      map["slash-quarter-sharp"] = AccidentalType::SHARP_SLASH3; // MIRRORED_FLAT_SLASH; ?
      map["slash-sharp"] = AccidentalType::SHARP_SLASH2; // SHARP_SLASH; ?
      map["slash-flat"] = AccidentalType::FLAT_SLASH;
      map["double-slash-flat"] = AccidentalType::FLAT_SLASH2;

      //map["sharp1"] = AccidentalType::NONE;
      //map["sharp2"] = AccidentalType::NONE;
      //map["sharp3"] = AccidentalType::NONE;
      //map["sharp4"] = AccidentalType::NONE;
      //map["flat1"] = AccidentalType::NONE;
      //map["flat2"] = AccidentalType::NONE;
      //map["flat3"] = AccidentalType::NONE;
      //map["flat4"] = AccidentalType::NONE;

      map["sori"] = AccidentalType::SORI;
      map["koron"] = AccidentalType::KORON;

      //map["other"] = AccidentalType::NONE; // actually pick up the SMuFL name or SymId

      if (map.contains(mxmlName))
            return map.value(mxmlName);
      else
            qDebug("mxmlString2accidentalType: unknown accidental '%s'", qPrintable(mxmlName));
      return AccidentalType::NONE;
      }

//---------------------------------------------------------
//   isAppr
//---------------------------------------------------------

/**
 Check if v approximately equals ref.
 Used to prevent floating point comparison for equality from failing
 */

static bool isAppr(const double v, const double ref, const double epsilon)
      {
      return v > ref - epsilon && v < ref + epsilon;
      }

//---------------------------------------------------------
//   microtonalGuess
//---------------------------------------------------------

/**
 Convert a MusicXML alter tag into a microtonal accidental in MuseScore enum AccidentalType.
 Works only for quarter tone, half tone, three-quarters tone and whole tone accidentals.
 */

AccidentalType microtonalGuess(double val)
      {
      const double eps = 0.001;
      if (isAppr(val, -2, eps))
            return AccidentalType::FLAT2;
      else if (isAppr(val, -1.5, eps))
            return AccidentalType::MIRRORED_FLAT2;
      else if (isAppr(val, -1, eps))
            return AccidentalType::FLAT;
      else if (isAppr(val, -0.5, eps))
            return AccidentalType::MIRRORED_FLAT;
      else if (isAppr(val, 0, eps))
            return AccidentalType::NATURAL;
      else if (isAppr(val, 0.5, eps))
            return AccidentalType::SHARP_SLASH;
      else if (isAppr(val, 1, eps))
            return AccidentalType::SHARP;
      else if (isAppr(val, 1.5, eps))
            return AccidentalType::SHARP_SLASH4;
      else if (isAppr(val, 2, eps))
            return AccidentalType::SHARP2;
      else
            qDebug("Guess for microtonal accidental corresponding to value %f failed.", val);        // TODO

      // default
      return AccidentalType::NONE;
      }

}

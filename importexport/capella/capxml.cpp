//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2013 Werner Schweer and others
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

//    CapXML import filter
//    Supports the CapXML 1.0 file format version 1.0.8 (capella 2008)
//    The implementation shares as much code as possible with the capella
//    (.cap) importer (capella.cpp and capella.h)
//    If statements in the parser match the element order in the schema definition

#include <assert.h>
#include "libmscore/score.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "capella.h"

namespace Ms {

//---------------------------------------------------------
//   capxReadFont
//---------------------------------------------------------

static QFont capxReadFont(XmlReader& e)
      {
      QFont f;
      QString family = e.attribute("face");
      if (family != "")
            f.setFamily(family);
      qreal pointSize = e.doubleAttribute("height", 0);
      if (pointSize > 0.5)
            f.setPointSizeF(pointSize);
      int weight = e.intAttribute("weight");
      if (weight < 200)
            f.setWeight(QFont::Light);
      else if (weight < 550)
            f.setWeight(QFont::Normal);
      else
            f.setWeight(QFont::Bold);
      f.setItalic(e.attribute("italic", "false") == "true");
      // qDebug("capxReadFont family '%s' ps %g w %d it '%s'", qPrintable(family), pointSize, weight, qPrintable(italic));
      e.readNext();
      return f;
      }

//---------------------------------------------------------
//   qstring2timestep -- convert string to TIMESTEP
//---------------------------------------------------------

static bool qstring2timestep(QString& str, TIMESTEP& tstp)
      {
      if      (str == "1/1")   { tstp = TIMESTEP::D1;      return true; }
      else if (str == "1/2")   { tstp = TIMESTEP::D2;      return true; }
      else if (str == "1/4")   { tstp = TIMESTEP::D4;      return true; }
      else if (str == "1/8")   { tstp = TIMESTEP::D8;      return true; }
      else if (str == "1/16")  { tstp = TIMESTEP::D16;     return true; }
      else if (str == "1/32")  { tstp = TIMESTEP::D32;     return true; }
      else if (str == "1/64")  { tstp = TIMESTEP::D64;     return true; }
      else if (str == "1/128") { tstp = TIMESTEP::D128;    return true; }
      else if (str == "2/1")   { tstp = TIMESTEP::D_BREVE; return true; }
      return false;
      }

//---------------------------------------------------------
//   BasicDrawObj::readCapx -- capx equivalent of BasicDrawObj::read
//---------------------------------------------------------

void BasicDrawObj::readCapx(XmlReader& e)
      {
      nNotes = e.intAttribute("noteRange", 0);
      qDebug("nNotes %d", nNotes);
      e.readNext();
      }

//---------------------------------------------------------
//   BasicDurationalObj::readCapxDisplay -- capx equivalent of BasicDurationalObj::read
//   reads the <display> element only
//---------------------------------------------------------

void BasicDurationalObj::readCapxDisplay(XmlReader& e)
      {
      invisible = e.attribute("invisible") == "true";
      e.readNext();
      }

//---------------------------------------------------------
//   BasicDurationalObj::readCapx -- capx equivalent of BasicDurationalObj::read
//---------------------------------------------------------

void BasicDurationalObj::readCapx(XmlReader& e, unsigned int& fullm)
      {
      nDots      = 0;
      noDuration = false;
      postGrace  = false;
      bSmall     = false;
      invisible  = false;
      notBlack   = false;
      color = Qt::black;
      t = TIMESTEP::D1;
      horizontalShift = 0;
      tupletDenominator = 0;
      tripartite   = 0;
      isProlonging = 0;
      QString base = e.attribute("base");
      bool res = false;
      // try to convert base to timestep ("first pattern")
      res = qstring2timestep(base, t);
      if (!res) {
            // try multi-measure rest ("second pattern")
            int i = base.toInt();
            if (i > 0)
                  fullm = i;
            else
                  qDebug("BasicDurationalObj::readCapx: invalid base: '%s'", qPrintable(base));
            }
      nDots = e.intAttribute("dots", 0);
      noDuration = e.attribute("noDuration", "false") == "true";
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "tuplet") {
                  tupletDenominator = e.attribute("count").toInt();
                  tripartite = e.attribute("tripartite", "false") == "true";
                  isProlonging = e.attribute("prolong", "false") == "true";
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      qDebug("DurationObj ndots %d nodur %d postgr %d bsm %d inv %d notbl %d t %d hsh %d den %d trp %d ispro %d fullm %d",
             nDots, noDuration, postGrace, bSmall, invisible, notBlack, int(t), horizontalShift, tupletDenominator, tripartite, isProlonging, fullm
             );
      }

//---------------------------------------------------------
//   BasicDurationalObj::readCapxObjectArray
//---------------------------------------------------------

void BasicDurationalObj::readCapxObjectArray(XmlReader& e)
      {
      objects = cap->readCapxDrawObjectArray(e);
      }

//---------------------------------------------------------
//   CapExplicitBarline::readCapx -- capx equivalent of CapExplicitBarline::read
//---------------------------------------------------------

void CapExplicitBarline::readCapx(XmlReader& e)
      {
      QString type = e.attribute("type", "single");
      if (type == "single") _type = BarLineType::NORMAL;
      else if (type == "double") _type = BarLineType::DOUBLE;
      else if (type == "end") _type = BarLineType::END;
      else if (type == "repEnd") _type = BarLineType::END_REPEAT;
      else if (type == "repBegin") _type = BarLineType::START_REPEAT;
      else if (type == "repEndBegin") _type = BarLineType::END_START_REPEAT;
      else if (type == "dashed") _type = BarLineType::BROKEN;
      else _type = BarLineType::NORMAL; // default
      _barMode = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "drawObjects") {
                  e.skipCurrentElement();
                  }
            else
                  e.unknown();
            }
      e.readNext();
      }

//---------------------------------------------------------
//   CapClef::readCapx -- capx equivalent of CapClef::read
//---------------------------------------------------------

void CapClef::readCapx(XmlReader& e)
      {
      QString clef = e.attribute("clef");
      if (clef == "G2-") {
            form = Form::G;
            line = ClefLine::L2;
            oct = Oct::OCT_BASSA;
            }
      else if (clef == "treble") {
            form = Form::G;
            line = ClefLine::L2;
            oct = Oct::OCT_NULL;
            }
      else if (clef == "bass") {
            form = Form::F;
            line = ClefLine::L4;
            oct = Oct::OCT_NULL;
            }
      else {
            /* default */ form = Form::G;
            line = ClefLine::L2;
            oct = Oct::OCT_NULL;
            }
      qDebug("Clef::read '%s' -> form %d line %d oct %d", qPrintable(clef), int(form), int(line), int(oct));
      e.readNext();
      }

//---------------------------------------------------------
//   CapKey::readCapx -- capx equivalent of CapKey::read
//---------------------------------------------------------

void CapKey::readCapx(XmlReader& e)
      {
      signature = e.intAttribute("fifths", 0);
      qDebug("Key %d", signature);
      e.readNext();
      }

//---------------------------------------------------------
//   qstring2timesig -- convert string to timesig
//   return true on success
//---------------------------------------------------------

static void qstring2timesig(QString& time, uchar& numerator, int& log2Denom, bool& allaBreve)
      {
      bool res = true;
      numerator = 4; log2Denom = 2; allaBreve = false; // set default
      if (time == "allaBreve") { numerator = 2; log2Denom = 1; allaBreve = true; }
      else if (time == "longAllaBreve") { numerator = 4; log2Denom = 1; allaBreve = true; }
      else if (time == "C") { numerator = 4; log2Denom = 2; allaBreve = false; }
      else if (time == "infinite") { qDebug("Meter infinite"); } // not supported by MuseScore ?
      else {
            QStringList splitTime = time.split("/");
            if (splitTime.size() == 2) {
                  numerator = splitTime.at(0).toInt();
                  QString denom = splitTime.at(1);
                  if (denom == "1") log2Denom = 0;
                  else if (denom == "2") log2Denom = 1;
                  else if (denom == "4") log2Denom = 2;
                  else if (denom == "8") log2Denom = 3;
                  else if (denom == "16") log2Denom = 4;
                  else if (denom == "32") log2Denom = 5;
                  else if (denom == "64") log2Denom = 6;
                  else if (denom == "128") log2Denom = 7;
                  else res = false;
                  }
            else res = false;
            }
      // TODO: recovery required if decoding the timesig failed ?
      qDebug("Meter '%s' res %d %d/%d allaBreve %d", qPrintable(time), res, numerator, log2Denom, allaBreve);
      }

//---------------------------------------------------------
//   CapMeter::readCapx -- capx equivalent of CapMeter::read
//---------------------------------------------------------

void CapMeter::readCapx(XmlReader& e)
      {
      qDebug("CapMeter::readCapx");
      QString time = e.attribute("time");
      qstring2timesig(time, numerator, log2Denom, allaBreve);
      e.readNext();
      }

//---------------------------------------------------------
//   ChordObj::readCapxStem -- read <stem> element
//---------------------------------------------------------

void ChordObj::readCapxStem(XmlReader& e)
      {
      QString dir = e.attribute("dir");
      if (dir == "up") stemDir = StemDir::UP;
      else if (dir == "down") stemDir = StemDir::DOWN;
      else if (dir == "none") stemDir = StemDir::NONE;
      e.readNext();
      }

//---------------------------------------------------------
//   ChordObj::readCapxArticulation -- read <articulation> element
//---------------------------------------------------------

void ChordObj::readCapxArticulation(XmlReader& e)
      {
      QString art = e.attribute("type");
      if      (art == "staccato")        articulation = 1;
      else if (art == "tenuto")          articulation = 2;
      else if (art == "staccato tenuto") articulation = 3;
      else if (art == "staccatissimo")   articulation = 4;
      else if (art == "normalAccent")    articulation = 5;
      else if (art == "strongAccent")    articulation = 6;
      else if (art == "weakBeat")        articulation = 7;
      else if (art == "strongBeat")      articulation = 8;
      else                               articulation = 0;
      e.readNext();
      }

//---------------------------------------------------------
//   ChordObj::readCapx -- capx equivalent of ChordObj::read
//---------------------------------------------------------

void ChordObj::readCapx(XmlReader& e)
      {
      stemDir      = StemDir::AUTO;
      dStemLength  = 0;
      nTremoloBars = 0;
      articulation = 0;
      leftTie      = false;
      rightTie     = false;
      beamShift    = 0;
      beamSlope    = 0;
      beamMode      = BeamMode::AUTO;
      notationStave = 0;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "duration") {
                  unsigned int dummy;
                  BasicDurationalObj::readCapx(e, dummy);
                  }
            else if (tag == "display") {
                  BasicDurationalObj::readCapxDisplay(e);
                  }
            else if (tag == "stem") {
                  readCapxStem(e);
                  }
            else if (tag == "beam") {
                  qDebug("ChordObj::readCapx: found beam (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "articulation") {
                  readCapxArticulation(e);
                  }
            else if (tag == "lyric") {
                  readCapxLyrics(e);
                  }
            else if (tag == "drawObjects") {
                  readCapxObjectArray(e);
                  }
            else if (tag == "heads") {
                  readCapxNotes(e);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   pitchStr2Char -- convert pitch string ("[A-G][0-9]") to signed char
//   notes:
//   - in .capx middle C is called C5 (which is C4 in MusicXML)
//   - middle C is MIDI note number 60
//   - in .cap, pitch contains the diatonic note number relative to clef and key
//   - as .capx contains absolute notes, store the MIDI note number instead
//---------------------------------------------------------

static signed char pitchStr2Char(QString& strPitch)
      {
      QRegExp pitchRegExp("[A-G][0-9]");

      if (!pitchRegExp.exactMatch(strPitch)) {
            qDebug("pitchStr2Char: illegal pitch '%s'", qPrintable(strPitch));
            return 0;
            }

      QString steps("C.D.EF.G.A.B");
      int istep  = steps.indexOf(strPitch.leftRef(1));
      int octave = strPitch.rightRef(1).toInt();
      int pitch  = istep + 12 * octave;

      if (pitch < 0)
            pitch = -1;
      if (pitch > 127)
            pitch = -1;

      qDebug("pitchStr2Char: '%s' -> %d", qPrintable(strPitch), pitch);

      return static_cast<signed char>(pitch);
      }

//---------------------------------------------------------
//   ChordObj::readCapxLyrics -- read the lyrics in a capx chord
//---------------------------------------------------------

void ChordObj::readCapxLyrics(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "verse") {
                  Verse v;
                  v.leftAlign = true;        // not used by capella.cpp
                  v.extender  = false;       // not used by capella.cpp
                  v.hyphen    = e.attribute("hyphen") == "true";
                  v.num       = e.intAttribute("i", 0);
                  v.verseNumber = e.attribute("verseNumber"); // not used by capella.cpp
                  v.text = e.readElementText();
                  verse.append(v);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   ChordObj::readCapxNotes -- read the notes in a capx chord
//---------------------------------------------------------

void ChordObj::readCapxNotes(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "head") {
                  QString pitch = e.attribute("pitch");
                  QString sstep;
                  QString shape = e.attribute("shape");
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "alter") {
                              sstep = e.attribute("step");
                              e.readNext();
                              }
                        else if (tag == "tie") {
                              rightTie = e.attribute("begin") == "true";
                              e.readNext();
                              }
                        else
                              e.unknown();
                        }
                  qDebug("ChordObj::readCapxNotes: pitch '%s' altstep '%s' shape '%s'",
                         qPrintable(pitch), qPrintable(sstep), qPrintable(shape));
                  int istep = sstep.toInt();
                  CNote n;
                  n.pitch = pitchStr2Char(pitch);
                  n.explAlteration = 0;
                  n.headType = 0;
                  if (shape == "none")
                       n.headGroup = int(NoteHead::Group::HEAD_CROSS);
                  else
                       n.headGroup = int(NoteHead::Group::HEAD_NORMAL);
                  n.alteration = istep;
                  n.silent = 0;
                  notes.append(n);
                  }
            else
                  e.unknown();
            }

      if (tupletDenominator <= 0)
            return; // no tuplet, so we are done

      // find a BracketObj in the drawObjects list, get the data and delete it
      // this is used to read Tuplets with mixed durations
      for (BasicDrawObj*& o : objects) {
            if (o->type == CapellaType::BRACKET) {
                  // BracketObj* bo = static_cast<BracketObj*>(o);
                  tupletStart = true;
                  tupletCount = o->nNotes;
                  objects.removeOne(o);
                  delete o;
                  qDebug("==> Tuplet start, plus %d notes till end", tupletCount);
                  return; // we are done, and we modified the iterator, so back out
                  }
            }
      qDebug("no tuplet start found in drawObjects, but tupletDenominator %d", tupletDenominator);
      }

//---------------------------------------------------------
//   RestObj::readCapx -- capx equivalent of RestObj::read
//---------------------------------------------------------

void RestObj::readCapx(XmlReader& e)
      {
      bVerticalCentered = false;
      fullMeasures = 0;
      vertShift    = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "duration") {
                  BasicDurationalObj::readCapx(e, fullMeasures);
                  }
            else if (tag == "display") {
                  BasicDurationalObj::readCapxDisplay(e);
                  }
            else if (tag == "verticalPos") {
                  qDebug("RestObj::readCapx: found verticalPos (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "drawObjects") {
                  readCapxObjectArray(e);
                  }
            else
                  e.unknown();
            }

      if (tupletDenominator <= 0)
            return; // no tuplet, so we are done

      // find a BracketObj in the drawObjects list, get the data and delete it
      // this is used to read Tuplets with mixed durations
      for (BasicDrawObj*& o : objects) {
            if (o->type == CapellaType::BRACKET) {
                  // BracketObj* bo = static_cast<BracketObj*>(o);
                  tupletStart = true;
                  tupletCount = o->nNotes;
                  objects.removeOne(o);
                  delete o;
                  qDebug("      ==> Tuplet start, plus %d notes till end", tupletCount);
                  return; // we are done, and we modified the iterator, so back out
                  }
            }
      qDebug("no tuplet start found in drawObjects, but tupletDenominator %d", tupletDenominator);
      }

//---------------------------------------------------------
//   SimpleTextObj::readCapx -- capx equivalent of SimpleTextObj::read
//---------------------------------------------------------

void SimpleTextObj::readCapx(XmlReader& e)
      {
      double x = e.doubleAttribute("x");
      double y = e.doubleAttribute("y");
      QString stralign = e.attribute("align", "left");
      align = 0;
      if (stralign == "center")
            align = 1;
      else if (stralign == "right")
            align = 2;
      relPos = QPointF(x, y);
      relPos *= 32.0;
      // qDebug("x %g y %g align %s", x, y, qPrintable(align));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "font") {
                  _font = capxReadFont(e);
                  }
            else if (tag == "content") {
                  _text = e.readElementText();
                  // qDebug("SimpleTextObj::readCapx: found content '%s'", qPrintable(_text));
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   TransposableObj::readCapx -- capx equivalent of TransposableObj::read
//---------------------------------------------------------

void TransposableObj::readCapx(XmlReader& e)
      {
      QString enharmonicNote = e.attribute("base");
      while (e.readNextStartElement()) {
            const QStringRef& tag1(e.name());
            if (tag1 == "drawObj") {
                  if (e.attribute("base") == enharmonicNote) {
                        while (e.readNextStartElement()) {
                              const QStringRef& tag2(e.name());
                              if (tag2 == "group") {
                                    variants.append(cap->readCapxDrawObjectArray(e));
                                    }
                              }
                        }
                  else {
                        e.skipCurrentElement();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   SlurObj::readCapx -- capx equivalent of SlurObj::read
//---------------------------------------------------------

void SlurObj::readCapx(XmlReader& e)
      {
      // nothing to be done yet
      e.skipCurrentElement();
      }

//---------------------------------------------------------
//   VoltaObj::readCapx -- capx equivalent of VoltaObj::read
//---------------------------------------------------------

void VoltaObj::readCapx(XmlReader& e)
      {
      bLeft           = e.attribute("leftBent", "true") == "true";
      bRight          = e.attribute("rightBent", "true") == "true";
      bDotted         = e.attribute("dotted", "false") == "true";
      allNumbers      = e.attribute("allNumbers", "false") == "true";
      int firstNumber = e.intAttribute("firstNumber", 0);
      int lastNumber  = e.intAttribute("lastNumber", 0);
      if (firstNumber == 0) {
            // don't know what this means (spec: no number)
            // cap status equivalent to no first or last number
            from = 1;
            to   = 0;
            }
      else {
            from = firstNumber;
            to   = (lastNumber == 0) ? firstNumber : lastNumber;
            }
      qDebug("VoltaObj::read firstNumber %d lastNumber %d", firstNumber, lastNumber);
      qDebug("VoltaObj::read x0 %d x1 %d y %d bLeft %d bRight %d bDotted %d allNumbers %d from %d to %d",
             x0, x1, y, bLeft, bRight, bDotted, allNumbers, from, to);
      e.readNext();
      }

//---------------------------------------------------------
//   TrillObj::readCapx -- capx equivalent of TrillObj::read
//---------------------------------------------------------

void TrillObj::readCapx(XmlReader& e)
      {
      double x0d  = e.doubleAttribute("x1", 0.0);
      double x1d  = e.doubleAttribute("x2", 0.0);
      double yd   = e.doubleAttribute("y", 0.0);
      trillSign   = e.attribute("tr", "true") == "true";
      x0 = (int)round(x0d * 32.0);
      x1 = (int)round(x1d * 32.0);
      y  = (int)round(yd * 32.0);
      // color       -> skipped
      qDebug("TrillObj::read x0 %d x1 %d y %d trillSign %d", x0, x1, y, trillSign);
      e.readNext();
      }

//---------------------------------------------------------
//   WedgeObj::readCapx -- capx equivalent of WedgeObj::read
//---------------------------------------------------------

void WedgeObj::readCapx(XmlReader& e)
      {
      // TODO: read LineObj properties
      decresc          = e.attribute("decrescendo", "false") == "true";
      double dheight   = e.doubleAttribute("span", 1.0);
      height = (int)round(dheight * 32.0);
      e.readNext();
      }

//------------------------------------------------------------
//   BracketObj::readCapx -- capx equivalent of WedgeObj::read
//------------------------------------------------------------

void BracketObj::readCapx(XmlReader& e)
      {
      // TODO: We're actually only interested in the <basic> element following and it's noteRange attribute ...
      // or to put in another way: Intentionally left empty ...
      e.skipCurrentElement();
      }

//---------------------------------------------------------
//   readCapxDrawObjectArray -- capx equivalent of readDrawObjectArray()
//---------------------------------------------------------

QList<BasicDrawObj*> Capella::readCapxDrawObjectArray(XmlReader& e)
      {
      QList<BasicDrawObj*> ol;
      while (e.readNextStartElement()) {
            if (e.name() == "drawObj") {
                  BasicDrawObj* bdo = 0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "basic") {
                              // note: the <basic> element always follows the DrawObject it applies to
                              if (bdo)
                                    bdo->readCapx(e);
                              else
                                    e.skipCurrentElement();
                              }
                        else if (tag == "line") {
                              qDebug("readCapxDrawObjectArray: found line (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "rectangle") {
                              qDebug("readCapxDrawObjectArray: found rectangle (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "ellipse") {
                              qDebug("readCapxDrawObjectArray: found ellipse (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "polygon") {
                              qDebug("readCapxDrawObjectArray: found polygon (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "metafile") {
                              qDebug("readCapxDrawObjectArray: found metafile (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "text") {
                              SimpleTextObj* o = new SimpleTextObj(this);
                              bdo = o; // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "richText") {
                              qDebug("readCapxDrawObjectArray: found richText (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "guitar") {
                              qDebug("readCapxDrawObjectArray: found guitar (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "slur") {
                              SlurObj* o = new SlurObj(this);
                              bdo = o; // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "wavyLine") {
                              qDebug("readCapxDrawObjectArray: found wavyLine (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "bracket") {
                              BracketObj* o = new BracketObj(this);
                              bdo = o;           // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "wedge") {
                              WedgeObj* o = new WedgeObj(this);
                              bdo = o; // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "notelines") {
                              qDebug("readCapxDrawObjectArray: found notelines (skipping)");
                              e.skipCurrentElement();
                              }
                        else if (tag == "volta") {
                              VoltaObj* o = new VoltaObj(this);
                              bdo = o; // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "trill") {
                              TrillObj* o = new TrillObj(this);
                              bdo = o; // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "transposable") {
                              TransposableObj* o = new TransposableObj(this);
                              bdo = o; // save o to handle the "basic" tag (which sometimes follows)
                              o->readCapx(e);
                              ol.append(o);
                              }
                        else if (tag == "group") {
                              qDebug("readCapxDrawObjectArray: found group (skipping)");
                              e.skipCurrentElement();
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return ol;
      }

//---------------------------------------------------------
//   readCapxVoice -- capx equivalent of readVoice(CapStaff* cs, int idx)
//---------------------------------------------------------

void Capella::readCapxVoice(XmlReader& e, CapStaff* cs, int idx)
      {
      CapVoice* v   = new CapVoice;
      v->voiceNo    = idx;
      v->y0Lyrics   = 0;
      v->dyLyrics   = 0;
      // v->lyricsFont = 0;
      v->stemDir    = 0;

      int tupletCounter = 0;
      Fraction tupletTicks = Fraction(0, 1);
      BasicDurationalObj* tupletObj = nullptr;

      while (e.readNextStartElement()) {
            if (e.name() == "lyricsSettings") {
                  qDebug("readCapxVoice: found lyricsSettings (skipping)");
                  e.skipCurrentElement();
                  }
            else if (e.name() == "noteObjects") {
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "clefSign") {
                              CapClef* clef = new CapClef(this);
                              clef->readCapx(e);
                              v->objects.append(clef);
                              }
                        else if (tag == "keySign") {
                              CapKey* key = new CapKey(this);
                              key->readCapx(e);
                              v->objects.append(key);
                              }
                        else if (tag == "timeSign") {
                              CapMeter* meter = new CapMeter(this);
                              meter->readCapx(e);
                              v->objects.append(meter);
                              }
                        else if (tag == "barline") {
                              CapExplicitBarline* bl = new CapExplicitBarline(this);
                              bl->readCapx(e);
                              v->objects.append(bl);
                              }
                        else if (tag == "chord") {
                              ChordObj* chord = new ChordObj(this);
                              chord->readCapx(e);
                              v->objects.append(chord);
                              if (chord->tupletStart) {
                                    tupletCounter = chord->tupletCount;
                                    tupletTicks   = chord->ticks();
                                    tupletObj     = chord;  // save the tuplet object for later
                                    qDebug("     ==| (C) start of tuplet with %d notes, ticks %d",
                                           1 + tupletCounter, tupletTicks.ticks());
                                    } else if (tupletCounter > 0) {
                                    if (!tupletObj) {
                                          qDebug("     !!! chord with tupletCounter %d without tupletObj", tupletCounter);
                                          } else {
                                          tupletTicks += chord->ticks();
                                          if (chord->tupletDenominator == 0) {
                                                qDebug("     !!! (C) chord with tupletCounter %d outside tuplet", tupletCounter);
                                                chord->tupletDenominator = tupletObj->tupletDenominator;  // copy denominator from the tuplet object
                                                }
                                          --tupletCounter;
                                          if (tupletCounter == 0) {
                                                chord->tupletEnd = true;
                                                tupletObj->tupletTicks = tupletTicks;      // set total ticks to the tuplet object
                                                tupletObj = nullptr;      // reset tuplet object
                                                qDebug("     ==| end of tuplet, total ticks %d", tupletTicks.ticks());
                                                } else {
                                                qDebug("     ==> chord with tupletCounter %d inside tuplet, ticks now %d",
                                                       tupletCounter, tupletTicks.ticks());
                                                }
                                          }
                                    }
                              }
                        else if (tag == "rest") {
                              RestObj* rest = new RestObj(this);
                              rest->readCapx(e);
                              v->objects.append(rest);
                              if (rest->tupletStart) {
                                    tupletCounter = rest->tupletCount;
                                    tupletTicks   = rest->ticks();
                                    tupletObj     = rest;  // save the tuplet object for later
                                    qDebug("     ==| (R) start of tuplet with %d notes, ticks %d",
                                                1 + tupletCounter, tupletTicks.ticks());
                                    } else if (tupletCounter > 0) {
                                    if (!tupletObj) {
                                          qDebug("     !!! rest with tupletCounter %d without tupletObj", tupletCounter);
                                          } else {
                                          // add the rest ticks to the tuplet ticks
                                          tupletTicks += rest->ticks();
                                          if (rest->tupletDenominator == 0) {
                                                qDebug("     !!! rest with tupletCounter %d outside tuplet", tupletCounter);
                                                rest->tupletDenominator = tupletObj->tupletDenominator;  // copy denominator from the tuplet object
                                                }
                                          --tupletCounter;
                                          if (tupletCounter == 0) {
                                                rest->tupletEnd = true;
                                                tupletObj->tupletTicks = tupletTicks;      // set total ticks to the tuplet object
                                                tupletObj = nullptr;      // reset tuplet object
                                                qDebug("     ==| end of tuplet, total ticks %d", tupletTicks.ticks());
                                                } else {
                                                qDebug("     ==> rest with tupletCounter %d inside tuplet, ticks now %d",
                                                            tupletCounter, tupletTicks.ticks());
                                                }
                                          }
                                    }
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }

      cs->voices.append(v);
      }

//---------------------------------------------------------
//   findStaffIndex -- find index of layout in staffLayouts
//---------------------------------------------------------

static int findStaffIndex(const QString& layout, const QList<CapStaffLayout*>& staffLayouts)
      {
      int index = 0;
      for (const auto& capStaffLayout : staffLayouts) {
            if (capStaffLayout->descr == layout)
                  return index;
            ++index;
            }
      qFatal("staff layout '%s' not found", qPrintable(layout));
      return 0;
      }

//---------------------------------------------------------
//   readCapxStaff -- capx equivalent of readStaff(CapSystem* system)
//---------------------------------------------------------

void Capella::readCapxStaff(XmlReader& e, CapSystem* system)
      {
      qDebug("Capella::readCapxStaff");
      CapStaff* staff = new CapStaff;
      QString layout = e.attribute("layout");
      QString time = e.attribute("defaultTime");
      qstring2timesig(time, staff->numerator, staff->log2Denom, staff->allaBreve);

      staff->iLayout   = findStaffIndex(layout,staffLayouts());
      staff->topDistX  = 0;
      staff->btmDistX  = 0;
      staff->color     = Qt::black;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "extraDistance") {
                  qDebug("readCapxStaff: found extraDistance (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "voices") {
                  int i = 0;
                  while (e.readNextStartElement()) {
                        if (e.name() == "voice") {
                              readCapxVoice(e, staff, i);
                              ++i;
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }

      system->staves[staff->iLayout] = staff;
      }

//---------------------------------------------------------
//   initUnreadStaves
//---------------------------------------------------------

static void initUnreadStaves(QList<CapStaff*>& staves)
      {
      // find the first staff actually read
      CapStaff* firstNonEmptyStaff = nullptr;
      for (const auto staff : staves) {
            if (staff && !firstNonEmptyStaff)
                  firstNonEmptyStaff = staff;
            }

      // copy its data to the staves present in the file
      for (int i = 0; i < staves.size(); ++i) {
            if (!(staves.at(i))) {
                  auto staff = new CapStaff;
                  staff->numerator = firstNonEmptyStaff->numerator;
                  staff->log2Denom = firstNonEmptyStaff->log2Denom;
                  staff->allaBreve = firstNonEmptyStaff->allaBreve;
                  staff->iLayout = i;
                  staves[i] = staff;
                  }
            }
      }

//---------------------------------------------------------
//   readCapxSystem -- capx equivalent of readSystem()
//---------------------------------------------------------

void Capella::readCapxSystem(XmlReader& e)
      {
      CapSystem* s = new CapSystem;
      s->nAddBarCount   = 0;
      s->bBarCountReset = 0;
      s->explLeftIndent = 0; // ?? TODO ?? use in capella.cpp commented out

      s->beamMode = BeamMode::AUTO;
      s->tempo    = 0;
      s->color    = Qt::black;

      unsigned char b  = 0;
      s->bJustified    = b & 2;
      s->bPageBreak    = (b & 4) != 0;
      s->instrNotation = (b >> 3) & 7;

      // set all staves in this system to "not yet read"
      for (int i = 0; i < _staffLayouts.size(); ++i)
            s->staves.append(nullptr);

      // read staves
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "barCount") {
                  qDebug("readCapxSystem: found barCount (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "staves") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "staff")
                              readCapxStaff(e, s);
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }

      // initializes staves not read
      initUnreadStaves(s->staves);

      systems.append(s);
      }

//---------------------------------------------------------
//   capxSystems -- read the capx <systems> element
//---------------------------------------------------------

void Capella::capxSystems(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "system") {
                  readCapxSystem(e);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   capxNotation -- read the capx <notation> element
//---------------------------------------------------------

static void capxNotation(XmlReader& e, uchar& barlineMode, uchar& barlineFrom, uchar& barlineTo)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "barlines") {
                  QString mode = e.attribute("mode", "full");
                  if (mode == "full") barlineMode = 3;
                  else if (mode == "none") barlineMode = 2;     // correct ? not handled by capella.cpp anyway
                  else if (mode == "internal") barlineMode = 1;
                  else if (mode == "external") barlineMode = 0; // correct ? not handled by capella.cpp anyway
                  else barlineMode = 1;
                  barlineFrom = e.intAttribute("from");
                  barlineTo   = e.intAttribute("to");
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readCapxStaveLayout -- capx equivalent of readStaveLayout(CapStaffLayout*, int)
//   read the staffLayout element
//---------------------------------------------------------

void Capella::readCapxStaveLayout(XmlReader& e, CapStaffLayout* sl, int /*idx*/)
      {
      // initialize same variables as readStaveLayout(CapStaffLayout*, int)

      sl->barlineMode = 0;
      sl->noteLines   = 0;

      sl->bSmall      = 0; // TODO (unclear where this is in the capx file)

      sl->topDist      = 0;
      sl->btmDist      = 0;
      sl->groupDist    = 0;
      sl->barlineFrom = 0;
      sl->barlineTo   = 0;

      unsigned char clef = 0;
      sl->form = Form(clef & 7);
      sl->line = ClefLine((clef >> 3) & 7);
      sl->oct  = Oct((clef >> 6));
      // qDebug("   clef %x  form %d, line %d, oct %d", clef, sl->form, sl->line, sl->oct);

      // Schlagzeuginformation
      unsigned char b   = 0; // ?? TODO ?? sl->soundMapIn and sl->soundMapOut are not used
      sl->bPercussion  = b & 1;    // Schlagzeugkanal verwenden
      sl->bSoundMapIn  = b & 2;
      sl->bSoundMapOut = b & 4;
      /*
      if (sl->bSoundMapIn) {      // Umleitungstabelle fr Eingabe vom Keyboard
            uchar iMin = readByte();
            Q_UNUSED(iMin);
            uchar n    = readByte();
            Q_ASSERT(n > 0 and iMin + n <= 128);
            f->read(sl->soundMapIn, n);
            curPos += n;
            }
      if (sl->bSoundMapOut) {     // Umleitungstabelle fr das Vorspielen
            unsigned char iMin = readByte();
            unsigned char n    = readByte();
            Q_ASSERT(n > 0 and iMin + n <= 128);
            f->read(sl->soundMapOut, n);
            curPos += n;
            }
      */
      sl->sound  = 0;
      sl->volume = 0;
      sl->transp = 0;

      qDebug("readCapxStaveLayout");
      sl->descr = e.attribute("description");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "notation") {
                  capxNotation(e, sl->barlineMode, sl->barlineFrom, sl->barlineTo);
                  }
            else if (tag == "distances") {
                  qDebug("readCapxStaveLayout: found distances (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "instrument") {
                  sl->name = e.attribute("name");
                  sl->abbrev = e.attribute("abbrev");
                  // elements name and abbrev overrule attributes name and abbrev
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "name")
                              sl->name = e.readElementText();
                        else if (t == "abbrev")
                              sl->abbrev = e.readElementText();
                        else
                              e.unknown();
                        }
                  }
            else if (tag == "sound") {
                  sl->sound = e.intAttribute("instr", 0);
                  sl->volume = e.intAttribute("volume", 0);
                  sl->transp = e.intAttribute("transpose", 0);
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      qDebug("   descr '%s' name '%s' abbrev '%s'",
             qPrintable(sl->descr), qPrintable(sl->name), qPrintable(sl->abbrev));
      qDebug("   sound %d vol %d transp %d", sl->sound, sl->volume, sl->transp);
      qDebug("readCapxStaveLayout done");
      }

//---------------------------------------------------------
//   capxLayoutBrackets -- read the capx <brackets> element (when child of <layout)
//---------------------------------------------------------

static void capxLayoutBrackets(XmlReader& e, QList<CapBracket>& bracketList)
      {
      // int i = 0; // bracket count
      while (e.readNextStartElement()) {
            if (e.name() == "bracket") {
                  CapBracket b;
                  b.from  = e.intAttribute("from");
                  b.to    = e.intAttribute("to");
                  b.curly = e.attribute("curly") == "true";
                  // qDebug("Bracket%d %d-%d curly %d", i, b.from, b.to, b.curly);
                  bracketList.append(b);
                  e.readNext();
                  // ++i;
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   capxLayoutDistances -- read the capx <Distances> element (when child of <layout)
//---------------------------------------------------------

static void capxLayoutDistances(XmlReader& e, double& smallLineDist, double& normalLineDist, int& topDist)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "staffLines") {
                  smallLineDist = e.doubleAttribute("small");
                  normalLineDist = e.doubleAttribute("normal");
                  e.readNext();
                  }
            else if (tag == "systems") {
                  topDist = e.intAttribute("top");
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   capxLayoutStaves -- read the capx <staves> element (when child of <layout)
//---------------------------------------------------------

void Capella::capxLayoutStaves(XmlReader& e)
      {
      int iStave = 0;
      while (e.readNextStartElement()) {
            if (e.name() == "staffLayout") {
                  CapStaffLayout* sl = new CapStaffLayout;
                  readCapxStaveLayout(e, sl, iStave);
                  _staffLayouts.append(sl);
                  ++iStave;
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   capxLayout -- read the capx <layout> element
//   rough equivalent of readLayout() read part
//---------------------------------------------------------

void Capella::capxLayout(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "pages") {
                  qDebug("capxLayout: found pages (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "distances") {
                  capxLayoutDistances(e, smallLineDist, normalLineDist, topDist);
                  // qDebug("Capella::capxLayout(): smallLineDist %g normalLineDist %g topDist %d",
                  //        smallLineDist, normalLineDist, topDist);
                  }
            else if (tag == "instrumentNames") {
                  qDebug("capxLayout: found instrumentNames (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "style") {
                  qDebug("capxLayout: found style (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "staves") {
                  capxLayoutStaves(e);
                  }
            else if (tag == "brackets") {
                  capxLayoutBrackets(e, brackets);
                  }
            else if (tag == "spacing") {
                  qDebug("capxLayout: found spacing (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "beamFlattening") {
                  qDebug("capxLayout: found beamFlattening (skipping)");
                  e.skipCurrentElement();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   initCapxLayout -- capx equivalent of readLayout() initialize part
//---------------------------------------------------------

void Capella::initCapxLayout()
      {
      // initialize same variables as readLayout()

      smallLineDist  = 1.2;  // TODO verify default
      normalLineDist = 1.76; // TODO verify default

      topDist        = 14;   // TODO verify default
      interDist      = 0;    // TODO verify default

      txtAlign   = 0;
      adjustVert = 0;

      unsigned char b          = 0;
      redundantKeys    = b & 1;
      modernDoubleNote = b & 2;
      Q_ASSERT((b & 0xFC) == 0); // bits 2...7 reserviert

      bSystemSeparators = 0;
      nUnnamed           = 0;

      // namesFont = ... // ?? not used ??

      // Note: readLayout() also reads stave layout (using readStaveLayout(sl, iStave))
      // and system brackets. Here this is handled by readCapx(XmlReader& e).
      }

//---------------------------------------------------------
//   readCapx -- capx equivalent of read(QFile* fp)
//---------------------------------------------------------

void Capella::readCapx(XmlReader& e)
      {
      // initialize same variables as read(QFile* fp)

      f      = 0;
      curPos = 0;

      author   = 0;
      keywords = 0;
      comment  = 0;

      nRel   = 0;
      nAbs   = 0;
      unsigned char b   = 0;
      bUseRealSize      = b & 1;
      bAllowCompression = b & 2;
      bPrintLandscape   = b & 16;

      beamRelMin0 = 0;
      beamRelMin1 = 0;
      beamRelMax0 = 0;
      beamRelMax1 = 0;

      backgroundChord = new ChordObj(this);         // contains graphic objects on the page background
      bShowBarCount    = 0;
      barNumberFrame   = 0;
      nBarDistX        = 0;
      nBarDistY        = 0;
      // QFont barNumFont = ... // not used ?
      nFirstPage       = 0;
      leftPageMargins  = 0;
      topPageMargins   = 0;
      rightPageMargins = 0;
      btmPageMargins   = 0;

      // Now do the equivalent of:
      // readLayout(); (called only once)
      // readExtra();  (this is a NOP)
      // readDrawObjectArray();
      // for (unsigned i = 0; i < nSystems; i++)
      //       readSystem();

      initCapxLayout();

      // read stave layout
      // read systems
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "info") {
                  qDebug("importCapXml: found info (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "layout") {
                  capxLayout(e);
                  }
            else if (tag == "gallery") {
                  qDebug("capxLayout: found gallery (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "pageObjects") {
                  backgroundChord->readCapxObjectArray(e);
                  }
            else if (tag == "barCount") {
                  qDebug("importCapXml: found barCount (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "systems") {
                  capxSystems(e);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   importCapXml
//---------------------------------------------------------

void convertCapella(Score* score, Capella* cap, bool capxMode);

Score::FileError importCapXml(MasterScore* score, const QString& name)
      {
      qDebug("importCapXml(score %p, name %s)", score, qPrintable(name));
      MQZipReader uz(name);
      if (!uz.exists()) {
            qDebug("importCapXml: <%s> not found", qPrintable(name));
            return Score::FileError::FILE_NOT_FOUND;
            }

      QByteArray dbuf = uz.fileData("score.xml");
      XmlReader e(dbuf);
      e.setDocName(name);
      Capella cf;

      while (e.readNextStartElement()) {
            if (e.name() == "score") {
                  const QString& xmlns = e.attribute("xmlns", "<none>"); // doesn't work ???
                  qDebug("importCapXml: found score, namespace '%s'", qPrintable(xmlns));
                  cf.readCapx(e);
                  }
            else
                  e.unknown();
            }

      convertCapella(score, &cf, true);
      return Score::FileError::FILE_NO_ERROR;
      }
}


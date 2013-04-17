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
#include "libmscore/qzipreader_p.h"
#include "capella.h"

//---------------------------------------------------------
//   qstring2timestep -- convert string to TIMESTEP
//---------------------------------------------------------

static bool qstring2timestep(QString& str, TIMESTEP& tstp)
      {
      if      (str == "1/1")   { tstp = D1;      return true; }
      else if (str == "1/2")   { tstp = D2;      return true; }
      else if (str == "1/4")   { tstp = D4;      return true; }
      else if (str == "1/8")   { tstp = D8;      return true; }
      else if (str == "1/16")  { tstp = D16;     return true; }
      else if (str == "1/32")  { tstp = D32;     return true; }
      else if (str == "1/64")  { tstp = D64;     return true; }
      else if (str == "1/128") { tstp = D128;    return true; }
      else if (str == "2/1")   { tstp = D_BREVE; return true; }
      return false;
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
      t = D1;
      horizontalShift = 0;
      count = 0;
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
                  count = e.attribute("count").toInt();
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      qDebug("DurationObj ndots %d nodur %d postgr %d bsm %d inv %d notbl %d t %d hsh %d cnt %d trp %d ispro %d fullm %d",
             nDots, noDuration, postGrace, bSmall, invisible, notBlack, t, horizontalShift, count, tripartite, isProlonging, fullm
             );
      }

//---------------------------------------------------------
//   CapExplicitBarline::readCapx -- capx equivalent of CapExplicitBarline::read
//---------------------------------------------------------

void CapExplicitBarline::readCapx(XmlReader& e)
      {
      _type = BAR_SINGLE; // TODO
      _barMode = 0;
      e.readNext();
      }

//---------------------------------------------------------
//   CapClef::readCapx -- capx equivalent of CapClef::read
//---------------------------------------------------------

void CapClef::readCapx(XmlReader& e)
      {
      QString clef = e.attribute("clef");
      if (clef == "G2-") { form = FORM_G; line = LINE_2; oct = OCT_BASSA; }
      else if (clef == "treble") { form = FORM_G; line = LINE_2; oct = OCT_NULL; }
      else if (clef == "bass") { form = FORM_F; line = LINE_4; oct = OCT_NULL; }
      else { /* default */ form = FORM_G; line = LINE_2; oct = OCT_NULL; }
      qDebug("Clef::read '%s' -> form %d line %d oct %d", qPrintable(clef), form, line, oct);
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
//   CapMeter::readCapx -- capx equivalent of CapMeter::read
//---------------------------------------------------------

void CapMeter::readCapx(XmlReader& e)
      {
      QString time = e.attribute("time");
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
                  }
            }
      qDebug("Meter %d/%d allaBreve %d", numerator, log2Denom, allaBreve);
      e.readNext();
      }

//---------------------------------------------------------
//   ChordObj::readCapx -- capx equivalent of ChordObj::read
//---------------------------------------------------------

void ChordObj::readCapx(XmlReader& e)
      {
      stemDir      = 0;
      dStemLength  = 0;
      nTremoloBars = 0;
      articulation = 0;
      leftTie      = false;
      rightTie     = false;
      beamShift    = 0;
      beamSlope    = 0;
      beamMode      = AUTO_BEAM;
      notationStave = 0;

      /*
      if (flags & 0x40) {
            unsigned nVerses = cap->readUnsigned();
            for (unsigned int i = 0; i < nVerses; ++i) {
                  bool bVerse = cap->readByte();
                  if (bVerse) {
                        Verse v;
                        unsigned char b = cap->readByte();
                        v.leftAlign = b & 1;
                        v.extender  = b & 2;
                        v.hyphen    = b & 4;
                        v.num       = i;
                        if (b & 8)
                              v.verseNumber = cap->readString();
                        if (b & 16)
                              v.text = cap->readString();
                        verse.append(v);
                        }
                  }
            }
            }
      */
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "duration") {
                  unsigned int dummy;
                  BasicDurationalObj::readCapx(e, dummy);
                  }
            else if (tag == "display") {
                  qDebug("ChordObj::readCapx: found display (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "stem") {
                  qDebug("ChordObj::readCapx: found stem (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "beam") {
                  qDebug("ChordObj::readCapx: found beam (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "articulation") {
                  qDebug("ChordObj::readCapx: found articulation (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "lyric") {
                  qDebug("ChordObj::readCapx: found lyric (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "drawObjects") {
                  qDebug("ChordObj::readCapx: found drawObjects (skipping)");
                  e.skipCurrentElement();
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
      int istep  = steps.indexOf(strPitch.left(1));
      int octave = strPitch.right(1).toInt();
      int pitch  = istep + 12 * octave;

      if (pitch < 0)
            pitch = -1;
      if (pitch > 127)
            pitch = -1;

      qDebug("pitchStr2Char: '%s' -> %d", qPrintable(strPitch), pitch);

      return static_cast<signed char>(pitch);
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
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "alter") {
                              sstep = e.attribute("step");
                              e.readNext();
                              }
                        else if (tag == "tie") {
                              qDebug("ChordObj::readCapxNotes: found tie (skipping)");
                              e.skipCurrentElement();
                              }
                        else
                              e.unknown();
                        }
                  qDebug("ChordObj::readCapxNotes: pitch '%s' altstep '%s'",
                         qPrintable(pitch), qPrintable(sstep));
                  int istep = sstep.toInt();
                  CNote n;
                  n.pitch = pitchStr2Char(pitch);
                  n.explAlteration = 0;
                  n.headType = 0;
                  n.alteration = istep;
                  n.silent = 0;
                  notes.append(n);
                  }
            else
                  e.unknown();
            }
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
                  qDebug("RestObj::readCapx: found display (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "verticalPos") {
                  qDebug("RestObj::readCapx: found verticalPos (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "drawObjects") {
                  qDebug("RestObj::readCapx: found drawObjects (skipping)");
                  e.skipCurrentElement();
                  }
            else
                  e.unknown();
            }
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
                              qDebug("readCapxVoice: found chord (skipping)");
                              ChordObj* chord = new ChordObj(this);
                              chord->readCapx(e);
                              v->objects.append(chord);
                              }
                        else if (tag == "rest") {
                              RestObj* rest = new RestObj(this);
                              rest->readCapx(e);
                              v->objects.append(rest);
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
//   readCapxStaff -- capx equivalent of readStaff(CapSystem* system)
//---------------------------------------------------------

void Capella::readCapxStaff(XmlReader& e, CapSystem* system, int iStave)
      {
      CapStaff* staff = new CapStaff;
      //Meter
      staff->numerator = 3; // TODO (required !)
      staff->log2Denom = 2; // TODO (required !)
      staff->allaBreve = 0;

      staff->iLayout   = iStave;
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

      system->staves.append(staff);
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

      s->beamMode = 0;
      s->tempo    = 0;
      s->color    = Qt::black;

      unsigned char b  = 0;
      s->bJustified    = b & 2;
      s->bPageBreak    = (b & 4) != 0;
      s->instrNotation = (b >> 3) & 7;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "barCount") {
                  qDebug("readCapxSystem: found barCount (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "staves") {
                  int iStave = 0;
                  while (e.readNextStartElement()) {
                        if (e.name() == "staff") {
                              readCapxStaff(e, s, iStave);
                              ++iStave;
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }

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
//   readCapxStaveLayout -- capx equivalent of readStaveLayout(CapStaffLayout*, int)
//   read the staffLayout element
//---------------------------------------------------------

void Capella::readCapxStaveLayout(XmlReader& e, CapStaffLayout* sl, int /*idx*/)
      {
      // initialize same variables as readStaveLayout(CapStaffLayout*, int)

      sl->barlineMode = 0; // TODO
      sl->noteLines   = 0;

      sl->bSmall      = 0; // TODO

      sl->topDist      = 0;
      sl->btmDist      = 0;
      sl->groupDist    = 0;
      sl->barlineFrom = 0;
      sl->barlineTo   = 0;

      unsigned char clef = 0;
      sl->form = FORM(clef & 7);
      sl->line = CLEF_LINE((clef >> 3) & 7);
      sl->oct  = OCT((clef >> 6));
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
            assert (n > 0 and iMin + n <= 128);
            f->read(sl->soundMapIn, n);
            curPos += n;
            }
      if (sl->bSoundMapOut) {     // Umleitungstabelle fr das Vorspielen
            unsigned char iMin = readByte();
            unsigned char n    = readByte();
            assert (n > 0 and iMin + n <= 128);
            f->read(sl->soundMapOut, n);
            curPos += n;
            }
      */
      sl->sound  = 0; // TODO
      sl->volume = 0;
      sl->transp = 0;
      // qDebug("   sound %d vol %d transp %d", sl->sound, sl->volume, sl->transp);

      sl->descr              = 0; // TODO
      sl->name               = 0; // TODO
      sl->abbrev             = 0; // TODO
      sl->intermediateName   = 0;
      sl->intermediateAbbrev = 0;
      // qDebug("   descr <%s> name <%s>  abbrev <%s> iname <%s> iabrev <%s>",
      // sl->descr, sl->name, sl->abbrev, sl->intermediateName, sl->intermediateAbbrev);

      qDebug("readCapxStaveLayout: descr '%s'", qPrintable(e.attribute("description")));
      // TODO: convert descr to char* and store in sl->descr
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "notation") {
                  qDebug("readCapxStaveLayout: found notation (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "distances") {
                  qDebug("readCapxStaveLayout: found distances (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "instrument") {
                  qDebug("readCapxStaveLayout: found instrument (skipping)");
                  e.skipCurrentElement();
                  }
            else if (tag == "sound") {
                  sl->sound = e.intAttribute("instr", 0);
                  sl->volume = e.intAttribute("volume", 0);
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      qDebug("   sound %d vol %d transp %d", sl->sound, sl->volume, sl->transp);
      qDebug("readCapxStaveLayout done");
      }

//---------------------------------------------------------
//   capxLayoutStaves -- read the capx <staves> element (when child of <layout)
//---------------------------------------------------------

void Capella::capxLayoutStaves(XmlReader& e)
      {
      int iStave = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "staffLayout") {
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
                  qDebug("capxLayout: found distances (skipping)");
                  e.skipCurrentElement();
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
                  qDebug("capxLayout: found brackets (skipping)");
                  e.skipCurrentElement();
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

      smallLineDist  = 1.2; // TODO verify default
      normalLineDist = 1.76; // TODO verify default

      topDist        = 14; // TODO verify default
      interDist      = 0;

      txtAlign   = 0;
      adjustVert = 0;

      unsigned char b          = 0;
      redundantKeys    = b & 1;
      modernDoubleNote = b & 2;
      assert ((b & 0xFC) == 0); // bits 2...7 reserviert

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

      backgroundChord = new ChordObj(this);
      // TODO backgroundChord->read();              // contains graphic objects on the page background
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
                  qDebug("capxLayout: found pageObjects (skipping)");
                  e.skipCurrentElement();
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

Score::FileError importCapXml(Score* score, const QString& name)
      {
      qDebug("importCapXml(score %p, name %s)", score, qPrintable(name));
      QZipReader uz(name);
      if (!uz.exists()) {
            qDebug("importCapXml: <%s> not found", qPrintable(name));
            MScore::lastError = QT_TRANSLATE_NOOP("file", "file not found");
            return Score::FILE_NOT_FOUND;
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
      return Score::FILE_NO_ERROR;
      }

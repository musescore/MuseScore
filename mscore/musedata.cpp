//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: musedata.cpp 5497 2012-03-26 10:59:16Z lasconic $
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "musedata.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/barline.h"
#include "libmscore/clef.h"
#include "libmscore/key.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/text.h"
#include "libmscore/bracket.h"
#include "libmscore/tuplet.h"
#include "libmscore/slur.h"
#include "libmscore/dynamic.h"
#include "libmscore/lyrics.h"
#include "libmscore/articulation.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"
#include "libmscore/segment.h"

namespace Ms {

//---------------------------------------------------------
//   musicalAttribute
//---------------------------------------------------------

void MuseData::musicalAttribute(QString s, Part* part)
      {
      QStringList al = s.mid(3).split(" ", QString::SkipEmptyParts);
      foreach(QString item, al) {
            if (item.startsWith("K:")) {
                  int key = item.mid(2).toInt();
                  foreach(Staff* staff, *(part->staves()))
                        (*staff->keymap())[curTick] = key;
                  }
            else if (item.startsWith("Q:")) {
                  _division = item.mid(2).toInt();
                  }
            else if (item.startsWith("T:")) {
                  QStringList tl = item.mid(2).split("/");
                  if (tl.size() != 2) {
                        qDebug("bad time sig <%s>\n", qPrintable(item));
                        continue;
                        }
                  int z = tl[0].toInt();
                  int n = tl[1].toInt();
                  if ((z > 0) && (n > 0)) {
//TODO                        score->sigmap()->add(curTick, Fraction(z, n));
                        TimeSig* ts = new TimeSig(score);
                        Staff* staff = part->staff(0);
                        ts->setTrack(staff->idx() * VOICES);
                        Measure* measure = score->tick2measure(curTick);
                        Segment* s = measure->getSegment(ts, curTick);
                        s->add(ts);
                        }
                  }
            else if (item.startsWith("X:"))
                  ;
            else if (item[0] == 'C') {
                  int staffIdx = 1;
//                  int col = 2;
                  if (item[1].isDigit()) {
                        staffIdx = item.mid(1,1).toInt();
//                        col = 3;
                        }
                  staffIdx -= 1;
/*                  int clef = item.mid(col).toInt();
                  ClefType mscoreClef = ClefType::G;
                  switch(clef) {
                        case 4:  mscoreClef = ClefType::G; break;
                        case 22: mscoreClef = ClefType::F; break;
                        case 13: mscoreClef = ClefType::C3; break;
                        case 14: mscoreClef = ClefType::C2; break;
                        case 15: mscoreClef = ClefType::C1; break;
                        default:
                              qDebug("unknown clef %d\n", clef);
                              break;
                        }
                  */
//                  Staff* staff = part->staff(staffIdx);
//                  staff->setClef(curTick, mscoreClef);
                  }
            else
                  qDebug("unknown $key <%s>\n", qPrintable(item));
            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void MuseData::readChord(Part*, const QString& s)
      {
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

      int step  = s[1].toLatin1() - 'A';
      int alter = 0;
      int octave = 0;
      for (int i = 2; i < 4; ++i) {
            if (s[i] == '#')
                  alter += 1;
            else if (s[i] == 'f')
                  alter -= 1;
            else if (s[i].isDigit()) {
                  octave = s.mid(i,1).toInt();
                  break;
                  }
            }
      int staffIdx = 0;
      if (s.size() >= 24) {
            if (s[23].isDigit())
                  staffIdx = s.mid(23,1).toInt() - 1;
            }
      int pitch = table[step] + alter + (octave + 1) * 12;
      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;

      Chord* chord = (Chord*)chordRest;
      Note* note = new Note(score);
      note->setPitch(pitch);
      note->setTpcFromPitch();
      note->setTrack(staffIdx * VOICES + voice);
      chord->add(note);
      }

//---------------------------------------------------------
//   openSlur
//---------------------------------------------------------

void MuseData::openSlur(int idx, int tick, Staff* staff, int voice)
      {
      int staffIdx = staff->idx();
      if (slur[idx]) {
            qDebug("%06d: slur %d already open\n", tick, idx+1);
            return;
            }
      slur[idx] = new Slur(score);
      slur[idx]->setTick(tick);
      slur[idx]->setTrack(staffIdx * VOICES + voice);
      score->add(slur[idx]);
      }

//---------------------------------------------------------
//   closeSlur
//---------------------------------------------------------

void MuseData::closeSlur(int idx, int tick, Staff* staff, int voice)
      {
      int staffIdx = staff->idx();
      if (slur[idx]) {
            slur[idx]->setTick2(tick);
            slur[idx]->setTrack2(staffIdx * VOICES + voice);
            slur[idx] = 0;
            }
      else
            qDebug("%06d: slur %d not open\n", tick, idx+1);
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void MuseData::readNote(Part* part, const QString& s)
      {
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

      int step  = s[0].toLatin1() - 'A';
      int alter = 0;
      int octave = 0;
      for (int i = 1; i < 3; ++i) {
            if (s[i] == '#')
                  alter += 1;
            else if (s[i] == 'f')
                  alter -= 1;
            else if (s[i].isDigit()) {
                  octave = s.mid(i,1).toInt();
                  break;
                  }
            }
      MScore::Direction dir = MScore::AUTO;
      if (s.size() >= 23) {
            if (s[22] == 'u')
                  dir = MScore::UP;
            else if (s[22] == 'd')
                  dir = MScore::DOWN;
            }

      int staffIdx = 0;
      if (s.size() >= 24) {
            if (s[23].isDigit())
                  staffIdx = s.mid(23,1).toInt() - 1;
            }
      Staff* staff = part->staff(staffIdx);
      int gstaff   = staff->idx();

      int pitch = table[step] + alter + (octave + 1) * 12;
      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;
      int ticks = s.mid(5, 3).toInt();
      ticks     = (ticks * MScore::division + _division/2) / _division;
      int tick  = curTick;
      curTick  += ticks;

      Tuplet* tuplet = 0;
      if (s.size() >= 22) {
            int a = 1;
            int b = 1;
            if (s[19] != ' ') {
                  a = s[19].toLatin1() - '0';
                  if (a == 3 && s[20] != ':')
                        b = 2;
                  else {
                        b = s[21].toLatin1() - '0';
                        }
                  }
            if (a == 3 && b == 2) {       // triplet
                  if (chordRest && chordRest->tuplet() && ntuplet) {
                        tuplet = chordRest->tuplet();
                        }
                  else {
                        tuplet = new Tuplet(score);
                        tuplet->setTrack(gstaff * VOICES);
                        tuplet->setTick(tick);
                        ntuplet = a;
                        tuplet->setRatio(Fraction(a, b));
                        measure->add(tuplet);
                        }
                  }
            else if (a == 1 && b == 1)
                  ;
            else
                  qDebug("unsupported tuple %d/%d\n", a, b);
            }

      Chord* chord = new Chord(score);
      chordRest = chord;
      chord->setTrack(gstaff * VOICES);
      chord->setStemDirection(dir);
      if (tuplet) {
            chord->setTuplet(tuplet);
            tuplet->add(chord);
            --ntuplet;
            }
      TDuration d;
      d.setVal(ticks);
      chord->setDurationType(d);

      Segment* segment = measure->getSegment(chord, tick);

      voice = 0;
      for (; voice < VOICES; ++voice) {
            Element* e = segment->element(gstaff * VOICES + voice);
            if (e == 0) {
                  chord->setTrack(gstaff * VOICES + voice);
                  segment->add(chord);
                  break;
                  }
            }
      if (voice == VOICES) {
            qDebug("cannot allocate voice\n");
            delete chord;
            return;
            }
      Note* note = new Note(score);
      note->setPitch(pitch);
      note->setTpcFromPitch();
      note->setTrack(gstaff * VOICES + voice);
      chord->add(note);

      QString dynamics;
      QString an = s.mid(31, 11);
      for (int i = 0; i < an.size(); ++i) {
            if (an[i] == '(')
                  openSlur(0, tick, staff, voice);
            else if (an[i] == ')')
                  closeSlur(0, tick, staff, voice);
            else if (an[i] == '[')
                  openSlur(1, tick, staff, voice);
            else if (an[i] == ']')
                  closeSlur(1, tick, staff, voice);
            else if (an[i] == '{')
                  openSlur(2, tick, staff, voice);
            else if (an[i] == '}')
                  closeSlur(2, tick, staff, voice);
            else if (an[i] == 'z')
                  openSlur(3, tick, staff, voice);
            else if (an[i] == 'x')
                  closeSlur(3, tick, staff, voice);
            else if (an[i] == '.') {
                  Articulation* atr = new Articulation(score);
                  atr->setArticulationType(Articulation_Staccato);
                  chord->add(atr);
                  }
            else if (an[i] == '_') {
                  Articulation* atr = new Articulation(score);
                  atr->setArticulationType(Articulation_Tenuto);
                  chord->add(atr);
                  }
            else if (an[i] == 'v') {
                  Articulation* atr = new Articulation(score);
                  atr->setArticulationType(Articulation_Upbow);
                  chord->add(atr);
                  }
            else if (an[i] == 'n') {
                  Articulation* atr = new Articulation(score);
                  atr->setArticulationType(Articulation_Downbow);
                  chord->add(atr);
                  }
            else if (an[i] == 't') {
                  Articulation* atr = new Articulation(score);
                  atr->setArticulationType(Articulation_Trill);
                  chord->add(atr);
                  }
            else if (an[i] == 'F') {
                  Articulation* atr = new Articulation(score);
                  atr->setUp(true);
                  atr->setArticulationType(Articulation_Fermata);
                  chord->add(atr);
                  }
            else if (an[i] == 'E') {
                  Articulation* atr = new Articulation(score);
                  atr->setUp(false);
                  atr->setArticulationType(Articulation_Fermata);
                  chord->add(atr);
                  }
            else if (an[i] == 'O') {
                  // Articulation* atr = new Articulation(score);
                  // atr->setArticulationType(Articulation_Downbow);
                  // chord->add(atr);
                  qDebug("%06d: open string '%c' not implemented\n", tick, an[i].toLatin1());
                  }
            else if (an[i] == '&') {
                  // skip editorial level
                  if (i <= an.size() && an[i+1].isDigit())
                        ++i;
                  }
            else if (an[i] == 'p')
                  dynamics += "p";
            else if (an[i] == 'm')
                  dynamics += "m";
            else if (an[i] == 'f')
                  dynamics += "f";
            else if (an[i] == '-')        // tie
                  ;
            else if (an[i] == '*')        // start tuplet
                  ;
            else if (an[i] == '!')        // stop tuplet
                  ;
            else if (an[i] == '+')        // cautionary accidental
                  ;
            else if (an[i] == 'X')        // ???
                  ;
            else if (an[i] == ' ')
                  ;
            else {
                  qDebug("%06d: notation '%c' not implemented\n", tick, an[i].toLatin1());
                  }
            }
      if (!dynamics.isEmpty()) {
            Dynamic* dyn = new Dynamic(score);
            dyn->setDynamicType(dynamics);
            dyn->setTrack(gstaff * VOICES);
            Segment* s = measure->getSegment(Segment::SegChordRest, tick);
            s->add(dyn);
            }

      QString txt = s.mid(43, 36);
      if (!txt.isEmpty()) {
            QStringList sl = txt.split("|");
            int no = 0;
            foreach(QString w, sl) {
                  w = diacritical(w);
                  Lyrics* l = new Lyrics(score);
                  l->setText(w);
                  l->setNo(no++);
                  l->setTrack(gstaff * VOICES);
                  Segment* segment = measure->tick2segment(tick);
                  segment->add(l);
                  }
            }
      }

//---------------------------------------------------------
//   diacritical
// TODO: not complete
//---------------------------------------------------------

QString MuseData::diacritical(QString s)
      {
      struct TAB {
            const char* a;
            const char* b;
            } tab[] = {
            { "\\\\", "\\" },
            { "\\2s", "ß" },
            { "\\3a", "ä" },
            { "\\3o", "ö" },
            { "\\3u", "ü" },

            { "\\s2", "ß" },
            { "\\a3", "ä" },
            { "\\o3", "ö" },
            { "\\u3", "ü" },
            };
      for (unsigned int i = 0; i < sizeof(tab)/sizeof(*tab); ++i) {
            s = s.replace(tab[i].a, QString::fromUtf8(tab[i].b));
            }
      return s;
      }

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

void MuseData::readRest(Part* part, const QString& s)
      {
      int ticks = s.mid(5, 3).toInt();
      ticks     = (ticks * MScore::division + _division/2) / _division;

      int tick  = curTick;
      curTick  += ticks;

      int staffIdx = 0;
      if (s.size() >= 24) {
            if (s[23].isDigit())
                  staffIdx = s.mid(23,1).toInt() - 1;
            }
      Staff* staff = part->staff(staffIdx);
      int gstaff   = staff->idx();

      TDuration d;
      d.setVal(ticks);
      Rest* rest = new Rest(score, d);
      rest->setDuration(d.fraction());
      chordRest  = rest;
      rest->setTrack(gstaff * VOICES);
      Segment* segment = measure->getSegment(rest, tick);

      voice = 0;
      for (; voice < VOICES; ++voice) {
            Element* e = segment->element(gstaff * VOICES + voice);
            if (e == 0) {
                  rest->setTrack(gstaff * VOICES + voice);
                  segment->add(rest);
                  break;
                  }
            }
      if (voice == VOICES) {
            qDebug("cannot allocate voice\n");
            delete rest;
            return;
            }
      }

//---------------------------------------------------------
//   readBackup
//---------------------------------------------------------

void MuseData::readBackup(const QString& s)
      {
      int ticks = s.mid(5, 3).toInt();
      ticks     = (ticks * MScore::division + _division/2) / _division;
      if (s[0] == 'b')
            curTick  -= ticks;
      else
            curTick += ticks;
      }

//---------------------------------------------------------
//   createMeasure
//---------------------------------------------------------

Measure* MuseData::createMeasure()
      {
      for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (mb->type() != Element::MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            int st = m->tick();
            int l  = m->ticks();
            if (curTick == st)
                  return m;
            if (curTick > st && curTick < (st+l)) {
                  // irregular measure
#if 0 // TODO
                  Fraction f = score->sigmap()->timesig(st).fraction();
                  score->sigmap()->add(st, curTick - st, f);
                  score->sigmap()->add(curTick, f);
#endif
                  break;
                  }
            if (curTick < st + l) {
                  qDebug("cannot create measure at %d\n", curTick);
                  return 0;
                  }
            }
      Measure* measure  = new Measure(score);
      measure->setTick(curTick);

#if 0
      foreach(Staff* s, score->staves()) {
	      if (s->isTop()) {
      	      BarLine* barLine = new BarLine(score);
            	barLine->setStaff(s);
	            measure->setEndBarLine(barLine);
      	      }
            }
#endif
      score->add(measure);
      return measure;
      }

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

void MuseData::readPart(QStringList sl, Part* part)
      {
      int line = 10;
      QString s;
      for (; line < sl.size(); ++line) {
            s = sl[line];
            if (!s.isEmpty() && s[0] == '$')
                  break;
            }
      if (line >= sl.size()) {
            qDebug(" $ not found in part\n");
            return;
            }
      curTick = 0;
      slur[0] = 0;
      slur[1] = 0;
      slur[2] = 0;
      slur[3] = 0;
      measure = 0;
      measure = createMeasure();
      for (; line < sl.size(); ++line) {
            s = sl[line];
// qDebug("%6d: <%s>\n", curTick, qPrintable(s));
            char c = s[0].toLatin1();
            switch(c) {
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                  case 'G':
                        readNote(part, s);
                        break;
                  case ' ':         // chord
                        readChord(part, s);
                        break;
                  case 'r':
                        readRest(part, s);
                        break;
                  case 'g':         // grace note
                  case 'c':         // cue note
                  case 'f':         // basso continuo
                        break;
                  case 'b':         // backspace
                  case 'i':         // forward space
                        readBackup(s);
                        break;
                  case 'm':         // measure line / bar line
                        measure = createMeasure();
                        break;
                  case '*':         // musical direction
                        break;
                  case 'P':         // print suggestion
                        break;
                  case 'S':         // sound record
                        break;
                  case '$':
                        musicalAttribute(s, part);
                        break;
                  default:
                        qDebug("unknown record <%s>\n", qPrintable(s));
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   countStaves
//---------------------------------------------------------

int MuseData::countStaves(const QStringList& sl)
      {
      int staves = 1;
      for (int i = 10; i < sl.size(); ++i) {
            QString s = sl[i];
            char c = s[0].toLatin1();
            switch(c) {
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                  case 'G':
                  case 'r':
                        {
                        int staffIdx = 1;
                        if (s.size() >= 24) {
                              if (s[23].isDigit())
                                    staffIdx = s.mid(23,1).toInt();
                              }
                        if (staffIdx > staves)
                              staves = staffIdx;
                        }
                        break;
                  }
            }
      return staves;
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool MuseData::read(const QString& name)
      {
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly)) {
            qDebug("cannot open file <%s>\n", qPrintable(name));
            return false;
            }
      QTextStream ts(&fp);
      QStringList part;
      bool commentMode = false;
      for (;;) {
            QString s(ts.readLine());
            if (s.isNull())
                  break;
            if (s.isEmpty()) {
                  if (!commentMode)
                        part.append(QString(""));
                  continue;
                  }
            if (s[0] == '&') {
                  commentMode = !commentMode;
                  continue;
                  }
            if (commentMode)
                  continue;
            if (s[0] == '@')
                  continue;
            if (s[0] == '/') {
                  parts.append(part);

                  Part* mpart = new Part(score);
                  int staves  = countStaves(part);
                  for (int i = 0; i < staves; ++i) {
                        Staff* staff = new Staff(score, mpart, i);
                        mpart->insertStaff(staff);
                        score->staves().push_back(staff);
                        if ((staves == 2) && (i == 0)) {
                              staff->setBracket(0, BRACKET_BRACE);
                              staff->setBracketSpan(0, 2);
                              }
                        }
                  score->appendPart(mpart);
                  if(part.size() > 8)
                        mpart->setLongName(part[8]);
                  part.clear();
                  continue;
                  }
            if (s[0] == 'a') {
                  part.back().append(s.mid(1));
                  continue;
                  }
            part.append(s);
            }
      fp.close();
      return true;
      }

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

void MuseData::convert()
      {
      for (int pn = 0; pn < parts.size(); ++pn) {
            Part* part = (score->parts())[pn];
            readPart(parts[pn], part);
            }
#if 0
      // crash if system does not fit on page (too many instruments)

      Measure* measure = score->tick2measure(0);
      if (measure) {
            Text* text = new Text(score);
            text->setSubtype(TEXT_TITLE);
            text->setText(parts[0][6]);
            text->setText("mops");
            measure->add(text);
            text = new Text(score);
            text->setSubtype(TEXT_SUBTITLE);
            text->setText(parts[0][6]);
            measure->add(text);
            }
#endif
      }

//---------------------------------------------------------
//   importMuseData
//    return true on success
//---------------------------------------------------------

Score::FileError importMuseData(Score* score, const QString& name)
      {
      if(!QFileInfo(name).exists())
            return Score::FILE_NOT_FOUND;
      MuseData md(score);
      if (!md.read(name))
            return Score::FILE_ERROR;
      md.convert();
      return Score::FILE_NO_ERROR;
      }
}


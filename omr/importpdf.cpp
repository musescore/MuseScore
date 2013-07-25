//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: importmidi.cpp 2721 2010-02-15 19:41:28Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "importpdf.h"
#include "libmscore/score.h"
#include "omr/omr.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/rest.h"
#include "omr/omrpage.h"
#include "libmscore/segment.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/page.h"
#include "libmscore/clef.h"
#include "libmscore/bracket.h"
#include "libmscore/mscore.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/utils.h"
#include "libmscore/timesig.h"
#include "libmscore/keysig.h"

namespace Ms {

//---------------------------------------------------------
//   OmrState
//---------------------------------------------------------

class OmrState {
   public:
      Score* score = 0;
      Fraction timesig { 4, 4};
      int tick = 0;

      void importPdfPage(OmrPage* omrPage);
      int importPdfSystem(int tick, OmrSystem* omrSystem);
      void importPdfMeasure(OmrMeasure* m, const OmrSystem* omrSystem);
      };

//---------------------------------------------------------
//   importPdfMeasure
//---------------------------------------------------------

void OmrState::importPdfMeasure(OmrMeasure* m, const OmrSystem* omrSystem)
      {
      Measure* measure = new Measure(score);
      measure->setTick(tick);
      if (m->timesig()) {
            timesig = m->timesig()->timesig;
            score->sigmap()->add(tick, SigEvent(timesig));
            }
      measure->setTimesig(timesig);
      measure->setLen(timesig);

      for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
            if (tick == 0) {
                  const OmrStaff& omrStaff = omrSystem->staves()[staffIdx];
                  int keySigType = omrStaff.keySig().type;
                  KeySig* ks     = new KeySig(score);
                  ks->setSig(keySigType, keySigType);
                  ks->setTrack(staffIdx * VOICES);
                  Segment* s = measure->getSegment(Segment::SegKeySig, 0);
                  s->add(ks);
                  score->staff(staffIdx)->setKey(0, keySigType);
                  }

            if (m->timesig()) {
                  TimeSig* ts = new TimeSig(score);
                  Segment* s = measure->getSegment(Segment::SegTimeSig, tick);
                  ts->setSig(timesig);
                  ts->setTrack(staffIdx * VOICES);
                  s->add(ts);
                  }
            Fraction nl;
            QList<OmrChord>& chords = m->chords()[staffIdx];
            if (timesig == Fraction(3,8)) {
                  if (chords.size() == 1) {
                        chords[0].duration = TDuration(timesig);
                        }
                  else if (chords.size() == 3) {
                        int i = 0;
                        for (;i < 3; ++i) {
                              if (chords[i].duration.fraction() != Fraction(1,4))
                                    break;
                              }
                        if (i == 3) {
                              for (i = 0;i < 3; ++i) {
                                    chords[i].duration = TDuration(Fraction(1, 8));
                                    }
                              }
                        }
                  }
            foreach(const OmrChord& omrChord, chords) {
                  nl += omrChord.duration.fraction();
                  }
            bool notesOk = nl == timesig;

            if (notesOk) {
                  int ltick = 0;
                  foreach(const OmrChord& omrChord, chords) {
                        Chord* chord = new Chord(score);
                        chord->setDurationType(omrChord.duration);
                        chord->setDuration(omrChord.duration.fraction());
                        chord->setTrack(staffIdx * VOICES);
                        Segment* s = measure->getSegment(Segment::SegChordRest, tick + ltick);
                        s->add(chord);
                        int keyType = score->staff(staffIdx)->key(tick + ltick).accidentalType();

                        foreach (OmrNote* omrNote, omrChord.notes) {
                              Note* note = new Note(score);
                              ClefType clef = score->staff(staffIdx)->initialClef()._concertClef;
                              int pitch = line2pitch(omrNote->line, clef, keyType);
                              note->setPitch(pitch);
                              note->setTpcFromPitch();
                              chord->add(note);
                              }
                        ltick += omrChord.duration.ticks();
                        }
                  }
            else {
                  TDuration d(TDuration::V_MEASURE);
                  Segment* s = measure->getSegment(Segment::SegChordRest, measure->tick());
                  Rest* rest = new Rest(score, d);
                  rest->setDuration(timesig);
                  rest->setTrack(staffIdx * VOICES);
                  s->add(rest);
                  }
            }

      score->measures()->add(measure);
      tick += measure->timesig().ticks();
      }

//---------------------------------------------------------
//   importPdfSystem
//---------------------------------------------------------

int OmrState::importPdfSystem(int tick, OmrSystem* omrSystem)
      {
      for (int i = 0; i < omrSystem->measures().size(); ++i) {
            OmrMeasure* m = &omrSystem->measures()[i];
            importPdfMeasure(m, omrSystem);
            }
      LayoutBreak* b = new LayoutBreak(score);
      b->setLayoutBreakType(LayoutBreakType::LAYOUT_BREAK_LINE);
      score->lastMeasure()->add(b);
      return tick;
      }

//---------------------------------------------------------
//   importPdfPage
//---------------------------------------------------------

void OmrState::importPdfPage(OmrPage* omrPage)
      {
      TDuration d(TDuration::V_MEASURE);
      int tick         = 0;

      int nsystems = omrPage->systems().size();
      int n = nsystems == 0 ? 1 : nsystems;
      for (int k = 0; k < n; ++k) {
            int numMeasures = 1;
            if (k < nsystems) {
                  tick = importPdfSystem(tick, omrPage->system(k));
                  }
            else {
                  Measure* measure;
                  for (int i = 0; i < numMeasures; ++i) {
                        measure = new Measure(score);
                        measure->setTick(tick);

      		      Rest* rest = new Rest(score, d);
                        rest->setDuration(Fraction(4,4));
                        rest->setTrack(0);
                        Segment* s = measure->getSegment(Segment::SegChordRest, tick);
      	            s->add(rest);
      		      rest = new Rest(score, d);
                        rest->setDuration(Fraction(4,4));
                        rest->setTrack(4);
      	            s->add(rest);
                        score->measures()->add(measure);
                        tick += MScore::division * 4;
                        }
                  if (k < (nsystems-1)) {
                        LayoutBreak* b = new LayoutBreak(score);
                        b->setLayoutBreakType(LAYOUT_BREAK_LINE);
                        measure->add(b);
                        }
                  }
            }
      Measure* measure = score->lastMeasure();
      if (measure) {
            LayoutBreak* b = new LayoutBreak(score);
            b->setLayoutBreakType(LAYOUT_BREAK_PAGE);
            measure->add(b);
            }
      }

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

Score::FileError importPdf(Score* score, const QString& path)
      {
      Omr* omr = new Omr(path, score);
      if (!omr->readPdf()) {
            delete omr;
            return Score::FILE_BAD_FORMAT;
            }

      score->setOmr(omr);
      qreal sp = omr->spatiumMM();
      if (sp == 0.0)
            sp = 1.5;
      score->setSpatium(sp * MScore::DPMM);
      score->style()->set(ST_lastSystemFillLimit, 0.0);
      score->style()->set(ST_staffLowerBorder, 0.0);
      score->style()->set(ST_measureSpacing, 1.0);

      PageFormat pF;
      pF.copy(*score->pageFormat());
      pF.setEvenLeftMargin(5.0 * MScore::DPMM / MScore::DPI);
      pF.setEvenTopMargin(0);
      pF.setEvenBottomMargin(0);
      pF.setOddLeftMargin(5.0 * MScore::DPMM / MScore::DPI);
      pF.setOddTopMargin(0);
      pF.setOddBottomMargin(0);
      score->setPageFormat(pF);

      score->style()->set(ST_minSystemDistance,   Spatium(omr->systemDistance()));
      score->style()->set(ST_maxSystemDistance,   Spatium(omr->systemDistance()));
      score->style()->set(ST_akkoladeDistance,    Spatium(omr->staffDistance()));

      Part* part   = new Part(score);
      Staff* staff = new Staff(score, part, 0);
      part->staves()->push_back(staff);
      score->staves().insert(0, staff);
      staff = new Staff(score, part, 1);
      part->staves()->push_back(staff);
      score->staves().insert(1, staff);
      part->staves()->front()->setBarLineSpan(part->nstaves());
      score->insertPart(part, 0);

      //---set initial clefs

      OmrPage* omrPage = omr->pages().front();
      int staves = score->nstaves();
      for (int i = 0; i < staves; ++i) {
            Staff* staff = score->staff(i);
            const OmrStaff& omrStaff = omrPage->systems().front().staves()[i];
            staff->setInitialClef(omrStaff.clef().type);
            }

      OmrState state;
      state.score = score;
      foreach(OmrPage* omrPage, omr->pages())
            state.importPdfPage(omrPage);

      //---create bracket

      score->staff(0)->setBracket(0, BRACKET_BRACE);
      score->staff(0)->setBracketSpan(0, 2);

      score->setShowOmr(true);
      omr->page(0)->readHeader(score);
      score->rebuildMidiMapping();
      return Score::FILE_NO_ERROR;
      }
}


//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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
#include "libmscore/spacer.h"
#include "libmscore/box.h"
#include "libmscore/spatium.h"

namespace Ms {

//---------------------------------------------------------
//   OmrState
//---------------------------------------------------------

class OmrState {
   public:
      MasterScore* score = 0;
      Fraction timesig { 4, 4};
      Fraction tick = Fraction(0, 1);

      void importPdfPage(OmrPage* omrPage, qreal top);
      Fraction importPdfSystem(OmrSystem* omrSystem);
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
            score->sigmap()->add(tick.ticks(), SigEvent(timesig));
            }
      measure->setTimesig(timesig);
      measure->setTicks(timesig);
      TDuration d(TDuration::DurationType::V_MEASURE);
      Rest* rest;
      Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
      for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
            rest = new Rest(score, d);
            rest->setTicks(timesig);
            rest->setTrack(staffIdx*4);
            s->add(rest);
            }
#if 0
      for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
            if (tick == 0) {
                  const OmrStaff& omrStaff = omrSystem->staves()[staffIdx];
                  int keySigType = omrStaff.keySig().type;
                  KeySig* ks     = new KeySig(score);
                  //ks->setSig(keySigType, keySigType);
                  ks->setTrack(staffIdx * VOICES);
                Segment* s = measure->getSegment(SegmentType::KeySig, 0);
                  s->add(ks);
                  //score->staff(staffIdx)->setKey(0, keySigType);
                  }

            if (m->timesig()) {
                  TimeSig* ts = new TimeSig(score);
                Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
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
                  Fraction ltick = Fraction(0, 1);
                  foreach(const OmrChord& omrChord, chords) {
                        Chord* chord = new Chord(score);
                        chord->setDurationType(omrChord.duration);
                        chord->setDuration(omrChord.duration.fraction());
                        chord->setTrack(staffIdx * VOICES);
                      Segment* s = measure->getSegment(SegmentType::ChordRest, tick + ltick);
                        s->add(chord);
                        //int keyType = score->staff(staffIdx)->key(tick + ltick).accidentalType();

                        foreach (OmrNote* omrNote, omrChord.notes) {
                              Note* note = new Note(score);
                              //ClefType clef = score->staff(staffIdx)->initialClef()._concertClef;
                              //int pitch = line2pitch(omrNote->line, clef, keyType);
                              //note->setPitch(pitch);
                              note->setTpcFromPitch();
                              chord->add(note);
                              }
                        ltick += omrChord.duration.fraction();
                        }
                  }
            else {
                TDuration d(TDuration::DurationType::V_MEASURE);
                Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  Rest* rest = new Rest(score, d);
                  rest->setDuration(timesig);
                  rest->setTrack(staffIdx * VOICES);
                  s->add(rest);
                  }
            }
#endif

      score->measures()->add(measure);
      tick += measure->timesig();
      }

//---------------------------------------------------------
//   importPdfSystem
//---------------------------------------------------------

Fraction OmrState::importPdfSystem(OmrSystem* omrSystem)
      {
      for (int i = 0; i < omrSystem->measures().size(); ++i) {
            OmrMeasure* m = &omrSystem->measures()[i];
            importPdfMeasure(m, omrSystem);
            }

      if(score->lastMeasure()){
            LayoutBreak* b = new LayoutBreak(score);
            b->setLayoutBreakType(LayoutBreak::Type::LINE);
            score->lastMeasure()->add(b);
            }

      return tick;
      }

//---------------------------------------------------------
//   importPdfPage
//---------------------------------------------------------

void OmrState::importPdfPage(OmrPage* omrPage, qreal top)
      {
      TDuration d(TDuration::DurationType::V_MEASURE);

      int nsystems = omrPage->systems().size();
      if(nsystems == 0) return;

      //add top margin for alignment
      MeasureBase* first_measure = score->first();
      if (first_measure == 0 || first_measure->isVBox()) {
            VBox* vbox = new VBox(score);
            vbox->setNext(score->first());
            vbox->setTick(Fraction(0,1));
            vbox->setBoxHeight(Spatium(top));
            vbox->setBottomMargin(0);
            vbox->setBottomGap(0);
            score->measures()->add(vbox);
      }

      for (int k = 0; k < nsystems; ++k) {
            importPdfSystem(omrPage->system(nsystems - k - 1));
            }

      Measure* measure = score->lastMeasure();
      if (measure) {
            LayoutBreak* b = new LayoutBreak(score);
            b->setLayoutBreakType(LayoutBreak::Type::PAGE);
            measure->add(b);
            }

      measure = score->firstMeasure();
      if (measure) {
            if (!measure->vspacerUp(0)){
                  Spacer* spacer = new Spacer(score);
                  spacer->setSpacerType(SpacerType::UP);
                  spacer->setTrack(0);
                  measure->add(spacer);
                  }
            Spacer* sp = measure->vspacerUp(0);
            sp->layout();
            sp->setPos(sp->rxpos(), top);
            }
      }

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

Score::FileError importPdf(MasterScore* score, const QString& path)
      {
      Omr* omr = new Omr(path, score);
      if (!omr->readPdf()) {
            delete omr;
            return Score::FileError::FILE_BAD_FORMAT;
            }

      score->setOmr(omr);
      qreal sp = omr->spatiumMM();
      if (qFuzzyIsNull(sp))
            sp = 1.5;
      score->setSpatium(sp * DPMM);
      score->style().set(Sid::lastSystemFillLimit,  0.0);
      score->style().set(Sid::staffLowerBorder,     0.0);
      score->style().set(Sid::measureSpacing,       1.0);
      score->style().set(Sid::frameSystemDistance,  0);
      score->style().set(Sid::pageEvenLeftMargin,   5.0 * DPMM / DPI);
      score->style().set(Sid::pageEvenTopMargin,    0);
      score->style().set(Sid::pageEvenBottomMargin, 0);
      score->style().set(Sid::pageOddLeftMargin,    5.0 * DPMM / DPI);
      score->style().set(Sid::pageOddTopMargin,     0);
      score->style().set(Sid::pageOddBottomMargin,  0);
      score->style().set(Sid::minSystemDistance,    Spatium(omr->systemDistance()));
      score->style().set(Sid::maxSystemDistance,    Spatium(omr->systemDistance()));
      score->style().set(Sid::akkoladeDistance,     Spatium(omr->staffDistance()));

      Part* part   = new Part(score);
      OmrPage* omrPage = omr->pages().front();

      //may need to identify maximal number of staff per system and then high some of those
      if (omrPage->systems().size() > 0) {
            for (int i = 0; i < omrPage->systems().front().staves().size(); i++) {
                  Staff* staff = new Staff(score);
                  staff->setPart(part);
                  part->insertStaff(staff, -1);
                  score->staves().append(staff);
                  }
            }
      score->appendPart(part);

      OmrState state;
      state.score = score;
      for (OmrPage* omrPage1 : omr->pages()) {
            if (omrPage1->systems().count() == 0)
                  continue;
            OmrSystem system = omrPage1->systems().last();
            if (system.staves().count() == 0)
                  continue;
            OmrStaff staff = system.staves().first();
            qreal top = staff.top()/omr->spatium();
            state.importPdfPage(omrPage1, top);
            }

      //---create bracket

      //score->staff(0)->setBracket(0, BracketType::BRACE);
      //score->staff(0)->setBracketSpan(0, 2);

      score->setShowOmr(true);
      omr->page(0)->readHeader(score);
      score->rebuildMidiMapping();
      return Score::FileError::FILE_NO_ERROR;
      }
}


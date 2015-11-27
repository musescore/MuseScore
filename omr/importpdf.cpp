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
      int importPdfSystem(OmrSystem* omrSystem);
<<<<<<< HEAD
<<<<<<< HEAD
      void importPdfMeasure(OmrMeasure* m, const OmrSystem* omrSystem);
=======
      void importPdfMeasure(OmrMeasure* m/*, const OmrSystem* omrSystem*/);
>>>>>>> 8d0232d... debug skeleton creation
=======
      void importPdfMeasure(OmrMeasure* m, const OmrSystem* omrSystem);
>>>>>>> 21738fc... debugging omr
      };

//---------------------------------------------------------
//   importPdfMeasure
//---------------------------------------------------------

void OmrState::importPdfMeasure(OmrMeasure* m, const OmrSystem* omrSystem)
      {
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 21738fc... debugging omr
          Measure* measure = new Measure(score);
          measure->setTick(tick);
          if (m->timesig()) {
              timesig = m->timesig()->timesig;
              score->sigmap()->add(tick, SigEvent(timesig));
          }
          measure->setTimesig(timesig);
          measure->setLen(timesig);
          TDuration d(TDuration::DurationType::V_MEASURE);
          Rest* rest;
<<<<<<< HEAD
          Segment* s = measure->getSegment(Segment::Type::ChordRest, tick);
          for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
              
              rest = new Rest(score, d);
              rest->setDuration(timesig);
              rest->setTrack(staffIdx*4);
              s->add(rest);
          }

=======
//      Measure* measure = new Measure(score);
//      measure->setTick(tick);
//      if (m->timesig()) {
//            timesig = m->timesig()->timesig;
//            score->sigmap()->add(tick, SigEvent(timesig));
//            }
//      measure->setTimesig(timesig);
//      measure->setLen(timesig);
//
>>>>>>> 57b9dad... compile omr module
//      for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
//            if (tick == 0) {
//                  const OmrStaff& omrStaff = omrSystem->staves()[staffIdx];
//                  int keySigType = omrStaff.keySig().type;
//                  KeySig* ks     = new KeySig(score);
<<<<<<< HEAD
//                  //ks->setSig(keySigType, keySigType);
//                  ks->setTrack(staffIdx * VOICES);
//                Segment* s = measure->getSegment(Segment::Type::KeySig, 0);
//                  s->add(ks);
//                  //score->staff(staffIdx)->setKey(0, keySigType);
=======
//                  ks->setSig(keySigType, keySigType);
//                  ks->setTrack(staffIdx * VOICES);
//                  Segment* s = measure->getSegment(Segment::SegKeySig, 0);
//                  s->add(ks);
//                  score->staff(staffIdx)->setKey(0, keySigType);
>>>>>>> 57b9dad... compile omr module
//                  }
//
//            if (m->timesig()) {
//                  TimeSig* ts = new TimeSig(score);
<<<<<<< HEAD
//                Segment* s = measure->getSegment(Segment::Type::TimeSig, tick);
=======
//                  Segment* s = measure->getSegment(Segment::SegTimeSig, tick);
>>>>>>> 57b9dad... compile omr module
//                  ts->setSig(timesig);
//                  ts->setTrack(staffIdx * VOICES);
//                  s->add(ts);
//                  }
//            Fraction nl;
//            QList<OmrChord>& chords = m->chords()[staffIdx];
//            if (timesig == Fraction(3,8)) {
//                  if (chords.size() == 1) {
//                        chords[0].duration = TDuration(timesig);
//                        }
//                  else if (chords.size() == 3) {
//                        int i = 0;
//                        for (;i < 3; ++i) {
//                              if (chords[i].duration.fraction() != Fraction(1,4))
//                                    break;
//                              }
//                        if (i == 3) {
//                              for (i = 0;i < 3; ++i) {
//                                    chords[i].duration = TDuration(Fraction(1, 8));
//                                    }
//                              }
//                        }
//                  }
//            foreach(const OmrChord& omrChord, chords) {
//                  nl += omrChord.duration.fraction();
//                  }
//            bool notesOk = nl == timesig;
//
//            if (notesOk) {
//                  int ltick = 0;
//                  foreach(const OmrChord& omrChord, chords) {
//                        Chord* chord = new Chord(score);
//                        chord->setDurationType(omrChord.duration);
//                        chord->setDuration(omrChord.duration.fraction());
//                        chord->setTrack(staffIdx * VOICES);
<<<<<<< HEAD
//                      Segment* s = measure->getSegment(Segment::Type::ChordRest, tick + ltick);
//                        s->add(chord);
//                        //int keyType = score->staff(staffIdx)->key(tick + ltick).accidentalType();
//
//                        foreach (OmrNote* omrNote, omrChord.notes) {
//                              Note* note = new Note(score);
//                              //ClefType clef = score->staff(staffIdx)->initialClef()._concertClef;
//                              //int pitch = line2pitch(omrNote->line, clef, keyType);
//                              //note->setPitch(pitch);
=======
//                        Segment* s = measure->getSegment(Segment::SegChordRest, tick + ltick);
//                        s->add(chord);
//                        int keyType = score->staff(staffIdx)->key(tick + ltick).accidentalType();
//
//                        foreach (OmrNote* omrNote, omrChord.notes) {
//                              Note* note = new Note(score);
//                              ClefType clef = score->staff(staffIdx)->initialClef()._concertClef;
//                              int pitch = line2pitch(omrNote->line, clef, keyType);
//                              note->setPitch(pitch);
>>>>>>> 57b9dad... compile omr module
//                              note->setTpcFromPitch();
//                              chord->add(note);
//                              }
//                        ltick += omrChord.duration.ticks();
//                        }
//                  }
//            else {
<<<<<<< HEAD
//                TDuration d(TDuration::DurationType::V_MEASURE);
//                Segment* s = measure->getSegment(Segment::Type::ChordRest, measure->tick());
=======
//                  TDuration d(TDuration::V_MEASURE);
//                  Segment* s = measure->getSegment(Segment::SegChordRest, measure->tick());
>>>>>>> 57b9dad... compile omr module
//                  Rest* rest = new Rest(score, d);
//                  rest->setDuration(timesig);
//                  rest->setTrack(staffIdx * VOICES);
//                  s->add(rest);
//                  }
//            }
<<<<<<< HEAD

      score->measures()->add(measure);
      tick += measure->timesig().ticks();
=======
//
//      score->measures()->add(measure);
//      tick += measure->timesig().ticks();
>>>>>>> 57b9dad... compile omr module
=======
      Measure* measure = new Measure(score);
      measure->setTick(tick);
      if (m->timesig()) {
            timesig = m->timesig()->timesig;
            score->sigmap()->add(tick, SigEvent(timesig));
            }
      measure->setTimesig(timesig);
      measure->setLen(timesig);
      TDuration d(TDuration::DurationType::V_MEASURE);
          
    
          Rest* rest = new Rest(score, d);
          rest->setDuration(timesig);
          rest->setTrack(0);
=======
>>>>>>> 21738fc... debugging omr
          Segment* s = measure->getSegment(Segment::Type::ChordRest, tick);
          for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
              
              rest = new Rest(score, d);
              rest->setDuration(timesig);
              rest->setTrack(staffIdx*4);
              s->add(rest);
          }

//      for (int staffIdx = 0; staffIdx < omrSystem->staves().size(); ++staffIdx) {
//            if (tick == 0) {
//                  const OmrStaff& omrStaff = omrSystem->staves()[staffIdx];
//                  int keySigType = omrStaff.keySig().type;
//                  KeySig* ks     = new KeySig(score);
//                  //ks->setSig(keySigType, keySigType);
//                  ks->setTrack(staffIdx * VOICES);
//                Segment* s = measure->getSegment(Segment::Type::KeySig, 0);
//                  s->add(ks);
//                  //score->staff(staffIdx)->setKey(0, keySigType);
//                  }
//
//            if (m->timesig()) {
//                  TimeSig* ts = new TimeSig(score);
//                Segment* s = measure->getSegment(Segment::Type::TimeSig, tick);
//                  ts->setSig(timesig);
//                  ts->setTrack(staffIdx * VOICES);
//                  s->add(ts);
//                  }
//            Fraction nl;
//            QList<OmrChord>& chords = m->chords()[staffIdx];
//            if (timesig == Fraction(3,8)) {
//                  if (chords.size() == 1) {
//                        chords[0].duration = TDuration(timesig);
//                        }
//                  else if (chords.size() == 3) {
//                        int i = 0;
//                        for (;i < 3; ++i) {
//                              if (chords[i].duration.fraction() != Fraction(1,4))
//                                    break;
//                              }
//                        if (i == 3) {
//                              for (i = 0;i < 3; ++i) {
//                                    chords[i].duration = TDuration(Fraction(1, 8));
//                                    }
//                              }
//                        }
//                  }
//            foreach(const OmrChord& omrChord, chords) {
//                  nl += omrChord.duration.fraction();
//                  }
//            bool notesOk = nl == timesig;
//
//            if (notesOk) {
//                  int ltick = 0;
//                  foreach(const OmrChord& omrChord, chords) {
//                        Chord* chord = new Chord(score);
//                        chord->setDurationType(omrChord.duration);
//                        chord->setDuration(omrChord.duration.fraction());
//                        chord->setTrack(staffIdx * VOICES);
//                      Segment* s = measure->getSegment(Segment::Type::ChordRest, tick + ltick);
//                        s->add(chord);
//                        //int keyType = score->staff(staffIdx)->key(tick + ltick).accidentalType();
//
//                        foreach (OmrNote* omrNote, omrChord.notes) {
//                              Note* note = new Note(score);
//                              //ClefType clef = score->staff(staffIdx)->initialClef()._concertClef;
//                              //int pitch = line2pitch(omrNote->line, clef, keyType);
//                              //note->setPitch(pitch);
//                              note->setTpcFromPitch();
//                              chord->add(note);
//                              }
//                        ltick += omrChord.duration.ticks();
//                        }
//                  }
//            else {
//                TDuration d(TDuration::DurationType::V_MEASURE);
//                Segment* s = measure->getSegment(Segment::Type::ChordRest, measure->tick());
//                  Rest* rest = new Rest(score, d);
//                  rest->setDuration(timesig);
//                  rest->setTrack(staffIdx * VOICES);
//                  s->add(rest);
//                  }
//            }

      score->measures()->add(measure);
      tick += measure->timesig().ticks();
>>>>>>> 37b26ee... debug omr module
      }

//---------------------------------------------------------
//   importPdfSystem
//---------------------------------------------------------

int OmrState::importPdfSystem(OmrSystem* omrSystem)
      {
      for (int i = 0; i < omrSystem->measures().size(); ++i) {
            OmrMeasure* m = &omrSystem->measures()[i];
            importPdfMeasure(m, omrSystem);
            }
      LayoutBreak* b = new LayoutBreak(score);
      b->setLayoutBreakType(LayoutBreak::Type::LINE);
      score->lastMeasure()->add(b);
      return tick;
      }

//---------------------------------------------------------
//   importPdfPage
//---------------------------------------------------------

void OmrState::importPdfPage(OmrPage* omrPage)
      {
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 37b26ee... debug omr module
          TDuration d(TDuration::DurationType::V_MEASURE);
      int tick         = 0;

      int nsystems = omrPage->systems().size();
      int n = nsystems == 0 ? 1 : nsystems;
      for (int k = 0; k < n; ++k) {
            int numMeasures = 1;
            if (k < nsystems) {
<<<<<<< HEAD
<<<<<<< HEAD
                  tick = importPdfSystem(omrPage->system(k));
=======
                  tick = importPdfSystem(tick, omrPage->system(k));
>>>>>>> 37b26ee... debug omr module
=======
                  tick = importPdfSystem(omrPage->system(k));
>>>>>>> 8d0232d... debug skeleton creation
                  }
            else {
                  Measure* measure;
                  for (int i = 0; i < numMeasures; ++i) {
                        measure = new Measure(score);
                        measure->setTick(tick);

      		      Rest* rest = new Rest(score, d);
                        rest->setDuration(Fraction(4,4));
                        rest->setTrack(0);
                      Segment* s = measure->getSegment(Segment::Type::ChordRest, tick);
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
                      b->setLayoutBreakType(LayoutBreak::Type::LINE);
                        measure->add(b);
                        }
                  }
            }
      Measure* measure = score->lastMeasure();
      if (measure) {
            LayoutBreak* b = new LayoutBreak(score);
            b->setLayoutBreakType(LayoutBreak::Type::LINE);
            measure->add(b);
            }
<<<<<<< HEAD
=======
//      TDuration d(TDuration::V_MEASURE);
//      int tick         = 0;
//
//      int nsystems = omrPage->systems().size();
//      int n = nsystems == 0 ? 1 : nsystems;
//      for (int k = 0; k < n; ++k) {
//            int numMeasures = 1;
//            if (k < nsystems) {
//                  tick = importPdfSystem(tick, omrPage->system(k));
//                  }
//            else {
//                  Measure* measure;
//                  for (int i = 0; i < numMeasures; ++i) {
//                        measure = new Measure(score);
//                        measure->setTick(tick);
//
//      		      Rest* rest = new Rest(score, d);
//                        rest->setDuration(Fraction(4,4));
//                        rest->setTrack(0);
//                        Segment* s = measure->getSegment(Segment::SegChordRest, tick);
//      	            s->add(rest);
//      		      rest = new Rest(score, d);
//                        rest->setDuration(Fraction(4,4));
//                        rest->setTrack(4);
//      	            s->add(rest);
//                        score->measures()->add(measure);
//                        tick += MScore::division * 4;
//                        }
//                  if (k < (nsystems-1)) {
//                        LayoutBreak* b = new LayoutBreak(score);
//                        b->setLayoutBreakType(LAYOUT_BREAK_LINE);
//                        measure->add(b);
//                        }
//                  }
//            }
//      Measure* measure = score->lastMeasure();
//      if (measure) {
//            LayoutBreak* b = new LayoutBreak(score);
//            b->setLayoutBreakType(LAYOUT_BREAK_PAGE);
//            measure->add(b);
//            }
>>>>>>> 57b9dad... compile omr module
=======
>>>>>>> 37b26ee... debug omr module
      }

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

Score::FileError importPdf(Score* score, const QString& path)
      {
      Omr* omr = new Omr(path, score);
      if (!omr->readPdf()) {
            delete omr;
            return Score::FileError::FILE_BAD_FORMAT;
            }

      score->setOmr(omr);
      qreal sp = omr->spatiumMM();
      if (sp == 0.0)
            sp = 1.5;
<<<<<<< HEAD
<<<<<<< HEAD
      score->setSpatium(sp * DPMM);
      score->style()->set(StyleIdx::lastSystemFillLimit, 0.0);
      score->style()->set(StyleIdx::staffLowerBorder, 0.0);
      score->style()->set(StyleIdx::measureSpacing, 1.0);
=======
      score->setSpatium(sp * MScore::DPMM);
<<<<<<< HEAD
<<<<<<< HEAD
      //score->style()->set(ST_lastSystemFillLimit, 0.0);//liang
      //score->style()->set(ST_staffLowerBorder, 0.0);//liang
      //score->style()->set(ST_measureSpacing, 1.0);//liang
>>>>>>> 57b9dad... compile omr module
=======
          score->style()->set(StyleIdx::lastSystemFillLimit, 0.0);
=======
=======
      score->setSpatium(sp * DPMM);
>>>>>>> d9c0942... fix DPI and DPMM bugs
      score->style()->set(StyleIdx::lastSystemFillLimit, 0.0);
<<<<<<< HEAD
>>>>>>> 995a1ff... debugging omr module
          score->style()->set(StyleIdx::staffLowerBorder, 0.0);
          score->style()->set(StyleIdx::measureSpacing, 1.0);
>>>>>>> 55570c6... activate pdf input and omr panel
=======
      score->style()->set(StyleIdx::staffLowerBorder, 0.0);
      score->style()->set(StyleIdx::measureSpacing, 1.0);
>>>>>>> 8d0232d... debug skeleton creation

      PageFormat pF;
      pF.copy(*score->pageFormat());
      pF.setEvenLeftMargin(5.0 * DPMM / DPI);
      pF.setEvenTopMargin(0);
      pF.setEvenBottomMargin(0);
      pF.setOddLeftMargin(5.0 * DPMM / DPI);
      pF.setOddTopMargin(0);
      pF.setOddBottomMargin(0);
      score->setPageFormat(pF);

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
      score->style()->set(StyleIdx::minSystemDistance,   Spatium(omr->systemDistance()));
      score->style()->set(StyleIdx::maxSystemDistance,   Spatium(omr->systemDistance()));
      score->style()->set(StyleIdx::akkoladeDistance,    Spatium(omr->staffDistance()));
          
          
          //return Score::FileError::FILE_NO_ERROR;

      Part* part   = new Part(score);
          OmrPage* omrPage = omr->pages().front();
         
          if(omrPage->systems().size() > 0){
              for (int i = 0; i < omrPage->systems().front().staves().size(); i++) {
                  Staff* staff = new Staff(score);
                  staff->setPart(part);
                  part->insertStaff(staff, -1);
                  score->staves().append(staff);
              }
          }
          score->appendPart(part);
    
    
//      Staff* staff1 = new Staff(score);
//      part->staves()->push_back(staff1);
//    
//      score->staves().insert(0, staff1);
////      Staff* staff2 = new Staff(score);
////      part->staves()->push_back(staff2);
////      score->staves().insert(1, staff2);
////          
//      staff1->setPart(part);
////      staff2->setPart(part);
////      
//      part->staves()->front()->setBarLineSpan(part->nstaves());
//      score->insertPart(part, 0);

//      //---set initial clefs
//
//
=======
      //score->style()->set(ST_minSystemDistance,   Spatium(omr->systemDistance()));
      //score->style()->set(ST_maxSystemDistance,   Spatium(omr->systemDistance()));
      //score->style()->set(ST_akkoladeDistance,    Spatium(omr->staffDistance()));
=======
          score->style()->set(StyleIdx::minSystemDistance,   Spatium(omr->systemDistance()));
          score->style()->set(StyleIdx::maxSystemDistance,   Spatium(omr->systemDistance()));
          score->style()->set(StyleIdx::akkoladeDistance,    Spatium(omr->staffDistance()));
>>>>>>> 55570c6... activate pdf input and omr panel
=======
      score->style()->set(StyleIdx::minSystemDistance,   Spatium(omr->systemDistance()));
      score->style()->set(StyleIdx::maxSystemDistance,   Spatium(omr->systemDistance()));
      score->style()->set(StyleIdx::akkoladeDistance,    Spatium(omr->staffDistance()));
>>>>>>> e3f6219... fix bugs in text size for pattern match

      Part* part   = new Part(score);
          OmrPage* omrPage = omr->pages().front();
          for (int i = 0; i < omrPage->systems().front().staves().size(); i++) {
              Staff* staff = new Staff(score);
              staff->setPart(part);
              part->insertStaff(staff, -1);
              score->staves().append(staff);
          }
          score->appendPart(part);
    
    
//      Staff* staff1 = new Staff(score);
//      part->staves()->push_back(staff1);
//    
//      score->staves().insert(0, staff1);
////      Staff* staff2 = new Staff(score);
////      part->staves()->push_back(staff2);
////      score->staves().insert(1, staff2);
////          
//      staff1->setPart(part);
////      staff2->setPart(part);
////      
//      part->staves()->front()->setBarLineSpan(part->nstaves());
//      score->insertPart(part, 0);

<<<<<<< HEAD
      //---set initial clefs

<<<<<<< HEAD
//      OmrPage* omrPage = omr->pages().front();
>>>>>>> 57b9dad... compile omr module
=======
//      //---set initial clefs
//
//
>>>>>>> 8d0232d... debug skeleton creation
//      int staves = score->nstaves();
//      for (int i = 0; i < staves; ++i) {
//            Staff* staff = score->staff(i);
//            const OmrStaff& omrStaff = omrPage->systems().front().staves()[i];
<<<<<<< HEAD
<<<<<<< HEAD
//            //staff->setInitialClef(omrStaff.clef().type);
//            }
=======
      OmrPage* omrPage = omr->pages().front();
      int staves = score->nstaves();
      for (int i = 0; i < staves; ++i) {
            Staff* staff = score->staff(i);
            const OmrStaff& omrStaff = omrPage->systems().front().staves()[i];
            //staff->setInitialClef(omrStaff.clef().type);
            }
>>>>>>> 55570c6... activate pdf input and omr panel
=======
//            //staff->setInitialClef(omrStaff.clef().type);
//            }
>>>>>>> 8d0232d... debug skeleton creation

      OmrState state;
      state.score = score;
      foreach(OmrPage* omrPage, omr->pages())
            state.importPdfPage(omrPage);
<<<<<<< HEAD

      //---create bracket

      //score->staff(0)->setBracket(0, BracketType::BRACE);
      //score->staff(0)->setBracketSpan(0, 2);
=======
//            staff->setInitialClef(omrStaff.clef().type);
//            }
//
//      OmrState state;
//      state.score = score;
//      foreach(OmrPage* omrPage, omr->pages())
//            //state.importPdfPage(omrPage);

      //---create bracket

      //score->staff(0)->setBracket(0, BRACKET_BRACE);
      //score->staff(0)->setBracketSpan(0, 2);//liang
>>>>>>> 57b9dad... compile omr module
=======

      //---create bracket

<<<<<<< HEAD
      score->staff(0)->setBracket(0, BracketType::BRACE);
      score->staff(0)->setBracketSpan(0, 2);
>>>>>>> 55570c6... activate pdf input and omr panel
=======
      //score->staff(0)->setBracket(0, BracketType::BRACE);
      //score->staff(0)->setBracketSpan(0, 2);
>>>>>>> 8d0232d... debug skeleton creation

      score->setShowOmr(true);
      omr->page(0)->readHeader(score);
      score->rebuildMidiMapping();
      return Score::FileError::FILE_NO_ERROR;
      }
}


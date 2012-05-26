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

//---------------------------------------------------------
//   noteCompare
//---------------------------------------------------------

static bool noteCompare(OmrNote* n1, OmrNote* n2)
      {
      return n1->r.x() < n2->r.x();
      }

//---------------------------------------------------------
//   importPdfMeasure
//---------------------------------------------------------

static void importPdfMeasure(Measure* measure, const OmrSystem* omrSystem, int staffIdx, int mi)
      {
      Score* score = measure->score();
      const OmrStaff& staff = omrSystem->staves().at(staffIdx);
      QList<OmrNote*> notes;
      int x1 = omrSystem->barLines[mi].x1();
      int x2 = omrSystem->barLines[mi + 1].x1();

      foreach (OmrNote* note, staff.notes()) {
            if (note->r.x() >= x1 && note->r.right() <= x2)
                  notes.append(note);
            }
      qSort(notes.begin(), notes.end(), noteCompare);
      Fraction f(measure->len());
      Fraction nl(0, 4);
      foreach(OmrNote* omrNote, notes) {
            if (omrNote->sym == quartheadSym)
                  nl += Fraction(1, 4);
            else if (omrNote->sym == halfheadSym)
                  nl += Fraction(2, 4);
            else
                  printf("unknown note head symbol %d\n", omrNote->sym);
            }
      if (nl == f) {
            int tick = measure->tick();
            foreach(OmrNote* omrNote, notes) {
                  TDuration d(TDuration::V_QUARTER);
                  Fraction duration;
                  if (omrNote->sym == quartheadSym) {
                        duration = Fraction(1,4);
                        d.setType(TDuration::V_QUARTER);
                        }
                  else {
                        duration = Fraction(2,4);
                        d.setType(TDuration::V_HALF);
                        }
                  Chord* chord = new Chord(score);
                  chord->setDurationType(d);
                  chord->setDuration(duration);
                  chord->setTrack(staffIdx * VOICES);
                  Note* note = new Note(score);
                  int clef = staffIdx == 0 ? CLEF_G : CLEF_F;
                  int pitch = line2pitch(omrNote->line, clef, 0);
                  note->setPitch(pitch);
                  note->setTpcFromPitch();
                  chord->add(note);

                  Segment* s = measure->getSegment(SegChordRest, tick);
                  s->add(chord);
                  tick += duration.ticks();
                  }
            }
      else {
            TDuration d(TDuration::V_MEASURE);
            Segment* s = measure->getSegment(SegChordRest, measure->tick());
            Rest* rest = new Rest(score, d);
            rest->setDuration(Fraction(4,4));
            rest->setTrack(staffIdx * VOICES);
            s->add(rest);
            }
      }

//---------------------------------------------------------
//   importPdfSystem
//---------------------------------------------------------

static int importPdfSystem(Score* score, int tick, const OmrSystem* omrSystem)
      {
      int numMeasures = 1;
      numMeasures = omrSystem->barLines.size() - 1;
      if (numMeasures < 1)
            numMeasures = 1;
      else if (numMeasures > 50)    // sanity check
            numMeasures = 50;

      for (int i = 0; i < numMeasures; ++i) {
            Measure* measure = new Measure(score);
            measure->setTick(tick);
            for (int staffIdx = 0; staffIdx < omrSystem->nstaves(); ++staffIdx)
                  importPdfMeasure(measure, omrSystem, staffIdx, i);
            score->measures()->add(measure);
            tick += MScore::division * 4;
            }
      LayoutBreak* b = new LayoutBreak(score);
      b->setSubtype(LAYOUT_BREAK_LINE);
      score->lastMeasure()->add(b);
      return tick;
      }

//---------------------------------------------------------
//   importPdfPage
//---------------------------------------------------------

static void importPdfPage(Score* score, const OmrPage* omrPage)
      {
      TDuration d(TDuration::V_MEASURE);
      int tick         = 0;

      int nsystems = omrPage->systems().size();
      int n = nsystems == 0 ? 1 : nsystems;
      for (int k = 0; k < n; ++k) {
            int numMeasures = 1;
            if (k < nsystems) {
                  const OmrSystem& omrSystem = omrPage->systems().at(k);
                  numMeasures = omrSystem.barLines.size() - 1;
                  if (numMeasures < 1)
                        numMeasures = 1;
                  else if (numMeasures > 50)    // sanity check
                        numMeasures = 50;
                  tick = importPdfSystem(score, tick, &omrSystem);
                  }
            else {
                  Measure* measure;
                  for (int i = 0; i < numMeasures; ++i) {
                        measure = new Measure(score);
                        measure->setTick(tick);

      		      Rest* rest = new Rest(score, d);
                        rest->setDuration(Fraction(4,4));
                        rest->setTrack(0);
                        Segment* s = measure->getSegment(SegChordRest, tick);
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
                        b->setSubtype(LAYOUT_BREAK_LINE);
                        measure->add(b);
                        }
                  }
            }
      Measure* measure = score->lastMeasure();
      if (measure) {
            LayoutBreak* b = new LayoutBreak(score);
            b->setSubtype(LAYOUT_BREAK_PAGE);
            measure->add(b);
            }
      }

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

bool importPdf(Score* score, const QString& path)
      {
      Omr* omr = new Omr(path, score);
      if (!omr->readPdf()) {
            delete omr;
            return false;
            }
      score->setOmr(omr);
      qreal sp = omr->spatiumMM();
      if (sp == 0.0)
            sp = 1.5;
      score->setSpatium(sp * MScore::DPMM);
      score->style()->set(StyleVal(ST_lastSystemFillLimit, 0.0));
      score->style()->set(StyleVal(ST_staffLowerBorder, 0.0));
      score->style()->set(StyleVal(ST_measureSpacing, 1.0));

      PageFormat pF(*score->pageFormat());
      pF.setEvenLeftMargin(5.0 * MScore::DPMM / MScore::DPI);
      pF.setEvenTopMargin(0);
      pF.setEvenBottomMargin(0);
      pF.setOddLeftMargin(5.0 * MScore::DPMM / MScore::DPI);
      pF.setOddTopMargin(0);
      pF.setOddBottomMargin(0);
      score->setPageFormat(pF);

      score->style()->set(StyleVal(ST_minSystemDistance,   Spatium(omr->systemDistance())));
      score->style()->set(StyleVal(ST_maxSystemDistance,   Spatium(omr->systemDistance())));
      score->style()->set(StyleVal(ST_akkoladeDistance, Spatium(omr->staffDistance())));

      Part* part   = new Part(score);
      Staff* staff = new Staff(score, part, 0);
      part->staves()->push_back(staff);
      score->staves().insert(0, staff);
      staff = new Staff(score, part, 1);
      part->staves()->push_back(staff);
      score->staves().insert(1, staff);
      part->staves()->front()->setBarLineSpan(part->nstaves());
      score->insertPart(part, 0);

      foreach(const OmrPage* omrPage, omr->pages())
            importPdfPage(score, omrPage);

      //---create bracket

      score->staff(0)->setBracket(0, BRACKET_AKKOLADE);
      score->staff(0)->setBracketSpan(0, 2);

      //---create clefs

      Measure* measure = score->firstMeasure();
      if (measure) {
            Clef* clef = new Clef(score);
            clef->setClefType(CLEF_G);
            clef->setTrack(0);
            Segment* segment = measure->getSegment(SegClef, 0);
            segment->add(clef);

            clef = new Clef(score);
            clef->setClefType(CLEF_F);
            clef->setTrack(4);
            segment->add(clef);
            }

      score->setShowOmr(true);
      omr->page(0)->readHeader(score);
      score->rebuildMidiMapping();
      return true;
      }


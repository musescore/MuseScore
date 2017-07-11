
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "scoreview.h"
#include "texttools.h"
#include "libmscore/chordrest.h"
#include "libmscore/harmony.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"

namespace Ms {

//---------------------------------------------------------
//   harmonyTab
//---------------------------------------------------------

void ScoreView::harmonyTab(bool back)
      {
      Harmony* harmony = toHarmony(editData.element);
      if (!harmony->parent() || harmony->parent()->type() != ElementType::SEGMENT){
            qDebug("harmonyTab: no segment parent");
            return;
            }
      int track        = harmony->track();
      Segment* segment = static_cast<Segment*>(harmony->parent());
      if (!segment) {
            qDebug("harmonyTicksTab: no segment");
            return;
            }

      // moving to next/prev measure

      Measure* measure = segment->measure();
      if (measure) {
            if (back)
                  measure = measure->prevMeasure();
            else
                  measure = measure->nextMeasure();
            }
      if (!measure) {
            qDebug("harmonyTab: no prev/next measure");
            return;
            }

      segment = measure->findSegment(SegmentType::ChordRest, measure->tick());
      if (!segment) {
            qDebug("harmonyTab: no ChordRest segment as measure");
            return;
            }

      changeState(ViewState::NORMAL);

      _score->startCmd();

      // search for next chord name
      harmony = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->isHarmony() && e->track() == track) {
                  Harmony* h = toHarmony(e);
                  harmony = h;
                  break;
                  }
            }

      if (!harmony) {
            harmony = new Harmony(_score);
            harmony->setTrack(track);
            harmony->setParent(segment);
            _score->undoAddElement(harmony);
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, Grip::NO_GRIP);
//      mscore->changeState(mscoreState());

      adjustCanvasPosition(harmony, false);
//TODO-edit      toHarmony(editData.element)->moveCursorToEnd();
      _score->update();
      }

//---------------------------------------------------------
//   harmonyBeatsTab
//    manages [;:], moving forward or back to the next beat
//    and Space/Shift-Space, to stop at next note, rest, harmony or beat.
//---------------------------------------------------------

void ScoreView::harmonyBeatsTab(bool noterest, bool back)
      {
      Harmony* harmony = toHarmony(editData.element);
      int track        = harmony->track();
      Segment* segment = toSegment(harmony->parent());
      if (!segment) {
            qDebug("harmonyBeatsTab: no segment");
            return;
            }
      Measure* measure = segment->measure();
      int tick = segment->tick();

      if (back && tick == measure->tick()) {
            // previous bar, if any
            measure = measure->prevMeasure();
            if (!measure) {
                  qDebug("harmonyBeatsTab: no previous measure");
                  return;
                  }
            }

      Fraction f = measure->len();
      int ticksPerBeat = f.ticks() / ((f.numerator()>3 && (f.numerator()%3)==0 && f.denominator()>4) ? f.numerator()/3 : f.numerator());
      int tickInBar = tick - measure->tick();
      int newTick   = measure->tick() + ((tickInBar + (back?-1:ticksPerBeat)) / ticksPerBeat) * ticksPerBeat;

      // look for next/prev beat, note, rest or chord
      for (;;) {
            segment = back ? segment->prev1(SegmentType::ChordRest) : segment->next1(SegmentType::ChordRest);

            if (!segment || (back ? (segment->tick() < newTick) : (segment->tick() > newTick))) {
                  // no segment or moved past the beat - create new segment
                  if (!back && newTick >= measure->tick() + f.ticks()) {
                        // next bar, if any
                        measure = measure->nextMeasure();
                        if (!measure) {
                              qDebug("harmonyBeatsTab: no next measure");
                              return;
                              }
                        }
                  segment = new Segment(measure, SegmentType::ChordRest, newTick - measure->tick());
                  if (!segment) {
                        qDebug("harmonyBeatsTab: no prev segment");
                        return;
                        }
                  _score->undoAddElement(segment);
                  break;
                  }

            if (segment->tick() == newTick)
                  break;

            if (noterest) {
                  int minTrack = (track / VOICES ) * VOICES;
                  int maxTrack = minTrack + (VOICES-1);
                  if (segment->findAnnotationOrElement(ElementType::HARMONY, minTrack, maxTrack))
                        break;
                  }
            }

      changeState(ViewState::NORMAL);

      _score->startCmd();

      // search for next chord name
      harmony = 0;
      foreach (Element* e, segment->annotations()) {
            if (e->isHarmony() && e->track() == track) {
                  Harmony* h = toHarmony(e);
                  harmony = h;
                  break;
                  }
            }

      if (!harmony) {
            harmony = new Harmony(_score);
            harmony->setTrack(track);
            harmony->setParent(segment);
            _score->undoAddElement(harmony);
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(harmony, false);
//TODO-edit      toHarmony(editData.element)->moveCursorToEnd();
      _score->update();
      }

//---------------------------------------------------------
//   harmonyTicksTab
//    manages [Ctrl] [1]-[9], moving forward the given number of ticks
//---------------------------------------------------------

void ScoreView::harmonyTicksTab(int ticks)
      {
      Harmony* harmony = static_cast<Harmony*>(editData.element);
      int track         = harmony->track();
      Segment* segment = static_cast<Segment*>(harmony->parent());
      if (!segment) {
            qDebug("harmonyTicksTab: no segment");
            return;
            }
      Measure* measure = segment->measure();

      int newTick   = segment->tick() + ticks;

      // find the measure containing the target tick
      while (newTick >= measure->tick() + measure->ticks()) {
            measure = measure->nextMeasure();
            if (!measure) {
                  qDebug("harmonyTicksTab: no next measure");
                  return;
                  }
            }

      // look for a segment at this tick; if none, create one
      while (segment && segment->tick() < newTick)
            segment = segment->next1(SegmentType::ChordRest);
      if (!segment || segment->tick() > newTick) {      // no ChordRest segment at this tick
            segment = new Segment(measure, SegmentType::ChordRest, newTick - measure->tick());
            if (!segment) {
                  qDebug("harmonyTicksTab: no next segment");
                  return;
                  }
            _score->undoAddElement(segment);
            }

      changeState(ViewState::NORMAL);

      _score->startCmd();

      // search for next chord name
      harmony = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->type() == ElementType::HARMONY && e->track() == track) {
                  Harmony* h = static_cast<Harmony*>(e);
                  harmony = h;
                  break;
                  }
            }

      if (!harmony) {
            harmony = new Harmony(_score);
            harmony->setTrack(track);
            harmony->setParent(segment);
            _score->undoAddElement(harmony);
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(harmony, false);
//TODO-edit      ((Harmony*)editData.element)->moveCursorToEnd();
      _score->update();
      }

//---------------------------------------------------------
//   harmonyEndEdit
//---------------------------------------------------------

void ScoreView::harmonyEndEdit()
      {
      Harmony* harmony = static_cast<Harmony*>(editData.element);

      if (harmony->empty())
            _score->undoRemoveElement(harmony);
      }

}


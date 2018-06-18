
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
      if (!harmony->parent() || !harmony->parent()->isSegment()) {
            qDebug("no segment parent");
            return;
            }
      int track        = harmony->track();
      Segment* segment = toSegment(harmony->parent());
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
            qDebug("no prev/next measure");
            return;
            }

      segment = measure->findSegment(SegmentType::ChordRest, measure->tick());
      if (!segment) {
            qDebug("no ChordRest segment as measure");
            return;
            }

      changeState(ViewState::NORMAL);

      // search for next chord name
      harmony = 0;
      for (Element* e : segment->annotations()) {
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
            _score->startCmd();
            _score->undoAddElement(harmony);
            _score->endCmd();
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, Grip::NO_GRIP);

      adjustCanvasPosition(harmony, false);
      TextCursor* cursor = harmony->cursor(editData);
      cursor->moveCursorToEnd();
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
            qDebug("no segment");
            return;
            }
      Measure* measure = segment->measure();
      int tick = segment->tick();

      if (back && tick == measure->tick()) {
            // previous bar, if any
            measure = measure->prevMeasure();
            if (!measure) {
                  qDebug("no previous measure");
                  return;
                  }
            }

      Fraction f = measure->len();
      int ticksPerBeat = f.ticks() / ((f.numerator()>3 && (f.numerator()%3)==0 && f.denominator()>4) ? f.numerator()/3 : f.numerator());
      int tickInBar = tick - measure->tick();
      int newTick   = measure->tick() + ((tickInBar + (back?-1:ticksPerBeat)) / ticksPerBeat) * ticksPerBeat;

      changeState(ViewState::NORMAL);

      _score->startCmd();
      // look for next/prev beat, note, rest or chord
      for (;;) {
            segment = back ? segment->prev1(SegmentType::ChordRest) : segment->next1(SegmentType::ChordRest);

            if (!segment || (back ? (segment->tick() < newTick) : (segment->tick() > newTick))) {
                  // no segment or moved past the beat - create new segment
                  if (!back && newTick >= measure->tick() + f.ticks()) {
                        // next bar, if any
                        measure = measure->nextMeasure();
                        if (!measure) {
                              qDebug("no next measure");
                              return;
                              }
                        }
                  segment = new Segment(measure, SegmentType::ChordRest, newTick - measure->tick());
                  if (!segment) {
                        qDebug("no prev segment");
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

      // search for next chord name
      harmony = 0;
      for (Element* e : segment->annotations()) {
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
      _score->endCmd();

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, Grip::NO_GRIP);

      adjustCanvasPosition(harmony, false);
      TextCursor* cursor = harmony->cursor(editData);
      cursor->moveCursorToEnd();
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
      Segment* segment = toSegment(harmony->parent());
      if (!segment) {
            qDebug("no segment");
            return;
            }
      Measure* measure = segment->measure();

      int newTick   = segment->tick() + ticks;

      // find the measure containing the target tick
      while (newTick >= measure->tick() + measure->ticks()) {
            measure = measure->nextMeasure();
            if (!measure) {
                  qDebug("no next measure");
                  return;
                  }
            }

      // look for a segment at this tick; if none, create one
      while (segment && segment->tick() < newTick)
            segment = segment->next1(SegmentType::ChordRest);
      if (!segment || segment->tick() > newTick) {      // no ChordRest segment at this tick
            segment = new Segment(measure, SegmentType::ChordRest, newTick - measure->tick());
            _score->startCmd();
            _score->undoAddElement(segment);
            _score->endCmd();
            }

      changeState(ViewState::NORMAL);

      // search for next chord name
      harmony = 0;
      for (Element* e : segment->annotations()) {
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
            _score->startCmd();
            _score->undoAddElement(harmony);
            _score->endCmd();
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, Grip::NO_GRIP);

      adjustCanvasPosition(harmony, false);
      TextCursor* cursor = harmony->cursor(editData);
      cursor->moveCursorToEnd();
      _score->update();
      }

//---------------------------------------------------------
//   harmonyEndEdit
//---------------------------------------------------------

void ScoreView::harmonyEndEdit()
      {
      Harmony* harmony = toHarmony(editData.element);

      if (harmony->empty()) {
            _score->startCmd();
            _score->undoRemoveElement(harmony);
            _score->endCmd();
            }
      }

}


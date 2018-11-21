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
#include "libmscore/figuredbass.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreView::figuredBassEndEdit
//    derived from harmonyEndEdit()
//    remove the FB if empty
//---------------------------------------------------------

void ScoreView::figuredBassEndEdit()
      {
      FiguredBass* fb = static_cast<FiguredBass*>(editData.element);

      if (fb->empty())
            _score->undoRemoveElement(fb);
      }

//---------------------------------------------------------
//   ScoreView::figuredBassTab
//    derived from harmonyTab() (for Harmony)
//    manages [Space] / [Shift][Space] keys, moving editing to FB of next/prev ChordRest
//    and [Tab] / [Shift][Tab] keys, moving to FB of next/prev measure
//---------------------------------------------------------

void ScoreView::figuredBassTab(bool bMeas, bool bBack)
      {
      FiguredBass* fb   = (FiguredBass*)editData.element;
      Segment* nextSegm;
      Segment* segm     = fb->segment();
      int track         = fb->track();

      if (!segm) {
            qDebug("figuredBassTab: no segment");
            return;
            }

      // if moving to next/prev measure

      if (bMeas) {
            Measure* meas = segm->measure();
            if (meas) {
                  if (bBack)
                        meas = meas->prevMeasure();
                  else
                        meas = meas->nextMeasure();
                  }
            if (!meas) {
                  qDebug("figuredBassTab: no prev/next measure");
                  return;
                  }
            // find initial ChordRest segment
            nextSegm = meas->findSegment(SegmentType::ChordRest, meas->tick());
            if (!nextSegm) {
                  qDebug("figuredBassTab: no ChordRest segment at measure");
                  return;
                  }
            }

      // if moving to next/prev chord segment

      else {
            // search next chord segment in same staff
            nextSegm = bBack ? segm->prev1(SegmentType::ChordRest) : segm->next1(SegmentType::ChordRest);
            int minTrack = (track / VOICES ) * VOICES;
            int maxTrack = minTrack + (VOICES-1);

            while (nextSegm) {                   // look for a ChordRest in the compatible track range
                  if(nextSegm->findAnnotationOrElement(ElementType::FIGURED_BASS, minTrack, maxTrack))
                        break;
                  nextSegm = bBack ? nextSegm->prev1(SegmentType::ChordRest) : nextSegm->next1(SegmentType::ChordRest);
                  }

            if (!nextSegm) {
                  qDebug("figuredBassTab: no prev/next segment");
                  return;
                  }
            }

      changeState(ViewState::NORMAL);

      bool bNew;
      // add a (new) FB element, using chord duration as default duration
      FiguredBass * fbNew = FiguredBass::addFiguredBassToSegment(nextSegm, track, 0, &bNew);
      if (bNew) {
            _score->startCmd();
            _score->undoAddElement(fbNew);
            _score->endCmd();
            }
      _score->select(fbNew, SelectType::SINGLE, 0);
      startEdit(fbNew, Grip::NO_GRIP);

      mscore->changeState(mscoreState());
      adjustCanvasPosition(fbNew, false);
      fbNew->cursor(editData)->moveCursorToEnd();
//      _score->update();                         // used by lyricsTab() but not by harmonyTab(): needed or not?
      }

//---------------------------------------------------------
//   figuredBassTicksTab
//    manages [Ctrl] [1]-[9], extending current FB of the given number of ticks
//---------------------------------------------------------

void ScoreView::figuredBassTicksTab(int ticks)
      {
      FiguredBass* fb   = toFiguredBass(editData.element);
      int track         = fb->track();
      Segment* segm     = fb->segment();
      if (!segm) {
            qDebug("figuredBassTicksTab: no segment");
            return;
            }
      Measure* measure = segm->measure();

      int nextSegTick   = segm->tick() + ticks;

      // find the measure containing the target tick
      while (nextSegTick >= measure->tick() + measure->ticks()) {
            measure = measure->nextMeasure();
            if (!measure) {
                  qDebug("figuredBassTicksTab: no next measure");
                  return;
                  }
            }

      // look for a segment at this tick; if none, create one
      Segment * nextSegm = segm;
      while (nextSegm && nextSegm->tick() < nextSegTick)
            nextSegm = nextSegm->next1(SegmentType::ChordRest);
      if (!nextSegm || nextSegm->tick() > nextSegTick) {      // no ChordRest segm at this tick
            nextSegm = new Segment(measure, SegmentType::ChordRest, nextSegTick - measure->tick());
            if (!nextSegm) {
                  qDebug("figuredBassTicksTab: no next segment");
                  return;
                  }
            _score->startCmd();
            _score->undoAddElement(nextSegm);
            _score->endCmd();
            }

      changeState(ViewState::NORMAL);

      bool bNew;
      FiguredBass * fbNew = FiguredBass::addFiguredBassToSegment(nextSegm, track, ticks, &bNew);
      if (bNew) {
            _score->startCmd();
            _score->undoAddElement(fbNew);
            _score->endCmd();
            }
      _score->select(fbNew, SelectType::SINGLE, 0);
      startEdit(fbNew, Grip::NO_GRIP);
      mscore->changeState(mscoreState());
      adjustCanvasPosition(fbNew, false);
//TODO-edit      fb->moveCursorToEnd();
      }

}


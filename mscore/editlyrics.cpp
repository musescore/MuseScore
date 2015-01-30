//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "scoreview.h"
#include "libmscore/chordrest.h"
#include "libmscore/lyrics.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"

namespace Ms {

//---------------------------------------------------------
//   lyricsUpDown
//---------------------------------------------------------

void ScoreView::lyricsUpDown(bool up, bool end)
      {
      Lyrics* lyrics   = static_cast<Lyrics*>(editObject);
      int track        = lyrics->track();
      ChordRest* cr    = lyrics->chordRest();
      int verse        = lyrics->no();
      const QList<Lyrics*>* ll = &lyrics->chordRest()->lyricsList();

      if (up) {
            if (verse == 0)
                  return;
            --verse;
            }
      else {
            ++verse;
            if (verse >= ll->size())
                  return;
            }
      endEdit();
      _score->startCmd();
      lyrics = ll->value(verse);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(cr);
            lyrics->setNo(verse);
            _score->undoAddElement(lyrics);
            }

      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());
      adjustCanvasPosition(lyrics, false);
      if (end)
            ((Lyrics*)editObject)->moveCursorToEnd();
      else
            ((Lyrics*)editObject)->moveCursorToStart();

      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   lyricsTab
//---------------------------------------------------------

void ScoreView::lyricsTab(bool back, bool end, bool moveOnly)
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      int track        = lyrics->track();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();

      Segment* nextSegment = segment;
      if (back) {
            // search prev chord
            while ((nextSegment = nextSegment->prev1(Segment::Type::ChordRest))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == Element::Type::CHORD)
                        break;
                  }
            }
      else {
            // search next chord
            while ((nextSegment = nextSegment->next1(Segment::Type::ChordRest))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == Element::Type::CHORD)
                        break;
                  }
            }
      if (nextSegment == 0)
            return;

      endEdit();

      // search previous lyric
      Lyrics* oldLyrics = 0;
      if (!back) {
            while (segment) {
                  const QList<Lyrics*>* nll = segment->lyricsList(track);
                  if (nll) {
                        oldLyrics = nll->value(verse);
                        if (oldLyrics)
                              break;
                        }
                  segment = segment->prev1(Segment::Type::ChordRest);
                  }
            }

      const QList<Lyrics*>* ll = nextSegment->lyricsList(track);
      if (ll == 0) {
            qDebug("no next lyrics list: %s", nextSegment->element(track)->name());
            return;
            }
      lyrics = ll->value(verse);

      bool newLyrics = false;
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            ChordRest* cr = static_cast<ChordRest*>(nextSegment->element(track));
            lyrics->setParent(cr);
            lyrics->setNo(verse);
            lyrics->setSyllabic(Lyrics::Syllabic::SINGLE);
            newLyrics = true;
            }

      _score->startCmd();

      if (oldLyrics && !moveOnly) {
            switch(lyrics->syllabic()) {
                  // as we arrived at the destination lyrics by a [Space], it can be
                  // the beginning of a multi-syllable, but cannot have syllabic dashes before
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::BEGIN:
                        break;
                  case Lyrics::Syllabic::END:
                        lyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));
                        break;
                  case Lyrics::Syllabic::MIDDLE:
                        lyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
                        break;
                  }
            // as we moved away from the previous lyrics by a [Space], it can be
            // the end of a multi-syllable, but cannot have syllabic dashes after
            switch(oldLyrics->syllabic()) {
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::END:
                        break;
                  case Lyrics::Syllabic::BEGIN:
                        oldLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));
                        break;
                  case Lyrics::Syllabic::MIDDLE:
                        oldLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
                        break;
                  }
            // for the same reason, it cannot have a melisma
            oldLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, 0);
            }

      if (newLyrics)
          _score->undoAddElement(lyrics);

      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(lyrics, false);
      if (end)
            ((Lyrics*)editObject)->moveCursorToEnd();
      else
            ((Lyrics*)editObject)->moveCursorToStart();

      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   lyricsMinus
//---------------------------------------------------------

void ScoreView::lyricsMinus()
      {
      Lyrics* lyrics   = static_cast<Lyrics*>(editObject);
      int track        = lyrics->track();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();

      endEdit();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(Segment::Type::ChordRest))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == Element::Type::CHORD)
                  break;
            }
      if (nextSegment == 0)
            return;

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            const QList<Lyrics*>* nll = segment->lyricsList(track);
            if (!nll) {
                  segment = segment->prev1(Segment::Type::ChordRest);
                  continue;
                  }
            oldLyrics = nll->value(verse);
            if (oldLyrics)
                  break;
            segment = segment->prev1(Segment::Type::ChordRest);
            }

      _score->startCmd();

      const QList<Lyrics*>* ll = nextSegment->lyricsList(track);
      lyrics         = ll->value(verse);
      bool newLyrics = (lyrics == 0);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment->element(track));
            lyrics->setNo(verse);
            lyrics->setSyllabic(Lyrics::Syllabic::END);
            }
      else {
            if (lyrics->syllabic() == Lyrics::Syllabic::BEGIN)
                  lyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::MIDDLE));
            else if (lyrics->syllabic() == Lyrics::Syllabic::SINGLE)
                  lyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
            }

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::Syllabic::BEGIN:
                  case Lyrics::Syllabic::MIDDLE:
                        break;
                  case Lyrics::Syllabic::SINGLE:
                        oldLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
                        break;
                  case Lyrics::Syllabic::END:
                        oldLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::MIDDLE));
                        break;
                  }
            oldLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, 0);
            }

      if (newLyrics)
          _score->undoAddElement(lyrics);

      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(lyrics, false);
      ((Lyrics*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   lyricsUnderscore
//---------------------------------------------------------

void ScoreView::lyricsUnderscore()
      {
      Lyrics* lyrics   = static_cast<Lyrics*>(editObject);
      int track        = lyrics->track();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();
      int endTick      = segment->tick();

      endEdit();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(Segment::Type::ChordRest))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == Element::Type::CHORD)
                  break;
            }

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            const QList<Lyrics*>* nll = segment->lyricsList(track);
            if (nll) {
                  oldLyrics = nll->value(verse);
                  if (oldLyrics)
                        break;
                  }
            segment = segment->prev1(Segment::Type::ChordRest);
            // if the segment has a rest in this track, stop going back
            Element* e = segment ? segment->element(track) : 0;
            if (e && e->type() != Element::Type::CHORD)
                  break;
            }

      _score->startCmd();

      // one-chord melisma?
      // if still at melisma initial chord and there is a valid next chord (if not,
      // there will be no melisma anyway), set a temporary melisma duration
      if (oldLyrics == lyrics && nextSegment)
            lyrics->undoChangeProperty(P_ID::LYRIC_TICKS, Lyrics::TEMP_MELISMA_TICKS);

      if (nextSegment == 0) {
            if (oldLyrics) {
                  switch(oldLyrics->syllabic()) {
                        case Lyrics::Syllabic::SINGLE:
                        case Lyrics::Syllabic::END:
                              break;
                        default:
                              oldLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
                              break;
                        }
                  if (oldLyrics->segment()->tick() < endTick)
                        oldLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, endTick - oldLyrics->segment()->tick());
                  }
            // leave edit mode, select something (just for user feedback) and update to show extended melisam
            mscore->changeState(STATE_NORMAL);
            if (oldLyrics)
                  _score->select(oldLyrics, SelectType::SINGLE, 0);
            //_score->update();
            _score->setLayoutAll(true);
            _score->endCmd();
            return;
            }

      // if a place for a new lyrics has been found, create a lyrics there

      const QList<Lyrics*>* ll = nextSegment->lyricsList(track);
      lyrics         = ll->value(verse);
      bool newLyrics = (lyrics == 0);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment->element(track));
            lyrics->setNo(verse);
            lyrics->setSyllabic(Lyrics::Syllabic::SINGLE);
            }
      else if (lyrics->syllabic() == Lyrics::Syllabic::MIDDLE)
            lyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
      else if (lyrics->syllabic() == Lyrics::Syllabic::END)
            lyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::END:
                        break;
                  default:
                        oldLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
                        break;
                  }
            if (oldLyrics->segment()->tick() < endTick)
                  oldLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, endTick - oldLyrics->segment()->tick());
            }
      if (newLyrics)
            _score->undoAddElement(lyrics);

      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(lyrics, false);
      ((Lyrics*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      //_score->update();
      _score->endCmd();
      }

//---------------------------------------------------------
//   lyricsReturn
//---------------------------------------------------------

void ScoreView::lyricsReturn()
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      Segment* segment = lyrics->segment();

      endEdit();

      _score->startCmd();

      Lyrics* oldLyrics = lyrics;

      lyrics = static_cast<Lyrics*>(Element::create(lyrics->type(), _score));
      lyrics->setTrack(oldLyrics->track());
      lyrics->setParent(segment->element(oldLyrics->track()));
      lyrics->setNo(oldLyrics->no() + 1);
      _score->undoAddElement(lyrics);
      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(lyrics, false);
      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   lyricsEndEdit
//---------------------------------------------------------

void ScoreView::lyricsEndEdit()
      {
      Lyrics* lyrics = static_cast<Lyrics*>(editObject);
      if (lyrics->isEmpty())
            lyrics->parent()->remove(lyrics);
      }

}


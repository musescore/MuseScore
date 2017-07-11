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
#include "libmscore/lyrics.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"

namespace Ms {

//---------------------------------------------------------
//   editKeyLyrics
//---------------------------------------------------------

bool ScoreView::editKeyLyrics(QKeyEvent* ev)
      {
      editData.key       = ev->key();
      editData.modifiers = ev->modifiers();
      editData.s         = ev->text();

      switch (editData.key) {
            case Qt::Key_Space:
                  if (!editData.control()) {
                        if (editData.s == "_")
                              lyricsUnderscore();
                        else // TODO: shift+tab events are filtered by qt
                              lyricsTab(editData.modifiers & Qt::ShiftModifier, true, false);
                        }
                  else
                        return false;
                  break;

            case Qt::Key_Left:
            case Qt::Key_Right:
                  if (!editData.control() && editData.element->edit(editData))
                        mscore->textTools()->updateTools(editData);
                  else {
                        bool kl = editData.key == Qt::Key_Left;
                        lyricsTab(kl, kl, true);      // go to previous/next lyrics
                        }
                  break;

            case Qt::Key_Up:
            case Qt::Key_Down:
                  lyricsUpDown(editData.key == Qt::Key_Up, true);
                  break;

            case Qt::Key_Return:
                  lyricsReturn();
                  break;

            default:
                  if (!editData.control()) {
                        if (editData.s == "-")
                              lyricsMinus();
                        else if (editData.s == "_")
                              lyricsUnderscore();
                        else
                              return false;
                        }
                  break;
            }
      return true;
      }

//---------------------------------------------------------
//   lyricsUpDown
//---------------------------------------------------------

void ScoreView::lyricsUpDown(bool up, bool end)
      {
      Lyrics* lyrics   = toLyrics(editData.element);
      int track        = lyrics->track();
      ChordRest* cr    = lyrics->chordRest();
      int verse        = lyrics->no();
      Element::Placement placement = lyrics->placement();

      if (placement == Element::Placement::ABOVE)
            up = !up;
      if (up) {
            if (verse == 0)
                  return;
            --verse;
            }
      else {
            ++verse;
            if (verse > cr->lastVerse(placement))
                  return;
            }

      changeState(ViewState::NORMAL);
      _score->startCmd();
      lyrics = cr->lyrics(verse, placement);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(cr);
            lyrics->setNo(verse);
            lyrics->setPlacement(placement);
            _score->undoAddElement(lyrics);
            }

      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());
      adjustCanvasPosition(lyrics, false);

      lyrics = toLyrics(editData.element);
      TextCursor* cursor = lyrics->cursor(editData);
      if (end) {
            cursor->movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            cursor->movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            }
      else {
            cursor->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            cursor->movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            }

      _score->setLayoutAll();
      _score->update();
      }

//---------------------------------------------------------
//   lyricsTab
//---------------------------------------------------------

void ScoreView::lyricsTab(bool back, bool end, bool moveOnly)
      {
      Lyrics* lyrics   = toLyrics(editData.element);
      int track        = lyrics->track();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();
      Element::Placement placement = lyrics->placement();

      Segment* nextSegment = segment;
      if (back) {
            // search prev chord
            while ((nextSegment = nextSegment->prev1(SegmentType::ChordRest))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == ElementType::CHORD)
                        break;
                  }
            }
      else {
            // search next chord
            while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == ElementType::CHORD)
                        break;
                  }
            }
      if (nextSegment == 0)
            return;

      changeState(ViewState::NORMAL);

      // look for the lyrics we are moving from; may be the current lyrics or a previous one
      // if we are skipping several chords with spaces
      Lyrics* fromLyrics = 0;
      if (!back) {
            while (segment) {
                  ChordRest* cr = toChordRest(segment->element(track));
                  if (cr) {
                        fromLyrics = cr->lyrics(verse, placement);
                        if (fromLyrics)
                              break;
                        }
                  segment = segment->prev1(SegmentType::ChordRest);
                  }
            }

      ChordRest* cr = toChordRest(nextSegment->element(track));
      if (!cr) {
            qDebug("no next lyrics list: %s", nextSegment->element(track)->name());
            return;
            }
      Lyrics* _toLyrics = cr->lyrics(verse, placement);

      bool newLyrics = false;
      if (!_toLyrics) {
            _toLyrics = new Lyrics(_score);
            _toLyrics->setTrack(track);
            ChordRest* cr = toChordRest(nextSegment->element(track));
            _toLyrics->setParent(cr);
            _toLyrics->setNo(verse);
            _toLyrics->setPlacement(placement);
            _toLyrics->setSyllabic(Lyrics::Syllabic::SINGLE);
            newLyrics = true;
            }

      if (fromLyrics && !moveOnly) {
            switch (_toLyrics->syllabic()) {
                  // as we arrived at toLyrics by a [Space], it can be the beginning
                  // of a multi-syllable, but cannot have syllabic dashes before
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::BEGIN:
                        break;
                  case Lyrics::Syllabic::END:
                        _toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));
                        break;
                  case Lyrics::Syllabic::MIDDLE:
                        _toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
                        break;
                  }
            // as we moved away from fromLyrics by a [Space], it can be
            // the end of a multi-syllable, but cannot have syllabic dashes after
            switch (fromLyrics->syllabic()) {
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::END:
                        break;
                  case Lyrics::Syllabic::BEGIN:
                        fromLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));
                        break;
                  case Lyrics::Syllabic::MIDDLE:
                        fromLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
                        break;
                  }
            // for the same reason, it cannot have a melisma
            fromLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, 0);
            }

      if (newLyrics)
          _score->undoAddElement(_toLyrics);

      _score->select(_toLyrics, SelectType::SINGLE, 0);
      startEdit(_toLyrics, Grip::NO_GRIP);

      adjustCanvasPosition(_toLyrics, false);

      TextCursor* cursor = toLyrics(editData.element)->cursor(editData);
      if (end) {
            cursor->movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            cursor->movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            }
      else {
            cursor->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            cursor->movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            }
      _score->setLayoutAll();
      }

//---------------------------------------------------------
//   lyricsMinus
//---------------------------------------------------------

void ScoreView::lyricsMinus()
      {
      Lyrics* lyrics   = toLyrics(editData.element);
      int track        = lyrics->track();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();
      Element::Placement placement = lyrics->placement();

      changeState(ViewState::NORMAL);

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == ElementType::CHORD)
                  break;
            }
      if (nextSegment == 0)
            return;

      // look for the lyrics we are moving from; may be the current lyrics or a previous one
      // we are extending with several dashes
      Lyrics* fromLyrics = 0;
      while (segment) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (!cr) {
                  segment = segment->prev1(SegmentType::ChordRest);
                  continue;
                  }
            fromLyrics = cr->lyrics(verse, placement);
            if (fromLyrics)
                  break;
            segment = segment->prev1(SegmentType::ChordRest);
            }

      _score->startCmd();

      ChordRest* cr = toChordRest(nextSegment->element(track));
      Lyrics* toLyrics           = cr->lyrics(verse, placement);
      bool newLyrics = (toLyrics == 0);
      if (!toLyrics) {
            toLyrics = new Lyrics(_score);
            toLyrics->setTrack(track);
            toLyrics->setParent(nextSegment->element(track));
            toLyrics->setNo(verse);
            toLyrics->setPlacement(placement);
            toLyrics->setSyllabic(Lyrics::Syllabic::END);
            }
      else {
            // as we arrived at toLyrics by a dash, it cannot be initial or isolated
            if (toLyrics->syllabic() == Lyrics::Syllabic::BEGIN)
                  toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::MIDDLE));
            else if (toLyrics->syllabic() == Lyrics::Syllabic::SINGLE)
                  toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
            }

      if (fromLyrics) {
            // as we moved away from fromLyrics by a dash,
            // it can have syll. dashes before and after but cannot be isolated or terminal
            switch(fromLyrics->syllabic()) {
                  case Lyrics::Syllabic::BEGIN:
                  case Lyrics::Syllabic::MIDDLE:
                        break;
                  case Lyrics::Syllabic::SINGLE:
                        fromLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
                        break;
                  case Lyrics::Syllabic::END:
                        fromLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::MIDDLE));
                        break;
                  }
            // for the same reason, it cannot have a melisma
            fromLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, 0);
            }

      if (newLyrics)
          _score->undoAddElement(toLyrics);

      _score->select(toLyrics, SelectType::SINGLE, 0);
      startEdit(toLyrics, Grip::NO_GRIP);

      adjustCanvasPosition(toLyrics, false);
      TextCursor* cursor = Ms::toLyrics(editData.element)->cursor(editData);
      Ms::toLyrics(editData.element)->selectAll(cursor);
      _score->setLayoutAll();
      }

//---------------------------------------------------------
//   lyricsUnderscore
//---------------------------------------------------------

void ScoreView::lyricsUnderscore()
      {
      Lyrics* lyrics   = toLyrics(editData.element);
      int track        = lyrics->track();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();
      Element::Placement placement = lyrics->placement();
      int endTick      = segment->tick(); // a previous melisma cannot extend beyond this point

      changeState(ViewState::NORMAL);

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == ElementType::CHORD)
                  break;
            }

      // look for the lyrics we are moving from; may be the current lyrics or a previous one
      // we are extending with several underscores
      Lyrics* fromLyrics = 0;
      while (segment) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (cr) {
                  fromLyrics = cr->lyrics(verse, placement);
                  if (fromLyrics)
                        break;
                  }
            segment = segment->prev1(SegmentType::ChordRest);
            // if the segment has a rest in this track, stop going back
            Element* e = segment ? segment->element(track) : 0;
            if (e && e->type() != ElementType::CHORD)
                  break;
            }

      _score->startCmd();

      // one-chord melisma?
      // if still at melisma initial chord and there is a valid next chord (if not,
      // there will be no melisma anyway), set a temporary melisma duration
      if (fromLyrics == lyrics && nextSegment)
            lyrics->undoChangeProperty(P_ID::LYRIC_TICKS, Lyrics::TEMP_MELISMA_TICKS);

      if (nextSegment == 0) {
            if (fromLyrics) {
                  switch(fromLyrics->syllabic()) {
                        case Lyrics::Syllabic::SINGLE:
                        case Lyrics::Syllabic::END:
                              break;
                        default:
                              fromLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
                              break;
                        }
                  if (fromLyrics->segment()->tick() < endTick)
                        fromLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
                  }
            // leave edit mode, select something (just for user feedback) and update to show extended melisam
            mscore->changeState(STATE_NORMAL);
            if (fromLyrics)
                  _score->select(fromLyrics, SelectType::SINGLE, 0);
            _score->setLayoutAll();
            return;
            }

      // if a place for a new lyrics has been found, create a lyrics there

      ChordRest* cr    = toChordRest(nextSegment->element(track));
      Lyrics* toLyrics = cr->lyrics(verse, placement);
      bool newLyrics   = (toLyrics == 0);
      if (!toLyrics) {
            toLyrics = new Lyrics(_score);
            toLyrics->setTrack(track);
            toLyrics->setParent(nextSegment->element(track));
            toLyrics->setNo(verse);
            toLyrics->setPlacement(placement);
            toLyrics->setSyllabic(Lyrics::Syllabic::SINGLE);
            }
      // as we arrived at toLyrics by an underscore, it cannot have syllabic dashes before
      else if (toLyrics->syllabic() == Lyrics::Syllabic::MIDDLE)
            toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
      else if (toLyrics->syllabic() == Lyrics::Syllabic::END)
            toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));

      if (fromLyrics) {
            // as we moved away from fromLyrics by an underscore,
            // it can be isolated or terminal but cannot have dashes after
            switch(fromLyrics->syllabic()) {
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::END:
                        break;
                  default:
                        fromLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::END));
                        break;
                  }
            // for the same reason, if it has a melisma, this cannot extend beyond toLyrics
            if (fromLyrics->segment()->tick() < endTick)
                  fromLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
            }
      if (newLyrics)
            _score->undoAddElement(toLyrics);

      _score->select(toLyrics, SelectType::SINGLE, 0);
      startEdit(toLyrics, Grip::NO_GRIP);

      adjustCanvasPosition(toLyrics, false);
      TextCursor* cursor = Ms::toLyrics(editData.element)->cursor(editData);
      Ms::toLyrics(editData.element)->selectAll(cursor);

      _score->setLayoutAll();
      }

//---------------------------------------------------------
//   lyricsReturn
//---------------------------------------------------------

void ScoreView::lyricsReturn()
      {
      Lyrics* lyrics   = toLyrics(editData.element);
      Segment* segment = lyrics->segment();

      changeState(ViewState::NORMAL);

      Lyrics* oldLyrics = lyrics;

      int newVerse;
      if (lyrics->placeAbove()) {
            newVerse = oldLyrics->no() - 1;
            if (newVerse == -1) {
                  // raise all lyrics above
                  newVerse = 0;
                  for (Segment* s = _score->firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
                        ChordRest* cr = s->cr(lyrics->track());
                        if (cr) {
                              for (Lyrics* l : cr->lyrics()) {
                                    if (l->placement() == oldLyrics->placement())
                                          l->undoChangeProperty(P_ID::VERSE, l->no() + 1);
                                    }
                              }
                        }
                  }
            }
      else
            newVerse = oldLyrics->no() + 1;
      lyrics = toLyrics(Element::create(lyrics->type(), _score));
      lyrics->setTrack(oldLyrics->track());
      lyrics->setParent(segment->element(oldLyrics->track()));
      lyrics->setPlacement(oldLyrics->placement());

      lyrics->setNo(newVerse);

      _score->undoAddElement(lyrics);
      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);

      adjustCanvasPosition(lyrics, false);
      _score->setLayoutAll();
      }

//---------------------------------------------------------
//   lyricsEndEdit
//---------------------------------------------------------

void ScoreView::lyricsEndEdit()
      {
      Lyrics* lyrics = toLyrics(editData.element);

      // if no text, just remove this lyrics
      if (lyrics->empty()) {
            qDebug("lyrics empty: remove");
            lyrics->parent()->remove(lyrics);
            }
      // if not empty, make sure this new lyrics does not fall in the middle
      // of an existing melisma from a previous lyrics; in case, shorten it
      else {
            int verse   = lyrics->no();
            Element::Placement placement = lyrics->placement();
            int track   = lyrics->track();

            // search previous lyric
            Lyrics*  prevLyrics  = 0;
            Segment* prevSegment = lyrics->segment()->prev1(SegmentType::ChordRest);
            Segment* segment     = prevSegment;
            while (segment) {
                  ChordRest* cr = toChordRest(segment->element(track));
                  if (cr) {
                        prevLyrics = cr->lyrics(verse, placement);
                        if (prevLyrics)
                              break;
                        }
                  segment = segment->prev1(SegmentType::ChordRest);
                  }
            if (prevLyrics && prevLyrics->syllabic() == Lyrics::Syllabic::END) {
                  int endTick = prevSegment->tick();      // a prev. melisma should not go beyond this segment
                  if (prevLyrics->endTick() >= endTick)
                        prevLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, endTick - prevLyrics->segment()->tick());
                  }
            }
      }

}


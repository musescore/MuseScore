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
      Lyrics* lyrics   = toLyrics(editObject);
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

      endEdit();
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
      if (end) {
            ((Lyrics*)editObject)->movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            ((Lyrics*)editObject)->movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            }
      else {
            ((Lyrics*)editObject)->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            ((Lyrics*)editObject)->movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            }

      _score->setLayoutAll();
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
      Element::Placement placement = lyrics->placement();

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
                  segment = segment->prev1(Segment::Type::ChordRest);
                  }
            }

      ChordRest* cr = toChordRest(nextSegment->element(track));
      if (!cr) {
            qDebug("no next lyrics list: %s", nextSegment->element(track)->name());
            return;
            }
      Lyrics* toLyrics = cr->lyrics(verse, placement);

      bool newLyrics = false;
      if (!toLyrics) {
            toLyrics = new Lyrics(_score);
            toLyrics->setTrack(track);
            ChordRest* cr = static_cast<ChordRest*>(nextSegment->element(track));
            toLyrics->setParent(cr);
            toLyrics->setNo(verse);
            toLyrics->setPlacement(placement);
            toLyrics->setSyllabic(Lyrics::Syllabic::SINGLE);
            newLyrics = true;
            }

      _score->startCmd();

      if (fromLyrics && !moveOnly) {
            switch(toLyrics->syllabic()) {
                  // as we arrived at toLyrics by a [Space], it can be the beginning
                  // of a multi-syllable, but cannot have syllabic dashes before
                  case Lyrics::Syllabic::SINGLE:
                  case Lyrics::Syllabic::BEGIN:
                        break;
                  case Lyrics::Syllabic::END:
                        toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::SINGLE));
                        break;
                  case Lyrics::Syllabic::MIDDLE:
                        toLyrics->undoChangeProperty(P_ID::SYLLABIC, int(Lyrics::Syllabic::BEGIN));
                        break;
                  }
            // as we moved away from fromLyrics by a [Space], it can be
            // the end of a multi-syllable, but cannot have syllabic dashes after
            switch(fromLyrics->syllabic()) {
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
          _score->undoAddElement(toLyrics);

      _score->select(toLyrics, SelectType::SINGLE, 0);
      startEdit(toLyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(toLyrics, false);
      if (end) {
            ((Lyrics*)editObject)->movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            ((Lyrics*)editObject)->movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            }
      else {
            ((Lyrics*)editObject)->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            ((Lyrics*)editObject)->movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            }
      _score->setLayoutAll();
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
      Element::Placement placement = lyrics->placement();

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

      // look for the lyrics we are moving from; may be the current lyrics or a previous one
      // we are extending with several dashes
      Lyrics* fromLyrics = 0;
      while (segment) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (!cr) {
                  segment = segment->prev1(Segment::Type::ChordRest);
                  continue;
                  }
            fromLyrics = cr->lyrics(verse, placement);
            if (fromLyrics)
                  break;
            segment = segment->prev1(Segment::Type::ChordRest);
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
      mscore->changeState(mscoreState());

      adjustCanvasPosition(toLyrics, false);
      ((Lyrics*)editObject)->selectAll();

      _score->setLayoutAll();
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
      Element::Placement placement = lyrics->placement();
      int endTick      = segment->tick(); // a previous melisma cannot extend beyond this point

      endEdit();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(Segment::Type::ChordRest))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == Element::Type::CHORD)
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
            //_score->update();
            _score->setLayoutAll();
            _score->endCmd();
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
      mscore->changeState(mscoreState());

      adjustCanvasPosition(toLyrics, false);
      ((Lyrics*)editObject)->selectAll();

      _score->setLayoutAll();
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

      int newVerse;
      if (lyrics->placeAbove()) {
            newVerse = oldLyrics->no() - 1;
            if (newVerse == -1) {
                  // raise all lyrics above
                  newVerse = 0;
                  for (Segment* s = _score->firstSegment(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
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
      lyrics = static_cast<Lyrics*>(Element::create(lyrics->type(), _score));
      lyrics->setTrack(oldLyrics->track());
      lyrics->setParent(segment->element(oldLyrics->track()));
      lyrics->setPlacement(oldLyrics->placement());

      lyrics->setNo(newVerse);

      _score->undoAddElement(lyrics);
      _score->select(lyrics, SelectType::SINGLE, 0);
      startEdit(lyrics, Grip::NO_GRIP);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(lyrics, false);
      _score->setLayoutAll();
      _score->update();
      }

//---------------------------------------------------------
//   lyricsEndEdit
//---------------------------------------------------------

void ScoreView::lyricsEndEdit()
      {
      Lyrics* lyrics = toLyrics(editObject);

      // if no text, just remove this lyrics
      if (lyrics->empty())
            lyrics->parent()->remove(lyrics);
      // if not empty, make sure this new lyrics does not fall in the middle
      // of an existing melisma from a previous lyrics; in case, shorten it
      else {
            int verse   = lyrics->no();
            Element::Placement placement = lyrics->placement();
            int track   = lyrics->track();

            // search previous lyric
            Lyrics*     prevLyrics  = 0;
            Segment*    prevSegment = lyrics->segment()->prev1(Segment::Type::ChordRest);
            Segment*    segment     = prevSegment;
            while (segment) {
                  ChordRest* cr = toChordRest(segment->element(track));
                  if (cr) {
                        prevLyrics = cr->lyrics(verse, placement);
                        if (prevLyrics)
                              break;
                        }
                  segment = segment->prev1(Segment::Type::ChordRest);
                  }
            if (prevLyrics && prevLyrics->syllabic() == Lyrics::Syllabic::END) {
                  int endTick = prevSegment->tick();      // a prev. melisma should not go beyond this segment
                  if (prevLyrics->endTick() >= endTick)
                        prevLyrics->undoChangeProperty(P_ID::LYRIC_TICKS, endTick - prevLyrics->segment()->tick());
                  }
            }
      }

}


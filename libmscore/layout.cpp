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

#include "accidental.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "chord.h"
#include "clef.h"
#include "element.h"
#include "fingering.h"
#include "glissando.h"
#include "harmony.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "layout.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "mscore.h"
#include "notedot.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "repeat.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stem.h"
#include "style.h"
#include "sym.h"
#include "system.h"
#include "text.h"
#include "tie.h"
#include "timesig.h"
#include "tremolo.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"

namespace Ms {

// #define PAGE_DEBUG

#ifdef PAGE_DEBUG
#define PAGEDBG(...)  qDebug(__VA_ARGS__)
#else
#define PAGEDBG(...)  ;
#endif

//---------------------------------------------------------
//   rebuildBspTree
//---------------------------------------------------------

void Score::rebuildBspTree()
      {
      for (Page* page : _pages)
            page->rebuildBspTree();
      }

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

ChordRest* Score::searchNote(int tick, int track) const
      {
      ChordRest* ipe = 0;
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st)) {
            ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
            if (!cr)
                  continue;
            if (cr->tick() == tick)
                  return cr;
            if (cr->tick() >  tick)
                  return ipe ? ipe : cr;
            ipe = cr;
            }
      return 0;
      }

//---------------------------------------------------------
//   layoutChords1
//    - layout upstem and downstem chords
//    - offset as necessary to avoid conflict
//---------------------------------------------------------

void Score::layoutChords1(Segment* segment, int staffIdx)
      {
      Staff* staff = Score::staff(staffIdx);

      // if (staff->isDrumStaff() || staff->isTabStaff())
      if (staff->isTabStaff())
            return;

      int upVoices = 0, downVoices = 0;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      QList<Note*> upStemNotes, downStemNotes;
      qreal nominalWidth = noteHeadWidth() * staff->mag();
      qreal maxUpWidth = 0.0;
      qreal maxDownWidth = 0.0;
      qreal maxUpMag = 0.0;
      qreal maxDownMag = 0.0;

      // dots and hooks can affect layout of notes as well as vice versa
      int upDots = 0;
      int downDots = 0;
      bool upHooks = false;
      bool downHooks = false;
      // also check for grace notes
      bool upGrace = false;
      bool downGrace = false;

      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (e && (e->type() == Element::Type::CHORD)) {
                  Chord* chord = static_cast<Chord*>(e);
                  bool hasGraceBefore = false;
                  for (Chord* c : chord->graceNotes()) {
                        if (c->isGraceBefore())
                              hasGraceBefore = true;
                        // layout grace note noteheads
                        layoutChords2(c->notes(), c->up());
                        // layout grace note chords
                        layoutChords3(c->notes(), staff, 0);
                        }
                  if (chord->up()) {
                        ++upVoices;
                        upStemNotes.append(chord->notes());
                        upDots = qMax(upDots, chord->dots());
                        maxUpMag = qMax(maxUpMag, chord->mag());
                        if (!upHooks)
                              upHooks = chord->hook();
                        if (hasGraceBefore)
                              upGrace = true;
                        }
                  else {
                        ++downVoices;
                        downStemNotes.append(chord->notes());
                        downDots = qMax(downDots, chord->dots());
                        maxDownMag = qMax(maxDownMag, chord->mag());
                        if (!downHooks)
                              downHooks = chord->hook();
                        if (hasGraceBefore)
                              downGrace = true;
                        }
                  }
            }

      if (upVoices + downVoices == 0)
            return;

      // TODO: use track as secondary sort criteria?
      // otherwise there might be issues with unisons between voices
      // in some corner cases

      maxUpWidth = nominalWidth * maxUpMag;
      maxDownWidth = nominalWidth * maxDownMag;

      // layout upstem noteheads
      if (upVoices > 1) {
            qSort(upStemNotes.begin(), upStemNotes.end(),
               [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );
            }
      if (upVoices) {
            qreal hw = layoutChords2(upStemNotes, true);
            maxUpWidth = qMax(maxUpWidth, hw);
            }

      // layout downstem noteheads
      if (downVoices > 1) {
            qSort(downStemNotes.begin(), downStemNotes.end(),
               [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );
            }
      if (downVoices) {
            qreal hw = layoutChords2(downStemNotes, false);
            maxDownWidth = qMax(maxDownWidth, hw);
            }

      qreal sp                      = staff->spatium();
      qreal upOffset                = 0.0;      // offset to apply to upstem chords
      qreal downOffset              = 0.0;      // offset to apply to downstem chords
      qreal dotAdjust               = 0.0;      // additional chord offset to account for dots
      qreal dotAdjustThreshold      = 0.0;      // if it exceeds this amount

      // centering adjustments for whole note, breve, and small chords
      qreal centerUp          = 0.0;      // offset to apply in order to center upstem chords
      qreal oversizeUp        = 0.0;      // adjustment to oversized upstem chord needed if laid out to the right
      qreal centerDown        = 0.0;      // offset to apply in order to center downstem chords
      qreal centerAdjustUp    = 0.0;      // adjustment to upstem chord needed after centering donwstem chord
      qreal centerAdjustDown  = 0.0;      // adjustment to downstem chord needed after centering upstem chord

      // only center chords if they differ from nominal by at least this amount
      // this avoids unnecessary centering on differences due only to floating point roundoff
      // it also allows for the possibility of disabling centering
      // for notes only "slightly" larger than nominal, like half notes
      // but this will result in them not being aligned with each other between voices
      // unless you change to left alignment as described in the comments below
      qreal centerThreshold   = 0.01 * sp;

      // amount by which actual width exceeds nominal, adjusted for staff mag() only
      qreal headDiff = maxUpWidth - nominalWidth;
      // amount by which actual width exceeds nominal, adjusted for staff & chord/note mag()
      qreal headDiff2 = maxUpWidth - nominalWidth * (maxUpMag / staff->mag());
      if (headDiff > centerThreshold) {
            // larger than nominal
            centerUp = headDiff * -0.5;
            // maxUpWidth is true width, but we no longer will care about that
            // instead, we care only about portion to right of origin
            maxUpWidth += centerUp;
            // to left align rather than center, delete both of the above
            if (headDiff2 > centerThreshold) {
                  // if max notehead is wider than nominal with chord/note mag() applied
                  // then noteheads extend to left of origin
                  // because stemPosX() is based on nominal width
                  // so we need to correct for that too
                  centerUp += headDiff2;
                  oversizeUp = headDiff2;
                  }
            }
      else if (-headDiff > centerThreshold) {
            // smaller than nominal
            centerUp = -headDiff * 0.5;
            if (headDiff2 > centerThreshold) {
                  // max notehead is wider than nominal with chord/note mag() applied
                  // perform same adjustment as above
                  centerUp += headDiff2;
                  oversizeUp = headDiff2;
                  }
            centerAdjustDown = centerUp;
            }

      headDiff = maxDownWidth - nominalWidth;
      if (headDiff > centerThreshold) {
            // larger than nominal
            centerDown = headDiff * -0.5;
            // to left align rather than center, change the above to
            //centerAdjustUp = headDiff;
            maxDownWidth = nominalWidth - centerDown;
            }
      else if (-headDiff > centerThreshold) {
            // smaller than nominal
            centerDown = -headDiff * 0.5;
            centerAdjustUp = centerDown;
            }

      // handle conflict between upstem and downstem chords

      if (upVoices && downVoices) {

            Note* bottomUpNote = upStemNotes.first();
            Note* topDownNote = downStemNotes.last();
            int separation;
            if (bottomUpNote->chord()->staffMove() == topDownNote->chord()->staffMove())
                  separation = topDownNote->line() - bottomUpNote->line();
            else
                  separation = 2;   // no conflict
            QList<Note*> overlapNotes;

            if (separation == 1) {
                  // second
                  downOffset = maxUpWidth;
                  // align stems if present, leave extra room if not
                  if (topDownNote->chord()->stem() && bottomUpNote->chord()->stem())
                        downOffset -= topDownNote->chord()->stem()->lineWidth();
                  else
                        downOffset += 0.1 * sp;
                  }

            else if (separation < 1) {

                  // overlap (possibly unison)

                  // build list of overlapping notes
                  for (int i = 0, n = upStemNotes.size(); i < n; ++i) {
                        if (upStemNotes[i]->line() >= topDownNote->line() - 1)
                              overlapNotes.append(upStemNotes[i]);
                        else
                              break;
                        }
                  for (int i = downStemNotes.size() - 1; i >= 0; --i) {
                        if (downStemNotes[i]->line() <= bottomUpNote->line() + 1)
                              overlapNotes.append(downStemNotes[i]);
                        else
                              break;
                        }
                  qSort(overlapNotes.begin(), overlapNotes.end(),
                     [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );

                  // determine nature of overlap
                  bool shareHeads = true;       // can all overlapping notes share heads?
                  bool matchPending = false;    // looking for a unison match
                  bool conflictUnison = false;  // unison found
                  bool conflictSecondUpHigher = false;      // second found
                  bool conflictSecondDownHigher = false;    // second found
                  int lastLine = 1000;
                  Note* p = overlapNotes[0];
                  for (int i = 0, count = overlapNotes.size(); i < count; ++i) {
                        Note* n = overlapNotes[i];
                        NoteHead::Type nHeadType;
                        NoteHead::Type pHeadType;
                        Chord* nchord = n->chord();
                        Chord* pchord = p->chord();
                        if (n->mirror()) {
                              if (separation < 0) {
                                    // don't try to share heads if there is any mirroring
                                    shareHeads = false;
                                    // don't worry about conflicts involving mirrored notes
                                    continue;
                                    }
                              }
                        int line = n->line();
                        int d = lastLine - line;
                        switch (d) {
                              case 0:
                                    // unison
                                    conflictUnison = true;
                                    matchPending = false;
                                    nHeadType = (n->headType() == NoteHead::Type::HEAD_AUTO) ? n->chord()->durationType().headType() : n->headType();
                                    pHeadType = (p->headType() == NoteHead::Type::HEAD_AUTO) ? p->chord()->durationType().headType() : p->headType();
                                    // the most important rules for sharing noteheads on unisons between voices are
                                    // that notes must be one same line with same tpc
                                    // noteheads must be unmirrored and of same group
                                    // and chords must be same size (or else sharing code won't work)
                                    if (n->headGroup() != p->headGroup() || n->tpc() != p->tpc() || n->mirror() || p->mirror() || nchord->small() != pchord->small()) {
                                          shareHeads = false;
                                          }
                                    else {
                                          // noteheads are potentially shareable
                                          // it is more subjective at this point
                                          // current default is to require *either* of the following:
                                          //    1) both chords have same number of dots, both have stems, and both noteheads are same type and are full size (automatic match)
                                          // or 2) one or more of the noteheads is not of type AUTO, but is explicitly set to match the other (user-forced match)
                                          // or 3) exactly one of the noteheads is invisible (user-forced match)
                                          // thus user can force notes to be shared despite differing number of dots or either being stemless
                                          // by setting one of the notehead types to match the other or by making one notehead invisible
                                          // TODO: consider adding a style option, staff properties, or note property to control sharing
                                          if ((nchord->dots() != pchord->dots() || !nchord->stem() || !pchord->stem() || nHeadType != pHeadType || n->small() || p->small()) &&
                                              ((n->headType() == NoteHead::Type::HEAD_AUTO && p->headType() == NoteHead::Type::HEAD_AUTO) || nHeadType != pHeadType) &&
                                              (n->visible() == p->visible())) {
                                                shareHeads = false;
                                                }
                                          }
                                    break;
                              case 1:
                                    // second
                                    // trust that this won't be a problem for single unison
                                    if (separation < 0) {
                                          if (n->chord()->up())
                                                conflictSecondUpHigher = true;
                                          else
                                                conflictSecondDownHigher = true;
                                          shareHeads = false;
                                          }
                                    break;
                              default:
                                    // no conflict
                                    if (matchPending)
                                          shareHeads = false;
                                    matchPending = true;
                              }
                        p = n;
                        lastLine = line;
                        }
                  if (matchPending)
                        shareHeads = false;

                  // calculate offsets
                  if (shareHeads) {
                        for (int i = overlapNotes.size() - 1; i >= 1; i -= 2) {
                              Note* p = overlapNotes[i-1];
                              Note* n = overlapNotes[i];
                              if (!(p->chord()->isNudged() || n->chord()->isNudged())) {
                                    if (p->chord()->dots() == n->chord()->dots()) {
                                          // hide one set dots
                                          bool onLine = !(p->line() & 1);
                                          if (onLine) {
                                                // hide dots for lower voice
                                                if (p->voice() & 1)
                                                      p->setDotsHidden(true);
                                                else
                                                      n->setDotsHidden(true);
                                                }
                                          else {
                                                // hide dots for upper voice
                                                if (!(p->voice() & 1))
                                                      p->setDotsHidden(true);
                                                else
                                                      n->setDotsHidden(true);
                                                }
                                          }
                                    // formerly we hid noteheads in an effort to fix playback
                                    // but this doesn't work for cases where noteheads cannot be shared
                                    // so better to solve the problem elsewhere
                                    }
                              }
                        }
                  else if (conflictUnison && separation == 0 && (!downGrace || upGrace))
                        downOffset = maxUpWidth + 0.3 * sp;
                  else if (conflictUnison)
                        upOffset = maxDownWidth + 0.3 * sp;
                  else if (conflictSecondUpHigher)
                        upOffset = maxDownWidth + 0.2 * sp;
                  else if ((downHooks && !upHooks) && !(upDots && !downDots))
                        downOffset = maxUpWidth + 0.3 * sp;
                  else if (conflictSecondDownHigher) {
                        if (downDots && !upDots)
                              downOffset = maxUpWidth + 0.3 * sp;
                        else {
                              upOffset = maxDownWidth - 0.2 * sp;
                              if (downHooks)
                                    upOffset += 0.3 * sp;
                              }
                        }
                  else {
                        // no direct conflict, so parts can overlap (downstem on left)
                        // just be sure that stems clear opposing noteheads
                        qreal clearLeft = 0.0, clearRight = 0.0;
                        if (topDownNote->chord()->stem())
                              clearLeft = topDownNote->chord()->stem()->lineWidth() + 0.3 * sp;
                        if (bottomUpNote->chord()->stem())
                              clearRight = bottomUpNote->chord()->stem()->lineWidth() + qMax(maxDownWidth - maxUpWidth, 0.0) + 0.3 * sp;
                        else
                              downDots = 0; // no need to adjust for dots in this case
                        upOffset = qMax(clearLeft, clearRight);
                        if (downHooks) {
                              // we will need more space to avoid collision with hook
                              // but we won't need as much dot adjustment
                              upOffset = qMax(upOffset, maxDownWidth + 0.1 * sp);
                              dotAdjustThreshold = maxUpWidth - 0.3 * sp;
                              }
                        // if downstem chord is small, don't center
                        // and we might not need as much dot adjustment either
                        if (centerDown > 0.0) {
                              centerDown = 0.0;
                              centerAdjustUp = 0.0;
                              dotAdjustThreshold = (upOffset - maxDownWidth) + maxUpWidth - 0.3 * sp;
                              }
                        }

                  }

            // adjust for dots
            if ((upDots && !downDots) || (downDots && !upDots)) {
                  // only one sets of dots
                  // place between chords
                  int dots;
                  qreal mag;
                  if (upDots) {
                        dots = upDots;
                        mag = maxUpMag;
                        }
                  else {
                        dots = downDots;
                        mag = maxDownMag;
                        }
                  qreal dotWidth = segment->symWidth(SymId::augmentationDot);
                  // first dot
                  dotAdjust = point(styleS(StyleIdx::dotNoteDistance)) + dotWidth;
                  // additional dots
                  if (dots > 1)
                        dotAdjust += point(styleS(StyleIdx::dotDotDistance)) * (dots - 1);
                  dotAdjust *= mag;
                  // only by amount over threshold
                  dotAdjust = qMax(dotAdjust - dotAdjustThreshold, 0.0);
                  }
            if (separation == 1)
                  dotAdjust += 0.1 * sp;

            }

      // apply chord offsets
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (e && (e->type() == Element::Type::CHORD)) {
                  Chord* chord = static_cast<Chord*>(e);
                  if (chord->up()) {
                        if (upOffset != 0.0) {
                              chord->rxpos() += upOffset + centerAdjustUp + oversizeUp;
                              if (downDots && !upDots)
                                    chord->rxpos() += dotAdjust;
                              }
                        else
                              chord->rxpos() += centerUp;
                        }
                  else {
                        if (downOffset != 0.0) {
                              chord->rxpos() += downOffset + centerAdjustDown;
                              if (upDots && !downDots)
                                    chord->rxpos() += dotAdjust;
                              }
                        else
                              chord->rxpos() += centerDown;
                        }
                  }
            }

      // layout chords
      QList<Note*> notes;
      if (upVoices)
            notes.append(upStemNotes);
      if (downVoices)
            notes.append(downStemNotes);
      if (upVoices + downVoices > 1)
            qSort(notes.begin(), notes.end(),
               [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );
      layoutChords3(notes, staff, segment);

      }

//---------------------------------------------------------
//   layoutChords2
//    - determine which notes need mirroring
//    - this is called once for each stem direction
//      eg, once for voices 1&3, once for 2&4
//      with all notes combined and sorted to resemble one chord
//    - return maximum non-mirrored notehead width
//---------------------------------------------------------

qreal Score::layoutChords2(QList<Note*>& notes, bool up)
      {
      int startIdx, endIdx, incIdx;
      qreal maxWidth = 0.0;

      // loop in correct direction so that first encountered notehead wins conflict
      if (up) {
            // loop bottom up
            startIdx = 0;
            endIdx = notes.size();
            incIdx = 1;
            }
      else {
            // loop top down
            startIdx = notes.size() - 1;
            endIdx = -1;
            incIdx = -1;
            }

      int ll        = 1000;         // line of previous note head
                                    // hack: start high so first note won't show as conflict
      bool lvisible = false;        // was last note visible?
      bool mirror   = false;        // should current note head be mirrored?
                                    // value is retained and may be used on next iteration
                                    // to track mirror status of previous note
      bool isLeft   = notes[startIdx]->chord()->up();             // is note head on left?
      int lmove     = notes[startIdx]->chord()->staffMove();      // staff offset of last note (for cross-staff beaming)

      for (int idx = startIdx; idx != endIdx; idx += incIdx) {

            Note* note    = notes[idx];                     // current note
            int line      = note->line();                   // line of current note
            Chord* chord  = note->chord();
            int move      = chord->staffMove();             // staff offset of current note

            // there is a conflict
            // if this is same or adjacent line as previous note (and chords are on same staff!)
            // but no need to do anything about it if either note is invisible
            bool conflict = (qAbs(ll - line) < 2) && (lmove == move) && note->visible() && lvisible;

            // this note is on opposite side of stem as previous note
            // if there is a conflict
            // or if this the first note *after* a conflict
            if (conflict || (chord->up() != isLeft))
                  isLeft = !isLeft;

            // determine if we would need to mirror current note
            // to get it to the correct side
            // this would be needed to get a note to left or downstem or right of upstem
            // whether or not we actually do this is determined later (based on user mirror property)
            bool nmirror = (chord->up() != isLeft);

            // by default, notes and dots are not hidden
            // this may be changed later to allow unisons to share note heads
            note->setHidden(false);
            note->setDotsHidden(false);

            // be sure chord position is initialized
            // chord may be moved to the right later
            // if there are conflicts between voices
            chord->rxpos() = 0.0;

            // let user mirror property override the default we calculated
            if (note->userMirror() == MScore::DirectionH::AUTO) {
                  mirror = nmirror;
                  }
            else {
                  mirror = note->chord()->up();
                  if (note->userMirror() == MScore::DirectionH::LEFT)
                        mirror = !mirror;
                  }
            note->setMirror(mirror);

            // accumulate return value
            if (!mirror)
                  maxWidth = qMax(maxWidth, note->headWidth());

            // prepare for next iteration
            lvisible      = note->visible();
            lmove         = move;
            ll            = line;

            }

      return maxWidth;
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      qreal x;          // actual x position of this accidental relative to origin
      qreal top;        // top of accidental bbox relative to staff
      qreal bottom;     // bottom of accidental bbox relative to staff
      int line;         // line of note
      int next;         // index of next accidental of same pitch class (ascending list)
      qreal width;      // width of accidental
      qreal ascent;     // amount (in sp) vertical strokes extend above body
      qreal descent;    // amount (in sp) vertical strokes extend below body
      qreal rightClear; // amount (in sp) to right of last vertical stroke above body
      qreal leftClear;  // amount (in sp) to left of last vertical stroke below body
      };

//---------------------------------------------------------
//   resolveAccidentals
//    lx = calculated position of rightmost edge of left accidental relative to origin
//---------------------------------------------------------

static bool resolveAccidentals(AcEl* left, AcEl* right, qreal& lx, qreal pd, qreal sp)
      {
      AcEl* upper;
      AcEl* lower;
      if (left->line >= right->line) {
            upper = right;
            lower = left;
            }
      else {
            upper = left;
            lower = right;
            }

      qreal gap = lower->top - upper->bottom;

      // no conflict at all if there is sufficient vertical gap between accidentals
      // the arrangement of accidentals into columns assumes accidentals an octave apart *do* clear
      if (gap >= pd || lower->line - upper->line >= 7)
            return false;

      qreal allowableOverlap = qMax(upper->descent, lower->ascent) - pd;

      // accidentals that are "close" (small gap or even slight overlap)
      if (qAbs(gap) <= 0.33 * sp) {
            // acceptable with slight offset
            // if one of the accidentals can subsume the overlap
            // and both accidentals allow it
            if (-gap <= allowableOverlap && qMin(upper->descent, lower->ascent) > 0.0) {
                  qreal align = qMin(left->width, right->width);
                  lx = qMin(lx, right->x + align - pd);
                  return true;
                  }
            }

      // amount by which overlapping accidentals will be separated
      // for example, the vertical stems of two flat signs
      // these need more space than we would need between non-overlapping accidentals
      qreal overlapShift = pd * 1.41;

      // accidentals with more significant overlap
      // acceptable if one accidental can subsume overlap
      if (left == lower && -gap <= allowableOverlap) {
            qreal offset = qMax(left->rightClear, right->leftClear);
            offset = qMin(offset, left->width) - overlapShift;
            lx = qMin(lx, right->x + offset);
            return true;
            }

      // accidentals with even more overlap
      // can work if both accidentals can subsume overlap
      if (left == lower && -gap <= upper->descent + lower->ascent - pd) {
            qreal offset = qMin(left->rightClear, right->leftClear) - overlapShift;
            if (offset > 0.0) {
                  lx = qMin(lx, right->x + offset);
                  return true;
                  }
            }

      // otherwise, there is real conflict
      lx = qMin(lx, right->x - pd);
      return true;
      }

//---------------------------------------------------------
//   layoutAccidental
//---------------------------------------------------------

static qreal layoutAccidental(AcEl* me, AcEl* above, AcEl* below, qreal colOffset, QList<Note*>& leftNotes, qreal pnd, qreal pd, qreal sp)
      {
      qreal lx = colOffset;
      Accidental* acc = me->note->accidental();
      qreal mag = acc->mag();
      pnd *= mag;
      pd *= mag;

      // extra space for ledger lines
      if (me->line <= -2 || me->line >= me->note->staff()->lines() * 2)
            lx = qMin(lx, -0.2 * sp);

      // clear left notes
      int lns = leftNotes.size();
      for (int i = 0; i < lns; ++i) {
            Note* ln = leftNotes[i];
            int lnLine = ln->line();
            qreal lnTop = (lnLine - 1) * 0.5 * sp;
            qreal lnBottom = lnTop + sp;
            if (me->top - lnBottom <= pnd && lnTop - me->bottom <= pnd) {
                  // undercut note above if possible
                  if (lnBottom - me->top <= me->ascent - pnd)
                        lx = qMin(lx, ln->x() + ln->chord()->x() + me->rightClear);
                  else
                        lx = qMin(lx, ln->x() + ln->chord()->x());
                  }
            else if (lnTop > me->bottom)
                  break;
            }

      // clear other accidentals
      bool conflictAbove = false;
      bool conflictBelow = false;

      if (above)
            conflictAbove = resolveAccidentals(me, above, lx, pd, sp);
      if (below)
            conflictBelow = resolveAccidentals(me, below, lx, pd, sp);
      if (conflictAbove || conflictBelow)
            me->x = lx - acc->width() - acc->bbox().x();
      else if (colOffset != 0.0)
            me->x = lx - pd - acc->width() - acc->bbox().x();
      else
            me->x = lx - pnd - acc->width() - acc->bbox().x();

      return me->x;

      }

//---------------------------------------------------------
//   layoutChords3
//    - calculate positions of notes, accidentals, dots
//---------------------------------------------------------

void Score::layoutChords3(QList<Note*>& notes, Staff* staff, Segment* segment)
      {
      //---------------------------------------------------
      //    layout accidentals
      //    find column for dots
      //---------------------------------------------------

      QList<Note*> leftNotes; // notes to left of origin
      QList<AcEl> aclist;     // accidentals
      // track columns of octave-separated accidentals
      int columnBottom[7] = { -1, -1, -1, -1, -1, -1, -1 };

      qreal sp           = staff->spatium();
      qreal stepDistance = sp * .5;
      int stepOffset     = staff->staffType()->stepOffset();

      qreal lx                = 10000.0;  // leftmost note head position
      qreal upDotPosX         = 0.0;
      qreal downDotPosX       = 0.0;

      int nNotes = notes.size();
      int nAcc = 0;
      for (int i = nNotes-1; i >= 0; --i) {
            Note* note     = notes[i];
            Accidental* ac = note->accidental();
            if (ac && !note->fixed()) {
                  ac->layout();
                  AcEl acel;
                  acel.note   = note;
                  int line    = note->line();
                  acel.line   = line;
                  acel.x      = 0.0;
                  acel.top    = line * 0.5 * sp + ac->bbox().top();
                  acel.bottom = line * 0.5 * sp + ac->bbox().bottom();
                  acel.width  = ac->width();
                  QPointF bboxNE = ac->symBbox(ac->symbol()).topRight();
                  QPointF bboxSW = ac->symBbox(ac->symbol()).bottomLeft();
                  QPointF cutOutNE = ac->symCutOutNE(ac->symbol());
                  QPointF cutOutSW = ac->symCutOutSW(ac->symbol());
                  if (!cutOutNE.isNull()) {
                        acel.ascent     = cutOutNE.y() - bboxNE.y();
                        acel.rightClear = bboxNE.x() - cutOutNE.x();
                        }
                  else {
                        acel.ascent     = 0.0;
                        acel.rightClear = 0.0;
                        }
                  if (!cutOutSW.isNull()) {
                        acel.descent   = bboxSW.y() - cutOutSW.y();
                        acel.leftClear = cutOutSW.x() - bboxSW.x();
                        }
                  else {
                        acel.descent   = 0.0;
                        acel.leftClear = 0.0;
                        }
                  int pitchClass = (line + 700) % 7;
                  acel.next = columnBottom[pitchClass];
                  columnBottom[pitchClass] = nAcc;
                  aclist.append(acel);
                  ++nAcc;
                  }

            qreal hw     = note->headWidth();   // actual head width, including note & chord mag
            Chord* chord = note->chord();
            bool _up     = chord->up();
            qreal stemX  = chord->stemPosX();   // stem position for nominal notehead, but allowing for mag

            qreal overlapMirror;
            if (chord->stem()) {
                  qreal stemWidth = chord->stem()->lineWidth();
                  qreal stemWidth5 = stemWidth * 0.5;
                  chord->stem()->rxpos() = _up ? stemX - stemWidth5 : stemWidth5;
                  overlapMirror = stemWidth;
                  }
            else if (chord->durationType().headType() == NoteHead::Type::HEAD_WHOLE)
                  overlapMirror = styleD(StyleIdx::stemWidth) * chord->mag() * sp;
            else
                  overlapMirror = 0.0;

            qreal x;
            if (note->mirror()) {
                  if (_up)
                        x = stemX - overlapMirror;
                  else
                        x = stemX - hw + overlapMirror;
                  }
            else {
                  if (_up)
                        x = stemX - hw;
                  else
                        x = 0.0;
                  }

            note->rypos()  = (note->line() + stepOffset) * stepDistance;
            note->rxpos()  = x;
            // we need to do this now
            // or else note pos / readPos / userOff will be out of sync
            // and we rely on note->x() throughout the layout process
            note->adjustReadPos();

            // find leftmost non-mirrored note to set as X origin for accidental layout
            // a mirrored note that extends to left of segment X origin
            // will displace accidentals only if there is conflict
            qreal sx = x + chord->x(); // segment-relative X position of note
            if (note->mirror() && !chord->up() && sx < 0.0)
                  leftNotes.append(note);
            else if (sx < lx)
                  lx = sx;

            //if (chord->stem())
            //      chord->stem()->rxpos() = _up ? x + hw - stemWidth5 : x + stemWidth5;

            qreal xx = x + hw + chord->pos().x();

            if (chord->dots()) {
                  if (chord->up())
                        upDotPosX = qMax(upDotPosX, xx);
                  else
                        downDotPosX = qMax(downDotPosX, xx);
                  MScore::Direction dotPosition = note->userDotPosition();

                  if (dotPosition == MScore::Direction::AUTO && nNotes > 1 && note->visible() && !note->dotsHidden()) {
                        // resolve dot conflicts
                        int line = note->line();
                        Note* above = (i < nNotes - 1) ? notes[i+1] : 0;
                        if (above && (!above->visible() || above->dotsHidden()))
                              above = 0;
                        int intervalAbove = above ? line - above->line() : 1000;
                        Note* below = (i > 0) ? notes[i-1] : 0;
                        if (below && (!below->visible() || below->dotsHidden()))
                              below = 0;
                        int intervalBelow = below ? below->line() - line : 1000;
                        if ((line & 1) == 0) {
                              // line
                              if (intervalAbove == 1 && intervalBelow != 1)
                                    dotPosition = MScore::Direction::DOWN;
                              else if (intervalBelow == 1 && intervalAbove != 1)
                                    dotPosition = MScore::Direction::UP;
                              else if (intervalAbove == 0 && above->chord()->dots()) {
                                    // unison
                                    if (((above->voice() & 1) == (note->voice() & 1))) {
                                          above->setDotY(MScore::Direction::UP);
                                          dotPosition = MScore::Direction::DOWN;
                                          }
                                    }
                              }
                        else {
                              // space
                              if (intervalAbove == 0 && above->chord()->dots()) {
                                    // unison
                                    if (!(note->voice() & 1))
                                          dotPosition = MScore::Direction::UP;
                                    else {
                                          if (!(above->voice() & 1))
                                                above->setDotY(MScore::Direction::UP);
                                          else
                                                dotPosition = MScore::Direction::DOWN;
                                          }
                                    }
                              }
                        }
                  note->setDotY(dotPosition);
                  }
            }

      if (segment) {
            // align all dots for segment/staff
            // it would be possible to dots for up & down chords separately
            // this would require space to have been allocated previously
            // when calculating chord offsets
            segment->setDotPosX(staff->idx(), qMax(upDotPosX, downDotPosX));
            }

      if (nAcc == 0)
            return;

      QList<int> umi;
      qreal pd  = point(styleS(StyleIdx::accidentalDistance));
      qreal pnd = point(styleS(StyleIdx::accidentalNoteDistance));
      qreal colOffset = 0.0;

      if (nAcc >= 2 && aclist[nAcc-1].line - aclist[0].line >= 7) {

            // accidentals spread over an octave or more
            // set up columns for accidentals with octave matches
            // these will start at right and work to the left
            // unmatched accidentals will use zig zag approach (see below)
            // starting to the left of the octave columns

            qreal minX = 0.0;
            int columnTop[7] = { -1, -1, -1, -1, -1, -1, -1 };

            // find columns of octaves
            for (int pc = 0; pc < 7; ++pc) {
                  if (columnBottom[pc] == -1)
                        continue;
                  // calculate column height
                  for (int j = columnBottom[pc]; j != -1; j = aclist[j].next)
                        columnTop[pc] = j;
                  }

            // compute reasonable column order
            // use zig zag
            QList<int> column;
            QList<int> unmatched;
            int n = nAcc - 1;
            for (int i = 0; i <= n; ++i, --n) {
                  int pc = (aclist[i].line + 700) % 7;
                  if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                        if (!column.contains(pc))
                              column.append(pc);
                        }
                  else
                        unmatched.append(i);
                  if (i == n)
                        break;
                  pc = (aclist[n].line + 700) % 7;
                  if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                        if (!column.contains(pc))
                              column.append(pc);
                        }
                  else
                        unmatched.append(n);
                  }
            int nColumns = column.size();
            int nUnmatched = unmatched.size();

            // handle unmatched accidentals
            for (int i = 0; i < nUnmatched; ++i) {
                  // first try to slot it into an existing column
                  AcEl* me = &aclist[unmatched[i]];
                  // find column
                  bool found = false;
                  for (int j = 0; j < nColumns; ++j) {
                        int pc = column[j];
                        int above = -1;
                        int below = -1;
                        // find slot within column
                        for (int k = columnBottom[pc]; k != -1; k = aclist[k].next) {
                              if (aclist[k].line < me->line) {
                                    above = k;
                                    break;
                                    }
                              below = k;
                              }
                        // check to see if accidental can fit in slot
                        qreal myPd = pd * me->note->accidental()->mag();
                        bool conflict = false;
                        if (above != -1 && me->top - aclist[above].bottom < myPd)
                              conflict = true;
                        else if (below != -1 && aclist[below].top - me->bottom < myPd)
                              conflict = true;
                        if (!conflict) {
                              // insert into column
                              found = true;
                              me->next = above;
                              if (above == -1)
                                    columnTop[pc] = unmatched[i];
                              if (below != -1)
                                    aclist[below].next = unmatched[i];
                              else
                                    columnBottom[pc] = unmatched[i];
                              break;
                              }
                        }
                  // if no slot found, then add to list of unmatched accidental indices
                  if (!found)
                        umi.append(unmatched[i]);
                  }
            nAcc = umi.size();
            if (nAcc > 1)
                  qSort(umi);

            // lay out columns
            for (int i = 0; i < nColumns; ++i) {
                  int pc = column[i];
                  AcEl* below = 0;
                  // lay out accidentals
                  for (int j = columnBottom[pc]; j != -1; j = aclist[j].next) {
                        qreal x = layoutAccidental(&aclist[j], 0, below, colOffset, leftNotes, pnd, pd, sp);
                        minX = qMin(minX, x);
                        below = &aclist[j];
                        }
                  // align within column
                  int next = -1;
                  for (int j = columnBottom[pc]; j != -1; j = next) {
                        next = aclist[j].next;
                        if (next != -1 && aclist[j].line == aclist[next].line)
                              continue;
                        aclist[j].x = minX;
                        }
                  // move to next column
                  colOffset = minX;
                  }

            }

      else {
            for (int i = 0; i < nAcc; ++i)
                  umi.append(i);
            }

      if (nAcc) {

            // for accidentals with no octave matches, use zig zag approach
            // layout right to left in pairs, (next) highest then lowest

            AcEl* me = &aclist[umi[0]];
            AcEl* above = 0;
            AcEl* below = 0;

            // layout top accidental
            layoutAccidental(me, above, below, colOffset, leftNotes, pnd, pd, sp);

            // layout bottom accidental
            int n = nAcc - 1;
            if (n > 0) {
                  above = me;
                  me = &aclist[umi[n]];
                  layoutAccidental(me, above, below, colOffset, leftNotes, pnd, pd, sp);
                  }

            // layout middle accidentals
            if (n > 1) {
                  for (int i = 1; i < n; ++i, --n) {
                        // next highest
                        below = me;
                        me = &aclist[umi[i]];
                        layoutAccidental(me, above, below, colOffset, leftNotes, pnd, pd, sp);
                        if (i == n - 1)
                              break;
                        // next lowest
                        above = me;
                        me = &aclist[umi[n-1]];
                        layoutAccidental(me, above, below, colOffset, leftNotes, pnd, pd, sp);
                        }
                  }

            }

      for (const AcEl& e : aclist) {
            // even though we initially calculate accidental position relative to segment
            // we must record pos for accidental relative to note,
            // since pos is always interpreted relative to parent
            Note* note = e.note;
            qreal x    = e.x + lx - (note->x() + note->chord()->x());
            note->accidental()->setPos(x, 0);
            note->accidental()->adjustReadPos();
            }

      }

#define beamModeMid(a) (a == Beam::Mode::MID || a == Beam::Mode::BEGIN32 || a == Beam::Mode::BEGIN64)


//---------------------------------------------------------
//   beamGraceNotes
//---------------------------------------------------------

void Score::beamGraceNotes(Chord* mainNote, bool after)
      {
      ChordRest* a1    = 0;      // start of (potential) beam
      Beam* beam       = 0;      // current beam
      Beam::Mode bm = Beam::Mode::AUTO;
      QList<Chord*> graceNotes = after ? mainNote->graceNotesAfter() : mainNote->graceNotesBefore();

      foreach (ChordRest* cr, graceNotes) {
            bm = Groups::endBeam(cr);
            if ((cr->durationType().type() <= TDuration::DurationType::V_QUARTER) || (bm == Beam::Mode::NONE)) {
                  if (beam) {
                        beam->layoutGraceNotes();
                        beam = 0;
                        }
                  if (a1) {
                        a1->removeDeleteBeam();
                        a1 = 0;
                        }
                  cr->removeDeleteBeam();
                  continue;
                  }
            if (beam) {
                  bool beamEnd = bm == Beam::Mode::BEGIN;
                  if (!beamEnd) {
                        cr->removeDeleteBeam(true);
                        beam->add(cr);
                        cr = 0;
                        beamEnd = (bm == Beam::Mode::END);
                        }
                  if (beamEnd) {
                        beam->layoutGraceNotes();
                        beam = 0;
                        }
                  }
            if (!cr)
                  continue;
            if (a1 == 0)
                  a1 = cr;
            else {
                  if (!beamModeMid(bm) && (bm == Beam::Mode::BEGIN)) {
                        a1->removeDeleteBeam();
                        a1 = cr;
                        }
                  else {
                        beam = a1->beam();
                        if (beam == 0 || beam->elements().front() != a1) {
                              beam = new Beam(this);
                              beam->setGenerated(true);
                              beam->setTrack(mainNote->track());
                              a1->removeDeleteBeam(true);
                              beam->add(a1);
                              }
                        cr->removeDeleteBeam(true);
                        beam->add(cr);
                        a1 = 0;
                        }
                  }
            }
      if (beam)
            beam->layoutGraceNotes();
      else if (a1)
            a1->removeDeleteBeam();
      }

//---------------------------------------------------------
//   layoutStage2
//    auto - beamer
//---------------------------------------------------------

void Score::layoutStage2()
      {
      int tracks = nstaves() * VOICES;
      bool crossMeasure = styleB(StyleIdx::crossMeasureValues);

      for (int track = 0; track < tracks; ++track) {
            Staff* stf = staff(track2staff(track));

            // dont compute beams for invisible staffs and tablature without stems
            if (!stf->show() || (stf->isTabStaff() && stf->staffType()->slashStyle()))
                  continue;

            ChordRest* a1    = 0;      // start of (potential) beam
            Beam* beam       = 0;      // current beam
            Measure* measure = 0;

            Beam::Mode bm    = Beam::Mode::AUTO;
            Segment::Type st = Segment::Type::ChordRest;
            ChordRest* prev  = 0;
            bool checkBeats  = false;
            Fraction stretch = 1;
            QHash<int, TDuration> beatSubdivision;

            for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st)) {
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                  if (cr == 0)
                        continue;

                  // handle grace notes and cross-measure beaming
                  if (cr->type() == Element::Type::CHORD) {
                        Chord* chord = static_cast<Chord*>(cr);
                        beamGraceNotes(chord, false); // grace before
                        beamGraceNotes(chord, true);  // grace after
                        // set up for cross-measure values as soon as possible
                        // to have all computations (stems, hooks, ...) consistent with it
                        if (!chord->isGrace())
                              chord->crossMeasureSetup(crossMeasure);
                        }

                  // if this is a new measure, and it's simple meter (actually X/4),
                  // then perform a prepass to determine the subdivision of each beat
                  if (segment->measure() != measure) {
                        Measure* m = segment->measure();
                        TimeSig* ts = cr->staff()->timeSig(m->tick());
                        beatSubdivision.clear();
                        checkBeats = false;
                        stretch = ts ? ts->stretch() : 1;
                        if (ts && ts->denominator() == 4) {
                              checkBeats = true;
                              for (Segment* s = m->first(st); s; s = s->next(st)) {
                                    ChordRest* mcr = static_cast<ChordRest*>(s->element(track));
                                    if (mcr == 0)
                                          continue;
                                    int beat = ((mcr->rtick() * stretch.numerator()) / stretch.denominator()) / MScore::division;
                                    if (beatSubdivision.contains(beat))
                                          beatSubdivision[beat] = qMin(beatSubdivision[beat], mcr->durationType());
                                    else
                                          beatSubdivision[beat] = mcr->durationType();
                                    }
                              }
                        }

                  // get defaults from time signature properties
                  bm = Groups::endBeam(cr, prev);

                  // perform additional context-dependent checks
                  if (bm == Beam::Mode::AUTO) {
                        // check if we need to break beams according to minimum duration in current / previous beat
                        if (checkBeats && cr->rtick()) {
                              int tick = (cr->rtick() * stretch.numerator()) / stretch.denominator();
                              // check if on the beat
                              if (tick % MScore::division == 0) {
                                    int beat = tick / MScore::division;
                                    // get minimum duration for this & previous beat
                                    TDuration minDuration = qMin(beatSubdivision[beat], beatSubdivision[beat - 1]);
                                    // re-calculate beam as if this were the duration of current chordrest
                                    TDuration saveDuration        = cr->actualDurationType();
                                    TDuration saveCMDuration      = cr->crossMeasureDurationType();
                                    CrossMeasure saveCrossMeasVal = cr->crossMeasure();
                                    cr->setDurationType(minDuration);
                                    bm = Groups::endBeam(cr, prev);
                                    cr->setDurationType(saveDuration);
                                    cr->setCrossMeasure(saveCrossMeasVal);
                                    cr->setCrossMeasureDurationType(saveCMDuration);
                                    }
                              }
                        }

                  prev = cr;

                  // if chord has hooks and is 2nd element of a cross-measure value
                  // set beam mode to NONE (do not combine with following chord beam/hook, if any)
                  if (cr->durationType().hooks() > 0 && cr->crossMeasure() == CrossMeasure::SECOND)
                        bm = Beam::Mode::NONE;

                  if (cr->measure() != measure) {
                        if (measure && !beamModeMid(bm)) {
                              if (beam) {
                                    beam->layout1();
                                    beam = 0;
                                    }
                              else if (a1) {
                                    a1->removeDeleteBeam();
                                    a1 = 0;
                                    }
                              }
                        measure = cr->measure();
                        if (!beamModeMid(bm)) {
                              a1      = 0;
                              beam    = 0;
                              }
                        }
                  if ((cr->durationType().type() <= TDuration::DurationType::V_QUARTER) || (bm == Beam::Mode::NONE)) {
                        if (beam) {
                              beam->layout1();
                              beam = 0;
                              }
                        if (a1) {
                              a1->removeDeleteBeam();
                              a1 = 0;
                              }
                        cr->removeDeleteBeam();
                        continue;
                        }

                  if (beam) {
                        bool beamEnd = (bm == Beam::Mode::BEGIN);
                        if (!beamEnd) {
                              cr->removeDeleteBeam(true);
                              beam->add(cr);
                              cr = 0;
                              beamEnd = (bm == Beam::Mode::END);
                              }
                        if (beamEnd) {
                              beam->layout1();
                              beam = 0;
                              }
                        }
                  if (!cr)
                        continue;

                  if (a1 == 0)
                        a1 = cr;
                  else {
                        if (!beamModeMid(bm)
                             &&
                             (bm == Beam::Mode::BEGIN
                             || (a1->segment()->segmentType() != cr->segment()->segmentType())
                             || (a1->tick() + a1->actualTicks() < cr->tick())
                             )
                           ) {
                              a1->removeDeleteBeam();
                              a1 = cr;
                              }
                        else {
                              beam = a1->beam();
                              if (beam == 0 || beam->elements().front() != a1) {
                                    beam = new Beam(this);
                                    beam->setGenerated(true);
                                    beam->setTrack(track);
                                    a1->removeDeleteBeam(true);
                                    beam->add(a1);
                                    }
                              cr->removeDeleteBeam(true);
                              beam->add(cr);
                              a1 = 0;
                              }
                        }
                  }
            if (beam)
                  beam->layout1();
            else if (a1)
                  a1->removeDeleteBeam();
            }
      }

//---------------------------------------------------------
//   layoutStage3
//---------------------------------------------------------

void Score::layoutStage3()
      {
      Segment::Type st = Segment::Type::ChordRest;
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            if (!staff(staffIdx)->show())
                  continue;
            for (Segment* segment = firstSegment(st); segment; segment = segment->next1(st))
                  layoutChords1(segment, staffIdx);
            }
      }

//---------------------------------------------------------
//   layout
//    - measures are akkumulated into systems
//    - systems are akkumulated into pages
//   already existent systems and pages are reused
//---------------------------------------------------------

void Score::doLayout()
      {
// printf("doLayout %p cmd %d undo empty %d\n", this, undo()->active(), undo()->isEmpty());

      if (!undo()->active() && !undo()->isEmpty() && !undoRedo()) {
            qDebug("layout outside cmd and dirty undo");
            // _layoutAll = false;
            // abort();
            // return;
            }
      if (_staves.isEmpty() || first() == 0) {
            // score is empty
            // qDeleteAll(_pages);
            _pages.clear();

            Page* page = addPage();
            page->layout();
            page->setNo(0);
            page->setPos(0.0, 0.0);
            page->rebuildBspTree();
            qDebug("layout: empty score");
            _layoutAll = false;
            return;
            }

      _scoreFont = ScoreFont::fontFactory(_style.value(StyleIdx::MusicalSymbolFont).toString());
      _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, spatium() / (MScore::DPI * SPATIUM20));

      if (layoutFlags & LayoutFlag::FIX_TICKS)
            fixTicks();
      if (layoutFlags & LayoutFlag::FIX_PITCH_VELO)
            updateVelo();
      if (layoutFlags & LayoutFlag::PLAY_EVENTS)
            createPlayEvents();
      layoutFlags = 0;

      int measureNo = 0;
      int nstaves = _staves.size();
      for (MeasureBase* m = first(); m; m = m->next()) {      // set layout break
            m->setPageBreak(false);
            m->setLineBreak(false);
            m->setSectionBreak(0);

            for (Element* e : m->el()) {
                  if (!tagIsValid(e->tag()) || (e->type() != Element::Type::LAYOUT_BREAK))
                        continue;
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  switch (lb->layoutBreakType()) {
                        case LayoutBreak::Type::PAGE:
                              m->setPageBreak(true);
                              break;
                        case LayoutBreak::Type::LINE:
                              m->setLineBreak(true);
                              break;
                        case LayoutBreak::Type::SECTION:
                              m->setSectionBreak(lb);
                              break;
                        }
                  }
            if (m->type() == Element::Type::MEASURE) {
                  Measure* measure = static_cast<Measure*>(m);
                  measureNo += measure->noOffset();
                  measure->setNo(measureNo);
                  if (measure->sectionBreak() && measure->sectionBreak()->startWithMeasureOne())
                        measureNo = 0;
                  else if (!measure->irregular())      // dont count measure
                        ++measureNo;
                  measure->layoutStage1();
                  }
            }

      if (styleB(StyleIdx::createMultiMeasureRests))
            createMMRests();

      layoutStage2();   // beam notes, finally decide if chord is up/down
      layoutStage3();   // compute note head horizontal positions

      if (layoutMode() == LayoutMode::LINE)
            layoutLinear();
      else
            layoutSystems();  // create list of systems

      //---------------------------------------------------
      //   place Spanner & beams
      //---------------------------------------------------

      int tracks = nstaves * VOICES;
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = firstSegmentMM(); segment; segment = segment->next1MM()) {
                  if (track == tracks-1) {
                        for (Element* e : segment->annotations())
                              e->layout();
                        }
                  Element* e = segment->element(track);
                  if (!e)
                        continue;
                  if (e->isChordRest()) {
                        if (!staff(track2staff(track))->show())
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr->beam() && cr->beam()->elements().front() == cr)
                              cr->beam()->layout();

                        if (cr->type() == Element::Type::CHORD) {
                              Chord* c = static_cast<Chord*>(cr);
                              for (Chord* cc : c->graceNotes()) {
                                    if (cc->beam() && cc->beam()->elements().front() == cc)
                                          cc->beam()->layout();
                                    for (Note* n : cc->notes()) {
                                          Tie* tie = n->tieFor();
                                          if (tie)
                                                tie->layout();
                                          for (Spanner* sp : n->spannerFor())
                                                sp->layout();
                                          }
                                    for (Element* e : cc->el()) {
                                          if (e->type() == Element::Type::SLUR)
                                                e->layout();
                                          }
                                    cc->layoutArticulations();
                                    }
                              c->layoutStem();
                              c->layoutArpeggio2();
                              for (Note* n : c->notes()) {
                                    Tie* tie = n->tieFor();
                                    if (tie)
                                          tie->layout();
                                    for (Spanner* sp : n->spannerFor())
                                          sp->layout();
                                    }
                              }
                        cr->layoutArticulations();
                        }
                  else if (e->type() == Element::Type::BAR_LINE)
                        e->layout();
                  }
            }

      if (lastSegment())
            checkSpanner(0, lastSegment()->tick());

      for (auto s : _spanner.map()) {
            Spanner* sp = s.second;
            if (sp->type() == Element::Type::OTTAVA && sp->ticks() == 0) {
                  sp->setTick2(lastMeasure()->endTick());
                  sp->staff()->updateOttava();
                  }
            // 1.3 scores can have ties in this list (ws: should not happen anymore)
            if (sp->type() != Element::Type::TIE) {
                  if (sp->tick() == -1) {
                        qDebug("bad spanner %s %d - %d", sp->name(), sp->tick(), sp->tick2());
                        }
                  else
                        sp->layout();
                  }
            }
      for (Spanner* s : _unmanagedSpanner)
            s->layout();

      if (layoutMode() != LayoutMode::LINE) {
            layoutSystems2();
            layoutPages();    // create list of pages
            }
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM())
            m->layout2();

      for (auto s : _spanner.map()) {           // DEBUG
            Spanner* sp = s.second;
            if (sp->type() == Element::Type::SLUR) {
                  sp->layout();
                  }
            }

      rebuildBspTree();

      for (MuseScoreView* v : viewer)
            v->layoutChanged();

      _layoutAll = false;

      // _mscVersion is used during read and first layout
      // but then it's used for drag and drop and should be set to new version
      if (_mscVersion <= 114)
            _mscVersion = MSCVERSION;     // for later drag & drop usage
      }

//---------------------------------------------------------
//   layoutSpanner
//    called after dragging a staff
//---------------------------------------------------------

void Score::layoutSpanner()
      {
      int tracks = ntracks();
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                  if (track == tracks-1) {
                        int n = segment->annotations().size();
                        for (int i = 0; i < n; ++i)
                              segment->annotations().at(i)->layout();
                        }
                  Chord* c = static_cast<Chord*>(segment->element(track));
                  if (c && c->type() == Element::Type::CHORD) {
                        c->layoutStem();
                        for (Note* n : c->notes()) {
                              Tie* tie = n->tieFor();
                              if (tie)
                                    tie->layout();
                              for (Spanner* sp : n->spannerFor())
                                    sp->layout();
                              }
                        }
                  }
            }
      rebuildBspTree();
      }

//-------------------------------------------------------------------
//   addSystemHeader
///   Add elements to make this measure suitable as the first measure
///   of a system.
//    The system header can contain a Clef, a KeySig and a
//    RepeatBarLine.
//-------------------------------------------------------------------

void Score::addSystemHeader(Measure* m, bool isFirstSystem)
      {
      if (undoRedo())   // no change possible in this state
            return;

      int tick = m->tick();
      int i    = 0;
      foreach (Staff* staff, _staves) {
            // At this time we don't know which staff is visible or not...
            // but let's not create the key/clef if there were no visible before this layout
            // sometimes we will be right, other time it will take another layout to be right...
            if (!m->system()->staff(i)->show()) {
                  ++i;
                  continue;
                  }

            KeySig* keysig = 0;
            Clef*   clef   = 0;
            int strack     = i * VOICES;

            // we assume that keysigs and clefs are only in the first
            // track (voice 0) of a staff

            KeySigEvent keyIdx = staff->keySigEvent(tick);

            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  // search only up to the first ChordRest/StartRepeatBarLine
                  if (seg->segmentType() & (Segment::Type::ChordRest | Segment::Type::StartRepeatBarLine))
                        break;
                  Element* el = seg->element(strack);
                  if (!el)
                        continue;
                  switch (el->type()) {
                        case Element::Type::KEYSIG:
                              keysig = static_cast<KeySig*>(el);
                              break;
                        case Element::Type::CLEF:
                              clef = static_cast<Clef*>(el);
                              clef->setSmall(false);
                              break;
                        default:
                              break;
                        }
                  }
            bool needKeysig =        // keep key sigs in TABs: TABs themselves should hide them
               isFirstSystem || styleB(StyleIdx::genKeysig);

            if (needKeysig && !keysig && ((keyIdx.key() != Key::C) || keyIdx.custom())) {
                  //
                  // create missing key signature
                  //
                  keysig = new KeySig(this);
                  keysig->setKeySigEvent(keyIdx);
                  keysig->setTrack(i * VOICES);
                  keysig->setGenerated(true);
                  Segment* seg = m->undoGetSegment(Segment::Type::KeySig, tick);
                  keysig->setParent(seg);
                  keysig->layout();
                  undo(new AddElement(keysig));
                  }
            else if (!needKeysig && keysig)
                  undoRemoveElement(keysig);
            else if (keysig && !(keysig->keySigEvent() == keyIdx))
                  undo(new ChangeKeySig(keysig, keyIdx, keysig->showCourtesy()));

            bool needClef = isFirstSystem
               || styleB(StyleIdx::genClef)
                  // real clef change?:
               || staff->clef(m->tick()) != staff->clef(m->tick()-1);

            if (needClef) {
                  if (!clef) {
                        //
                        // create missing clef
                        //
                        clef = new Clef(this);
                        clef->setTrack(i * VOICES);
                        clef->setSmall(false);
                        clef->setGenerated(true);

                        Segment* ns = m->findSegment(Segment::Type::Clef, tick);
                        Segment* s;
                        if (ns && !ns->element(clef->track())) {
                              s = ns;
                              ns = 0;
                              }
                        else {
                              s  = new Segment(m, Segment::Type::Clef, tick);
                              undoAddElement(s);
                              }
                        clef->setParent(s);

                        // if there is already a clef at the same tick position,
                        // then this is the real clef change and we have to
                        // show the previous clef type at tick-1

                        ClefTypeList clefType = staff->clefType(ns ? tick - 1 : tick);
                        clef->layout();
                        clef->setClefType(clefType);  // set before add !
                        undo(new AddElement(clef));
                        }
                  else if (clef->generated()) {
                        ClefTypeList cl = staff->clefType(tick);
                        if (cl != clef->clefTypeList())
                              undo(new ChangeClefType(clef, cl._concertClef, cl._transposingClef));
                        }
                  }
            else {
                  if (clef && clef->generated())
                        undo(new RemoveElement(clef));
                  }
            ++i;
            }
      m->setStartRepeatBarLine(m->repeatFlags() & Repeat::START);
      }

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* Score::getNextSystem(bool isFirstSystem, bool isVbox)
      {
      System* system;
      if (curSystem >= _systems.size()) {
            system = new System(this);
            _systems.append(system);
            }
      else {
            system = _systems[curSystem];
            system->clear();   // remove measures from system
            }
      system->setFirstSystem(isFirstSystem);
      system->setVbox(isVbox);
      if (!isVbox) {
            int nstaves = Score::nstaves();
            for (int i = system->staves()->size(); i < nstaves; ++i)
                  system->insertStaff(i);
            int dn = system->staves()->size() - nstaves;
            for (int i = 0; i < dn; ++i)
                  system->removeStaff(system->staves()->size()-1);
            }
      return system;
      }

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

static bool validMMRestMeasure(Measure* m)
      {
      if (!m->isEmpty())
            return false;

#if 0
      auto l = m->score()->spannerMap().findOverlapping(m->tick(), m->endTick());
      for (::Interval<Spanner*> isp : l) {
            Spanner* s = isp.value;
            if (s->type() == Element::Type::VOLTA && (s->tick() == m->tick() || s->tick2() == m->endTick()))
                  return false;
            }
#endif

      for (Segment* s = m->first(); s; s = s->next()) {
            for (Element* e : s->annotations()) {
                  if (e->type() != Element::Type::REHEARSAL_MARK &&
                      e->type() != Element::Type::TEMPO_TEXT &&
                      e->type() != Element::Type::HARMONY &&
                      e->type() != Element::Type::STAFF_TEXT)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//  breakMultiMeasureRest
//    return true if this measure should start a new
//    multi measure rest
//---------------------------------------------------------

static bool breakMultiMeasureRest(Measure* m)
      {
      if (m->breakMultiMeasureRest())
            return true;
      auto sl = m->score()->spannerMap().findOverlapping(m->tick(), m->endTick());
      foreach (auto i, sl) {
            Spanner* s = i.value;
            if (s->type() == Element::Type::VOLTA && (s->tick() == m->tick() || s->tick2() == m->tick()))
                  return true;
            }

      // break for marker in this measure
      for (Element* e : m->el()) {
            if (e->type() == Element::Type::MARKER) {
                  Marker* mark = static_cast<Marker*>(e);
                  if (!(mark->textStyle().align() & AlignmentFlags::RIGHT))
                        return true;
                  }
            }

      // break for marker & jump in previous measure
      Measure* pm = m->prevMeasure();
      if (pm) {
            for (Element* e : pm->el()) {
                  if (e->type() == Element::Type::JUMP) {
                        return true;
                        }
                  else if (e->type() == Element::Type::MARKER) {
                        Marker* mark = static_cast<Marker*>(e);
                        if (mark->textStyle().align() & AlignmentFlags::RIGHT)
                              return true;
                        }
                  }
            }

      // break for end of volta
      auto l = m->score()->spannerMap().findOverlapping(m->tick(), m->endTick());
      for (::Interval<Spanner*> isp : l) {
            Spanner* s = isp.value;
            if (s->type() == Element::Type::VOLTA && (s->tick2() == m->endTick()))
                  return false;
            }

      for (Segment* s = m->first(); s; s = s->next()) {
            for (Element* e : s->annotations()) {
                  if (e->type() == Element::Type::REHEARSAL_MARK ||
                      e->type() == Element::Type::TEMPO_TEXT ||
                      ((e->type() == Element::Type::HARMONY || e->type() == Element::Type::STAFF_TEXT) && (e->systemFlag() || m->score()->staff(e->staffIdx())->show())))
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   createMMRests
//---------------------------------------------------------

void Score::createMMRests()
      {
      //
      //  create mm rest measures
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            Measure* nm = m;
            Measure* lm = nm;
            int n = 0;
            Fraction len;
            while (validMMRestMeasure(nm)) {
                  m->setMMRestCount(0);
                  MeasureBase* mb = _showVBox ? nm->next() : nm->nextMeasure();
                  if (breakMultiMeasureRest(nm) && n)
                        break;
                  ++n;
                  len += nm->len();
                  lm = nm;
                  nm = static_cast<Measure*>(mb);
                  if (!nm || (nm->type() != Element::Type::MEASURE))
                        break;
                  }

            if (n >= styleI(StyleIdx::minEmptyMeasures)) {
                  //
                  // create a multi measure rest from m to lm (inclusive)
                  // attach the measure to m
                  //

                  for (Measure* mm = m->nextMeasure(); mm; mm = mm->nextMeasure()) {
                        mm->setMMRestCount(-1);
                        if (mm->mmRest())
                              undo(new ChangeMMRest(mm, 0));
                        if (mm == lm)
                              break;
                        }

                  Measure* mmr = m->mmRest();
                  if (mmr) {
                        if (mmr->len() != len) {
                              Segment* s = mmr->findSegment(Segment::Type::EndBarLine, mmr->endTick());
                              mmr->setLen(len);
                              if (s)
                                    s->setTick(mmr->endTick());
                              }
                        }
                  else {
                        mmr = new Measure(this);
                        mmr->setLen(len);
                        mmr->setTick(m->tick());
                        undo(new ChangeMMRest(m, mmr));
                        }

                  mmr->setMMRestCount(n);
                  mmr->setNo(m->no());
                  mmr->setPageBreak(lm->pageBreak());
                  mmr->setLineBreak(lm->lineBreak());

                  BarLineType t;
                  // End repeats do not set endBarLineGenerated to false because
                  // they can be generated from the repeatFlags. So we need to test it separately.
                  if (lm->endBarLineGenerated() && !(lm->repeatFlags() & Repeat::END))
                        t = BarLineType::NORMAL;
                  else
                        t = lm->endBarLineType();
                  mmr->setEndBarLineType(t, false, lm->endBarLineVisible(), lm->endBarLineColor());
                  mmr->setRepeatFlags(m->repeatFlags() | lm->repeatFlags());

                  ElementList oldList = mmr->takeElements();
                  ElementList newList = lm->el();

                  for (Element* e : m->el()) {
                        if (e->type() == Element::Type::MARKER)
                              newList.append(e);
                        }
                  for (Element* e : newList) {
                        bool found = false;
                        for (Element* ee : oldList) {
                              if (ee->type() == e->type()) {
                                    mmr->add(ee);
                                    oldList.removeOne(ee);
                                    found = true;
                                    break;
                                    }
                              }
                        if (!found)
                              mmr->add(e->clone());
                        }
                  for (Element* e : oldList)
                        delete e;
                  Segment* s = mmr->undoGetSegment(Segment::Type::ChordRest, mmr->tick());
                  for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                        int track = staffIdx * VOICES;
                        if (s->element(track) == 0) {
                              Rest* r = new Rest(this);
                              r->setDurationType(TDuration::DurationType::V_MEASURE);
                              r->setDuration(mmr->len());
                              r->setTrack(track);
                              r->setParent(s);
                              undo(new AddElement(r));
                              }
                        }

                  //
                  // check for clefs
                  //
                  Segment* cs = lm->findSegment(Segment::Type::Clef, lm->endTick());
                  Segment* ns = mmr->findSegment(Segment::Type::Clef, lm->endTick());
                  if (cs) {
                        if (ns == 0)
                              ns = mmr->undoGetSegment(Segment::Type::Clef, lm->endTick());
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Clef* clef = static_cast<Clef*>(cs->element(track));
                              if (clef) {
                                    if (ns->element(track) == 0)
                                          ns->add(clef->clone());
                                    else {
                                          //TODO: check if same clef
                                          }
                                    }
                              }
                        }
                  else if (ns)
                        undo(new RemoveElement(ns));
                  //
                  // check for time signature
                  //
                  cs = m->findSegment(Segment::Type::TimeSig, m->tick());
                  ns = mmr->findSegment(Segment::Type::TimeSig, m->tick());
                  if (cs) {
                        if (ns == 0)
                              ns = mmr->undoGetSegment(Segment::Type::TimeSig, m->tick());
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              TimeSig* ts = static_cast<TimeSig*>(cs->element(track));
                              if (ts) {
                                    TimeSig* nts = static_cast<TimeSig*>(ns->element(track));
                                    if (!nts) {
                                          nts = ts->clone();
                                          nts->setParent(ns);
                                          undo(new AddElement(nts));
                                          }
                                    else {
                                          nts->setSig(ts->sig(), ts->timeSigType());
                                          nts->layout();
                                          }
                                    }
                              }
                        }
                  else if (ns)
                        undo(new RemoveElement(ns));

                  //
                  // check for key signature
                  //
                  cs = m->findSegment(Segment::Type::KeySig, m->tick());
                  ns = mmr->findSegment(Segment::Type::KeySig, m->tick());
                  if (cs) {
                        if (ns == 0)
                              ns = mmr->undoGetSegment(Segment::Type::KeySig, m->tick());
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              KeySig* ts  = static_cast<KeySig*>(cs->element(track));
                              KeySig* nts = static_cast<KeySig*>(ns->element(track));
                              if (ts) {
                                    if (!nts) {
                                          KeySig* nks = ts->clone();
                                          nks->setParent(ns);
                                          undo(new AddElement(nks));
                                          }
                                    else {
                                          if (!(nts->keySigEvent() == ts->keySigEvent())) {
                                                undo(new ChangeKeySig(nts, ts->keySigEvent(), nts->showCourtesy()));
                                                }
                                          }
                                    }
                              }
                        }
                  else if (ns && ns->isEmpty())
                        undo(new RemoveElement(ns));

                  //
                  // check for rehearsal mark etc.
                  //
                  cs = m->findSegment(Segment::Type::ChordRest, m->tick());
                  if (cs) {
                        for (Element* e : cs->annotations()) {
                              if (e->type() != Element::Type::REHEARSAL_MARK &&
                                  e->type() != Element::Type::TEMPO_TEXT &&
                                  e->type() != Element::Type::HARMONY &&
                                  e->type() != Element::Type::STAFF_TEXT)
                                    continue;

                              bool found = false;
                              for (Element* ee : s->annotations()) {
                                    if (ee->type() == e->type() && ee->track() == e->track()) {
                                          found = true;
                                          break;
                                          }
                                    }
                              if (!found) {
                                    Element* ne = e->linkedClone();
                                    ne->setParent(s);
                                    undo(new AddElement(ne));
                                    }
                              }
                        }
                  for (Element* e : s->annotations()) {
                        if (e->type() != Element::Type::REHEARSAL_MARK &&
                            e->type() != Element::Type::TEMPO_TEXT &&
                            e->type() != Element::Type::HARMONY &&
                            e->type() != Element::Type::STAFF_TEXT)
                              continue;
                        bool found = false;
                        for (Element* ee : cs->annotations()) {
                              if (ee->type() == e->type() && ee->track() == e->track()) {
                                    found = true;
                                    break;
                                    }
                              }
                        if (!found)
                              undo(new RemoveElement(e));
                        }

                  mmr->setNext(nm);
                  mmr->setPrev(m->prev());
                  m = lm;
                  }
            else {
                  if (m->mmRest())
                        undo(new ChangeMMRest(m, 0));
                  m->setMMRestCount(0);
                  }
            }
      }

//---------------------------------------------------------
//   cautionaryWidth
//    Compute the width of required courtesy of time signature
//    and key signature elements if m were the last measure
//    in a staff.
//    Return hasCourtesy == true if courtesy elements are
//    already present. The value is undefined if no
//    courtesy elements are required.
//---------------------------------------------------------

qreal Score::cautionaryWidth(Measure* m, bool& hasCourtesy)
      {
      hasCourtesy = false;
      if (m == 0)
            return 0.0;
      Measure* nm = m->nextMeasure();
      if (nm == 0 || (m->sectionBreak() && _layoutMode != LayoutMode::FLOAT))
            return 0.0;

      int tick = m->endTick();

      // locate a time sig. in the next measure and, if found,
      // check if it has caut. sig. turned off

      Segment* ns       = nm->findSegment(Segment::Type::TimeSig, tick);
      bool showCourtesy = styleB(StyleIdx::genCourtesyTimesig);

      qreal w = 0.0;
      if (showCourtesy && ns) {
            TimeSig* ts = static_cast<TimeSig*>(ns->element(0));
            if (ts && ts->showCourtesySig()) {
                  qreal leftMargin  = point(styleS(StyleIdx::timesigLeftMargin));
                  Segment* s = m->findSegment(Segment::Type::TimeSigAnnounce, tick);
                  if (s && s->element(0)) {
                        w = static_cast<TimeSig*>(s->element(0))->width() + leftMargin;
                        hasCourtesy = true;
                        }
                  else {
                        ts->layout();
                        w = ts->width() + leftMargin;
                        hasCourtesy = false;
                        }
                  }
            }

      // courtesy key signatures

      showCourtesy = styleB(StyleIdx::genCourtesyKeysig);
      ns           = nm->findSegment(Segment::Type::KeySig, tick);

      qreal wwMax  = 0.0;
      if (showCourtesy && ns) {
            qreal leftMargin = point(styleS(StyleIdx::keysigLeftMargin));
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;

                  KeySig* nks = static_cast<KeySig*>(ns->element(track));

                  if (nks && nks->showCourtesy() && !nks->generated()) {
                        Segment* s  = m->findSegment(Segment::Type::KeySigAnnounce, tick);

                        if (s && s->element(track)) {
                              wwMax = qMax(wwMax, s->element(track)->width()) + leftMargin;
                              hasCourtesy = true;
                              }
                        else {
                              nks->layout();
                              wwMax = qMax(wwMax, nks->width()) + leftMargin;
                              //hasCourtesy = false;
                              }
                        }
                  }
            }
      w += wwMax;

      return w;   //* 1.5
      }

//---------------------------------------------------------
//   layoutSystem
//    return true if line continues
//---------------------------------------------------------

bool Score::layoutSystem(qreal& minWidth, qreal systemWidth, bool isFirstSystem, bool longName)
      {
      if (undoRedo())   // no change possible in this state
            return layoutSystem1(minWidth, isFirstSystem, longName);
      System* system = getNextSystem(isFirstSystem, false);

      qreal xo = 0;
      if (curMeasure->type() == Element::Type::HBOX)
            xo = point(static_cast<Box*>(curMeasure)->boxWidth());

      system->setInstrumentNames(longName);
      system->layoutSystem(xo);

      qreal minMeasureWidth = point(styleS(StyleIdx::minMeasureWidth));
      minWidth              = system->leftMargin();
      bool continueFlag     = false;
      bool isFirstMeasure   = true;
      Measure* firstMeasure = 0;
      Measure* lastMeasure  = 0;

      qreal measureSpacing = styleD(StyleIdx::measureSpacing);

      for (; curMeasure;) {
            MeasureBase* nextMeasure;
            if (curMeasure->type() == Element::Type::MEASURE && !_showVBox)
                  nextMeasure = curMeasure->nextMeasureMM();
            else
                  nextMeasure = curMeasure->nextMM();

            Q_ASSERT(nextMeasure != curMeasure);

            System* oldSystem = curMeasure->system();
            curMeasure->setSystem(system);

            bool hasCourtesy;
            qreal cautionaryW = 0.0;
            qreal ww          = 0.0;

            if (curMeasure->type() == Element::Type::HBOX) {
                  ww = point(static_cast<Box*>(curMeasure)->boxWidth());
                  if (!isFirstMeasure) {
                        // try to put another system on current row
                        // if not a line break
                        switch(_layoutMode) {
                              case LayoutMode::FLOAT:
                                    break;
                              case LayoutMode::LINE:
                              case LayoutMode::PAGE:
                              case LayoutMode::SYSTEM:
                                    continueFlag = !(curMeasure->lineBreak()
                                                     || curMeasure->sectionBreak()
                                                     || curMeasure->pageBreak());
                                    break;
                              }
                        }
                  }
            else if (curMeasure->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  m->createEndBarLines();       // TODO: type not set right here
                  if (isFirstMeasure) {
                        firstMeasure = m;
                        addSystemHeader(m, isFirstSystem);
                        ww = m->minWidth2();
                        }
                  else
                        ww = m->minWidth1();

                  Segment* s = m->last();
                  if ((s->segmentType() == Segment::Type::EndBarLine) && s->element(0)) {
                        BarLine*    bl = static_cast<BarLine*>(s->element(0));
                        BarLineType ot = bl->barLineType();
                        BarLineType nt = m->endBarLineType();

                        if (m->repeatFlags() & Repeat::END)
                              nt = BarLineType::END_REPEAT;
                        else {
                              Measure* nm = m->nextMeasureMM();
                              if (nm && (nm->repeatFlags() & Repeat::START))
                                    nt = BarLineType::START_REPEAT;
                              }
                        if (ot != nt) {
                              qreal mag = bl->magS();
                              ww += BarLine::layoutWidth(this, nt, mag)
                                    - BarLine::layoutWidth(this, ot, mag);
                              }
                        }
                  qreal stretch = m->userStretch() * measureSpacing;
                  ww            *= stretch;
                  cautionaryW   = cautionaryWidth(m, hasCourtesy) * stretch;

                  // if measure does not already have courtesy elements,
                  // add in the amount of space that courtesy elements would take if needed
                  // (if measure *does* already have courtesy elements, these are included in width already)
                  if (!hasCourtesy)
                        ww += cautionaryW;

                  if (ww < minMeasureWidth)
                        ww = minMeasureWidth;
                  isFirstMeasure = false;
                  }

            // collect at least one measure
            bool empty = system->measures().isEmpty();

            if (!empty && (minWidth + ww > systemWidth)) {
                  curMeasure->setSystem(oldSystem);
                  continueFlag = false;
                  break;
                  }
            if (curMeasure->type() == Element::Type::MEASURE)
                  lastMeasure = static_cast<Measure*>(curMeasure);

            system->measures().append(curMeasure);

            Element::Type nt;
            if (_showVBox)
                  nt = curMeasure->nextMM() ? curMeasure->nextMM()->type() : Element::Type::INVALID;
            else
                  nt = curMeasure->nextMeasureMM() ? curMeasure->nextMeasureMM()->type() : Element::Type::INVALID;

            bool pbreak;
            switch (_layoutMode) {
                  case LayoutMode::PAGE:
                  case LayoutMode::SYSTEM:
                        pbreak = curMeasure->pageBreak() || curMeasure->lineBreak() || curMeasure->sectionBreak();
                        break;
                  case LayoutMode::FLOAT:
                  case LayoutMode::LINE:
                        pbreak = false;
                        break;
                  }
            if (continueFlag
               || pbreak
               || (nt == Element::Type::VBOX || nt == Element::Type::TBOX || nt == Element::Type::FBOX)
               ) {
                  if (_layoutMode != LayoutMode::SYSTEM)
                        system->setPageBreak(curMeasure->pageBreak());
                  minWidth += ww;
                  curMeasure = nextMeasure;
                  break;
                  }
            curMeasure = nextMeasure;
            if (minWidth + minMeasureWidth > systemWidth)   // small optimization
                  break;      // next measure will not fit

            minWidth += ww;
            // whether the measure actually has courtesy elements or whether we added space for hypothetical ones,
            // we should remove the width of courtesy elements for this measure from the accumulated total
            // since at this point we are assuming we may be able to fit another measure
            minWidth -= cautionaryW;
            }

      //
      // remember line breaks in list of measures
      //
      int n = system->measures().size() - 1;
      if (n >= 0) {
            for (int i = 0; i < n; ++i)
                  undoChangeProperty(system->measure(i), P_ID::BREAK_HINT, false);
            undoChangeProperty(system->measures().last(), P_ID::BREAK_HINT, true);
            }

      if (firstMeasure && lastMeasure && firstMeasure != lastMeasure)
            removeGeneratedElements(firstMeasure, lastMeasure);

      hideEmptyStaves(system, isFirstSystem);

      return continueFlag && curMeasure;
      }

//---------------------------------------------------------
//   hideEmptyStaves
//---------------------------------------------------------

void Score::hideEmptyStaves(System* system, bool isFirstSystem)
      {
       //
      //    hide empty staves
      //
      int staves = _staves.size();
      int staffIdx = 0;
      bool systemIsEmpty = true;

      foreach (Staff* staff, _staves) {
            SysStaff* s  = system->staff(staffIdx);
            bool oldShow = s->show();
            if (styleB(StyleIdx::hideEmptyStaves)
               && (staves > 1)
               && !(isFirstSystem && styleB(StyleIdx::dontHideStavesInFirstSystem))
               && !staff->neverHide()
               ) {
                  bool hideStaff = true;
                  foreach(MeasureBase* m, system->measures()) {
                        if (m->type() != Element::Type::MEASURE)
                              continue;
                        Measure* measure = static_cast<Measure*>(m);
                        if (!measure->isMeasureRest(staffIdx)) {
                              hideStaff = false;
                              break;
                              }
                        }
                  // check if notes moved into this staff
                  Part* part = staff->part();
                  int n = part->nstaves();
                  if (hideStaff && (n > 1)) {
                        int idx = part->staves()->front()->idx();
                        for (int i = 0; i < part->nstaves(); ++i) {
                              int st = idx + i;

                              foreach(MeasureBase* mb, system->measures()) {
                                    if (mb->type() != Element::Type::MEASURE)
                                          continue;
                                    Measure* m = static_cast<Measure*>(mb);
                                    for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
                                          for (int voice = 0; voice < VOICES; ++voice) {
                                                ChordRest* cr = static_cast<ChordRest*>(s->element(st * VOICES + voice));
                                                if (cr == 0 || cr->type() == Element::Type::REST)
                                                      continue;
                                                int staffMove = cr->staffMove();
                                                if (staffIdx == st + staffMove) {
                                                      hideStaff = false;
                                                      break;
                                                      }
                                                }
                                          }
                                    if (!hideStaff)
                                          break;
                                    }
                              if (!hideStaff)
                                    break;
                              }
                        }
                  s->setShow(hideStaff ? false : staff->show());
                  if (s->show()) {
                        systemIsEmpty = false;
                        }
                  }
            else {
                  systemIsEmpty = false;
                  s->setShow(true);
                  }

            if (oldShow != s->show()) {
                  foreach (MeasureBase* mb, system->measures()) {
                        if (mb->type() != Element::Type::MEASURE)
                              continue;
                        static_cast<Measure*>(mb)->createEndBarLines();
                        }
                  }
            ++staffIdx;
            }
      if (systemIsEmpty) {
            foreach (Staff* staff, _staves) {
                  SysStaff* s  = system->staff(staff->idx());
                  if (staff->showIfEmpty() && !s->show()) {
                        s->setShow(true);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutSystem1
//    used in undoRedo state
//    return true if line continues
//---------------------------------------------------------

bool Score::layoutSystem1(qreal& minWidth, bool isFirstSystem, bool longName)
      {
      System* system = getNextSystem(isFirstSystem, false);

      qreal xo = 0;
      if (curMeasure->type() == Element::Type::HBOX)
            xo = point(static_cast<Box*>(curMeasure)->boxWidth());

      system->setInstrumentNames(longName);
      system->layoutSystem(xo);

      qreal minMeasureWidth = point(styleS(StyleIdx::minMeasureWidth));
      minWidth              = system->leftMargin();
      bool continueFlag     = false;
      bool isFirstMeasure   = true;

      for (; curMeasure;) {
            MeasureBase* nextMeasure;
            if (curMeasure->type() == Element::Type::MEASURE && !_showVBox)
                  nextMeasure = curMeasure->nextMeasureMM();
            else
                  nextMeasure = curMeasure->nextMM();

            // System* oldSystem = curMeasure->system();
            curMeasure->setSystem(system);
            qreal ww = 0.0;

            if (curMeasure->type() == Element::Type::HBOX) {
                  ww = point(static_cast<Box*>(curMeasure)->boxWidth());
                  if (!isFirstMeasure) {
                        // try to put another system on current row
                        // if not a line break
                        switch(_layoutMode) {
                              case LayoutMode::FLOAT:
                                    break;
                              case LayoutMode::LINE:
                              case LayoutMode::PAGE:
                              case LayoutMode::SYSTEM:
                                    continueFlag = !(curMeasure->lineBreak()
                                                    || curMeasure->sectionBreak()
                                                    || curMeasure->pageBreak());
                                    break;
                              }
                        }
                  }
            else if (curMeasure->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  m->createEndBarLines();       // TODO: type not set right here
                  if (isFirstMeasure) {
                        addSystemHeader(m, isFirstSystem);
                        ww = m->minWidth2();
                        }
                  else
                        ww = m->minWidth1();

                  ww *= m->userStretch() * styleD(StyleIdx::measureSpacing);
                  if (ww < minMeasureWidth)
                        ww = minMeasureWidth;
                  isFirstMeasure = false;
                  }

            minWidth += ww;

            system->measures().append(curMeasure);
            Element::Type nt = curMeasure->next() ? curMeasure->next()->type() : Element::Type::INVALID;
            bool pbreak;
            switch (_layoutMode) {
                  case LayoutMode::PAGE:
                  case LayoutMode::SYSTEM:
                        pbreak = curMeasure->pageBreak() || curMeasure->lineBreak() || curMeasure->sectionBreak();
                        break;
                  case LayoutMode::FLOAT:
                  case LayoutMode::LINE:
                        pbreak = false;
                        break;
                  }
            if (continueFlag || pbreak || (nt == Element::Type::VBOX || nt == Element::Type::TBOX || nt == Element::Type::FBOX)) {
                  if (_layoutMode != LayoutMode::SYSTEM)
                        system->setPageBreak(curMeasure->pageBreak());
                  curMeasure = nextMeasure;
                  break;
                  }
            // do not change line break
            if (curMeasure->breakHint()) {
                  curMeasure = nextMeasure;
                  break;
                  }
            curMeasure = nextMeasure;
            }

      hideEmptyStaves(system,isFirstSystem);

      return continueFlag && curMeasure;
      }

//---------------------------------------------------------
//   removeGeneratedElements (System Header + TimeSig Announce)
//    helper function
//---------------------------------------------------------

void Score::removeGeneratedElements(Measure* sm, Measure* em)
      {
      Measure* sectionStart = sm;
      for (Measure* m = sm; m; m = m->nextMeasureMM()) {
            //
            // remove generated elements from all measures in [sm;em]
            //    assume: generated elements are only living in voice 0
            //    - do not remove end bar lines
            //    - set size of clefs to small
            //
            if (m->sectionBreak() && m->nextMeasureMM())
                  sectionStart = m->nextMeasureMM();
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  Segment::Type st = seg->segmentType();
                  if (st == Segment::Type::EndBarLine)
                        continue;
                  if (st == Segment::Type::StartRepeatBarLine && m != sm) {
                        if (!undoRedo())
                              undoRemoveElement(seg);
                        else
                              qDebug("remove repeat segment in undo/redo");
                        continue;
                        }
                  for (int staffIdx = 0;  staffIdx < nstaves(); ++staffIdx) {
                        Element* el = seg->element(staffIdx * VOICES);
                        if (el == 0)
                              continue;

                        // courtesy time sigs and key sigs: remove if not in last measure (generated or not!)
                        // clefs & keysig: remove if generated and not at beginning of first measure of a section
                        if ( ((st == Segment::Type::TimeSigAnnounce || st == Segment::Type::KeySigAnnounce) && m != em)
                              || ((el->type() == Element::Type::CLEF || el->type() == Element::Type::KEYSIG) && el->generated() && seg->tick() != sectionStart->tick())
                        )
                              {
                              undoRemoveElement(el);
                              }
                        else if (el->type() == Element::Type::CLEF) {
                              Clef* clef = static_cast<Clef*>(el);
                              System* s = m->system();
                              bool small = seg != m->first() || s->firstMeasure() != m;
                              if (clef->small() != small) {
                                    clef->setSmall(small);
                                    m->setDirty();
                                    }
                              }
                        }
                  }
            if (m == em)
                  break;
            }
      }

//---------------------------------------------------------
//   addPage
//---------------------------------------------------------

Page* Score::addPage()
      {
      Page* page = new Page(this);
      page->setNo(_pages.size());
      _pages.push_back(page);
      return page;
      }

//---------------------------------------------------------
//   connectTies
///   Rebuild tie connections.
//---------------------------------------------------------

void Score::connectTies(bool silent)
      {
      int tracks = nstaves() * VOICES;
      Measure* m = firstMeasure();
      if (!m)
            return;
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* s = m->first(st); s; s = s->next1(st)) {
            for (int i = 0; i < tracks; ++i) {
                  Chord* c = static_cast<Chord*>(s->element(i));
                  if (c == 0 || c->type() != Element::Type::CHORD)
                        continue;
                  // connect grace note tie to main note in 1.3 scores
                  if (_mscVersion <= 114) {
                        for (Chord* gc : c->graceNotes()) {
                              for (Note* gn : gc->notes()) {
                                    Tie* tie = gn->tieFor();
                                    if (tie && !tie->endNote()) {
                                          for (Note* n : c->notes()) {
                                                if (n->pitch() == gn->pitch()) {
                                                      tie->setEndNote(n);
                                                       n->setTieBack(tie);
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
                  for (Note* n : c->notes()) {
                        // connect a tie without end note
                        Tie* tie = n->tieFor();
                        if (tie && !tie->endNote()) {
                              Note* nnote;
                              if (_mscVersion <= 114)
                                    nnote = searchTieNote114(n);
                              else
                                    nnote = searchTieNote(n);
                              if (nnote == 0) {
                                    if (!silent) {
                                          qDebug("next note at %d track %d for tie not found (version %d)", s->tick(), i, _mscVersion);
                                          delete tie;
                                          n->setTieFor(0);
                                          }
                                    }
                              else {
                                    tie->setEndNote(nnote);
                                    nnote->setTieBack(tie);
                                    }
                              }
                        // connect a glissando without initial note (old glissando format)
                        for (Spanner* spanner : n->spannerBack())
                              if (spanner->type() == Element::Type::GLISSANDO
                                          && spanner->startElement() == nullptr) {
                                    Note* initialNote = Glissando::guessInitialNote(n->chord());
                                    n->removeSpannerBack(spanner);
                                    if (initialNote != nullptr) {
                                          spanner->setStartElement(initialNote);
                                          spanner->setEndElement(n);
                                          spanner->setTick(initialNote->chord()->tick());
                                          spanner->setTick2(n->chord()->tick());
                                          spanner->setTrack(n->track());
                                          spanner->setTrack2(n->track());
                                          spanner->setParent(initialNote);
                                          initialNote->add(spanner);
                                          }
                                    else
                                          delete spanner;
                                    }
                        }
                  // connect two note tremolos
                  Tremolo* tremolo = c->tremolo();
                  if (tremolo && tremolo->twoNotes() && !tremolo->chord2()) {
                        for (Segment* ls = s->next1(st); ls; ls = ls->next1(st)) {
                              Chord* nc = static_cast<Chord*>(ls->element(i));
                              if (nc == 0)
                                    continue;
                              if (nc->type() != Element::Type::CHORD)
                                    qDebug("cannot connect tremolo");
                              else {
                                    nc->setTremolo(tremolo);
                                    tremolo->setChords(c, nc);
                                    // cross-measure tremolos are not supported
                                    // but can accidentally result from copy & paste
                                    // remove them now
                                    if (c->measure() != nc->measure())
                                          c->remove(tremolo);
                                    }
                              break;
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutFingering
//    - place numbers above a note execpt for the last
//      staff in a multi stave part (piano)
//    - does not handle chords
//---------------------------------------------------------

void Score::layoutFingering(Fingering* f)
      {
      if (f == 0)
            return;
      TextStyleType tst = f->textStyleType();
      if (tst != TextStyleType::FINGERING && tst != TextStyleType::RH_GUITAR_FINGERING && tst != TextStyleType::STRING_NUMBER)
            return;

      Note* note   = f->note();
      Chord* chord = note->chord();
      Staff* staff = chord->staff();
      Part* part   = staff->part();
      int n        = part->nstaves();
      bool voices  = chord->measure()->hasVoices(staff->idx());
      bool below   = voices ? !chord->up() : (n > 1) && (staff->rstaff() == n-1);
      bool tight   = voices && !chord->beam();

      f->layout();
      qreal x = 0.0;
      qreal y = 0.0;
      qreal headWidth = note->headWidth();
      qreal headHeight = note->headHeight();
      qreal fh = headHeight;        // TODO: fingering number height

      if (chord->notes().size() == 1) {
            x = headWidth * .5;
            if (below) {
                  // place fingering below note
                  y = fh + spatium() * .4;
                  if (tight) {
                        y += 0.5 * spatium();
                        if (chord->stem())
                              x += 0.5 * spatium();
                        }
                  else if (chord->stem() && !chord->up()) {
                        // on stem side
                        y += chord->stem()->height();
                        x -= spatium() * .4;
                        }
                  }
            else {
                  // place fingering above note
                  y = -headHeight - spatium() * .4;
                  if (tight) {
                        y -= 0.5 * spatium();
                        if (chord->stem())
                              x -= 0.5 * spatium();
                        }
                  else if (chord->stem() && chord->up()) {
                        // on stem side
                        y -= chord->stem()->height();
                        x += spatium() * .4;
                        }
                  }
            }
      else {
            x -= spatium();
            }
      f->setUserOff(QPointF(x, y));
      }

//---------------------------------------------------------
//   layoutSystemRow
//    return height in h
//---------------------------------------------------------

QList<System*> Score::layoutSystemRow(qreal rowWidth, bool isFirstSystem, bool useLongName)
      {
      bool raggedRight = MScore::layoutDebug;

      QList<System*> sl;

      qreal ww = rowWidth;
      qreal minWidth;
      for (bool a = true; a;) {
            a = layoutSystem(minWidth, ww, isFirstSystem, useLongName);
            sl.append(_systems[curSystem]);
            ++curSystem;
            ww -= minWidth;
            }
      //
      // dont stretch last system row, if minWidth is <= lastSystemFillLimit
      //
      if (curMeasure == 0 && ((minWidth / rowWidth) <= styleD(StyleIdx::lastSystemFillLimit)))
            raggedRight = true;

      //-------------------------------------------------------
      //    Round II
      //    stretch measures
      //    "nm" measures fit on this line of score
      //-------------------------------------------------------

      bool needRelayout = false;

      foreach (System* system, sl) {
            // set system initial bar line type here, as in System::layout...() methods
            // it is either too early (in System::layout() measures are not added to the system yet)
            // or too late (in System::layout2(), horizontal spacing has already been done
            //    and bar line width will be ignored).
            // We only set bar line type here; track and bar line span values are set in System::layout2()
            if (system->barLine() && system->firstMeasure())
                  system->barLine()->setBarLineType(system->firstMeasure()->systemInitialBarLineType());

            //
            //    add cautionary time/key signatures if needed
            //

            if (system->measures().isEmpty()) {
                  qFatal("System %p is empty", system);
                  }
            Measure* m = system->lastMeasure();
            bool hasCourtesyKeysig = false;
            Measure* nm = m ? m->nextMeasure() : 0;
            Segment* s;

            if (m && nm) {
                  int tick = m->tick() + m->ticks();

                  // locate a time sig. in the next measure and, if found,
                  // check if it has cout. sig. turned off
                  TimeSig* ts;
                  Segment* tss         = nm->findSegment(Segment::Type::TimeSig, tick);
                  bool showCourtesySig = tss && styleB(StyleIdx::genCourtesyTimesig) && !(m->sectionBreak() && _layoutMode != LayoutMode::FLOAT);
                  if (showCourtesySig) {
                        ts = static_cast<TimeSig*>(tss->element(0));
                        if (ts && !ts->showCourtesySig())
                              showCourtesySig = false;     // this key change has court. sig turned off
                        }
                  if (showCourtesySig) {
                        // if due, create a new courtesy time signature for each staff
                        s  = m->undoGetSegment(Segment::Type::TimeSigAnnounce, tick);
                        int nstaves = Score::nstaves();
                        for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                              TimeSig* nts = static_cast<TimeSig*>(tss->element(track));
                              if (!nts)
                                    continue;
                              ts = static_cast<TimeSig*>(s->element(track));
                              if (ts == 0) {
                                    ts = new TimeSig(this);
                                    ts->setTrack(track);
                                    ts->setGenerated(true);
                                    ts->setParent(s);
                                    undoAddElement(ts);
                                    }
                              ts->setFrom(nts);
                              m->setDirty();
                              }
                        }
                  else {
                        // remove any existing time signatures
                        Segment* tss = m->findSegment(Segment::Type::TimeSigAnnounce, tick);
                        if (tss) {
                              undoRemoveElement(tss);
                              }
                        }

                  // courtesy key signatures
                  int n = _staves.size();
                  for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                        int track = staffIdx * VOICES;
                        Staff* staff = _staves[staffIdx];
                        showCourtesySig = styleB(StyleIdx::genCourtesyKeysig) && !(m->sectionBreak() && _layoutMode != LayoutMode::FLOAT);

                        KeySigEvent key1 = staff->keySigEvent(tick - 1);
                        KeySigEvent key2 = staff->keySigEvent(tick);
                        if (showCourtesySig && !(key1 == key2)) {
                              // locate a key sig. in next measure and, if found,
                              // check if it has court. sig turned off
                              s = nm->findSegment(Segment::Type::KeySig, tick);
                              showCourtesySig = true; // assume this key change has court. sig turned on
                              if (s) {
                                    KeySig* ks = static_cast<KeySig*>(s->element(track));
                                    if (ks && !ks->showCourtesy())
                                          showCourtesySig = false;     // this key change has court. sig turned off
                                    }

                              if (showCourtesySig) {
                                    hasCourtesyKeysig = true;
                                    s  = m->undoGetSegment(Segment::Type::KeySigAnnounce, tick);
                                    KeySig* ks = static_cast<KeySig*>(s->element(track));

                                    if (!ks) {
                                          ks = new KeySig(this);
                                          ks->setKeySigEvent(key2);
                                          ks->setTrack(track);
                                          ks->setGenerated(true);
                                          ks->setParent(s);
                                          undoAddElement(ks);
                                          }
                                    else if (!(ks->keySigEvent() == key2)) {
                                          undo(new ChangeKeySig(ks, key2, ks->showCourtesy()));
                                          }
                                    // change bar line to qreal bar line
                                    // m->setEndBarLineType(BarLineType::DOUBLE, true); // this caused issue #12918
                                    }
                              }
                        if (!showCourtesySig) {
                              // remove any existent courtesy key signature
                              Segment* s = m->findSegment(Segment::Type::KeySigAnnounce, tick);
                              if (s && s->element(track))
                                    undoRemoveElement(s->element(track));
                              }
                        }

                  // courtesy clefs: show/hide of courtesy clefs moved to Clef::layout()
                  }

            //
            //    compute repeat bar lines
            //
            bool firstMeasure = true;
            const QList<MeasureBase*>& ml = system->measures();
            MeasureBase* lmb = ml.back();

            for (MeasureBase* mb : ml) {
                  if (mb->type() != Element::Type::MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  // first measure repeat?
                  bool fmr = firstMeasure && (m->repeatFlags() & Repeat::START);

                  if (mb == lmb) {       // last measure in system?
                        //
                        // if last bar has a courtesy key signature,
                        // create a double bar line as end bar line
                        //
                        BarLineType bl = hasCourtesyKeysig ? BarLineType::DOUBLE : BarLineType::NORMAL;
                        if (m->repeatFlags() & Repeat::END)
                              m->setEndBarLineType(BarLineType::END_REPEAT, m->endBarLineGenerated());
                        else if (m->endBarLineGenerated())
                              m->setEndBarLineType(bl, true);
                        if (m->setStartRepeatBarLine(fmr))
                              m->setDirty();
                        }
                  else {
                        MeasureBase* mb = m->next();
                        while (mb && mb->type() != Element::Type::MEASURE && (mb != lmb))
                              mb = mb->next();

                        Measure* nm = 0;
                        if (mb && mb->type() == Element::Type::MEASURE)
                              nm = static_cast<Measure*>(mb);

                        needRelayout |= m->setStartRepeatBarLine(fmr);
                        if (m->repeatFlags() & Repeat::END) {
                              if (nm && (nm->repeatFlags() & Repeat::START))
                                    m->setEndBarLineType(BarLineType::END_START_REPEAT, m->endBarLineGenerated());
                              else
                                    m->setEndBarLineType(BarLineType::END_REPEAT, m->endBarLineGenerated());
                              }
                        else if (nm && (nm->repeatFlags() & Repeat::START))
                              m->setEndBarLineType(BarLineType::START_REPEAT, m->endBarLineGenerated());
                        else if (m->endBarLineGenerated())
                              m->setEndBarLineType(BarLineType::NORMAL, m->endBarLineGenerated());
                        }
                  if (m->createEndBarLines())
                        m->setDirty();
                  firstMeasure = false;
                  }
            }

      minWidth          = 0.0;
      qreal totalWeight = 0.0;

      foreach(System* system, sl) {
            foreach (MeasureBase* mb, system->measures()) {
                  if (mb->type() == Element::Type::HBOX)
                        minWidth += point(((Box*)mb)->boxWidth());
                  else if (mb->type() == Element::Type::MEASURE) {
                        Measure* m = (Measure*)mb;
                        if (needRelayout)
                              m->setDirty();
                        minWidth    += m->minWidth2();
                        totalWeight += m->ticks() * m->userStretch();
                        }
                  }
            minWidth += system->leftMargin();
            }

      // stretch incomplete row
      qreal rest;
      if (MScore::layoutDebug)
            rest = 0;
      else {
            rest = rowWidth - minWidth;
            if (raggedRight) {
                  if (minWidth > rest)
                        rest = rest * .5;
                  else
                        rest = minWidth;
                  }
            rest /= totalWeight;
            }
      qreal xx   = 0.0;
      qreal y    = 0.0;

      foreach(System* system, sl) {
            QPointF pos;

            bool firstMeasure = true;
            foreach(MeasureBase* mb, system->measures()) {
                  qreal ww = 0.0;
                  if (mb->type() == Element::Type::MEASURE) {
                        if (firstMeasure) {
                              pos.rx() += system->leftMargin();
                              firstMeasure = false;
                              }
                        mb->setPos(pos);
                        Measure* m    = static_cast<Measure*>(mb);
                        qreal weight = m->ticks() * m->userStretch();
                        ww           = m->minWidth2() + rest * weight;
                        m->layoutWidth(ww);
                        }
                  else if (mb->type() == Element::Type::HBOX) {
                        mb->setPos(pos);
                        ww = point(static_cast<Box*>(mb)->boxWidth());
                        mb->layout();
                        }
                  pos.rx() += ww;
                  }
            system->setPos(xx, y);
            qreal w = pos.x();
            system->setWidth(w);
            system->layout2();
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() == Element::Type::HBOX) {
                        mb->setHeight(system->height());
                        }
                  }
            xx += w;
            }
      return sl;
      }

//---------------------------------------------------------
//   layoutSystems
//   create list of systems
//---------------------------------------------------------

void Score::layoutSystems()
      {
      curMeasure              = _showVBox ? firstMM() : firstMeasureMM();
      curSystem               = 0;
      bool firstSystem        = true;
      bool startWithLongNames = true;

      qreal w  = pageFormat()->printableWidth() * MScore::DPI;

      while (curMeasure) {
            Element::Type t = curMeasure->type();
            if (t == Element::Type::VBOX || t == Element::Type::TBOX || t == Element::Type::FBOX) {
                  System* system = getNextSystem(false, true);
                  system->setSameLine(false);
                  foreach(SysStaff* ss, *system->staves())
                        delete ss;
                  system->staves()->clear();

                  system->setWidth(w);
                  VBox* vbox = static_cast<VBox*>(curMeasure);
                  vbox->setParent(system);
                  vbox->layout();
                  system->setHeight(vbox->height());
                  system->rxpos() = 0.0;
                  system->setPageBreak(vbox->pageBreak());
                  system->measures().push_back(vbox);
                  curMeasure = curMeasure->nextMM();
                  ++curSystem;
                  }
            else {
                  QList<System*> sl  = layoutSystemRow(w, firstSystem, startWithLongNames);
                  for (int i = 0; i < sl.size(); ++i)
                        sl[i]->setSameLine(i != 0);
                  firstSystem = false;
                  startWithLongNames = false;
                  if (!sl.isEmpty()) {
                        Measure* lm = sl.back()->lastMeasure();
                        firstSystem = lm && lm->sectionBreak() && _layoutMode != LayoutMode::FLOAT;
                        startWithLongNames = firstSystem && lm->sectionBreak()->startWithLongNames();
                        }
                  else
                        qDebug("empty system!");
                  }
            }
      // TODO: make undoable:
      while (_systems.size() > curSystem)
            _systems.takeLast();
      }

//---------------------------------------------------------
//   layoutSystems2
//    update distanceUp, distanceDown
//---------------------------------------------------------

void Score::layoutSystems2()
      {
      int n = _systems.size();
      for (int i = 0; i < n; ++i) {
            System* system = _systems.at(i);
            if (!system->isVbox()) {
                  system->layout2();
                  }
            }
      }

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Score::layoutLinear()
      {
      curMeasure     = first();
      curSystem      = 0;
      System* system = getNextSystem(true, false);
      system->setInstrumentNames(true);
      qreal xo = 0;

      Measure* fm = firstMeasure();
      for (MeasureBase* m = first(); m != fm ; m = m->next()) {
            if (m->type() == Element::Type::HBOX)
                  xo += point(static_cast<Box*>(m)->boxWidth());
            }

      system->layoutSystem(xo);
      system->setPos(0.0, spatium() * 10.0);
      curPage = 0;
      Page* page = getEmptyPage();
      page->appendSystem(system);

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            Element::Type t = mb->type();
            if (t == Element::Type::VBOX || t == Element::Type::TBOX || t == Element::Type::FBOX) {
                  continue;
                  }
            if (styleB(StyleIdx::createMultiMeasureRests) && mb->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  if (m->hasMMRest())
                        mb = m->mmRest();
                  }
            mb->setSystem(system);
            system->measures().append(mb);
            }
      if (system->measures().isEmpty())
            return;
      addSystemHeader(firstMeasureMM(), true);
      // also add a system header after a section break
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
            if (m->sectionBreak() && m->nextMeasureMM())
                  addSystemHeader(m->nextMeasureMM(), true);
            }
      removeGeneratedElements(firstMeasureMM(), lastMeasureMM());

      QPointF pos(0.0, 0.0);
      bool isFirstMeasure = true;
      foreach (MeasureBase* mb, system->measures()) {
            qreal w = 0.0;
            if (mb->type() == Element::Type::MEASURE) {
                  if (isFirstMeasure)
                        pos.rx() += system->leftMargin();
                  Measure* m = static_cast<Measure*>(mb);
                  Measure* nm = m->nextMeasure();
                  if (m->repeatFlags() & Repeat::END) {
                        if (nm && (nm->repeatFlags() & Repeat::START))
                              m->setEndBarLineType(BarLineType::END_START_REPEAT, m->endBarLineGenerated());
                        else
                              m->setEndBarLineType(BarLineType::END_REPEAT, m->endBarLineGenerated());
                        }
                  else if (nm && (nm->repeatFlags() & Repeat::START))
                        m->setEndBarLineType(BarLineType::START_REPEAT, m->endBarLineGenerated());
                  m->createEndBarLines();
                  if (isFirstMeasure) {
                        // width with header
                        qreal w2 = m->minWidth2();
                        // width *completely* excluding header
                        // minWidth1() includes the initial key / time signatures since they are considered non-generated
                        Segment* s = m->first();
                        while (s && s->segmentType() != Segment::Type::ChordRest)
                              s = s->next();
                        qreal w1 = s ? computeMinWidth(s, true) : m->minWidth1();
                        w = (w2 - w1) + w1 * styleD(StyleIdx::linearStretch);
                        }
                  else {
                        w = m->minWidth1() * styleD(StyleIdx::linearStretch);
                        }
                  qreal minMeasureWidth = point(styleS(StyleIdx::minMeasureWidth));
                  if (w < minMeasureWidth)
                        w = minMeasureWidth;
                  m->layoutWidth(w);
                  isFirstMeasure = false;
                  }
            else {
                  mb->layout();
                  w = mb->width();
                 }

            mb->setPos(pos);
            pos.rx() += w;
            }
      system->setWidth(pos.x());
      page->setWidth(pos.x());
      system->layout2();
      page->setHeight(system->height() + 20 * spatium());

      while (_pages.size() > 1)
            _pages.takeLast();
      }

//---------------------------------------------------------
//   getEmptyPage
//---------------------------------------------------------

Page* Score::getEmptyPage()
      {
      Page* page = curPage >= _pages.size() ? addPage() : _pages[curPage];
      page->setNo(curPage);
      page->layout();
      qreal x = (curPage == 0) ? 0.0 : _pages[curPage - 1]->pos().x()
         + page->width() + (((curPage+_pageNumberOffset) & 1) ? 50.0 : 1.0);
      ++curPage;
      page->setPos(x, 0.0);
      page->systems()->clear();
      return page;
      }

//---------------------------------------------------------
//   SystemRow
//---------------------------------------------------------

struct SystemRow {
      QList<System*> systems;

      qreal height() const {
            qreal h = 0.0;
            foreach(System* s, systems) {
                  if (s->height() > h)
                        h = s->height();
                  }
            return h;
            }
      bool isVbox() const {
            return (systems.size() > 0) ? systems.back()->isVbox() : false;
            }
      VBox* vbox() const {
            return (systems.size() > 0) ? systems.back()->vbox() : 0;
            }
      bool pageBreak() const {
            return (systems.size() > 0) ? systems.back()->pageBreak() : false;
            }
      qreal tm() const {
            qreal v = 0.0;
            foreach(System* s, systems) {
                  if (!s->staves()->isEmpty())
                        v = qMax(s->distanceUp(0), v);
                  }
            return v;
            }
      qreal bm() const {
            qreal v = 0.0;
            foreach(System* s, systems) {
                  int staffIdx = s->staves()->size() - 1;
                  if (staffIdx >= 0)
                        v = qMax(s->distanceDown(staffIdx), v);
                  }
            return v;
            }
      qreal extraDistance(int i) const {
            qreal v = 0.0;
            foreach(System* s, systems) {
                  foreach(MeasureBase* mb, s->measures())
                        v = qMax(mb->distanceDown(i), v);
                  }
            return v;
            }
      void clear() {
            systems.clear();
            }
      };

//---------------------------------------------------------
//   PageContext
//---------------------------------------------------------

struct PageContext {
      qreal _spatium;
      qreal slb;
      Score* score;
      Page* page;
      qreal ey;
      qreal y;
      int gaps;
      SystemRow sr;

      System* lastSystem;
      qreal prevDist;

      PageContext(Score* s) : score(s) {
            _spatium = score->spatium();
            slb      = score->styleS(StyleIdx::staffLowerBorder).val() * _spatium;
            }
      void newPage() {
            page       = score->getEmptyPage();
            ey         = page->height() - page->bm();
            y          = page->tm();
            gaps       = 0;
            lastSystem = 0;
            prevDist   = 0.0;
            }
      void layoutPage() {
            qreal d = sr.isVbox() ? sr.vbox()->bottomGap() : slb;
            score->layoutPage(*this, d);
            }
      qreal bm() const { return sr.bm(); }
      qreal tm() const { return sr.tm(); }
      };


//---------------------------------------------------------
//   layoutPages
//    create list of pages
//---------------------------------------------------------

void Score::layoutPages()
      {
      const qreal _spatium            = spatium();
      const qreal slb                 = styleS(StyleIdx::staffLowerBorder).val()    * _spatium;
      const qreal sub                 = styleS(StyleIdx::staffUpperBorder).val()    * _spatium;
      const qreal systemDist          = styleS(StyleIdx::minSystemDistance).val()   * _spatium;
      const qreal systemFrameDistance = styleS(StyleIdx::systemFrameDistance).val() * _spatium;
      const qreal frameSystemDistance = styleS(StyleIdx::frameSystemDistance).val() * _spatium;

      curPage = 0;

      PageContext pC(this);
      pC.newPage();

      int nSystems = _systems.size();

PAGEDBG("layoutPages");
      for (int i = 0; i < nSystems; ++i) {
PAGEDBG("  system %d", i);
            //
            // collect system row
            //
            pC.sr.clear();
            for (;;) {
                  System* system = _systems[i];
                  pC.sr.systems.append(system);
                  if (i+1 == nSystems)
                        break;
                  if (!_systems[i+1]->sameLine())
                        break;
                  ++i;
                  }
            qreal tmargin;    // top system margin
            qreal bmargin;    // bottom system margin

            if (pC.sr.isVbox()) {
                  VBox* vbox = pC.sr.vbox();
                  bmargin  = vbox->bottomGap();
                  tmargin  = vbox->topGap();
                  if (pC.lastSystem) {
                        if (pC.lastSystem->isVbox())
                              tmargin += pC.lastSystem->vbox()->bottomGap();
                        else
                              tmargin += systemFrameDistance;
                        }
                  }
            else {
                  if (pC.lastSystem) {
                        if (pC.lastSystem->isVbox())
                              tmargin = pC.lastSystem->vbox()->bottomGap() + frameSystemDistance;
                        else
                              tmargin = qMax(pC.tm(), systemDist);
                        }
                  else
                        tmargin = qMax(pC.tm(), sub);
                  bmargin = pC.bm();
                  }

            tmargin = qMax(tmargin, pC.prevDist);
            qreal h = pC.sr.height();

PAGEDBG("   y:%f + h:%f + tm:%f + qMax(bm:%f,%f)[=%f] > %f",
    pC.y    / MScore::DPMM,
    h       / MScore::DPMM,
    tmargin / MScore::DPMM,
    bmargin / MScore::DPMM,
    slb     / MScore::DPMM,
    (pC.y + h + tmargin + qMax(bmargin, slb)) / MScore::DPMM,
    pC.ey   / MScore::DPMM
    );

            if (pC.lastSystem && (pC.y + h + tmargin + qMax(bmargin, slb) > pC.ey)) {
PAGEDBG("      == page break");
                  //
                  // prepare next page
                  //
                  qreal d;
                  if (pC.lastSystem->isVbox())
                        d = pC.lastSystem->vbox()->bottomGap();
                  else
                        d = slb;
                  layoutPage(pC, d);
                  pC.newPage();
                  if (pC.sr.isVbox())
                        tmargin = pC.sr.vbox()->topGap();
                  else
                        tmargin = qMax(pC.sr.tm(), sub);
                  }

            qreal x = pC.page->lm();
            pC.y   += tmargin;
            pC.prevDist = bmargin;

            for (System* system : pC.sr.systems) {
                  system->setPos(x, pC.y);
                  x += system->width();
                  pC.page->appendSystem(system);
                  system->setAddStretch(false);
                  }

            if (pC.lastSystem) {
                  bool addStretch = !pC.lastSystem->isVbox() && !pC.sr.isVbox();
                  pC.lastSystem->setAddStretch(addStretch);
                  if (addStretch)
                        ++pC.gaps;
                  }

            pC.y += h;
            if (pC.sr.pageBreak() && (_layoutMode == LayoutMode::PAGE)) {
                  if ((i + 1) == nSystems)
                        break;
                  pC.layoutPage();
                  pC.newPage();
                  }
            else
                  pC.lastSystem = pC.sr.systems.back();
            }
      if (pC.page)
            pC.layoutPage();

      // Remove not needed pages. TODO: make undoable:
      while (_pages.size() > curPage)
            _pages.takeLast();
      }

//---------------------------------------------------------
//   systemDistCompare
//---------------------------------------------------------

static bool systemDistCompare(System* s1, System* s2)
      {
      return s1->distance() < s2->distance();
      }

//---------------------------------------------------------
//   layoutPage
//    increase system distance upto maxSystemDistance
//    to fill page
//---------------------------------------------------------

void Score::layoutPage(const PageContext& pC, qreal d)
      {
      Page* page       = pC.page;
      int gaps         = pC.gaps;
      qreal restHeight = pC.ey - pC.y - d;

      if (!gaps || MScore::layoutDebug) {
            if (_layoutMode == LayoutMode::FLOAT) {
                  qreal y = restHeight * .5;
                  int n = page->systems()->size();
                  for (int i = 0; i < n; ++i) {
                        System* system = page->systems()->at(i);
                        system->move(QPointF(0.0, y));
                        }
                  }
            return;
            }

      // allow room for lyrics (and/or spacer) on last visible staff of last system
      // these are not included in system height or in previous margin calculations
      // TODO: why *not* include this space in system height?
      System* lastSystem = page->systems()->last();
      int lastStaff = nstaves() - 1;
      if (!lastSystem->isVbox()) {
            int lastVisible = -1;
            for (int i = lastStaff; i >= 0; --i) {
                  if (staff(i)->show() && lastSystem->staff(i)->show()) {
                        lastVisible = i;
                        break;
                        }
                  }
            if (lastVisible >= 0) {
                  // allow lyrics to extend into the "music bottom margin" (as opposed to page margin)
                  // on the assumption that this is consistent with its main purpose
                  // the extra distance includes the "lyrics bottom margin"
                  // we could strip this off too, but then the lyrics might crowd the page margin too much
                  // TODO: consider another style parameter here
                  qreal allowableMargin = d;    // + styleP(StyleIdx::lyricsMinBottomDistance);
                  qreal extra = qMax(pC.sr.extraDistance(lastVisible) - allowableMargin, 0.0);
                  // for last staff of system, this distance has not been accounted for at all
                  // for interior staves (with last staves hidden), this is only partially accounted for
                  if (lastVisible == lastStaff)
                        restHeight -= extra;
                  else
                        restHeight -= qMin(extra, styleP(StyleIdx::staffDistance));
                  }
            }

      const qreal maxStretch = styleP(StyleIdx::maxSystemDistance) - styleP(StyleIdx::minSystemDistance);

      QList<System*> slist;
      int n = page->systems()->size();
      for (int i = 0; i < n; ++i) {
            System* system = page->systems()->at(i);
            qreal lastY1   = system->pos().y() + system->height();

            if (system->addStretch()) {
                  System* ns = page->systems()->at(i + 1);
                  qreal dist = ns->pos().y() - lastY1;
                  system->setDistance(dist);
                  slist.append(system);
                  system->setStretchDistance(0.0);
                  }
            }
      qSort(slist.begin(), slist.end(), systemDistCompare);

      n = slist.size();
      for (int i = 0; i < (n-1); ++i) {
            System* s1 = slist.at(i);
            System* s2 = slist.at(i + 1);
            qreal td = s2->distance() - s1->distance();
            if (td > 0.001) {
                  int nn = i + 1;
                  qreal tdd = td * nn;
                  if (tdd > restHeight) {
                        tdd = restHeight;
                        td  = tdd / nn;
                        }
                  if ((s1->stretchDistance() + td) > maxStretch) {
                        td = maxStretch - s1->stretchDistance();
                        tdd = td * nn;
                        }
                  for (int k = 0; k <= i; ++k)
                        slist.at(k)->addStretchDistance(td);
                  restHeight -= tdd;
                  }
            }
      qreal td = restHeight / n;
      if (td > 0.001) {
            qreal sd = slist.at(0)->stretchDistance();
            if (sd + td > maxStretch)
                  td = maxStretch - sd;
            for (int k = 0; k < n; ++k)
                  slist.at(k)->addStretchDistance(td);
            }

      qreal y = 0.0;

      for (System* system : *page->systems()) {
            system->move(QPointF(0.0, y));
            if (system->addStretch())
                  y += system->stretchDistance();
            }
      }

//---------------------------------------------------------
//   doLayoutSystems
//    layout staves in a system
//    layout pages
//---------------------------------------------------------

void Score::doLayoutSystems()
      {
      foreach(System* system, _systems)
            system->layout2();
      if (layoutMode() != LayoutMode::LINE)
            layoutPages();
      rebuildBspTree();
      _updateAll = true;

      for (MuseScoreView* v : viewer)
            v->layoutChanged();
      }

//---------------------------------------------------------
//   doLayoutPages
//    small wrapper for layoutPages()
//---------------------------------------------------------

void Score::doLayoutPages()
      {
      layoutPages();
      rebuildBspTree();
      _updateAll = true;
      foreach(MuseScoreView* v, viewer)
            v->layoutChanged();
      }

//---------------------------------------------------------
//   sff
//    compute 1/Force for a given Extend
//---------------------------------------------------------

qreal sff(qreal x, qreal xMin, const SpringMap& springs)
      {
      if (x <= xMin)
            return 0.0;
      iSpring i = springs.begin();
      qreal c  = i->second.stretch;
      if (c == 0.0)           //DEBUG
            c = 1.1;
      qreal f = 0.0;
      for (; i != springs.end();) {
            xMin -= i->second.fix;
            f = (x - xMin) / c;
            ++i;
            if (i == springs.end() || f <= i->first)
                  break;
            c += i->second.stretch;
            }
      return f;
      }

//---------------------------------------------------------
//   respace
//---------------------------------------------------------

void Score::respace(QList<ChordRest*>* elements)
      {
      ChordRest* cr1 = elements->front();
      ChordRest* cr2 = elements->back();
      int n          = elements->size();
      qreal x1       = cr1->segment()->pos().x();
      qreal x2       = cr2->segment()->pos().x();

      qreal width[n-1];
      int ticksList[n-1];
      int minTick = 100000;

      for (int i = 0; i < n-1; ++i) {
            ChordRest* cr  = (*elements)[i];
            ChordRest* ncr = (*elements)[i+1];
            Space space(cr->space());
            Space nspace(ncr->space());
            width[i] = space.rw() + nspace.lw();
            ticksList[i] = ncr->segment()->tick() - cr->segment()->tick();
            minTick = qMin(ticksList[i], minTick);
            }

      //---------------------------------------------------
      // compute stretches
      //---------------------------------------------------

      SpringMap springs;
      qreal minimum = 0.0;
      for (int i = 0; i < n-1; ++i) {
            qreal w   = width[i];
            int t     = ticksList[i];
            // qreal str = 1.0 + .6 * log(qreal(t) / qreal(minTick)) / log(2.0);
            qreal str = 1.0 + 0.865617 * log(qreal(t) / qreal(minTick));
            qreal d   = w / str;

            springs.insert(std::pair<qreal, Spring>(d, Spring(i, str, w)));
            minimum += w;
            }

      //---------------------------------------------------
      //    distribute stretch to elements
      //---------------------------------------------------

      qreal force = sff(x2 - x1, minimum, springs);
      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            qreal stretch = force * i->second.stretch;
            if (stretch < i->second.fix)
                  stretch = i->second.fix;
            width[i->second.seg] = stretch;
            }
      qreal x = x1;
      for (int i = 1; i < n-1; ++i) {
            x += width[i-1];
            ChordRest* cr = (*elements)[i];
            qreal dx = x - cr->segment()->pos().x();
            cr->rxpos() += dx;
            }
      }

//---------------------------------------------------------
//   computeMinWidth
///    compute the minimum width of a measure with
///    segment list fs
//---------------------------------------------------------

qreal Score::computeMinWidth(Segment* fs, bool firstMeasureInSystem)
      {
      int _nstaves = nstaves();
      if (_nstaves == 0)
            return 1.0;

      qreal _spatium              = spatium();
      qreal clefKeyRightMargin    = styleS(StyleIdx::clefKeyRightMargin).val() * _spatium;
      qreal minNoteDistance       = styleS(StyleIdx::minNoteDistance).val()    * _spatium;
      qreal minHarmonyDistance    = styleS(StyleIdx::minHarmonyDistance).val() * _spatium;
      qreal maxHarmonyBarDistance = styleS(StyleIdx::maxHarmonyBarDistance).val() * _spatium;

      qreal rest[_nstaves];   // fixed space needed from previous segment
      memset(rest, 0, _nstaves * sizeof(qreal));

      qreal hRest[_nstaves];  // fixed space needed from previous harmony
      memset(hRest, 0, _nstaves * sizeof(qreal));

      qreal clefWidth[_nstaves];
      memset(clefWidth, 0, _nstaves * sizeof(qreal));

      std::vector<QRectF> hLastBbox(_nstaves);    // bbox of previous harmony to test vertical separation

      int segmentIdx = 0;
      qreal x        = 0.0;

      const Segment* pSeg = 0;
      for (Segment* s = fs; s; s = s->next(), ++segmentIdx) {
            qreal elsp = s->extraLeadingSpace().val()  * _spatium;
            qreal etsp = s->extraTrailingSpace().val() * _spatium;

            if ((s->segmentType() == Segment::Type::Clef) && (s != fs)) {
                  --segmentIdx;
                  for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
                        if (!staff(staffIdx)->show())
                              continue;
                        Element* e = s->element(staffIdx * VOICES);
                        if (e) {
                              e->layout();
                              clefWidth[staffIdx] = e->width() + _spatium + elsp;
                              }
                        }
                  continue;
                  }
            bool rest2[_nstaves];
            bool hRest2[_nstaves];
            bool spaceHarmony     = false;
            Segment::Type segType = s->segmentType();
            qreal segmentWidth    = 0.0;
            qreal harmonyWidth    = 0.0;
            qreal stretchDistance = 0.0;
            Segment::Type pt      = pSeg ? pSeg->segmentType() : Segment::Type::BarLine;

            for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
                  if (!staff(staffIdx)->show())
                        continue;
                  qreal minDistance = 0.0;
                  Space space;
                  Space hSpace;
                  Space naSpace;          // space needed for full measure rests (and potentially other non-aligned elements)
                  QRectF hBbox;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  bool hFound = false;
                  bool eFound = false;

                  if (segType & (Segment::Type::ChordRest)) {
                        qreal llw = 0.0;
                        qreal rrw = 0.0;
                        Lyrics* lyrics = 0;
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (pt & (Segment::Type::StartRepeatBarLine | Segment::Type::BarLine | Segment::Type::TimeSig)) {
                                    // check for accidentals in chord
                                    bool accidental = false;
                                    bool grace = false;
                                    qreal accidentalX = 0.0;
                                    qreal noteX = 0.0;
                                    if (cr->type() == Element::Type::CHORD) {
                                          Chord* c = static_cast<Chord*>(cr);
                                          if (c->graceNotesBefore().size())
                                                grace = true;
                                          else {
                                                for (Note* note : c->notes()) {
                                                      if (note->accidental() && !note->fixed()) {
                                                            accidental = true;
                                                            // segment-relative
                                                            accidentalX = qMin(accidentalX, note->accidental()->x() + note->x() + c->x());
                                                            }
                                                      else
                                                            // segment-relative
                                                            noteX = qMin(noteX, note->x() + c->x());
                                                      }
                                                }
                                          }
                                    qreal sp;
                                    qreal bnd = styleS(StyleIdx::barNoteDistance).val() * _spatium;
                                    if (accidental) {
                                          qreal bad = styleS(StyleIdx::barAccidentalDistance).val() * _spatium;
                                          qreal diff = qMax(noteX - accidentalX, 0.0);
                                          sp = qMax(bad, bnd - diff);
                                          }
                                    else if (grace)
                                          sp = styleS(StyleIdx::barGraceDistance).val() * _spatium;
                                    else
                                          sp = bnd;
                                    if (pt & Segment::Type::TimeSig)
                                          sp += clefKeyRightMargin - bnd;
                                    minDistance = qMax(minDistance, sp);
                                    if (!(pt & Segment::Type::TimeSig))
                                          stretchDistance = sp * .7;
                                    }
                              else if (pt & Segment::Type::ChordRest) {
                                    minDistance = qMax(minDistance, minNoteDistance);
                                    }
                              else {
                                    // if (pt & (Segment::Type::KeySig | Segment::Type::Clef))
                                    bool firstClef = (pt == Segment::Type::Clef) && (pSeg && pSeg->rtick() == 0);
                                    if ((pt & Segment::Type::KeySig) || firstClef)
                                          minDistance = qMax(minDistance, clefKeyRightMargin);
                                    }
                              cr->layout();

                              // special case:
                              // make extra space for ties or glissandi continued from previous system

                              if (firstMeasureInSystem && cr->type() == Element::Type::CHORD && cr->tick() == cr->measure()->tick()) {
                                    Chord* c = static_cast<Chord*>(cr);
                                    if (c->endsGlissando()) {
                                          minDistance = qMax(minDistance, _spatium * GLISS_STARTOFSYSTEM_WIDTH);
                                          }
                                    else {
                                          for (Note* note : c->notes()) {
                                                if (note->tieBack()) {
                                                      minDistance = qMax(minDistance, _spatium * 2);
                                                      break;
                                                      }
                                                }
                                          }
                                    }

                              // calculate space needed for segment
                              // take cr position into account
                              // by converting to segment-relative space
                              // chord space itself already has ipos offset built in
                              // but lyrics do not
                              // and neither have user offsets
                              qreal cx = cr->ipos().x();
                              qreal cxu = cr->userOff().x();
                              qreal lx = qMax(cxu, 0.0); // nudge left shouldn't require more leading space
                              qreal rx = qMin(cxu, 0.0); // nudge right shouldn't require more trailing space
                              Space crSpace = cr->space();
                              Space segRelSpace(crSpace.lw()-lx, crSpace.rw()+rx);
                              // always allocate sufficient space
                              // but in Measure::layoutX() we will ignore full measure rests
                              // since they do not need to affect spacing in other voices
                              if (cr->durationType() != TDuration::DurationType::V_MEASURE)
                                    space.max(segRelSpace);
                              else
                                    naSpace.max(segRelSpace);

                              // lyrics
                              foreach (Lyrics* l, cr->lyricsList()) {
                                    if (!l)
                                          continue;
                                    l->layout();      // need to create layout even if empty
                                    if (!l->isEmpty()) {
                                          lyrics = l;
                                          QRectF b(l->bbox().translated(l->pos()));
                                          llw = qMax(llw, -(b.left()+lx+cx));
                                          rrw = qMax(rrw, b.right()+rx+cx);
                                          }
                                    }
                              }

                        space.max(naSpace);
                        if (lyrics)
                              space.max(Space(llw, rrw));

                        // add spacing for chord symbols
                        foreach (Element* e, s->annotations()) {
                              if (e->type() != Element::Type::HARMONY || e->track() < track || e->track() >= track+VOICES)
                                    continue;
                              Harmony* h = static_cast<Harmony*>(e);
                              // call full layout here
                              // which also triggers layout of associated fret diagram if present
                              // otherwise the vertical position of the chord symbols cannot be known
                              h->layout(); // h->calculateBoundingRect();
                              QRectF b(h->bboxtight().translated(h->pos()));
                              if (hFound)
                                    hBbox |= b;
                              else
                                    hBbox = b;
                              hFound = true;
                              spaceHarmony = true;
                              // allow chord to be dragged
                              qreal xoff = h->pos().x();
                              qreal bl = -b.left() + qMin(xoff, 0.0);
                              qreal br = b.right() - qMax(xoff, 0.0);
                              hSpace.max(Space(bl, br));
                              }
                        }
                  else {
                        // current segment (s) is not a ChordRest
                        Element* e = s->element(track);
                        if (segType == Segment::Type::StartRepeatBarLine && !e) {
                              for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                                    Element* ee = s->element(staffIdx * VOICES);
                                    if (ee) {
                                          BarLine* bl = static_cast<BarLine*>(ee);
                                          int strack = staffIdx * VOICES;
                                          int etrack = (staffIdx + bl->span()) * VOICES;
                                          if (track >= strack && track < etrack) {
                                                e = ee;
                                                break;
                                                }
                                          }
                                    }
                              }
                        if ((segType == Segment::Type::Clef) && (pt != Segment::Type::ChordRest))
                              minDistance = styleP(StyleIdx::clefLeftMargin);
                        else if (segType == Segment::Type::StartRepeatBarLine)
                              minDistance = .5 * _spatium;
                        else if (segType == Segment::Type::TimeSig && pt == Segment::Type::Clef) {
                              // missing key signature, but allocate default margin anyhow
                              minDistance = styleP(StyleIdx::keysigLeftMargin);
                              }
                        else if ((segType == Segment::Type::EndBarLine) && segmentIdx) {
                              if (pt == Segment::Type::Clef)
                                    minDistance = styleP(StyleIdx::clefBarlineDistance);
                              else
                                    stretchDistance = styleP(StyleIdx::noteBarDistance);
                              if (e == 0) {
                                    // look for barline
                                    for (int i = track - VOICES; i >= 0; i -= VOICES) {
                                          e = s->element(i);
                                          if (e)
                                                break;
                                          }
                                    }
                              }
                        if (e) {
                              eFound = true;
                              if (!s->next())               // segType & Segment::Type::EndBarLine
                                    spaceHarmony = true;    // to space last Harmony to end of measure
                              e->layout();
                              space.max(e->space());
                              }
                        }
                  space += Space(elsp, etsp);

                  if (found || eFound) {
                        space.addL(clefWidth[staffIdx]);
                        qreal sp        = minDistance + rest[staffIdx] + qMax(space.lw(), stretchDistance);
                        rest[staffIdx]  = space.rw();
                        rest2[staffIdx] = false;
                        segmentWidth    = qMax(segmentWidth, sp);
                        }
                  else
                        rest2[staffIdx] = true;

                  // space chord symbols separately from segments
                  if (hFound || eFound) {
                        qreal sp = 0.0;

                        // space chord symbols unless they miss each other vertically
                        if (eFound || (hFound && hBbox.top() < hLastBbox[staffIdx].bottom() && hBbox.bottom() > hLastBbox[staffIdx].top()))
                              sp = hRest[staffIdx] + minHarmonyDistance + hSpace.lw();

                        // barline: limit space to maxHarmonyBarDistance
                        if (eFound && !hFound && spaceHarmony)
                              sp = qMin(sp, maxHarmonyBarDistance);

                        hLastBbox[staffIdx] = hBbox;
                        hRest[staffIdx] = hSpace.rw();
                        hRest2[staffIdx] = false;
                        harmonyWidth = qMax(harmonyWidth, sp);
                        }
                  else
                        hRest2[staffIdx] = true;

                  clefWidth[staffIdx] = 0.0;
                  }

            // make room for harmony if needed
            segmentWidth = qMax(segmentWidth, harmonyWidth);

            x += segmentWidth;

//            if (segmentIdx && pSeg)
//                  pSeg->setbbox(QRectF(0.0, 0.0, segmentWidth, _spatium * 5));  //??

            for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
                  if (!staff(staffIdx)->show())
                        continue;
                  if (rest2[staffIdx])
                        rest[staffIdx] -= qMin(rest[staffIdx], segmentWidth);
                  if (hRest2[staffIdx])
                        hRest[staffIdx] -= qMin(hRest[staffIdx], segmentWidth);
                  }

            //
            // set pSeg only to used segments
            //
            for (int voice = 0; voice < _nstaves * VOICES; ++voice) {
                  if (!staff(voice/VOICES)->show()) {
                        voice += VOICES-1;
                        continue;
                        }
                  if (s->element(voice)) {
                        pSeg = s;
                        break;
                        }
                  }
            }

      qreal segmentWidth = 0.0;
      for (int staffIdx = 0; staffIdx < _nstaves; ++staffIdx) {
            if (!staff(staffIdx)->show())
                  continue;
            segmentWidth = qMax(segmentWidth, rest[staffIdx]);
            segmentWidth = qMax(segmentWidth, hRest[staffIdx]);
            }
      x += segmentWidth;
      return x;
      }

//---------------------------------------------------------
//   updateBarLineSpans
///   updates bar line span(s) when the number of lines of a staff changes
//---------------------------------------------------------

void Score::updateBarLineSpans(int idx, int linesOld, int linesNew)
      {
      int         nStaves = nstaves();
      Staff*      _staff;

      // scan staves and check the destination staff of each bar line span
      // barLineSpan is not changed; barLineFrom and barLineTo are changed if they occur in the bottom half of a staff
      // in practice, a barLineFrom/To from/to the top half of the staff is linked to the staff top line,
      // a barLineFrom/To from/to the bottom half of the staff is linked to staff bottom line;
      // this ensures plainchant and mensurstrich special bar lines keep their relationships to the staff lines.
      // 1-line staves are traited as a special case.
      for(int sIdx = 0; sIdx < nStaves; sIdx++) {
            _staff = staff(sIdx);
            // if this is the modified staff
            if(sIdx == idx) {
                  // if it has no bar line, set barLineTo to a default value
                  if(_staff->barLineSpan() == 0)
                        _staff->setBarLineTo( (linesNew-1) * 2);
                  // if new line count is 1, set default From for 1-line staves
                  else if(linesNew == 1)
                        _staff->setBarLineFrom(BARLINE_SPAN_1LINESTAFF_FROM);
                  // if old line count was 1, set default From for normal staves
                  else if (linesOld == 1)
                        _staff->setBarLineFrom(0);
                  // if barLineFrom was below the staff middle position
                  // raise or lower it to account for new number of lines
                  else if(_staff->barLineFrom() > linesOld - 1)
                        _staff->setBarLineFrom(_staff->barLineFrom() + (linesNew - linesOld)*2);
            }

            // if the modified staff is the destination of the current staff bar span:
            if(sIdx + _staff->barLineSpan() - 1 == idx) {
                  // if new line count is 1, set default To for 1-line staves
                  if(linesNew == 1)
                        _staff->setBarLineTo(BARLINE_SPAN_1LINESTAFF_TO);
                  // if old line count was 1, set default To for normal staves
                  else if (linesOld == 1)
                        _staff->setBarLineTo((linesNew - 1) * 2);
                  // if barLineTo was below its middle position, raise or lower it
                  else if(_staff->barLineTo() > linesOld - 1)
                        _staff->setBarLineTo(_staff->barLineTo() + (linesNew - linesOld)*2);
                  }
            }
      }

}


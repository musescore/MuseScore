//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
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
#include "breath.h"
#include "tempotext.h"
#include "systemdivider.h"
#include "hook.h"
#include "ambitus.h"
#include "hairpin.h"
#include "stafflines.h"
#include "articulation.h"
#include "bracket.h"
#include "spacer.h"
#include "fermata.h"
#include "measurenumber.h"

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
      for (Page* page : pages())
            page->rebuildBspTree();
      }

//---------------------------------------------------------
//   layoutSegmentElements
//---------------------------------------------------------

static void layoutSegmentElements(Segment* segment, int startTrack, int endTrack)
      {
      for (int track = startTrack; track < endTrack; ++track) {
            if (Element* e = segment->element(track))
                  e->layout();
            }
      }

//---------------------------------------------------------
//   layoutChords1
//    - layout upstem and downstem chords
//    - offset as necessary to avoid conflict
//---------------------------------------------------------

void Score::layoutChords1(Segment* segment, int staffIdx)
      {
      const Staff* staff = Score::staff(staffIdx);
      const int startTrack = staffIdx * VOICES;
      const int endTrack   = startTrack + VOICES;

      if (staff->isTabStaff(segment->tick())) {
            layoutSegmentElements(segment, startTrack, endTrack);
            return;
            }

      std::vector<Note*> upStemNotes;
      std::vector<Note*> downStemNotes;
      int upVoices       = 0;
      int downVoices     = 0;
      qreal nominalWidth = noteHeadWidth() * staff->mag(segment->tick());
      qreal maxUpWidth   = 0.0;
      qreal maxDownWidth = 0.0;
      qreal maxUpMag     = 0.0;
      qreal maxDownMag   = 0.0;

      // dots and hooks can affect layout of notes as well as vice versa
      int upDots         = 0;
      int downDots       = 0;
      bool upHooks       = false;
      bool downHooks     = false;

      // also check for grace notes
      bool upGrace       = false;
      bool downGrace     = false;

      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (e && e->isChord()) {
                  Chord* chord = toChord(e);
                  bool hasGraceBefore = false;
                  for (Chord* c : chord->graceNotes()) {
                        if (c->isGraceBefore())
                              hasGraceBefore = true;
                        layoutChords2(c->notes(), c->up());       // layout grace note noteheads
                        layoutChords3(c->notes(), staff, 0);      // layout grace note chords
                        }
                  if (chord->up()) {
                        ++upVoices;
                        upStemNotes.insert(upStemNotes.end(), chord->notes().begin(), chord->notes().end());
                        upDots   = qMax(upDots, chord->dots());
                        maxUpMag = qMax(maxUpMag, chord->mag());
                        if (!upHooks)
                              upHooks = chord->hook();
                        if (hasGraceBefore)
                              upGrace = true;
                        }
                  else {
                        ++downVoices;
                        downStemNotes.insert(downStemNotes.end(), chord->notes().begin(), chord->notes().end());
                        downDots = qMax(downDots, chord->dots());
                        maxDownMag = qMax(maxDownMag, chord->mag());
                        if (!downHooks)
                              downHooks = chord->hook();
                        if (hasGraceBefore)
                              downGrace = true;
                        }
                  }
            }

      if (upVoices + downVoices) {
            // TODO: use track as secondary sort criteria?
            // otherwise there might be issues with unisons between voices
            // in some corner cases

            maxUpWidth   = nominalWidth * maxUpMag;
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

            qreal sp                 = staff->spatium(segment->tick());
            qreal upOffset           = 0.0;      // offset to apply to upstem chords
            qreal downOffset         = 0.0;      // offset to apply to downstem chords
            qreal dotAdjust          = 0.0;      // additional chord offset to account for dots
            qreal dotAdjustThreshold = 0.0;      // if it exceeds this amount

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
            qreal headDiff2 = maxUpWidth - nominalWidth * (maxUpMag / staff->mag(segment->tick()));
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
                  Note* bottomUpNote = upStemNotes.front();
                  Note* topDownNote  = downStemNotes.back();
                  int separation;
                  if (bottomUpNote->chord()->staffMove() == topDownNote->chord()->staffMove())
                        separation = topDownNote->line() - bottomUpNote->line();
                  else
                        separation = 2;   // no conflict
                  QVector<Note*> overlapNotes;
                  overlapNotes.reserve(8);

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
                        for (size_t i = 0, n = upStemNotes.size(); i < n; ++i) {
                              if (upStemNotes[i]->line() >= topDownNote->line() - 1)
                                    overlapNotes.append(upStemNotes[i]);
                              else
                                    break;
                              }
                        for (int i = int(downStemNotes.size()) - 1; i >= 0; --i) {
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
                                    Note* previousNote = overlapNotes[i-1];
                                    Note* n = overlapNotes[i];
                                    if (!(previousNote->chord()->isNudged() || n->chord()->isNudged())) {
                                          if (previousNote->chord()->dots() == n->chord()->dots()) {
                                                // hide one set dots
                                                bool onLine = !(previousNote->line() & 1);
                                                if (onLine) {
                                                      // hide dots for lower voice
                                                      if (previousNote->voice() & 1)
                                                            previousNote->setDotsHidden(true);
                                                      else
                                                            n->setDotsHidden(true);
                                                      }
                                                else {
                                                      // hide dots for upper voice
                                                      if (!(previousNote->voice() & 1))
                                                            previousNote->setDotsHidden(true);
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
                        dotAdjust = styleP(Sid::dotNoteDistance) + dotWidth;
                        // additional dots
                        if (dots > 1)
                              dotAdjust += styleP(Sid::dotDotDistance) * (dots - 1);
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
                  if (e && e->isChord()) {
                        Chord* chord = toChord(e);
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
            std::vector<Note*> notes;
            if (upVoices)
                  notes.insert(notes.end(), upStemNotes.begin(), upStemNotes.end());
            if (downVoices)
                  notes.insert(notes.end(), downStemNotes.begin(), downStemNotes.end());
            if (upVoices + downVoices > 1)
                  qSort(notes.begin(), notes.end(),
                     [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );
            layoutChords3(notes, staff, segment);
            }

      layoutSegmentElements(segment, startTrack, endTrack);
      }

//---------------------------------------------------------
//   layoutChords2
//    - determine which notes need mirroring
//    - this is called once for each stem direction
//      eg, once for voices 1&3, once for 2&4
//      with all notes combined and sorted to resemble one chord
//    - return maximum non-mirrored notehead width
//---------------------------------------------------------

qreal Score::layoutChords2(std::vector<Note*>& notes, bool up)
      {
      int startIdx, endIdx, incIdx;
      qreal maxWidth = 0.0;

      // loop in correct direction so that first encountered notehead wins conflict
      if (up) {
            // loop bottom up
            startIdx = 0;
            endIdx = int(notes.size());
            incIdx = 1;
            }
      else {
            // loop top down
            startIdx = int(notes.size()) - 1;
            endIdx = -1;
            incIdx = -1;
            }

      int ll        = 1000;         // line of previous notehead
                                    // hack: start high so first note won't show as conflict
      bool lvisible = false;        // was last note visible?
      bool mirror   = false;        // should current notehead be mirrored?
                                    // value is retained and may be used on next iteration
                                    // to track mirror status of previous note
      bool isLeft   = notes[startIdx]->chord()->up();             // is notehead on left?
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
            // this may be changed later to allow unisons to share noteheads
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
                  maxWidth = qMax(maxWidth, note->bboxRightPos());

            // prepare for next iteration
            lvisible = note->visible();
            lmove    = move;
            ll       = line;
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

static qreal layoutAccidental(AcEl* me, AcEl* above, AcEl* below, qreal colOffset, QVector<Note*>& leftNotes, qreal pnd, qreal pd, qreal sp)
      {
      qreal lx = colOffset;
      Accidental* acc = me->note->accidental();
      qreal mag = acc->mag();
      pnd *= mag;
      pd *= mag;

      // extra space for ledger lines
      if (me->line <= -2 || me->line >= me->note->staff()->lines(me->note->chord()->tick()) * 2)
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

void Score::layoutChords3(std::vector<Note*>& notes, const Staff* staff, Segment* segment)
      {
      //---------------------------------------------------
      //    layout accidentals
      //    find column for dots
      //---------------------------------------------------

      QVector<Note*> leftNotes; // notes to left of origin
      leftNotes.reserve(8);
      QVector<AcEl> aclist;       // accidentals
      aclist.reserve(8);

      // track columns of octave-separated accidentals
      int columnBottom[7] = { -1, -1, -1, -1, -1, -1, -1 };

      Fraction tick      =  notes.front()->chord()->segment()->tick();
      qreal sp           = staff->spatium(tick);
      qreal stepDistance = sp * staff->lineDistance(tick) * .5;
      int stepOffset     = staff->staffType(tick)->stepOffset();

      qreal lx           = 10000.0;  // leftmost notehead position
      qreal upDotPosX    = 0.0;
      qreal downDotPosX  = 0.0;

      int nNotes = int(notes.size());
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

            Chord* chord = note->chord();
            bool _up     = chord->up();

            qreal overlapMirror;
            Stem* stem = chord->stem();
            if (stem)
                  overlapMirror = stem->lineWidth();
            else if (chord->durationType().headType() == NoteHead::Type::HEAD_WHOLE)
                  overlapMirror = styleP(Sid::stemWidth) * chord->mag();
            else
                  overlapMirror = 0.0;

            qreal x = 0.0;
            if (note->mirror())
                  if (_up)
                        x = chord->stemPosX() - overlapMirror;
                  else
                        x = -note->headBodyWidth() + overlapMirror;
            else if (_up)
                  x = chord->stemPosX() - note->headBodyWidth();

            qreal ny = (note->line() + stepOffset) * stepDistance;
            if (note->rypos() != ny) {
                  note->rypos() = ny;
                  if (chord->stem()) {
                        chord->stem()->layout();
                        if (chord->hook())
                              chord->hook()->rypos() = chord->stem()->hookPos().y();
                        }
                  }
            note->rxpos()  = x;

            // find leftmost non-mirrored note to set as X origin for accidental layout
            // a mirrored note that extends to left of segment X origin
            // will displace accidentals only if there is conflict
            qreal sx = x + chord->x(); // segment-relative X position of note
            if (note->mirror() && !chord->up() && sx < 0.0)
                  leftNotes.append(note);
            else if (sx < lx)
                  lx = sx;

            qreal xx = x + note->headBodyWidth() + chord->pos().x();

            Direction dotPosition = note->userDotPosition();
            if (chord->dots()) {
                  if (chord->up())
                        upDotPosX = qMax(upDotPosX, xx);
                  else {
                        downDotPosX = qMax(downDotPosX, xx);
                        }

                  if (dotPosition == Direction::AUTO && nNotes > 1 && note->visible() && !note->dotsHidden()) {
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
                                    dotPosition = Direction::DOWN;
                              else if (intervalBelow == 1 && intervalAbove != 1)
                                    dotPosition = Direction::UP;
                              else if (intervalAbove == 0 && above->chord()->dots()) {
                                    // unison
                                    if (((above->voice() & 1) == (note->voice() & 1))) {
                                          above->setDotY(Direction::UP);
                                          dotPosition = Direction::DOWN;
                                          }
                                    }
                              }
                        else {
                              // space
                              if (intervalAbove == 0 && above->chord()->dots()) {
                                    // unison
                                    if (!(note->voice() & 1))
                                          dotPosition = Direction::UP;
                                    else {
                                          if (!(above->voice() & 1))
                                                above->setDotY(Direction::UP);
                                          else
                                                dotPosition = Direction::DOWN;
                                          }
                                    }
                              }
                        }
                  }
            note->setDotY(dotPosition);  // also removes invalid dots
            }

      // if there are no non-mirrored notes in a downstem chord,
      // then use the stem X position as X origin for accidental layout
      if (nNotes && leftNotes.size() == nNotes)
            lx = notes.front()->chord()->stemPosX();

      if (segment) {
            // align all dots for segment/staff
            // it would be possible to dots for up & down chords separately
            // this would require space to have been allocated previously
            // when calculating chord offsets
            segment->setDotPosX(staff->idx(), qMax(upDotPosX, downDotPosX));
            }

      if (nAcc == 0)
            return;

      QVector<int> umi;
      qreal pd  = styleP(Sid::accidentalDistance);
      qreal pnd = styleP(Sid::accidentalNoteDistance);
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
            QVector<int> column;
            QVector<int> unmatched;
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
                        umi.push_back(unmatched[i]);
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
                  umi.push_back(i);
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
            }
      }

#define beamModeMid(a) (a == Beam::Mode::MID || a == Beam::Mode::BEGIN32 || a == Beam::Mode::BEGIN64)

bool beamNoContinue(Beam::Mode mode)
      {
      return mode == Beam::Mode::END || mode == Beam::Mode::NONE || mode == Beam::Mode::INVALID;
      }

//---------------------------------------------------------
//   beamGraceNotes
//---------------------------------------------------------

void Score::beamGraceNotes(Chord* mainNote, bool after)
      {
      ChordRest* a1    = 0;      // start of (potential) beam
      Beam* beam       = 0;      // current beam
      Beam::Mode bm = Beam::Mode::AUTO;
      QVector<Chord*> graceNotes = after ? mainNote->graceNotesAfter() : mainNote->graceNotesBefore();

      for (ChordRest* cr : graceNotes) {
            bm = Groups::endBeam(cr);
            if ((cr->durationType().type() <= TDuration::DurationType::V_QUARTER) || (bm == Beam::Mode::NONE)) {
                  if (beam) {
                        beam->layoutGraceNotes();
                        beam = 0;
                        }
                  if (a1) {
                        a1->removeDeleteBeam(false);
                        a1 = 0;
                        }
                  cr->removeDeleteBeam(false);
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
                        a1->removeDeleteBeam(false);
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
            a1->removeDeleteBeam(false);
      }

#if 0 // unused
//---------------------------------------------------------
//   layoutSpanner
//    called after dragging a staff
//---------------------------------------------------------

void Score::layoutSpanner()
      {
      int tracks = ntracks();
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = firstSegment(SegmentType::All); segment; segment = segment->next1()) {
                  if (track == tracks-1) {
                        size_t n = segment->annotations().size();
                        for (size_t i = 0; i < n; ++i)
                              segment->annotations().at(i)->layout();
                        }
                  Element* e = segment->element(track);
                  if (e && e->isChord()) {
                        Chord* c = toChord(segment->element(track));
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
#endif

//---------------------------------------------------------
//   hideEmptyStaves
//---------------------------------------------------------

void Score::hideEmptyStaves(System* system, bool isFirstSystem)
      {
      int staves   = _staves.size();
      int staffIdx = 0;
      bool systemIsEmpty = true;

      for (Staff* staff : _staves) {
            SysStaff* ss  = system->staff(staffIdx);

            Staff::HideMode hideMode = staff->hideWhenEmpty();

            if (hideMode == Staff::HideMode::ALWAYS
                || (styleB(Sid::hideEmptyStaves)
                    && (staves > 1)
                    && !(isFirstSystem && styleB(Sid::dontHideStavesInFirstSystem))
                    && hideMode != Staff::HideMode::NEVER)) {
                  bool hideStaff = true;
                  for (MeasureBase* m : system->measures()) {
                        if (!m->isMeasure())
                              continue;
                        Measure* measure = toMeasure(m);
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

                              for (MeasureBase* mb : system->measures()) {
                                    if (!mb->isMeasure())
                                          continue;
                                    Measure* m = toMeasure(mb);
                                    if (staff->hideWhenEmpty() == Staff::HideMode::INSTRUMENT && !m->isMeasureRest(st)) {
                                          hideStaff = false;
                                          break;
                                          }
                                    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                                          for (int voice = 0; voice < VOICES; ++voice) {
                                                ChordRest* cr = s->cr(st * VOICES + voice);
                                                if (cr == 0 || cr->isRest())
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
                  ss->setShow(hideStaff ? false : staff->show());
                  if (ss->show())
                        systemIsEmpty = false;
                  }
            else {
                  systemIsEmpty = false;
                  ss->setShow(true);
                  }

            ++staffIdx;
            }
      Staff* firstVisible = nullptr;
      if (systemIsEmpty) {
            for (Staff* staff : _staves) {
                  SysStaff* ss  = system->staff(staff->idx());
                  if (staff->showIfEmpty() && !ss->show()) {
                        ss->setShow(true);
                        systemIsEmpty = false;
                        }
                  else if (!firstVisible && staff->show()) {
                        firstVisible = staff;
                        }
                  }
            }
      // don’t allow a complete empty system
      if (systemIsEmpty) {
            Staff* staff = firstVisible ? firstVisible : _staves.front();
            SysStaff* ss = system->staff(staff->idx());
            ss->setShow(true);
            }
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

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = m->first(st); s; s = s->next1(st)) {
            for (int i = 0; i < tracks; ++i) {
                  Element* e = s->element(i);
                  if (e == 0 || !e->isChord())
                        continue;
                  Chord* c = toChord(e);
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
                                          qDebug("next note at %d track %d for tie not found (version %d)", s->tick().ticks(), i, _mscVersion);
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
                        for (Spanner* spanner : n->spannerBack()) {
                              if (spanner->isGlissando() && !spanner->startElement()) {
                                    Note* initialNote = Glissando::guessInitialNote(n->chord());
                                    n->removeSpannerBack(spanner);
                                    if (initialNote) {
                                          spanner->setStartElement(initialNote);
                                          spanner->setEndElement(n);
                                          spanner->setTick(initialNote->chord()->tick());
                                          spanner->setTick2(n->chord()->tick());
                                          spanner->setTrack(n->track());
                                          spanner->setTrack2(n->track());
                                          spanner->setParent(initialNote);
                                          initialNote->add(spanner);
                                          }
                                    else {
                                          delete spanner;
                                          }
                                    }
                              }
                        // spanner with no end element can happen during copy/paste
                        for (Spanner* spanner : n->spannerFor()) {
                              if (spanner->endElement() == nullptr) {
                                    n->removeSpannerFor(spanner);
                                    delete spanner;
                                    }
                              }
                        }
#if 0    // chords are set in tremolo->layout()
                  // connect two note tremolos
                  Tremolo* tremolo = c->tremolo();
                  if (tremolo && tremolo->twoNotes() && !tremolo->chord2()) {
                        for (Segment* ls = s->next1(st); ls; ls = ls->next1(st)) {
                              Element* element = ls->element(i);
                              if (!element)
                                    continue;
                              if (!element->isChord())
                                    qDebug("cannot connect tremolo");
                              else {
                                    Chord* nc = toChord(element);
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
#endif
                  }
            }
      }

//---------------------------------------------------------
//   checkDivider
//---------------------------------------------------------

static void checkDivider(bool left, System* s, qreal yOffset)
      {
      SystemDivider* divider = left ? s->systemDividerLeft() : s->systemDividerRight();
      if (s->score()->styleB(left ? Sid::dividerLeft : Sid::dividerRight)) {
            if (!divider) {
                  divider = new SystemDivider(s->score());
                  divider->setDividerType(left ? SystemDivider::Type::LEFT : SystemDivider::Type::RIGHT);
                  divider->setGenerated(true);
                  s->add(divider);
                  }
            divider->layout();
            divider->rypos() = divider->height() * .5 + yOffset;
            if (left) {
                  divider->rypos() += s->score()->styleD(Sid::dividerLeftY) * SPATIUM20;
                  divider->rxpos() =  s->score()->styleD(Sid::dividerLeftX) * SPATIUM20;
                  }
            else {
                  divider->rypos() += s->score()->styleD(Sid::dividerRightY) * SPATIUM20;
                  divider->rxpos() =  s->score()->styleD(Sid::pagePrintableWidth) * DPI - divider->width();
                  divider->rxpos() += s->score()->styleD(Sid::dividerRightX) * SPATIUM20;
                  }
            }
      else if (divider) {
            if (divider->generated()) {
                  s->remove(divider);
                  delete divider;
                  }
            else
                  s->score()->undoRemoveElement(divider);
            }
      }

//---------------------------------------------------------
//   layoutPage
//    restHeight - vertical space which has to be distributed
//                 between systems
//    The algorithm tries to produce most equally spaced
//    systems.
//---------------------------------------------------------

static void layoutPage(Page* page, qreal restHeight)
      {
      if (restHeight < 0.0) {
            qDebug("restHeight < 0.0: %f\n", restHeight);
            restHeight = 0;
            }

      Score* score = page->score();
      int gaps     = page->systems().size() - 1;

      QList<System*> sList;

      for (int i = 0; i < gaps; ++i) {
            System* s1 = page->systems().at(i);
            System* s2 = page->systems().at(i+1);
            s1->setDistance(s2->y() - s1->y());
            if (s1->vbox() || s2->vbox() || s1->hasFixedDownDistance())
                  continue;
            sList.push_back(s1);
            }

      if (sList.empty() || MScore::noVerticalStretch || score->layoutMode() == LayoutMode::SYSTEM) {
            if (score->layoutMode() == LayoutMode::FLOAT) {
                  qreal y = restHeight * .5;
                  for (System* system : page->systems())
                        system->move(QPointF(0.0, y));
                  }
            // remove system dividers
            for (System* s : page->systems()) {
                  SystemDivider* sd = s->systemDividerLeft();
                  if (sd) {
                        s->remove(sd);
                        delete sd;
                        }
                  sd = s->systemDividerRight();
                  if (sd) {
                        s->remove(sd);
                        delete sd;
                        }
                  }
            return;
            }

      std::sort(sList.begin(), sList.end(), [](System* a, System* b) { return a->distance() < b->distance(); });
      qreal maxDist = score->styleP(Sid::maxSystemDistance);

      qreal dist = sList[0]->distance();
      for (int i = 1; i < sList.size(); ++i) {
            qreal ndist = sList[i]->distance();
            qreal fill  = ndist - dist;
            if (fill > 0.0) {
                  qreal totalFill = fill * i;
                  if (totalFill > restHeight) {
                        totalFill = restHeight;
                        fill = restHeight / i;
                        }
                  for (int k = 0; k < i; ++k) {
                        System* s = sList[k];
                        qreal d = s->distance() + fill;
                        if ((d - s->height()) > maxDist)
                              d = qMax(maxDist + s->height(), s->distance());
                        s->setDistance(d);
                        }
                  restHeight -= totalFill;
                  if (restHeight <= 0)
                        break;
                  }
            dist = ndist;
            }

      if (restHeight > 0.0) {
            qreal fill = restHeight / sList.size();
            for (System* s : sList) {
                  qreal d = s->distance() + fill;
                  if ((d - s->height()) > maxDist)
                        d = qMax(maxDist + s->height(), s->distance());
                  s->setDistance(d);
                  }
            }

      qreal y = page->systems().at(0)->y();
      for (int i = 0; i < gaps; ++i) {
            System* s1  = page->systems().at(i);
            System* s2  = page->systems().at(i+1);
            s1->rypos() = y;
            y          += s1->distance();

            if (!(s1->vbox() || s2->vbox() || s1->hasFixedDownDistance())) {
                  qreal yOffset = s1->height() + (s1->distance()-s1->height()) * .5;
                  checkDivider(true,  s1, yOffset);
                  checkDivider(false, s1, yOffset);
                  }
            }
      page->systems().back()->rypos() = y;
      }

//---------------------------------------------------------
//   Spring
//---------------------------------------------------------

struct Spring {
      int seg;
      qreal stretch;
      qreal fix;
      Spring(int i, qreal s, qreal f) : seg(i), stretch(s), fix(f) {}
      };

typedef std::multimap<qreal, Spring, std::less<qreal> > SpringMap;

//---------------------------------------------------------
//   sff2
//    compute 1/Force for a given Extend
//---------------------------------------------------------

static qreal sff2(qreal width, qreal xMin, const SpringMap& springs)
      {
      if (width <= xMin)
            return 0.0;
      auto i = springs.begin();
      qreal c  = i->second.stretch;
      if (c == 0.0)           //DEBUG
            c = 1.1;
      qreal f = 0.0;
      for (; i != springs.end();) {
            xMin -= i->second.fix;
            f = (width - xMin) / c;
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

void Score::respace(std::vector<ChordRest*>* elements)
      {
      ChordRest* cr1 = elements->front();
      ChordRest* cr2 = elements->back();
      int n          = int(elements->size());
      qreal x1       = cr1->segment()->pos().x();
      qreal x2       = cr2->segment()->pos().x();

#if (!defined (_MSCVER) && !defined (_MSC_VER))
      qreal width[n-1];
      int ticksList[n-1];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<qreal> width(n-1);
      std::vector<int> ticksList(n-1);
#endif
      int minTick = 100000;

      for (int i = 0; i < n-1; ++i) {
            ChordRest* cr  = (*elements)[i];
            ChordRest* ncr  = (*elements)[i+1];
            width[i]       = cr->shape().minHorizontalDistance(ncr->shape());
            ticksList[i]   = cr->ticks().ticks();
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
            qreal str = 1.0 + 0.865617 * log(qreal(t) / qreal(minTick));
            qreal d   = w / str;

            springs.insert(std::pair<qreal, Spring>(d, Spring(i, str, w)));
            minimum += w;
            }

      //---------------------------------------------------
      //    distribute stretch to elements
      //---------------------------------------------------

      qreal force = sff2(x2 - x1, minimum, springs);
      for (auto i = springs.begin(); i != springs.end(); ++i) {
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
//   getNextPage
//---------------------------------------------------------

void LayoutContext::getNextPage()
      {
      if (!page || curPage >= score->npages()) {
            page = new Page(score);
            score->pages().push_back(page);
            prevSystem = nullptr;
            }
      else {
            page = score->pages()[curPage];
            QList<System*>& systems = page->systems();
            const int i = systems.indexOf(curSystem);
            if (i > 0 && systems[i-1]->page() == page) {
                  // Current and previous systems are on the current page.
                  // Erase only the current and the following systems
                  // as the previous one will not participate in layout.
                  systems.erase(systems.begin() + i, systems.end());
                  }
            else // system is not on the current page (or will be the first one)
                  systems.clear();
            prevSystem = systems.empty() ? nullptr : systems.back();
            }
      page->bbox().setRect(0.0, 0.0, score->loWidth(), score->loHeight());
      page->setNo(curPage);
      qreal x = 0.0;
      qreal y = 0.0;
      if (curPage) {
            Page* prevPage = score->pages()[curPage - 1];
            if (MScore::verticalOrientation())
                  y = prevPage->pos().y() + page->height() + MScore::verticalPageGap;
            else {
                  qreal gap = (curPage + score->pageNumberOffset()) & 1 ? MScore::horizontalPageGapOdd : MScore::horizontalPageGapEven;
                  x = prevPage->pos().x() + page->width() + gap;
                  }
            }
      ++curPage;
      page->setPos(x, y);
      }

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* Score::getNextSystem(LayoutContext& lc)
      {
      bool isVBox = lc.curMeasure->isVBox();
      System* system;
      if (lc.systemList.empty()) {
            system = new System(this);
            lc.systemOldMeasure = 0;
            }
      else {
            system = lc.systemList.takeFirst();
            lc.systemOldMeasure = system->measures().empty() ? 0 : system->measures().back();
            system->clear();   // remove measures from system
            }
      _systems.append(system);
      if (!isVBox) {
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
//   createMMRest
//    create a multi measure rest from m to lm (inclusive)
//---------------------------------------------------------

void Score::createMMRest(Measure* m, Measure* lm, const Fraction& len)
      {
      int n = 1;
      if (m != lm) {
            for (Measure* mm = m->nextMeasure(); mm; mm = mm->nextMeasure()) {
                  ++n;
                  mm->setMMRestCount(-1);
                  if (mm->mmRest())
                        undo(new ChangeMMRest(mm, 0));
                  if (mm == lm)
                        break;
                  }
            }

      Measure* mmr = m->mmRest();
      if (mmr) {
            if (mmr->ticks() != len) {
                  Segment* s = mmr->findSegmentR(SegmentType::EndBarLine, mmr->ticks());
                  mmr->setTicks(len);
                  if (s)
                        s->setRtick(mmr->ticks());
                  }
            }
      else {
            mmr = new Measure(this);
            mmr->setTicks(len);
            mmr->setTick(m->tick());
            mmr->setPageBreak(lm->pageBreak());
            mmr->setLineBreak(lm->lineBreak());
            undo(new ChangeMMRest(m, mmr));
            }
      mmr->setMMRestCount(n);
      mmr->setNo(m->no());

      Segment* ss = lm->findSegmentR(SegmentType::EndBarLine, lm->ticks());
      if (ss) {
            Segment* ds = mmr->undoGetSegment(SegmentType::EndBarLine, lm->endTick());
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Element* e = ss->element(staffIdx * VOICES);
                  if (e) {
                        if (!ds->element(staffIdx * VOICES)) {
                              Element* ee = e->clone();
                              ee->setParent(ds);
                              undoAddElement(ee);
                              }
                        else {
                              BarLine* bd = toBarLine(ds->element(staffIdx * VOICES));
                              BarLine* bs = toBarLine(e);
                              if (bd->barLineType() != bs->barLineType()) {
                                    bd->undoChangeProperty(Pid::BARLINE_TYPE, QVariant::fromValue(bs->barLineType()));
                                    bd->undoChangeProperty(Pid::GENERATED, true);
                                    }
                              }
                        }
                  }
            }

      Segment* clefSeg = lm->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, lm->ticks());
      if (clefSeg) {
            Segment* mmrClefSeg = mmr->undoGetSegment(clefSeg->segmentType(), lm->endTick());
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  const int track = staff2track(staffIdx);
                  Element* e = clefSeg->element(track);
                  if (e && e->isClef()) {
                        Clef* clef = toClef(e);
                        if (!mmrClefSeg->element(track)) {
                              Clef* mmrClef = clef->clone();
                              mmrClef->setParent(mmrClefSeg);
                              undoAddElement(mmrClef);
                              }
                        else {
                              Clef* mmrClef = toClef(mmrClefSeg->element(track));
                              mmrClef->setClefType(clef->clefType());
                              }
                        }
                  }
            }

      mmr->setRepeatStart(m->repeatStart() || lm->repeatStart());
      mmr->setRepeatEnd(m->repeatEnd() || lm->repeatEnd());

      ElementList oldList = mmr->takeElements();
      ElementList newList = lm->el();

      for (Element* e : m->el()) {
            if (e->isMarker())
                  newList.push_back(e);
            }
      for (Element* e : newList) {
            bool found = false;
            for (Element* ee : oldList) {
                  if (ee->type() == e->type()) {
                        mmr->add(ee);
                        auto i = std::find(oldList.begin(), oldList.end(), ee);
                        if (i != oldList.end())
                              oldList.erase(i);
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  Element* e1 = e->clone();
                  e1->setParent(mmr);
                  undo(new AddElement(e1));
                  }
            }
      for (Element* e : oldList)
            delete e;
      Segment* s = mmr->undoGetSegmentR(SegmentType::ChordRest, Fraction(0,1));
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            if (s->element(track) == 0) {
                  Rest* r = new Rest(this);
                  r->setDurationType(TDuration::DurationType::V_MEASURE);
                  r->setTicks(mmr->ticks());
                  r->setTrack(track);
                  r->setParent(s);
                  undo(new AddElement(r));
                  }
            }

      //
      // check for clefs
      //
      Segment* cs = lm->findSegmentR(SegmentType::Clef, lm->ticks());
      Segment* ns = mmr->findSegment(SegmentType::Clef, lm->endTick());
      if (cs) {
            if (ns == 0)
                  ns = mmr->undoGetSegmentR(SegmentType::Clef, lm->ticks());
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  Clef* clef = toClef(cs->element(track));
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
      cs = m->findSegmentR(SegmentType::TimeSig, Fraction(0,1));
      ns = mmr->findSegment(SegmentType::TimeSig, m->tick());
      if (cs) {
            if (ns == 0)
                  ns = mmr->undoGetSegmentR(SegmentType::TimeSig, Fraction(0,1));
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  TimeSig* ts = toTimeSig(cs->element(track));
                  if (ts) {
                        TimeSig* nts = toTimeSig(ns->element(track));
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
      // check for ambitus
      //
      cs = m->findSegmentR(SegmentType::Ambitus, Fraction(0,1));
      ns = mmr->findSegment(SegmentType::Ambitus, m->tick());
      if (cs) {
            if (ns == 0)
                  ns = mmr->undoGetSegmentR(SegmentType::Ambitus, Fraction(0,1));
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  Ambitus* a = toAmbitus(cs->element(track));
                  if (a) {
                        Ambitus* na = toAmbitus(ns->element(track));
                        if (!na) {
                              na = a->clone();
                              na->setParent(ns);
                              undo(new AddElement(na));
                              }
                        else {
                              na->initFrom(a);
                              na->layout();
                              }
                        }
                  }
            }
      else if (ns)
            undo(new RemoveElement(ns));

      //
      // check for key signature
      //
      cs = m->findSegmentR(SegmentType::KeySig, Fraction(0,1));
      ns = mmr->findSegmentR(SegmentType::KeySig, Fraction(0,1));
      if (cs) {
            if (ns == 0)
                  ns = mmr->undoGetSegmentR(SegmentType::KeySig, Fraction(0,1));
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  KeySig* ts  = toKeySig(cs->element(track));
                  KeySig* nts = toKeySig(ns->element(track));
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
      else if (ns && ns->empty())
            undo(new RemoveElement(ns));

      //
      // check for rehearsal mark etc.
      //
      cs = m->findSegmentR(SegmentType::ChordRest, Fraction(0,1));
      if (cs) {
            for (Element* e : cs->annotations()) {
                  if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()))
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
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()))
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

      MeasureBase* nm = _showVBox ? lm->next() : lm->nextMeasure();
      mmr->setNext(nm);
      mmr->setPrev(m->prev());
      }

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

static bool validMMRestMeasure(Measure* m)
      {
      if (m->irregular())
            return false;

      int n = 0;
      for (Segment* s = m->first(); s; s = s->next()) {
            for (Element* e : s->annotations()) {
                  if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()))
                        return false;
                  }
            if (s->isChordRestType()) {
                  bool restFound = false;
                  int tracks = m->score()->ntracks();
                  for (int track = 0; track < tracks; ++track) {
                        if ((track % VOICES) == 0 && !m->score()->staff(track/VOICES)->show()) {
                              track += VOICES-1;
                              continue;
                              }
                        if (s->element(track))  {
                              if (!s->element(track)->isRest())
                                    return false;
                              restFound = true;
                              }
                        }
                  for (Element* e : s->annotations()) {
                        if (e->isFermata())
                              return false;
                        }
                  if (restFound)
                        ++n;
                  // measure is not empty if there is more than one rest
                  if (n > 1)
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

      if (m->repeatStart()
         || (m->prevMeasure() && m->prevMeasure()->repeatEnd())
         || (m->isIrregular())
         || (m->prevMeasure() && m->prevMeasure()->isIrregular())
         || (m->prevMeasure() && (m->prevMeasure()->sectionBreak())))
            return true;

      auto sl = m->score()->spannerMap().findOverlapping(m->tick().ticks(), m->endTick().ticks());
      for (auto i : sl) {
            Spanner* s = i.value;
            if (s->isVolta() && (s->tick() == m->tick() || s->tick2() == m->tick()))
                  return true;
            }

      // break for marker in this measure
      for (Element* e : m->el()) {
            if (e->isMarker()) {
                  Marker* mark = toMarker(e);
                  if (!(mark->align() & Align::RIGHT))
                        return true;
                  }
            }

      // break for marker & jump in previous measure
      Measure* pm = m->prevMeasure();
      if (pm) {
            for (Element* e : pm->el()) {
                  if (e->isJump())
                        return true;
                  else if (e->isMarker()) {
                        Marker* mark = toMarker(e);
                        if (mark->align() & Align::RIGHT)
                              return true;
                        }
                  }
            }

      // break for end of volta
      auto l = m->score()->spannerMap().findOverlapping(m->tick().ticks(), m->endTick().ticks());
      for (auto isp : l) {
            Spanner* s = isp.value;
            if (s->isVolta() && (s->tick2() == m->endTick()))
                  return true;
            }

      for (Segment* s = m->first(); s; s = s->next()) {
            for (Element* e : s->annotations()) {
                  if (!e->visible())
                        continue;
                  if (e->isRehearsalMark() ||
                      e->isTempoText() ||
                      ((e->isHarmony() || e->isStaffText() || e->isSystemText()) && (e->systemFlag() || m->score()->staff(e->staffIdx())->show())))
                        return true;
                  }
            for (int staffIdx = 0; staffIdx < m->score()->nstaves(); ++staffIdx) {
                  if (!m->score()->staff(staffIdx)->show())
                        continue;
                  Element* e = s->element(staffIdx * VOICES);
                  if (!e || e->generated())
                        continue;
                  if (s->isStartRepeatBarLineType())
                        return true;
                  if (s->isType(SegmentType::KeySig | SegmentType::TimeSig) && m->tick().isNotZero())
                        return true;
                  if (s->isClefType()) {
                        if (s->tick() != m->endTick() && m->tick().isNotZero())
                              return true;
                        }
                  }
            }
      if (pm) {
            Segment* s = pm->findSegmentR(SegmentType::EndBarLine, pm->ticks());
            if (s) {
                  for (int staffIdx = 0; staffIdx < s->score()->nstaves(); ++staffIdx) {
                        BarLine* bl = toBarLine(s->element(staffIdx * VOICES));
                        if (bl) {
                              BarLineType t = bl->barLineType();
                              if (t != BarLineType::NORMAL && t != BarLineType::BROKEN && t != BarLineType::DOTTED && !bl->generated())
                                    return true;
                              else
                                    break;
                              }
                        }
                  }
            if (pm->findSegment(SegmentType::Clef, m->tick()))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   adjustMeasureNo
//---------------------------------------------------------

int LayoutContext::adjustMeasureNo(MeasureBase* m)
      {
      measureNo += m->noOffset();
      m->setNo(measureNo);
      if (!m->irregular())          // don’t count measure
            ++measureNo;
      if (m->sectionBreak())
            measureNo = 0;
      return measureNo;
      }

//---------------------------------------------------------
//   createBeams
//    helper function
//---------------------------------------------------------

void Score::createBeams(Measure* measure)
      {
      bool crossMeasure = styleB(Sid::crossMeasureValues);

      for (int track = 0; track < ntracks(); ++track) {
            Staff* stf = staff(track2staff(track));

            // don’t compute beams for invisible staffs and tablature without stems
            if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->slashStyle()))
                  continue;

            ChordRest* a1    = 0;      // start of (potential) beam
            bool firstCR     = true;
            Beam* beam       = 0;      // current beam
            Beam::Mode bm    = Beam::Mode::AUTO;
            ChordRest* prev  = 0;
            bool checkBeats  = false;
            Fraction stretch = Fraction(1,1);
            QHash<int, TDuration> beatSubdivision;

            // if this measure is simple meter (actually X/4),
            // then perform a prepass to determine the subdivision of each beat

            beatSubdivision.clear();
            TimeSig* ts = stf->timeSig(measure->tick());
            checkBeats  = false;
            stretch     = ts ? ts->stretch() : Fraction(1,1);

            const SegmentType st = SegmentType::ChordRest;
            if (ts && ts->denominator() == 4) {
                  checkBeats = true;
                  for (Segment* s = measure->first(st); s; s = s->next(st)) {
                        ChordRest* mcr = toChordRest(s->element(track));
                        if (mcr == 0)
                              continue;
                        int beat = (mcr->rtick() * stretch).ticks() / MScore::division;
                        if (beatSubdivision.contains(beat))
                              beatSubdivision[beat] = qMin(beatSubdivision[beat], mcr->durationType());
                        else
                              beatSubdivision[beat] = mcr->durationType();
                        }
                  }

            for (Segment* segment = measure->first(st); segment; segment = segment->next(st)) {
                  ChordRest* cr = segment->cr(track);
                  if (cr == 0)
                        continue;

                  if (firstCR) {
                        firstCR = false;
                        // Handle cross-measure beams
                        Beam::Mode mode = cr->beamMode();
                        if (mode == Beam::Mode::MID || mode == Beam::Mode::END) {
                              ChordRest* prevCR = findCR(measure->tick() - Fraction::fromTicks(1), track);
                              if (prevCR) {
                                    const Measure* pm = prevCR->measure();
                                    if (!beamNoContinue(prevCR->beamMode())
                                        && !pm->lineBreak() && !pm->pageBreak() && !pm->sectionBreak()) {
                                          beam = prevCR->beam();
                                          a1 = beam ? beam->elements().front() : prevCR;
                                          }
                                    }
                              }
                        }
#if 0
                  for (Lyrics* l : cr->lyrics()) {
                        if (l)
                              l->layout();
                        }
#endif
                  // handle grace notes and cross-measure beaming
                  // (tied chords?)
                  if (cr->isChord()) {
                        Chord* chord = toChord(cr);
                        beamGraceNotes(chord, false); // grace before
                        beamGraceNotes(chord, true);  // grace after
                        // set up for cross-measure values as soon as possible
                        // to have all computations (stems, hooks, ...) consistent with it
                        if (!chord->isGrace())
                              chord->crossMeasureSetup(crossMeasure);
                        }

                  // get defaults from time signature properties
                  bm = Groups::endBeam(cr, prev);

                  // perform additional context-dependent checks
                  if (bm == Beam::Mode::AUTO) {
                        // check if we need to break beams according to minimum duration in current / previous beat
                        if (checkBeats && cr->rtick().isNotZero()) {
                              Fraction tick = cr->rtick() * stretch;
                              // check if on the beat
                              if ((tick.ticks() % MScore::division) == 0) {
                                    int beat = tick.ticks() / MScore::division;
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

                  if ((cr->durationType().type() <= TDuration::DurationType::V_QUARTER) || (bm == Beam::Mode::NONE)) {
                        bool removeBeam = true;
                        if (beam) {
                              beam->layout1();
                              removeBeam = (beam->elements().size() <= 1);
                              beam = 0;
                              }
                        if (a1) {
                              if (removeBeam)
                                    a1->removeDeleteBeam(false);
                              a1 = 0;
                              }
                        cr->removeDeleteBeam(false);
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
                           )
                              {
                              a1->removeDeleteBeam(false);
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
                  a1->removeDeleteBeam(false);
            }
      }

//---------------------------------------------------------
//   breakCrossMeasureBeams
//---------------------------------------------------------

static void breakCrossMeasureBeams(Measure* measure)
      {
      MeasureBase* mbNext = measure->next();
      if (!mbNext || !mbNext->isMeasure())
            return;

      Measure* next = toMeasure(mbNext);
      Score* score = measure->score();
      const int ntracks = score->ntracks();
      Segment* fstSeg = next->first(SegmentType::ChordRest);

      for (int track = 0; track < ntracks; ++track) {
            Staff* stf = score->staff(track2staff(track));

            // don’t compute beams for invisible staffs and tablature without stems
            if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->slashStyle()))
                  continue;

            Element* e = fstSeg->element(track);
            if (!e || !e->isChordRest())
                  continue;

            ChordRest* cr = toChordRest(e);
            Beam* beam = cr->beam();
            if (!beam || beam->elements().front()->measure() == next) // no beam or not cross-measure beam
                  continue;

            std::vector<ChordRest*> mElements;
            std::vector<ChordRest*> nextElements;

            for (ChordRest* beamCR : beam->elements()) {
                  if (beamCR->measure() == measure)
                        mElements.push_back(beamCR);
                  else
                        nextElements.push_back(beamCR);
                  }

            if (mElements.size() == 1)
                  mElements[0]->removeDeleteBeam(false);

            Beam* newBeam = nullptr;
            if (nextElements.size() > 1) {
                  newBeam = new Beam(score);
                  newBeam->setGenerated(true);
                  newBeam->setTrack(track);
                  }

            const bool nextBeamed = bool(newBeam);
            for (ChordRest* nextCR : nextElements) {
                  nextCR->removeDeleteBeam(nextBeamed);
                  if (newBeam)
                        newBeam->add(nextCR);
                  }

            if (newBeam)
                  newBeam->layout1();
            }
      }

//---------------------------------------------------------
//   layoutDrumsetChord
//---------------------------------------------------------

void layoutDrumsetChord(Chord* c, const Drumset* drumset, const StaffType* st, qreal spatium)
      {
      for (Note* note : c->notes()) {
            int pitch = note->pitch();
            if (!drumset->isValid(pitch)) {
                  // qDebug("unmapped drum note %d", pitch);
                  }
            else if (!note->fixed()) {
                  note->undoChangeProperty(Pid::HEAD_GROUP, int(drumset->noteHead(pitch)));
                  int line = drumset->line(pitch);
                  note->setLine(line);

                  int off  = st->stepOffset();
                  qreal ld = st->lineDistance().val();
                  note->rypos()  = (line + off * 2.0) * spatium * .5 * ld;
                  }
            }
      }

//---------------------------------------------------------
//   connectTremolo
//    Connect two-notes tremolo and update duration types
//    for the involved chords.
//---------------------------------------------------------

static void connectTremolo(Measure* m)
      {
      const int ntracks = m->score()->ntracks();
      constexpr SegmentType st = SegmentType::ChordRest;
      for (Segment* s = m->first(st); s; s = s->next(st)) {
            for (int i = 0; i < ntracks; ++i) {
                  Element* e = s->element(i);
                  if (!e || !e->isChord())
                        continue;

                  Chord* c = toChord(e);
                  Tremolo* tremolo = c->tremolo();
                  if (tremolo && tremolo->twoNotes()) {
                        // Ensure correct duration type for chord
                        c->setDurationType(tremolo->durationType());

                        // If it is the first tremolo's chord, find the second
                        // chord for tremolo, if needed.
                        if (!tremolo->chord1())
                              tremolo->setChords(c, tremolo->chord2());
                        else if (tremolo->chord1() != c || tremolo->chord2())
                              continue;

                        for (Segment* ls = s->next(st); ls; ls = ls->next(st)) {
                              if (Element* element = ls->element(i)) {
                                    if (!element->isChord()) {
                                          qDebug("cannot connect tremolo");
                                          continue;
                                          }
                                    Chord* nc = toChord(element);
                                    tremolo->setChords(c, nc);
                                    nc->setTremolo(tremolo);
                                    break;
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   getNextMeasure
//---------------------------------------------------------

void Score::getNextMeasure(LayoutContext& lc)
      {
      lc.prevMeasure = lc.curMeasure;
      lc.curMeasure  = lc.nextMeasure;
      if (!lc.curMeasure)
            lc.nextMeasure = _showVBox ? first() : firstMeasure();
      else
            lc.nextMeasure = _showVBox ? lc.curMeasure->next() : lc.curMeasure->nextMeasure();
      if (!lc.curMeasure)
            return;

      int mno = lc.adjustMeasureNo(lc.curMeasure);

      if (lc.curMeasure->isMeasure()) {
            if (score()->styleB(Sid::createMultiMeasureRests)) {
                  Measure* m = toMeasure(lc.curMeasure);
                  Measure* nm = m;
                  Measure* lm = nm;
                  int n       = 0;
                  Fraction len;

                  lc.measureNo = m->no();

                  while (validMMRestMeasure(nm)) {
                        MeasureBase* mb = _showVBox ? nm->next() : nm->nextMeasure();
                        if (breakMultiMeasureRest(nm) && n)
                              break;
                        lc.adjustMeasureNo(nm);
                        ++n;
                        len += nm->ticks();
                        lm = nm;
                        if (!(mb && mb->isMeasure()))
                              break;
                        nm = toMeasure(mb);
                        }
                  if (n >= styleI(Sid::minEmptyMeasures)) {
                        createMMRest(m, lm, len);
                        lc.curMeasure  = m->mmRest();
                        lc.nextMeasure = _showVBox ?  lm->next() : lm->nextMeasure();
                        }
                  else {
                        if (m->mmRest())
                              undo(new ChangeMMRest(m, 0));
                        m->setMMRestCount(0);
                        lc.measureNo = mno;
                        }
                  }
            else if (toMeasure(lc.curMeasure)->isMMRest()) {
                  qDebug("mmrest: no %d += %d", lc.measureNo, toMeasure(lc.curMeasure)->mmRestCount());
                  lc.measureNo += toMeasure(lc.curMeasure)->mmRestCount() - 1;
                  }
            }
      if (!lc.curMeasure->isMeasure()) {
            lc.curMeasure->setTick(lc.tick);
            return;
            }

      //-----------------------------------------
      //    process one measure
      //-----------------------------------------

      Measure* measure = toMeasure(lc.curMeasure);
      measure->moveTicks(lc.tick - measure->tick());

      connectTremolo(measure);

      //
      // calculate accidentals and note lines,
      // create stem and set stem direction
      //
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            const Staff* staff     = Score::staff(staffIdx);
            const Drumset* drumset = staff->part()->instrument()->useDrumset() ? staff->part()->instrument()->drumset() : 0;
            AccidentalState as;      // list of already set accidentals for this measure
            as.init(staff->keySigEvent(measure->tick()), staff->clef(measure->tick()));

            for (Segment& segment : measure->segments()) {
                  if (segment.isKeySigType()) {
                        KeySig* ks = toKeySig(segment.element(staffIdx * VOICES));
                        if (!ks)
                              continue;
                        Fraction tick = segment.tick();
                        as.init(staff->keySigEvent(tick), staff->clef(tick));
                        ks->layout();
                        }
                  else if (segment.isChordRestType()) {
                        const StaffType* st = staff->staffType(segment.tick());
                        int track     = staffIdx * VOICES;
                        int endTrack  = track + VOICES;

                        for (int t = track; t < endTrack; ++t) {
                              ChordRest* cr = segment.cr(t);
                              if (!cr)
                                    continue;
                              qreal m = staff->mag(segment.tick());
                              if (cr->small())
                                    m *= score()->styleD(Sid::smallNoteMag);

                              if (cr->isChord()) {
                                    Chord* chord = toChord(cr);
                                    chord->cmdUpdateNotes(&as);
                                    for (Chord* c : chord->graceNotes()) {
                                          c->setMag(m * score()->styleD(Sid::graceNoteMag));
                                          c->computeUp();
                                          if (c->stemDirection() != Direction::AUTO)
                                                c->setUp(c->stemDirection() == Direction::UP);
                                          else
                                                c->setUp(!(t % 2));
                                          if (drumset)
                                                layoutDrumsetChord(c, drumset, st, spatium());
                                          c->layoutStem1();
                                          }
                                    if (drumset)
                                          layoutDrumsetChord(chord, drumset, st, spatium());
                                    chord->computeUp();
                                    chord->layoutStem1();   // create stems needed to calculate spacing
                                                            // stem direction can change later during beam processing
                                    }
                              cr->setMag(m);
                              }
                        }
                  else if (segment.isClefType()) {
                        Element* e = segment.element(staffIdx * VOICES);
                        if (e) {
                              toClef(e)->setSmall(true);
                              e->layout();
                              }
                        }
                  else if (segment.isType(SegmentType::TimeSig | SegmentType::Ambitus | SegmentType::HeaderClef)) {
                        Element* e = segment.element(staffIdx * VOICES);
                        if (e)
                              e->layout();
                        }
                  }
            }

      createBeams(measure);

      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            for (Segment& segment : measure->segments()) {
                  if (segment.isChordRestType()) {
                        layoutChords1(&segment, staffIdx);
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = segment.cr(staffIdx * VOICES + voice);
                              if (cr) {
                                    for (Lyrics* l : cr->lyrics()) {
                                          if (l)
                                                l->layout();
                                          }
                                    if (cr->isChord()) {
                                          Chord* c = toChord(cr);
                                          c->layoutArticulations();
                                          }
                                    }
                              }
                        }
                  }
            }

      measure->computeTicks();

      if (isMaster()) {
            // Reset tempo to set correct time stretch for fermata.
            const Fraction& startTick = measure->tick();
            resetTempoRange(startTick, measure->endTick());

            // Implement section break rest
            for (MeasureBase* mb = measure->prev(); mb && mb->endTick() == startTick; mb = mb->prev()) {
                  if (mb->pause())
                        setPause(startTick, mb->pause());
                  }

            // Add pauses from the end of the previous measure (at measure->tick()):
            for (Segment* s = measure->first()->prev1(); s && s->tick() == startTick; s = s->prev1()) {
                  if (!s->isBreathType())
                        continue;
                  qreal length = 0.0;
                  for (Element* e : s->elist()) {
                        if (e && e->isBreath())
                              length = qMax(length, toBreath(e)->pause());
                        }
                  if (length != 0.0)
                        setPause(startTick, length);
                  }
            }

      for (Segment& segment : measure->segments()) {
            if (segment.isBreathType()) {
                  qreal length = 0.0;
                  Fraction tick = segment.tick();
                  // find longest pause
                  for (int i = 0, n = ntracks(); i < n; ++i) {
                        Element* e = segment.element(i);
                        if (e && e->isBreath()) {
                              Breath* b = toBreath(e);
                              b->layout();
                              length = qMax(length, b->pause());
                              }
                        }
                  if (length != 0.0)
                        setPause(tick, length);
                  }
            else if (segment.isTimeSigType()) {
                  for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                        TimeSig* ts = toTimeSig(segment.element(staffIdx * VOICES));
                        if (ts)
                              staff(staffIdx)->addTimeSig(ts);
                        }
                  }
            else if (segment.isChordRestType()) {
                  for (Element* e : segment.annotations()) {
                        if (e->isSymbol())
                              e->layout();
                        }
                  if (!isMaster())
                        continue;
#if 0 // ws
                  for (Element* e : segment.annotations()) {
                        if (!(e->isTempoText()
                           || e->isDynamic()
                           || e->isFermata()
                           || e->isRehearsalMark()
                           || e->isFretDiagram()
                           || e->isHarmony()
                           || e->isStaffText()
                           || e->isFiguredBass())) {
                              e->layout();             // system text ?
                              }
                        }
#endif
                  // TODO, this is not going to work, we just cleaned the tempomap
                  // it breaks the test midi/testBaroqueOrnaments.mscx where first note has stretch 2
                  // Also see fixTicks
                  qreal stretch = 0.0;
                  for (Element* e : segment.annotations()) {
                        if (e->isFermata())
                              stretch = qMax(stretch, toFermata(e)->timeStretch());
                        else if (e->isTempoText()) {
                              if (isMaster()) {
                                    TempoText* tt = toTempoText(e);
                                    setTempo(tt->segment(), tt->tempo());
                                    }
                              }
                        }
                  if (stretch != 0.0 && stretch != 1.0) {
                        qreal otempo = tempomap()->tempo(segment.tick().ticks());
                        qreal ntempo = otempo / stretch;
                        setTempo(segment.tick(), ntempo);
                        Fraction etick = segment.tick() + segment.ticks() - Fraction(1, 480*4);
                        auto e = tempomap()->find(etick.ticks());
                        if (e == tempomap()->end())
                              setTempo(etick, otempo);
                        }
                  }
            }

      // update time signature map
      // create event if measure len and time signature are different
      // even if they are equivalent 4/4 vs 2/2
      // also check if nominal time signature has changed

      if (isMaster() && ((!measure->ticks().identical(lc.sig)
         && measure->ticks() != lc.sig * measure->mmRestCount())
         || (lc.prevMeasure && lc.prevMeasure->isMeasure()
         && !measure->timesig().identical(toMeasure(lc.prevMeasure)->timesig()))))
            {
            if (measure->isMMRest())
                  lc.sig = measure->mmRestFirst()->ticks();
            else
                  lc.sig = measure->ticks();
            sigmap()->add(lc.tick.ticks(), SigEvent(lc.sig, measure->timesig(), measure->no()));
            }

      Segment* seg = measure->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0,1));
      if (measure->repeatStart()) {
            if (!seg)
                  seg = measure->getSegmentR(SegmentType::StartRepeatBarLine, Fraction(0,1));
            measure->barLinesSetSpan(seg);      // this also creates necessary barlines
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  BarLine* b = toBarLine(seg->element(staffIdx * VOICES));
                  if (b) {
                        b->setBarLineType(BarLineType::START_REPEAT);
                        b->layout();
                        }
                  }
            }
      else if (seg)
            score()->undoRemoveElement(seg);

      for (Segment& s : measure->segments()) {
            // DEBUG: relayout grace notes as beaming/flags may have changed
            if (s.isChordRestType()) {
                  for (Element* e : s.elist()) {
                        if (e && e->isChord()) {
                              Chord* chord = toChord(e);
                              chord->layout();
//                              if (chord->tremolo())            // debug
//                                    chord->tremolo()->layout();
                              }
                        }
                  }
            else if (s.isEndBarLineType())
                  continue;
            s.createShapes();
            }

      lc.tick += measure->ticks();
      }

//---------------------------------------------------------
//   isTopBeam
//---------------------------------------------------------

bool isTopBeam(ChordRest* cr)
      {
      Beam* b = cr->beam();
      if (b && !b->cross() && b->elements().front() == cr) {
            for (ChordRest* cr1 : b->elements()) {
                  if (cr1->staffMove() >= 0)
                        return true;
                  }
            }

      return false;
      }

//---------------------------------------------------------
//   notTopBeam
//---------------------------------------------------------

bool notTopBeam(ChordRest* cr)
      {
      Beam* b = cr->beam();
      if (b && b->elements().front() == cr) {
            if (b->cross())
                  return true;

            for (ChordRest* cr1 : b->elements()) {
                  if (cr1->staffMove() >= 0)
                        return false;
            }
      }

      return false;
      }

//---------------------------------------------------------
//   findLyricsMaxY
//---------------------------------------------------------

static qreal findLyricsMaxY(Segment& s, int staffIdx)
      {
      qreal yMax = 0.0;
      if (!s.isChordRestType())
            return yMax;

      qreal lyricsMinTopDistance = s.score()->styleP(Sid::lyricsMinTopDistance);

      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest* cr = s.cr(staffIdx * VOICES + voice);
            if (cr && !cr->lyrics().empty()) {
                  SkylineLine sk(true);

                  for (Lyrics* l : cr->lyrics()) {
                        if (l->autoplace() && l->placeBelow()) {
                              l->ryoffset() = 0.0;
                              QPointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                              QRectF r = l->bbox().translated(offset);
                              sk.add(r.x(), r.top(), r.width());
                              }
                        }
                  SysStaff* ss = s.measure()->system()->staff(staffIdx);
                  for (Lyrics* l : cr->lyrics()) {
                        if (l->autoplace() && l->placeBelow()) {
                              qreal y = ss->skyline().south().minDistance(sk);
                              if (y > -lyricsMinTopDistance)
                                    yMax = qMax(yMax, y + lyricsMinTopDistance);
                              }
                        }
                  }
            }
      return yMax;
      }

//---------------------------------------------------------
//   findLyricsMinY
//---------------------------------------------------------

static qreal findLyricsMinY(Segment& s, int staffIdx)
      {
      qreal yMin = 0.0;
      if (!s.isChordRestType())
            return yMin;
      qreal lyricsMinTopDistance = s.score()->styleP(Sid::lyricsMinTopDistance);
      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest* cr = s.cr(staffIdx * VOICES + voice);
            if (cr && !cr->lyrics().empty()) {
                  SkylineLine sk(false);

                  for (Lyrics* l : cr->lyrics()) {
                        if (l->autoplace() && l->placeAbove()) {
                              l->ryoffset() = 0.0;
                              QRectF r = l->bbox().translated(l->pos() + cr->pos() + s.pos() + s.measure()->pos());
                              sk.add(r.x(), r.bottom(), r.width());
                              }
                        }
                  SysStaff* ss = s.measure()->system()->staff(staffIdx);
                  for (Lyrics* l : cr->lyrics()) {
                        if (l->autoplace() && l->placeAbove()) {
                              qreal y = sk.minDistance(ss->skyline().north());
                              if (y > -lyricsMinTopDistance)
                                    yMin = qMin(yMin, -y -lyricsMinTopDistance);
                              }
                        }
                  }
            }
      return yMin;
      }

static qreal findLyricsMaxY(Measure* m, int staffIdx)
      {
      qreal yMax = 0.0;
      for (Segment& s : m->segments())
            yMax = qMax(yMax, findLyricsMaxY(s, staffIdx));
      return yMax;
      }

static qreal findLyricsMinY(Measure* m, int staffIdx)
      {
      qreal yMin = 0.0;
      for (Segment& s : m->segments())
            yMin = qMin(yMin, findLyricsMinY(s, staffIdx));
      return yMin;
      }

//---------------------------------------------------------
//   applyLyricsMax
//---------------------------------------------------------

static void applyLyricsMax(Segment& s, int staffIdx, qreal yMax)
      {
      if (!s.isChordRestType())
            return;
      Skyline& sk = s.measure()->system()->staff(staffIdx)->skyline();
      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest* cr = s.cr(staffIdx * VOICES + voice);
            if (cr && !cr->lyrics().empty()) {
                  qreal lyricsMinBottomDistance = s.score()->styleP(Sid::lyricsMinBottomDistance);
                  for (Lyrics* l : cr->lyrics()) {
                        if (l->visible() && l->autoplace() && l->placeBelow()) {
                              l->ryoffset() = yMax;
                              QPointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                              sk.add(l->bbox().translated(offset).adjusted(0.0, 0.0, 0.0, lyricsMinBottomDistance));
                              }
                        }
                  }
            }
      }

static void applyLyricsMax(Measure* m, int staffIdx, qreal yMax)
      {
      for (Segment& s : m->segments())
            applyLyricsMax(s, staffIdx, yMax);
      }

//---------------------------------------------------------
//   applyLyricsMin
//---------------------------------------------------------

static void applyLyricsMin(ChordRest* cr, int staffIdx, qreal yMin)
      {
      Skyline& sk = cr->measure()->system()->staff(staffIdx)->skyline();
      for (Lyrics* l : cr->lyrics()) {
            if (l->visible() && l->autoplace() && l->placeAbove()) {
                  l->ryoffset() = yMin;
                  QPointF offset = l->pos() + cr->pos() + cr->segment()->pos() + cr->segment()->measure()->pos();
                  sk.add(l->bbox().translated(offset));
                  }
            }
      }

static void applyLyricsMin(Measure* m, int staffIdx, qreal yMin)
      {
      for (Segment& s : m->segments()) {
            if (s.isChordRestType()) {
                  for (int voice = 0; voice < VOICES; ++voice) {
                        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                        if (cr)
                              applyLyricsMin(cr, staffIdx, yMin);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   restoreBeams
//---------------------------------------------------------

static void restoreBeams(Measure* m)
      {
      for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (Element* e : s->elist()) {
                  if (e && e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        if (isTopBeam(cr)) {
                              cr->beam()->layout();
                              cr->beam()->addSkyline(m->system()->staff(cr->beam()->staffIdx())->skyline());
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutLyrics
//
//    vertical align lyrics
//
//---------------------------------------------------------

void Score::layoutLyrics(System* system)
      {
      std::vector<int> visibleStaves;
      for (int staffIdx = system->firstVisibleStaff(); staffIdx < nstaves(); staffIdx = system->nextVisibleStaff(staffIdx))
            visibleStaves.push_back(staffIdx);

      //int nAbove[nstaves()];
      std::vector<int> VnAbove(nstaves());

      for (int staffIdx : visibleStaves) {
            VnAbove[staffIdx] = 0;
            for (MeasureBase* mb : system->measures()) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  for (Segment& s : m->segments()) {
                        if (s.isChordRestType()) {
                              for (int voice = 0; voice < VOICES; ++voice) {
                                    ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                                    if (cr) {
                                          int nA = 0;
                                          for (Lyrics* l : cr->lyrics()) {
                                                if (l->placeAbove())
                                                      ++nA;
                                                }
                                          VnAbove[staffIdx] = qMax(VnAbove[staffIdx], nA);
                                          }
                                    }
                              }
                        }
                  }
            }

      for (int staffIdx : visibleStaves) {
            for (MeasureBase* mb : system->measures()) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  for (Segment& s : m->segments()) {
                        if (s.isChordRestType()) {
                              for (int voice = 0; voice < VOICES; ++voice) {
                                    ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                                    if (cr) {
                                          for (Lyrics* l : cr->lyrics())
                                                l->layout2(VnAbove[staffIdx]);
                                          }
                                    }
                              }
                        }
                  }
            }

      VerticalAlignRange ar = VerticalAlignRange(styleI(Sid::autoplaceVerticalAlignRange));

      switch (ar) {
            case VerticalAlignRange::MEASURE:
                  for (MeasureBase* mb : system->measures()) {
                        if (!mb->isMeasure())
                              continue;
                        Measure* m = toMeasure(mb);
                        for (int staffIdx : visibleStaves) {
                              qreal yMax = findLyricsMaxY(m, staffIdx);
                              applyLyricsMax(m, staffIdx, yMax);
                              }
                        }
                  break;
            case VerticalAlignRange::SYSTEM:
                  for (int staffIdx : visibleStaves) {
                        qreal yMax = 0.0;
                        qreal yMin = 0.0;
                        for (MeasureBase* mb : system->measures()) {
                              if (!mb->isMeasure())
                                    continue;
                              yMax = qMax<qreal>(yMax, findLyricsMaxY(toMeasure(mb), staffIdx));
                              yMin = qMin(yMin, findLyricsMinY(toMeasure(mb), staffIdx));
                              }
                        for (MeasureBase* mb : system->measures()) {
                              if (!mb->isMeasure())
                                    continue;
                              applyLyricsMax(toMeasure(mb), staffIdx, yMax);
                              applyLyricsMin(toMeasure(mb), staffIdx, yMin);
                              }
                        }
                  break;
            case VerticalAlignRange::SEGMENT:
                  for (MeasureBase* mb : system->measures()) {
                        if (!mb->isMeasure())
                              continue;
                        Measure* m = toMeasure(mb);
                        for (int staffIdx : visibleStaves) {
                              for (Segment& s : m->segments()) {
                                    qreal yMax = findLyricsMaxY(s, staffIdx);
                                    applyLyricsMax(s, staffIdx, yMax);
                                    }
                              }
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   layoutTies
//---------------------------------------------------------

void layoutTies(Chord* ch, System* system, const Fraction& stick)
      {
      for (Note* note : ch->notes()) {
            Tie* t = note->tieFor();
            if (t) {
                  TieSegment* ts = t->layoutFor(system);
                  system->staff(ch->staffIdx())->skyline().add(ts->shape().translated(ts->pos()));
                  }
            t = note->tieBack();
            if (t) {
                  if (t->startNote()->tick() < stick) {
                        TieSegment* ts = t->layoutBack(system);
                        system->staff(ch->staffIdx())->skyline().add(ts->shape().translated(ts->pos()));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutHarmonies
//---------------------------------------------------------

void layoutHarmonies(const std::vector<Segment*>& sl)
      {
      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isHarmony()) {
                        Harmony* h = toHarmony(e);
                        // For chord symbols that coincide with a chord or rest,
                        // a partial layout can also happen (if needed) during ChordRest layout
                        // in order to calculate a bbox and allocate its shape to the ChordRest.
                        // But that layout (if it happens at all) does not do autoplace,
                        // so we need the full layout here.
                        h->layout();
                        h->autoplaceSegmentElement(s->score()->styleP(Sid::minHarmonyDistance));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   processLines
//---------------------------------------------------------

static void processLines(System* system, std::vector<Spanner*> lines, bool align)
      {
      std::vector<SpannerSegment*> segments;
      for (Spanner* sp : lines) {
            SpannerSegment* ss = sp->layoutSystem(system);     // create/layout spanner segment for this system
            if (ss->visible() && ss->autoplace())
                  segments.push_back(ss);
            }

      if (align && segments.size() > 1) {
            qreal y = segments[0]->rypos();
            for (unsigned i = 1; i < segments.size(); ++i)
                  y = qMax(y, segments[i]->rypos());
            for (auto ss : segments)
                  ss->rypos() = y;
            }

      //
      // add shapes to skyline
      //
      for (SpannerSegment* ss : segments)
            system->staff(ss->staffIdx())->skyline().add(ss->shape().translated(ss->pos()));
      }

//---------------------------------------------------------
//   collectSystem
//---------------------------------------------------------

System* Score::collectSystem(LayoutContext& lc)
      {
      if (!lc.curMeasure)
            return 0;
      Measure* measure  = _systems.empty() ? 0 : _systems.back()->lastMeasure();
      if (measure) {
            lc.firstSystem        = measure->sectionBreak() && _layoutMode != LayoutMode::FLOAT;
            lc.startWithLongNames = lc.firstSystem && measure->sectionBreakElement()->startWithLongNames();
            }
      System* system = getNextSystem(lc);
      Fraction lcmTick = lc.curMeasure ? lc.curMeasure->tick() : Fraction(0,1);
      system->setInstrumentNames(lc.startWithLongNames, lcmTick);

      qreal minWidth    = 0;
      qreal layoutSystemMinWidth = 0;
      bool firstMeasure = true;
      bool createHeader = false;
      qreal systemWidth = styleD(Sid::pagePrintableWidth) * DPI;
      system->setWidth(systemWidth);

      while (lc.curMeasure) {    // collect measure for system
            System* oldSystem = lc.curMeasure->system();
            system->appendMeasure(lc.curMeasure);

            qreal ww  = 0;      // width of current measure

            if (lc.curMeasure->isMeasure()) {
                  Measure* m = toMeasure(lc.curMeasure);
                  if (firstMeasure) {
                        layoutSystemMinWidth = minWidth;
                        system->layoutSystem(minWidth);
                        minWidth += system->leftMargin();
                        if (m->repeatStart()) {
                              Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0,1));
                              if (!s->enabled())
                                    s->setEnabled(true);
                              }
                        m->addSystemHeader(lc.firstSystem);
                        firstMeasure = false;
                        createHeader = false;
                        }
                  else {
                        if (createHeader) {
                              m->addSystemHeader(false);
                              createHeader = false;
                              }
                        else if (m->header())
                              m->removeSystemHeader();
                        }

                  m->createEndBarLines(true);
                  Measure* nm = m->nextMeasure();
                  if (nm)
                        m->addSystemTrailer(nm);
                  m->computeMinWidth();
                  ww = m->width();
                  }
            else if (lc.curMeasure->isHBox()) {
                  lc.curMeasure->computeMinWidth();
                  ww = lc.curMeasure->width();
                  createHeader = toHBox(lc.curMeasure)->createSystemHeader();
                  }
            else {
                  // vbox:
                  getNextMeasure(lc);
                  system->layout2();   // compute staff distances
                  return system;
                  }
            // check if lc.curMeasure fits, remove if not
            // collect at least one measure and the break

            bool doBreak = (system->measures().size() > 1) && ((minWidth + ww) > systemWidth);
            if (doBreak) {
                  if (lc.prevMeasure->noBreak() && system->measures().size() > 2) {
                        // remove last two measures
                        // TODO: check more measures for noBreak()
                        system->removeLastMeasure();
                        system->removeLastMeasure();
                        lc.curMeasure->setSystem(oldSystem);
                        lc.prevMeasure->setSystem(oldSystem);
                        lc.nextMeasure = lc.curMeasure;
                        lc.curMeasure  = lc.prevMeasure;
                        lc.prevMeasure = lc.curMeasure->prevMeasure();
                        break;
                        }
                  else if (!lc.prevMeasure->noBreak()) {
                        // remove last measure
                        system->removeLastMeasure();
                        lc.curMeasure->setSystem(oldSystem);
                        break;
                        }
                  }

            if (lc.prevMeasure && lc.prevMeasure->isMeasure() && lc.prevMeasure->system() == system) {
                  //
                  // now we know that the previous measure is not the last
                  // measure in the system and we finally can create the end barline for it

                  Measure* m = toMeasure(lc.prevMeasure);
                  if (m->trailer()) {
                        qreal ow = m->width();
                        m->removeSystemTrailer();
                        minWidth += m->width() - ow;
                        }
                  // if the prev measure is an end repeat and the cur measure
                  // is an repeat, the createEndBarLines() created an start-end repeat barline
                  // and we can remove the start repeat barline of the current barline

                  if (lc.curMeasure->isMeasure()) {
                        Measure* m1 = toMeasure(lc.curMeasure);
                        if (m1->repeatStart()) {
                              Segment* s = m1->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0,1));
                              if (!s->enabled()) {
                                    s->setEnabled(true);
                                    m1->computeMinWidth();
                                    ww = m1->width();
                                    }
                              }
                        }
                  minWidth += m->createEndBarLines(false);    // create final barLine
                  }

            MeasureBase* mb = lc.curMeasure;
            bool lineBreak  = false;
            switch (_layoutMode) {
                  case LayoutMode::PAGE:
                  case LayoutMode::SYSTEM:
                        lineBreak = mb->pageBreak() || mb->lineBreak() || mb->sectionBreak();
                        break;
                  case LayoutMode::FLOAT:
                  case LayoutMode::LINE:
                        lineBreak = false;
                        break;
                  }

            getNextMeasure(lc);

            minWidth += ww;
            if (lc.endTick < lc.prevMeasure->tick()) {
                  // TODO: we may check if another measure fits in this system
                  if (lc.prevMeasure == lc.systemOldMeasure) {
                        lc.rangeDone = true;
                        if (lc.curMeasure && lc.curMeasure->isMeasure()) {
                              Measure* m = toMeasure(lc.curMeasure);
                              restoreBeams(m);
                              m->stretchMeasure(lc.curMeasure->width());
                              }
                        break;
                        }
                  }
            // ElementType nt = lc.curMeasure ? lc.curMeasure->type() : ElementType::INVALID;
            mb = lc.curMeasure;
            bool tooWide = false; // minWidth + minMeasureWidth > systemWidth;  // TODO: noBreak
            if (lineBreak || !mb || mb->isVBox() || mb->isTBox() || mb->isFBox() || tooWide)
                  break;
            }

      //
      // now we have a complete set of measures for this system
      //
      // prevMeasure is the last measure in the system
      if (lc.prevMeasure && lc.prevMeasure->isMeasure()) {
            breakCrossMeasureBeams(toMeasure(lc.prevMeasure));
            qreal w = toMeasure(lc.prevMeasure)->createEndBarLines(true);
            minWidth += w;
            }

      hideEmptyStaves(system, lc.firstSystem);
      bool allShown = true;
      for (const SysStaff* ss : *system->staves()) {
            if (!ss->show()) {
                  allShown = false;
                  break;
                  }
            }
      if (!allShown) {
            // Relayout system decorations to reuse space properly for
            // hidden staves' instrument names or other hidden elements.
            minWidth -= system->leftMargin();
            system->layoutSystem(layoutSystemMinWidth);
            minWidth += system->leftMargin();
            }

      //-------------------------------------------------------
      //    add system trailer if needed
      //    (cautionary time/key signatures etc)
      //-------------------------------------------------------

      Measure* lm  = system->lastMeasure();
      if (lm) {
            Measure* nm = lm->nextMeasure();
            if (nm) {
                  qreal w = lm->width();
                  lm->addSystemTrailer(nm);
                  if (lm->trailer())
                        lm->computeMinWidth();
                  minWidth += lm->width() - w;
                  }
            }

      //
      // stretch incomplete row
      //
      qreal rest;
      if (MScore::noHorizontalStretch)
            rest = 0;
      else {
            qreal mw          = system->leftMargin();      // DEBUG
            qreal totalWeight = 0.0;

            for (MeasureBase* mb : system->measures()) {
                  if (mb->isHBox()) {
                        mw += mb->width();
                        }
                  else if (mb->isMeasure()) {
                        Measure* m  = toMeasure(mb);
                        mw          += m->width();               // measures are stretched already with basicStretch()
                        totalWeight += m->ticks().ticks() * m->basicStretch();
                        }
                  }

#ifndef NDEBUG
            if (!qFuzzyCompare(mw, minWidth))
                  qDebug("==layoutSystem %6d old %.1f new %.1f", system->measures().front()->tick().ticks(), minWidth, mw);
#endif
            rest = systemWidth - minWidth;
            //
            // don’t stretch last system row, if accumulated minWidth is <= lastSystemFillLimit
            //
            if (lc.curMeasure == 0 && ((minWidth / systemWidth) <= styleD(Sid::lastSystemFillLimit))) {
                  if (minWidth > rest)
                        rest = rest * .5;
                  else
                        rest = minWidth;
                  }
            rest /= totalWeight;
            }

      QPointF pos;
      firstMeasure = true;
      for (MeasureBase* mb : system->measures()) {
            qreal ww = mb->width();
            if (mb->isMeasure()) {
                  if (firstMeasure) {
                        pos.rx() += system->leftMargin();
                        firstMeasure = false;
                        }
                  mb->setPos(pos);
                  Measure* m = toMeasure(mb);
                  qreal stretch = m->basicStretch();
                  ww  += rest * m->ticks().ticks() * stretch;
                  m->stretchMeasure(ww);
                  m->layoutStaffLines();
                  }
            else if (mb->isHBox()) {
                  mb->setPos(pos + QPointF(toHBox(mb)->topGap(), 0.0));
                  mb->layout();
                  }
            else if (mb->isVBox())
                  mb->setPos(pos);
            pos.rx() += ww;
            }
      system->setWidth(pos.x());

      layoutSystemElements(system, lc);
      system->layout2();   // compute staff distances

      lm  = system->lastMeasure();
      if (lm) {
            lc.firstSystem        = lm->sectionBreak() && _layoutMode != LayoutMode::FLOAT;
            lc.startWithLongNames = lc.firstSystem && lm->sectionBreakElement()->startWithLongNames();
            }

      return system;
      }

//---------------------------------------------------------
//   layoutSystemElements
//---------------------------------------------------------

void Score::layoutSystemElements(System* system, LayoutContext& lc)
      {
      //-------------------------------------------------------------
      //    create cr segment list to speed up computations
      //-------------------------------------------------------------

      std::vector<Segment*> sl;
      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            Measure* m = toMeasure(mb);
            m->layoutMeasureNumber();
            for (Segment* s = m->first(); s; s = s->next()) {
                  if (s->isChordRestType() || !s->annotations().empty())
                        sl.push_back(s);
                  }
            }

      //-------------------------------------------------------------
      //    create skylines
      //-------------------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            SysStaff* ss = system->staff(staffIdx);
            Skyline& skyline = ss->skyline();
            skyline.clear();
            for (MeasureBase* mb : system->measures()) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  for (Segment& s : m->segments()) {
                        if (!s.enabled() || s.isTimeSigType())       // hack: ignore time signatures
                              continue;
                        QPointF p(s.pos() + m->pos());
                        if (s.segmentType() & (SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine)) {
                              BarLine* bl = toBarLine(s.element(0));
                              if (bl) {
                                    qreal w = BarLine::layoutWidth(score(), bl->barLineType());
                                    skyline.add(QRectF(0.0, 0.0, w, spatium() * 4.0).translated(bl->pos() + p));
                                    }
                              }
                        else {
                              int strack = staffIdx * VOICES;
                              int etrack = strack + VOICES;
                              for (Element* e : s.elist()) {
                                    if (!e)
                                          continue;
                                    // clear layout for chord-based fingerings
                                    if (e->isChord()) {
                                          Chord* c = toChord(e);
                                          std::list<Note*> notes;
                                          for (auto gc : c->graceNotes()) {
                                                for (auto n : gc->notes())
                                                      notes.push_back(n);
                                                }
                                          for (auto n : c->notes())
                                                notes.push_back(n);
                                          std::list<Fingering*> fingerings;
                                          for (Note* note : notes) {
                                                for (Element* e : note->el()) {
                                                      if (e->isFingering()) {
                                                            Fingering* f = toFingering(e);
                                                            if (f->layoutType() == ElementType::CHORD) {
                                                                  f->setPos(QPointF());
                                                                  f->setbbox(QRectF());
                                                                  }
                                                            }
                                                      }
                                                }
                                          }
                                    if (!e->visible())
                                          continue;
                                    int effectiveTrack = e->vStaffIdx() * VOICES + e->voice();
                                    if (effectiveTrack >= strack && effectiveTrack < etrack)
                                          skyline.add(e->shape().translated(e->pos() + p));
                                    if (e->isChord() && toChord(e)->tremolo()) {
                                          Tremolo* t = toChord(e)->tremolo();
                                          skyline.add(t->shape().translated(t->pos() + p));
                                          }
                                    }
                              }
                        }
                  ss->skyline().add(m->staffLines(staffIdx)->bbox().translated(m->pos()));
                  MeasureNumber* mno = m->noText(staffIdx);
                  if (mno)
                        ss->skyline().add(mno->bbox().translated(m->pos() + mno->pos()));
                  }
            }

      //-------------------------------------------------------------
      // layout beams + fingering
      //-------------------------------------------------------------

      for (Segment* s : sl) {
            for (Element* e : s->elist()) {
                  if (!e || !e->isChordRest() || !score()->staff(e->staffIdx())->show())
                        continue;
                  ChordRest* cr = toChordRest(e);

                  // layout beam
                  if (isTopBeam(cr)) {
                        cr->beam()->layout();
                        cr->beam()->addSkyline(system->staff(cr->beam()->staffIdx())->skyline());
                        }

                  // layout chord-based fingerings
                  if (e->isChord()) {
                        Chord* c = toChord(e);
                        std::list<Note*> notes;
                        for (auto gc : c->graceNotes()) {
                              for (auto n : gc->notes())
                                    notes.push_back(n);
                              }
                        for (auto n : c->notes())
                              notes.push_back(n);
                        std::list<Fingering*> fingerings;
                        for (Note* note : notes) {
                              for (Element* el : note->el()) {
                                    if (el->isFingering()) {
                                          Fingering* f = toFingering(el);
                                          if (f->layoutType() == ElementType::CHORD) {
                                                if (f->placeAbove())
                                                      fingerings.push_back(f);
                                                else
                                                      fingerings.push_front(f);
                                                }
                                          }
                                    }
                              }
                        for (Fingering* f : fingerings) {
                              f->layout();
                              if (f->autoplace() && f->visible()) {
                                    Note* n = f->note();
                                    QRectF r = f->bbox().translated(f->pos() + n->pos() + n->chord()->pos() + s->pos() + s->measure()->pos());
                                    system->staff(f->note()->chord()->vStaffIdx())->skyline().add(r);
                                    }
                              }
                        }
                  }
            }

      //-------------------------------------------------------------
      // layout articulations, tuplet
      //-------------------------------------------------------------

      for (Segment* s : sl) {
            for (Element* e : s->elist()) {
                  if (!e || !e->isChordRest() || !score()->staff(e->staffIdx())->show())
                        continue;
                  ChordRest* cr = toChordRest(e);
                  // articulations
                  if (cr->isChord())
                        toChord(cr)->layoutArticulations2();
                  // tuplets
                  // sanity check
                  if (notTopBeam(cr))
                        continue;
                  DurationElement* de = cr;
                  while (de->tuplet() && de->tuplet()->elements().front() == de) {
                        Tuplet* t = de->tuplet();
                        t->layout();
                        if (t->autoplace() && t->visible())
                              system->staff(t->staffIdx())->skyline().add(t->shape().translated(t->pos() + t->measure()->pos()));
                        de = t;
                        }
                  }
            }

      //-------------------------------------------------------------
      // layout slurs
      //-------------------------------------------------------------

      Fraction stick = system->measures().front()->tick();
      Fraction etick = system->measures().back()->endTick();
      auto spanners = score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks());

      std::vector<Spanner*> spanner;
      for (auto interval : spanners) {
            Spanner* sp = interval.value;
            sp->computeStartElement();
            sp->computeEndElement();
            lc.processedSpanners.insert(sp);
            if (sp->tick() < etick && sp->tick2() >= stick) {
                  if (sp->isSlur())
                        spanner.push_back(sp);
                  }
            }
      processLines(system, spanner, false);
      for (auto s : spanner) {
            Slur* slur = toSlur(s);
            ChordRest* scr = s->startCR();
            ChordRest* ecr = s->endCR();
            if (scr && scr->isChord())
                  toChord(scr)->layoutArticulations3(slur);
            if (ecr && ecr->isChord())
                  toChord(ecr)->layoutArticulations3(slur);
            }

      std::vector<Dynamic*> dynamics;
      for (Segment* s : sl) {
            for (Element* e : s->elist()) {
                  if (!e)
                        continue;
                  if (e->isChord()) {
                        Chord* c = toChord(e);
                        for (Chord* ch : c->graceNotes())
                              layoutTies(ch, system, stick);
                        layoutTies(c, system, stick);
                        }
                  }
            for (Element* e : s->annotations()) {
                  if (e->isDynamic()) {
                        Dynamic* d = toDynamic(e);
                        d->layout();

                        if (e->visible() && d->autoplace()) {
                              d->doAutoplace();
                              dynamics.push_back(d);
                              }
                        }
                  else if (e->isFiguredBass())
                        e->layout();
                  }
            }

      // add dynamics shape to skyline

      for (Dynamic* d : dynamics) {
            int si = d->staffIdx();
            Segment* s = d->segment();
            Measure* m = s->measure();
            system->staff(si)->skyline().add(d->shape().translated(d->pos() + s->pos() + m->pos()));
            }

      //-------------------------------------------------------------
      // layout SpannerSegments for current system
      // ottavas, pedals, voltas are collected here, but layouted later
      //-------------------------------------------------------------

      spanner.clear();
      std::vector<Spanner*> hairpins;
      std::vector<Spanner*> ottavas;
      std::vector<Spanner*> pedal;
      std::vector<Spanner*> voltas;

      for (auto interval : spanners) {
            Spanner* sp = interval.value;
            if (sp->tick() < etick && sp->tick2() > stick) {
                  if (sp->isOttava())
                        ottavas.push_back(sp);
                  else if (sp->isPedal())
                        pedal.push_back(sp);
                  else if (sp->isVolta())
                        voltas.push_back(sp);
                  else if (sp->isHairpin())
                        hairpins.push_back(sp);
                  else if (!sp->isSlur() && !sp->isVolta())    // slurs are already
                        spanner.push_back(sp);
                  }
            }
      processLines(system, hairpins, false);
      processLines(system, spanner, false);

      //-------------------------------------------------------------
      // Fermata, TremoloBar
      //-------------------------------------------------------------

      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isFermata() || e->isTremoloBar())
                        e->layout();
                  }
            }

      //-------------------------------------------------------------
      // Ottava, Pedal
      //-------------------------------------------------------------

      processLines(system, ottavas, false);
      processLines(system, pedal,   true);

      //-------------------------------------------------------------
      // Lyric
      //-------------------------------------------------------------

      layoutLyrics(system);

      // here are lyrics dashes and melisma
      for (Spanner* sp : _unmanagedSpanner) {
            if (sp->tick() >= etick || sp->tick2() <= stick)
                  continue;
            sp->layoutSystem(system);
            }

      //
      // We need to known if we have FretDiagrams in the system to decide when to layout the Harmonies
      //

      bool hasFretDiagram = false;
      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isFretDiagram()) {
                        hasFretDiagram = true;
                        break;
                        }
                  }

            if (hasFretDiagram)
                  break;
            }

      //-------------------------------------------------------------
      // Harmony, 1st place
      // If we have FretDiagrams, we want the Harmony above this and
      // above the volta, therefore we delay the layout.
      //-------------------------------------------------------------

      if (!hasFretDiagram)
            layoutHarmonies(sl);

      //-------------------------------------------------------------
      // StaffText, InstrumentChange
      //-------------------------------------------------------------

      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isStaffText() || e->isSystemText() || e->isInstrumentChange())
                        e->layout();
                  }
            }

      //-------------------------------------------------------------
      // Jump, Marker
      //-------------------------------------------------------------

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            Measure* m = toMeasure(mb);
            for (Element* e : m->el()) {
                  if (e->isJump() || e->isMarker())
                        e->layout();
                  }
            }

      //-------------------------------------------------------------
      // TempoText
      //-------------------------------------------------------------

      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isTempoText())
                        e->layout();
                  }
            }

      //-------------------------------------------------------------
      // layout Voltas for current sytem
      //-------------------------------------------------------------

      processLines(system, voltas, false);

      //
      // vertical align volta segments
      //
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            std::vector<SpannerSegment*> voltaSegments;
            for (SpannerSegment* ss : system->spannerSegments()) {
                  if (ss->isVoltaSegment() && ss->staffIdx() == staffIdx)
                        voltaSegments.push_back(ss);
                  }
            while (!voltaSegments.empty()) {
                  // we assume voltas are sorted left to right (by tick values)
                  qreal y = 0;
                  int idx = 0;
                  Volta* prevVolta = 0;
                  for (SpannerSegment* ss : voltaSegments) {
                        Volta* volta = toVolta(ss->spanner());
                        if (prevVolta && prevVolta != volta) {
                              // check if volta is adjacent to prevVolta
                              if (prevVolta->tick2() != volta->tick())
                                    break;
                              }
                        y = qMin(y, ss->rypos());
                        ++idx;
                        prevVolta = volta;
                        }

                  for (int i = 0; i < idx; ++i) {
                        SpannerSegment* ss = voltaSegments[i];
                        ss->rypos() = y;
                        system->staff(staffIdx)->skyline().add(ss->shape().translated(ss->pos()));
                        }

                  voltaSegments.erase(voltaSegments.begin(), voltaSegments.begin() + idx);
                  }
            }

      //-------------------------------------------------------------
      // FretDiagram
      //-------------------------------------------------------------

      if (hasFretDiagram) {
            for (const Segment* s : sl) {
                  for (Element* e : s->annotations()) {
                        if (e->isFretDiagram())
                              e->layout();
                        }
                  }

            //-------------------------------------------------------------
            // Harmony, 2nd place
            // We have FretDiagrams, we want the Harmony above this and
            // above the volta.
            //-------------------------------------------------------------

            layoutHarmonies(sl);
            }

      //-------------------------------------------------------------
      // RehearsalMark
      //-------------------------------------------------------------

      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isRehearsalMark())
                        e->layout();
                  }
            }
      }

//---------------------------------------------------------
//   collectPage
//---------------------------------------------------------

void LayoutContext::collectPage()
      {
      const qreal slb = score->styleP(Sid::staffLowerBorder);
      bool breakPages = score->layoutMode() != LayoutMode::SYSTEM;
      qreal y         = prevSystem ? prevSystem->y() + prevSystem->height() : page->tm();
      qreal ey        = page->height() - page->bm();

      System* nextSystem = 0;
      int systemIdx = -1;

      for (int k = 0;;++k) {
            //
            // calculate distance to previous system
            //
            qreal distance;
            if (prevSystem)
                  distance = prevSystem->minDistance(curSystem);
            else {
                  // this is the first system on page
                  if (curSystem->vbox())
                        distance = 0.0;
                  else {
                        distance = score->styleP(Sid::staffUpperBorder);
                        bool fixedDistance = false;
                        for (MeasureBase* mb : curSystem->measures()) {
                              if (mb->isMeasure()) {
                                    Measure* m = toMeasure(mb);
                                    Spacer* sp = m->vspacerUp(0);
                                    if (sp) {
                                          if (sp->spacerType() == SpacerType::FIXED) {
                                                distance = sp->gap();
                                                fixedDistance = true;
                                                break;
                                                }
                                          else
                                                distance = qMax(distance, sp->gap());
                                          }
//TODO::ws                                    distance = qMax(distance, -m->staffShape(0).top());
                                    }
                              }
                        if (!fixedDistance)
                              distance = qMax(distance, -curSystem->minTop());
                        }
                  }
//TODO-ws ??
//          distance += score->staves().front()->userDist();

            y += distance;
            curSystem->setPos(page->lm(), y);
            page->appendSystem(curSystem);
            y += curSystem->height();

            //
            //  check for page break or if next system will fit on page
            //
            const bool rangeWasDone = rangeDone;
            if (rangeDone) {
                  // take next system unchanged
                  if (systemIdx > 0) {
                        nextSystem = score->systems().value(systemIdx++);
                        if (!nextSystem) {
                              // TODO: handle next movement
                              }
                        }
                  else {
                        nextSystem = systemList.empty() ? 0 : systemList.takeFirst();
                        if (nextSystem)
                              score->systems().append(nextSystem);
                        else if (score->isMaster()) {
                              MasterScore* ms = static_cast<MasterScore*>(score)->next();
                              if (ms) {
                                    score     = ms;
                                    systemIdx = 0;
                                    nextSystem = score->systems().value(systemIdx++);
                                    }
                              }
                        }
                  }
            else {
                  nextSystem = score->collectSystem(*this);
                  if (!nextSystem && score->isMaster()) {
                        MasterScore* ms = static_cast<MasterScore*>(score)->next();
                        if (ms) {
                              score = ms;
                              QList<System*>& systems = ms->systems();
                              if (systems.empty() || systems.front()->measures().empty()) {
                                    systemList         = systems;
                                    systems.clear();
                                    measureNo          = 0;
                                    startWithLongNames = true;
                                    firstSystem        = true;
                                    tick               = Fraction(0,1);
                                    prevMeasure        = 0;
                                    curMeasure         = 0;
                                    nextMeasure        = ms->measures()->first();
                                    ms->getNextMeasure(*this);
                                    nextSystem         = ms->collectSystem(*this);
                                    ms->setScoreFont(ScoreFont::fontFactory(ms->styleSt(Sid::MusicalSymbolFont)));
                                    ms->setNoteHeadWidth(ms->scoreFont()->width(SymId::noteheadBlack, ms->spatium() / SPATIUM20));
                                    }
                              else {
                                    rangeDone = true;
                                    systemIdx = 0;
                                    nextSystem = score->systems().value(systemIdx++);
                                    }
                              }
                        }
                  }
            prevSystem = curSystem;
            Q_ASSERT(curSystem != nextSystem);
            curSystem  = nextSystem;

            bool breakPage = !curSystem || (breakPages && prevSystem->pageBreak());

            if (!breakPage) {
                  qreal dist = prevSystem->minDistance(curSystem) + curSystem->height();
                  Box* vbox = curSystem->vbox();
                  if (vbox)
                        dist += vbox->bottomGap();
                  else if (!prevSystem->hasFixedDownDistance())
                        dist += qMax(curSystem->minBottom(), slb);

                  breakPage  = (y + dist) >= ey && breakPages;
                  }
            if (breakPage) {
                  qreal dist = qMax(prevSystem->minBottom(), slb);
                  layoutPage(page, ey - (y + dist));
                  // We don't accept current system to this page
                  // so rollback rangeDone variable as well.
                  rangeDone = rangeWasDone;
                  break;
                  }
            }

      qreal height = 0;
      Fraction stick = Fraction(-1,1);
      for (System* s : page->systems()) {
            Score* currentScore = s->score();
            height += s->rypos();
            for (MeasureBase* mb : s->measures()) {
                  if (!mb->isMeasure())
                        continue;
                  Measure* m = toMeasure(mb);
                  if (stick == Fraction(-1,1))
                        stick = m->tick();

                  for (int track = 0; track < currentScore->ntracks(); ++track) {
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              Element* e = segment->element(track);
                              if (!e)
                                    continue;
                              if (e->isChordRest()) {
                                    if (!currentScore->staff(track2staff(track))->show())
                                          continue;
                                    ChordRest* cr = toChordRest(e);
                                    if (notTopBeam(cr)) {                   // layout cross staff beams
                                          cr->beam()->layout();

                                          // fix layout of tuplets
                                          DurationElement* de = cr;
                                          while (de->tuplet() && de->tuplet()->elements().front() == de) {
                                                Tuplet* t = de->tuplet();
                                                t->layout();
                                                m->system()->staff(t->staffIdx())->skyline().add(t->shape().translated(t->measure()->pos()));
                                                de = de->tuplet();
                                                }
                                          }

                                    if (cr->isChord()) {
                                          Chord* c = toChord(cr);
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
                                                for (Element* element : cc->el()) {
                                                      if (element->isSlur())
                                                            element->layout();
                                                      }
                                                }
                                          c->layoutArpeggio2();
                                          for (Note* n : c->notes()) {
                                                Tie* tie = n->tieFor();
                                                if (tie)
                                                      tie->layout();
                                                for (Spanner* sp : n->spannerFor())
                                                      sp->layout();
                                                }
                                          }
                                    }
                              else if (e->isBarLine())
                                    toBarLine(e)->layout2();
                              }
                        }
                  m->layout2();
                  }
            }

      if (score->systemMode())
            page->bbox().setRect(0.0, 0.0, score->loWidth(), height);

      page->rebuildBspTree();
      }

//---------------------------------------------------------
//   doLayout
//    do a complete (re-) layout
//---------------------------------------------------------

void Score::doLayout()
      {
      doLayoutRange(Fraction(0,1), Fraction(-1,1));
      }

//---------------------------------------------------------
//   doLayoutRange
//---------------------------------------------------------

void Score::doLayoutRange(const Fraction& st, const Fraction& et)
      {
      Fraction stick(st);
      Fraction etick(et);
      Q_ASSERT(!(stick == Fraction(-1,1) && etick == Fraction(-1,1)));

      if (!last() || (lineMode() && !firstMeasure())) {
            qDebug("empty score");
            qDeleteAll(_systems);
            _systems.clear();
            qDeleteAll(pages());
            pages().clear();
            return;
            }
//      if (!_systems.isEmpty())
//            return;
      bool layoutAll = stick <= Fraction(0,1) && (etick < Fraction(0,1) || etick >= last()->endTick());
      if (stick < Fraction(0,1))
            stick = Fraction(0,1);
      if (etick < Fraction(0,1))
            etick = last()->endTick();

      LayoutContext lc;
      lc.endTick     = etick;
      _scoreFont     = ScoreFont::fontFactory(style().value(Sid::MusicalSymbolFont).toString());
      _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, spatium() / SPATIUM20);

      if (cmdState().layoutFlags & LayoutFlag::FIX_PITCH_VELO)
            updateVelo();
#if 0 // TODO: needed? It was introduced in ab9774ec4098512068b8ef708167d9aa6e702c50
      if (cmdState().layoutFlags & LayoutFlag::PLAY_EVENTS)
            createPlayEvents();
#endif

      //---------------------------------------------------
      //    initialize layout context lc
      //---------------------------------------------------

      MeasureBase* m = tick2measure(stick);
      if (m == 0)
            m = first();
      // start layout one measure earlier to handle clefs and cautionary elements
      if (m->prevMeasureMM())
            m = m->prevMeasureMM();
      else if (m->prev())
            m = m->prev();
      while (!m->isMeasure() && m->prev())
            m = m->prev();

      // if the first measure of the score is part of a multi measure rest
      // m->system() will return a nullptr. We need to find the multi measure
      // rest which replaces the measure range

      if (!m->system() && m->isMeasure() && toMeasure(m)->hasMMRest()) {
            qDebug("  don’t start with mmrest");
            m = toMeasure(m)->mmRest();
            }

//      qDebug("start <%s> tick %d, system %p", m->name(), m->tick(), m->system());
      lc.score        = m->score();

      if (lineMode()) {
            lc.prevMeasure = 0;
            lc.nextMeasure = _showVBox ? first() : firstMeasure();
            layoutLinear(layoutAll, lc);
            return;
            }
      if (!layoutAll && m->system()) {
            System* system  = m->system();
            int systemIndex = _systems.indexOf(system);
            lc.page         = system->page();
            lc.curPage      = pageIdx(lc.page);
            if (lc.curPage == -1)
                  lc.curPage = 0;
            lc.curSystem   = system;
            lc.systemList  = _systems.mid(systemIndex);

            if (systemIndex == 0)
                  lc.nextMeasure = _showVBox ? first() : firstMeasure();
            else {
                  System* prevSystem = _systems[systemIndex-1];
                  lc.nextMeasure = prevSystem->measures().back()->next();
                  }

            _systems.erase(_systems.begin() + systemIndex, _systems.end());
            if (!lc.nextMeasure->prevMeasure()) {
                  lc.measureNo = 0;
                  lc.tick      = Fraction(0,1);
                  }
            else {
                  if (lc.nextMeasure->prevMeasure()->sectionBreak())
                        lc.measureNo = 0;
                  else
                        lc.measureNo = lc.nextMeasure->prevMeasure()->no() + 1; // will be adjusted later with respect
                                                                                // to the user-defined offset.
                  lc.tick      = lc.nextMeasure->tick();
                  }
            }
      else {
//  qDebug("layoutAll, systems %p %d", &_systems, int(_systems.size()));
            //lc.measureNo   = 0;
            //lc.tick        = 0;
            // qDeleteAll(_systems);
            // _systems.clear();
                  // lc.systemList  = _systems;
                  // _systems.clear();

            for (System* s : _systems) {
                  for (Bracket* b : s->brackets()) {
                        if (b->selected()) {
                              _selection.remove(b);
                              setSelectionChanged(true);
                              }
                        }
//                  for (SpannerSegment* ss : s->spannerSegments())
//                        ss->setParent(0);
                  s->setParent(nullptr);
                  }
            for (MeasureBase* mb = first(); mb; mb = mb->next()) {
                  mb->setSystem(0);
                  if (mb->isMeasure() && toMeasure(mb)->mmRest())
                        toMeasure(mb)->mmRest()->setSystem(0);
                  }
            qDeleteAll(_systems);
            _systems.clear();

            qDeleteAll(pages());
            pages().clear();

            lc.nextMeasure = _showVBox ? first() : firstMeasure();
            }

      lc.prevMeasure = 0;

      getNextMeasure(lc);
      lc.curSystem = collectSystem(lc);

      lc.layout();

      for (MuseScoreView* v : viewer)
            v->layoutChanged();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LayoutContext::layout()
      {
      do {
            getNextPage();
            collectPage();
            } while (curSystem && !(rangeDone && page->system(0)->measures().back()->tick() > endTick)); // FIXME: perhaps the first measure was meant? Or last system?
      if (!curSystem) {
            // The end of the score. The remaining systems are not needed...
            qDeleteAll(systemList);
            systemList.clear();
            // ...and the remaining pages too
            while (score->npages() > curPage)
                  delete score->pages().takeLast();
            }
      else {
            Page* p = curSystem->page();
            if (p && (p != page))
                  p->rebuildBspTree();
            }
      score->systems().append(systemList);     // TODO
      }

//---------------------------------------------------------
//   LayoutContext::~LayoutContext
//---------------------------------------------------------

LayoutContext::~LayoutContext()
      {
      for (Spanner* s : processedSpanners)
            s->layoutSystemsDone();
      }
}

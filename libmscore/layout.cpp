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
#include "arpeggio.h"
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
#include "mmrestrange.h"
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
#include "stemslash.h"
#include "sticking.h"
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

#if 0
//---------------------------------------------------------
//   vUp
//    reurns true if chord should be treated as up
//    for purpose of setting horizontal position
//    for most chords, this is just chord->up()
//    but for notes on cross-staff beams, we take care to produce more consistent results
//    since the initial guess for up() may change during layout
//---------------------------------------------------------
static bool vUp(Chord* chord)
      {
      if (!chord)
            return true;
      else if (!chord->beam() || !chord->beam()->cross()) {
            return chord->up();
            }
      else {
            // cross-staff beam: we cannot know the actual direction of this chord until the beam layout,
            // but that's too late - it won't work to lay out as if the chord is up on pass one but then down on pass two
            // so just assign a logical direction based on attributes that won't change
            // so chords can be laid out consistently on both passes
            bool up;
            if (chord->stemDirection() != Direction::AUTO)
                  up = chord->stemDirection() == Direction::UP;
            else if (chord->staffMove())
                  up = chord->staffMove() > 0;
            else if (chord->track() < chord->beam()->track())
                  up = false;
            else if (chord->track() > chord->beam()->track())
                  up = true;
            else if (chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks()))
                  up = !(chord->track() % 2);
            else
                  up = !chord->staff()->isTop();
            return up;
            }
      }
#endif

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
      const Fraction tick = segment->tick();

      if (staff->isTabStaff(tick)) {
            layoutSegmentElements(segment, startTrack, endTrack);
            return;
            }

      bool crossBeamFound = false;
      std::vector<Note*> upStemNotes;
      std::vector<Note*> downStemNotes;
      int upVoices       = 0;
      int downVoices     = 0;
      qreal nominalWidth = noteHeadWidth() * staff->mag(tick);
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
                  if (chord->beam() && chord->beam()->cross())
                        crossBeamFound = true;
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
                  std::sort(upStemNotes.begin(), upStemNotes.end(),
                     [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );
                  }
            if (upVoices) {
                  qreal hw = layoutChords2(upStemNotes, true);
                  maxUpWidth = qMax(maxUpWidth, hw);
                  }

            // layout downstem noteheads
            if (downVoices > 1) {
                  std::sort(downStemNotes.begin(), downStemNotes.end(),
                     [](Note* n1, const Note* n2) ->bool {return n1->line() > n2->line(); } );
                  }
            if (downVoices) {
                  qreal hw = layoutChords2(downStemNotes, false);
                  maxDownWidth = qMax(maxDownWidth, hw);
                  }

            qreal sp                 = staff->spatium(tick);
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
            qreal headDiff2 = maxUpWidth - nominalWidth * (maxUpMag / staff->mag(tick));
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
                  // TODO: handle conflicts for cross-staff notes and notes on cross-staff beams
                  // for now we simply treat these as though there is no conflict
                  if (bottomUpNote->chord()->staffMove() == topDownNote->chord()->staffMove() && !crossBeamFound)
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
                        for (size_t i = downStemNotes.size(); i > 0; --i) { // loop most probably needs to be in this reverse order
                              if (downStemNotes[i-1]->line() <= bottomUpNote->line() + 1)
                                    overlapNotes.append(downStemNotes[i-1]);
                              else
                                    break;
                              }
                        std::sort(overlapNotes.begin(), overlapNotes.end(),
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
                                          if (n->headGroup() != p->headGroup() || n->tpc() != p->tpc() || n->mirror() || p->mirror() || nchord->isSmall() != pchord->isSmall()) {
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
                                                if ((nchord->dots() != pchord->dots() || !nchord->stem() || !pchord->stem() || nHeadType != pHeadType || n->isSmall() || p->isSmall()) &&
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
                  std::sort(notes.begin(), notes.end(),
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

static QPair<qreal, qreal> layoutAccidental(AcEl* me, AcEl* above, AcEl* below, qreal colOffset, QVector<Note*>& leftNotes, qreal pnd, qreal pd, qreal sp)
      {
      qreal lx = colOffset;
      Accidental* acc = me->note->accidental();
      qreal mag = acc->mag();
      pnd *= mag;
      pd *= mag;

      Chord* chord = me->note->chord();
      Staff* staff = chord->staff();
      Fraction tick = chord->tick();

      // extra space for ledger lines
      qreal ledgerAdjust = 0.0;
      qreal ledgerVerticalClear = 0.0;
      bool ledgerAbove = chord->upNote()->line() <= -2;
      bool ledgerBelow = chord->downNote()->line() >= staff->lines(tick) * 2;
      if (ledgerAbove || ledgerBelow) {
            // ledger lines are present
            // check for collision with lines above & below staff
            // note that on 1-line staff, both collisions are possible at once
            // TODO: account for cutouts in accidental
            qreal lds = staff->lineDistance(tick) * sp;
            if ((ledgerAbove && me->top + lds <= pnd) || (ledgerBelow && staff->lines(tick) * lds - me->bottom <= pnd)) {
                  ledgerAdjust = -acc->score()->styleS(Sid::ledgerLineLength).val() * sp;
                  ledgerVerticalClear = acc->score()->styleS(Sid::ledgerLineWidth).val() * 0.5 * sp;
                  lx = qMin(lx, ledgerAdjust);
                  }
            }

      // clear left notes
      int lns = leftNotes.size();
      for (int i = 0; i < lns; ++i) {
            Note* ln = leftNotes[i];
            int lnLine = ln->line();
            qreal lnTop = (lnLine - 1) * 0.5 * sp;
            qreal lnBottom = lnTop + sp;
            if (me->top - lnBottom <= pnd && lnTop - me->bottom <= pnd) {
                  qreal lnLedgerAdjust = 0.0;
                  if (lnLine <= -2 || lnLine >= staff->lines(tick) * 2) {
                        // left note has a ledger line we probably need to clear horizontally as well
                        // except for accidentals that clear the last extended ledger line vertically
                        // in these cases, the accidental may tuck closer
                        Note* lastLnNote = lnLine < 0  ? leftNotes[0] : leftNotes[lns - 1];
                        int lastLnLine = lastLnNote->line();
                        qreal ledgerY = (lastLnLine / 2) * sp;
                        if (me->line < 0 && ledgerY - me->bottom < ledgerVerticalClear)
                              lnLedgerAdjust = ledgerAdjust;
                        else if (me->line > 0 && me->top - ledgerY < ledgerVerticalClear)
                              lnLedgerAdjust = ledgerAdjust;
                        }
                  // undercut note above if possible
                  if (lnBottom - me->top <= me->ascent - pnd)
                        lx = qMin(lx, ln->x() + ln->chord()->x() + lnLedgerAdjust + me->rightClear);
                  else
                        lx = qMin(lx, ln->x() + ln->chord()->x() + lnLedgerAdjust);
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

      return QPair<qreal, qreal> (me->x, me->x + me->width);
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
                  if (!ac->visible()) {
                        ac->setPos(ac->bbox().x() - ac->width(), 0.0);
                        }
                  else {
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
                  }

            Chord* chord = note->chord();
            bool _up     = chord->up();

            if (chord->stemSlash())
                  chord->stemSlash()->layout();

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
                  std::sort(umi.begin(), umi.end());

            bool alignLeft = score()->styleB(Sid::alignAccidentalsLeft);

            // through columns
            for (int i = 0; i < nColumns; ++i) {
                  // column index
                  const int pc = column[i];

                  qreal minX = 0.0;
                  qreal maxX = 0.0;

                  AcEl* below = 0;
                  // through accidentals in this column
                  for (int j = columnBottom[pc]; j != -1; j = aclist[j].next) {
                        QPair<qreal, qreal> x = layoutAccidental(&aclist[j], 0, below, colOffset, leftNotes, pnd, pd, sp);
                        minX = qMin(minX, x.first);
                        maxX = qMin(maxX, x.second);
                        below = &aclist[j];
                        }

                  // align
                  int next = -1;
                  for (int j = columnBottom[pc]; j != -1; j = next) {
                        AcEl* current = &aclist[j];
                        next = current->next;
                        if (next != -1 && current->line == aclist[next].line)
                                continue;

                        if (alignLeft)
                                current->x = minX;
                        else
                              current->x = maxX - current->width;
                        }
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

      for (const AcEl& e : qAsConst(aclist)) {
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

      for (ChordRest* cr : qAsConst(graceNotes)) {
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
                        cr->replaceBeam(beam);
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
                              a1->replaceBeam(beam);
                              }
                        cr->replaceBeam(beam);
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

      Fraction stick = system->measures().front()->tick();
      Fraction etick = system->measures().back()->endTick();
      auto spanners = score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks() - 1);

      for (Staff* staff : qAsConst(_staves)) {
            SysStaff* ss  = system->staff(staffIdx);

            Staff::HideMode hideMode = staff->hideWhenEmpty();

            if (hideMode == Staff::HideMode::ALWAYS
                || (styleB(Sid::hideEmptyStaves)
                    && (staves > 1)
                    && !(isFirstSystem && styleB(Sid::dontHideStavesInFirstSystem))
                    && hideMode != Staff::HideMode::NEVER)) {
                  bool hideStaff = true;
                  for (auto spanner : spanners) {
                        if (spanner.value->staff() == staff
                         && !spanner.value->systemFlag()) {
                              hideStaff = false;
                              break;
                              }
                        }
                  for (MeasureBase* m : system->measures()) {
                        if (!m->isMeasure())
                              continue;
                        Measure* measure = toMeasure(m);
                        if (!measure->isEmpty(staffIdx)) {
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
                                    if (staff->hideWhenEmpty() == Staff::HideMode::INSTRUMENT && !m->isEmpty(st)) {
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
            else if (!staff->show()) {
                  // TODO: OK to check this first and not bother with checking if empty?
                  ss->setShow(false);
                  }
            else {
                  systemIsEmpty = false;
                  ss->setShow(true);
                  }

            ++staffIdx;
            }
      Staff* firstVisible = nullptr;
      if (systemIsEmpty) {
            for (Staff* staff : qAsConst(_staves)) {
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
                  for (Chord* gc : c->graceNotes()) {
                        for (Note* n : gc->notes()) {
                              // spanner with no end element apparently happens when reading some 206 files
                              // (and possibly in other situations too)
                              for (Spanner* spanner : n->spannerFor()) {
                                   if (spanner->endElement() == nullptr) {
                                         n->removeSpannerFor(spanner);
                                         delete spanner;
                                         }
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   connectArpeggios
//  Fake cross-voice arpeggios by hiding all but the first
//  and extending the first to cover the others.
//  Retains the other properties of the first arpeggio.
//---------------------------------------------------------

void Score::connectArpeggios()
      {
      for (auto segment = firstSegment(SegmentType::ChordRest); segment; segment = segment->next1(SegmentType::ChordRest)) {
            for (int staff = 0; staff < nstaves(); ++staff) {
                  qreal minTop = 10000;
                  qreal maxBottom = -10000;
                  int firstArpeggio = -1;
                  bool multipleArpeggios = false;
                  for (int i = staff2track(staff); i < staff2track(staff + 1); ++i) {
                        if (segment->elist()[i] && segment->elist()[i]->isChord()) {
                              Chord* chord = toChord(segment->elist()[i]);
                              if (chord->arpeggio() && chord->arpeggio()->visible()) {
                                    if (chord->pagePos() == QPointF(0, 0)) doLayout();
                                    qreal localTop = chord->arpeggio()->pageBoundingRect().top();                   
                                    qreal localBottom = chord->arpeggio()->pageBoundingRect().bottom();                             
                                    minTop = qMin(localTop, minTop);
                                    maxBottom = qMax(localBottom, maxBottom);
                                    if (firstArpeggio == -1)
                                          // Leave arpeggio, adjust height after collecting
                                          firstArpeggio = i;
                                    else {
                                          // Hide arpeggio; firstArpeggio will be extended to cover it.
                                          chord->arpeggio()->setVisible(false);                                          
                                          multipleArpeggios = true;
                                          }
                                    }
                              }
                        }
                  if (firstArpeggio != -1 && multipleArpeggios) {
                        // Stretch first arpeggio to cover deleted
                        Chord* firstArpeggioChord = toChord(segment->elist()[firstArpeggio]);
                        Arpeggio* arpeggio = firstArpeggioChord->arpeggio();
                        qreal topDiff = minTop - arpeggio->pageBoundingRect().top();
                        qreal bottomDiff = maxBottom - arpeggio->pageBoundingRect().bottom();
                        arpeggio->setUserLen1(topDiff);
                        arpeggio->setUserLen2(bottomDiff);
                        arpeggio->setPropertyFlags(Pid::ARP_USER_LEN1, PropertyFlags::UNSTYLED);
                        arpeggio->setPropertyFlags(Pid::ARP_USER_LEN2, PropertyFlags::UNSTYLED);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   fixupLaissezVibrer
//    This is a temporary hack to improve the placement of
//    l.v. articulations when importing MusciXML.
//    TODO: vastly improve the automatic placement of the
//    l.v. articulation.
//---------------------------------------------------------

void Score::fixupLaissezVibrer()
      {
      int tracks = nstaves() * VOICES;
      Measure* m = firstMeasure();
      if (!m)
            return;
      if (m->canvasPos() == QPointF(0, 0))
            doLayout();

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = m->first(st); s; s = s->next1(st)) {
            for (int i = 0; i < tracks; ++i) {
                  Element* e = s->element(i);
                  if (e == 0 || !e->isChord())
                        continue;
                  Chord* c = toChord(e);
                  for (auto a : c->articulations()) {
                        if (a->symId() != SymId::articLaissezVibrerAbove && a->symId() != SymId::articLaissezVibrerBelow)
                              continue;

                        // Manually override placement
                        a->setAutoplace(false);
                        a->setMinDistance(Spatium(0));
                        c->layoutArticulations();
                        c->layoutArticulations2();
                        bool below = a->symId() == SymId::articLaissezVibrerBelow;
                        Note* n = below ? c->notes().front() : c->notes().back();

                        QPointF target = below  ? n->canvasBoundingRect().bottomLeft() + QPointF(0.5 * n->width(), 0.25 * spatium())
                                                : n->canvasBoundingRect().topLeft() + QPointF(0.5 * n->width(), -0.25 * spatium());
                        QPointF current = below ? a->canvasBoundingRect().topLeft() : a->canvasBoundingRect().bottomLeft();
                        a->setOffset(a->offset() + target - current);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   checkDivider
//---------------------------------------------------------

void LayoutContext::checkDivider(bool left, System* s, qreal yOffset, bool remove)
      {
      SystemDivider* divider = left ? s->systemDividerLeft() : s->systemDividerRight();
      if ((s->score()->styleB(left ? Sid::dividerLeft : Sid::dividerRight)) && !remove) {
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
//   almostZero
//---------------------------------------------------------

bool inline almostZero(qreal value)
      {
      // 1e-3 is close enough to zero to see it as zero.
      return value > -1e-3 && value < 1e-3;
      }

//---------------------------------------------------------
//   distributeStaves
//---------------------------------------------------------

void LayoutContext::distributeStaves(Page* page, qreal footerPadding)
      {
      Score* score { page->score() };
      VerticalGapDataList vgdl;

      // Find and classify all gaps between staves.
      int    ngaps { 0 };
      qreal  prevYBottom  { page->tm() };
      qreal  yBottom      { 0.0        };
      qreal  spacerOffset { 0.0        };
      bool   vbox         { false      };
      Spacer* nextSpacer  { nullptr    };
      bool transferNormalBracket { false };
      bool transferCurlyBracket  { false };
      for (System* system : page->systems()) {
            if (system->vbox()) {
                  VerticalGapData* vgd = new VerticalGapData(!ngaps++, system, nullptr, nullptr, nullptr, prevYBottom);
                  vgd->addSpaceAroundVBox(true);
                  prevYBottom = system->y();
                  yBottom     = system->y() + system->height();
                  vbox        = true;
                  vgdl.append(vgd);
                  transferNormalBracket = false;
                  transferCurlyBracket  = false;
                  }
            else {
                  bool newSystem       { true };
                  bool addSpaceAroundNormalBracket { false };
                  bool addSpaceAroundCurlyBracket  { false };
                  int endNormalBracket { -1   };
                  int endCurlyBracket  { -1   };
                  int staffNr { -1 };
                  for (SysStaff* sysStaff : *system->staves()) {
                        Staff* staff { score->staff(++staffNr)};
                        addSpaceAroundNormalBracket |= endNormalBracket == staffNr;
                        addSpaceAroundCurlyBracket  |= endCurlyBracket == staffNr;
                        for (const BracketItem* bi : staff->brackets()) {
                              if  (bi->bracketType() == BracketType::NORMAL) {
                                    addSpaceAroundNormalBracket |= staff->idx() > (endNormalBracket - 1);
                                    endNormalBracket = qMax(endNormalBracket, staff->idx() + bi->bracketSpan());
                                    }
                              else if(bi->bracketType() == BracketType::BRACE) {
                                    addSpaceAroundCurlyBracket |= staff->idx() > (endCurlyBracket - 1);
                                    endCurlyBracket = qMax(endCurlyBracket, staff->idx() + bi->bracketSpan());
                                    }
                              }

                        if (!sysStaff->show())
                              continue;

                        VerticalGapData* vgd = new VerticalGapData(!ngaps++, system, staff, sysStaff, nextSpacer, prevYBottom);
                        nextSpacer = system->downSpacer(staff->idx());

                        if (newSystem) {
                              vgd->addSpaceBetweenSections();
                              newSystem = false;
                              }
                        if (addSpaceAroundNormalBracket || transferNormalBracket) {
                              vgd->addSpaceAroundNormalBracket();
                              addSpaceAroundNormalBracket = false;
                              transferNormalBracket = false;
                              }
                        if (addSpaceAroundCurlyBracket || transferCurlyBracket) {
                              vgd->addSpaceAroundCurlyBracket();
                              addSpaceAroundCurlyBracket = false;
                              transferCurlyBracket = false;
                              }
                        else if (staffNr < endCurlyBracket) {
                              vgd->insideCurlyBracket();
                              }

                        if (vbox) {
                              vgd->addSpaceAroundVBox(false);
                              vbox = false;
                              }

                        prevYBottom  = system->y() + sysStaff->y() + sysStaff->bbox().height();
                        yBottom      = system->y() + sysStaff->y() + sysStaff->skyline().south().max();
                        spacerOffset = sysStaff->skyline().south().max() - sysStaff->bbox().height();
                        vgdl.append(vgd);
                        }
                  transferNormalBracket = endNormalBracket >= 0;
                  transferCurlyBracket  = endCurlyBracket >= 0;
                  }
            }
      --ngaps;

      qreal spaceRemaining { page->height() - page->bm() - footerPadding - score->styleP(Sid::staffLowerBorder) - yBottom };
      if (nextSpacer)
            spaceRemaining -= qMax(0.0, nextSpacer->gap() - spacerOffset - score->styleP(Sid::staffLowerBorder));
      if (spaceRemaining <= 0.0)
            return;

      // Try to make the gaps equal, taking the spread factors and maximum spacing into account.
      static const int maxPasses { 20 };   // Saveguard to prevent endless loops.
      int pass { 0 };
      while (!almostZero(spaceRemaining) && (ngaps > 0) && (++pass < maxPasses)) {
            ngaps = 0;
            qreal smallest     { vgdl.smallest()         };
            qreal nextSmallest { vgdl.smallest(smallest) };
            if (almostZero(smallest) || almostZero(nextSmallest))
                  break;

            if ((nextSmallest - smallest) * vgdl.sumStretchFactor() > spaceRemaining)
                  nextSmallest = smallest + spaceRemaining / vgdl.sumStretchFactor();

            qreal addedSpace { 0.0 };
            VerticalGapDataList modified;
            for (VerticalGapData* vgd : vgdl) {
                  if (!almostZero(vgd->spacing() - smallest))
                        continue;
                  qreal step { nextSmallest - vgd->spacing() };
                  if (step < 0.0)
                        continue;
                  step = vgd->addSpacing(step);
                  if (!almostZero(step)) {
                        addedSpace += step * vgd->factor();
                        modified.append(vgd);
                        ++ngaps;
                        }
                  if ((spaceRemaining - addedSpace) <= 0.0)
                        break;
                  }
            if ((spaceRemaining - addedSpace) <= 0.0)
                  {
                  for (VerticalGapData* vgd : modified) {
                        vgd->undoLastAddSpacing();
                        }
                  ngaps = 0;
                  }
            else {
                  spaceRemaining -= addedSpace;
                  }
            }

      // If there is still space left, distribute the space of the staves.
      // However, there is a limit on how much space is added per gap.
      const qreal maxPageFill { score->styleP(Sid::maxPageFillSpread) };
      spaceRemaining = qMin(maxPageFill * vgdl.length(), spaceRemaining);
      pass = 0;
      ngaps = 1;
      while (!almostZero(spaceRemaining) && !almostZero(maxPageFill) && (ngaps > 0) && (++pass < maxPasses)) {
            ngaps = 0;
            qreal addedSpace { 0.0 };
            qreal step {spaceRemaining / vgdl.sumStretchFactor() };
            for (VerticalGapData* vgd : vgdl) {
                  qreal res { vgd->addFillSpacing(step, maxPageFill) };
                  if (!almostZero(res)) {
                        addedSpace += res * vgd->factor();
                        ++ngaps;
                        }
                  }
            spaceRemaining -= addedSpace;
            }

      QSet<System*> systems;
      qreal systemShift { 0.0 };
      qreal staffShift  { 0.0 };
      System* prvSystem { nullptr };
      for (VerticalGapData* vgd : vgdl) {
            if (vgd->sysStaff)
                  systems.insert(vgd->system);
            systemShift += vgd->actualAddedSpace();
            if (prvSystem == vgd->system) {
                  staffShift += vgd->actualAddedSpace();
                  }
            else {
                  vgd->system->rypos() += systemShift;
                  if (prvSystem) {
                        prvSystem->setDistance(vgd->system->y() - prvSystem->y());
                        prvSystem->setHeight(prvSystem->height() + staffShift);
                        }
                  staffShift = 0.0;
                  }

            if (vgd->sysStaff)
                  vgd->sysStaff->bbox().translate(0.0, staffShift);

            prvSystem = vgd->system;
            }
      if (prvSystem) {
            prvSystem->setHeight(prvSystem->height() + staffShift);
            }

      for (System* system : systems) {
            system->setMeasureHeight(system->height());
            system->layoutBracketsVertical();
            system->layoutInstrumentNames();
            }
      vgdl.deleteAll();
      }

//---------------------------------------------------------
//   layoutPage
//    restHeight - vertical space which has to be distributed
//                 between systems
//    The algorithm tries to produce most equally spaced
//    systems.
//---------------------------------------------------------

void LayoutContext::layoutPage(Page* page, qreal restHeight, qreal footerPadding)
      {
      if (restHeight < 0.0) {
            qDebug("restHeight < 0.0: %f\n", restHeight);
            restHeight = 0;
            }

      Score* score = page->score();
      int gaps     = page->systems().size() - 1;

      QList<System*> sList;

      // build list of systems (excluding last)
      // set initial distance for each to the unstretched minimum distance to next
      for (int i = 0; i < gaps; ++i) {
            System* s1 = page->systems().at(i);
            System* s2 = page->systems().at(i+1);
            s1->setDistance(s2->y() - s1->y());
            if (s1->vbox() || s2->vbox() || s1->hasFixedDownDistance()) {
                  if (s2->vbox()) {
                        checkDivider(true, s1, 0.0, true);      // remove
                        checkDivider(false, s1, 0.0, true);     // remove
                        checkDivider(true, s2, 0.0, true);      // remove
                        checkDivider(false, s2, 0.0, true);     // remove
                        }
                  continue;
                  }
            sList.push_back(s1);
            }

      // last system needs no divider
      System* lastSystem = page->systems().back();
      checkDivider(true, lastSystem, 0.0, true);      // remove
      checkDivider(false, lastSystem, 0.0, true);     // remove

      if (sList.empty() || MScore::noVerticalStretch || score->enableVerticalSpread() || score->layoutMode() == LayoutMode::SYSTEM) {
            if (score->layoutMode() == LayoutMode::FLOAT) {
                  qreal y = restHeight * .5;
                  for (System* system : page->systems())
                        system->move(QPointF(0.0, y));
                  }
            else if ((score->layoutMode() != LayoutMode::SYSTEM) && score->enableVerticalSpread())
                  distributeStaves(page, footerPadding);

            // system dividers
            for (int i = 0; i < gaps; ++i) {
                  System* s1 = page->systems().at(i);
                  System* s2 = page->systems().at(i+1);
                  if (!(s1->vbox() || s2->vbox())) {
                        qreal yOffset = s1->height() + (s1->distance()-s1->height()) * .5;
                        checkDivider(true,  s1, yOffset);
                        checkDivider(false, s1, yOffset);
                        }
                  }
            return;
            }

      qreal maxDist = score->maxSystemDistance();

      // allocate space as needed to normalize system distance (bottom of one system to top of next)
      std::sort(sList.begin(), sList.end(), [](System* a, System* b) { return a->distance() - a->height() < b->distance() - b->height(); });
      System* s0 = sList[0];
      qreal dist = s0->distance() - s0->height();           // distance for shortest system
      for (int i = 1; i < sList.size(); ++i) {
            System* si = sList[i];
            qreal ndist = si->distance() - si->height();    // next taller system
            qreal fill  = ndist - dist;                     // amount by which this system distance exceeds next shorter
            if (fill > 0.0) {
                  qreal totalFill = fill * i;               // space required to add this amount to all shorter systems
                  if (totalFill > restHeight) {
                        totalFill = restHeight;             // too much; adjust amount
                        fill = restHeight / i;
                        }
                  for (int k = 0; k < i; ++k) {             // add amount to all shorter systems
                        System* s = sList[k];
                        qreal d = s->distance() + fill;
                        if ((d - s->height()) > maxDist)    // but don't exceed max system distance
                              d = qMax(maxDist + s->height(), s->distance());
                        s->setDistance(d);
                        }
                  restHeight -= totalFill;                  // reduce available space for next iteration
                  if (restHeight <= 0)
                        break;                              // no space left
                  }
            dist = ndist;                                   // set up for next iteration
            }

      if (restHeight > 0.0) {                               // space left?
            qreal fill = restHeight / sList.size();
            for (System* s : qAsConst(sList)) {                       // allocate it to systems equally
                  qreal d = s->distance() + fill;
                  if ((d - s->height()) > maxDist)          // but don't exceed max system distance
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

            if (!(s1->vbox() || s2->vbox())) {
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
            pageOldMeasure = nullptr;
            }
      else {
            page = score->pages()[curPage];
            QList<System*>& systems = page->systems();
            pageOldMeasure = systems.isEmpty() ? nullptr : systems.back()->measures().back();
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
            system->adjustStavesNumber(nstaves);
            for (int i = 0; i < nstaves; ++i)
                  system->staff(i)->setShow(score()->staff(i)->show());
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
            // reuse existing mmrest
            if (mmr->ticks() != len) {
                  Segment* s = mmr->findSegmentR(SegmentType::EndBarLine, mmr->ticks());
                  // adjust length
                  mmr->setTicks(len);
                  // move existing end barline
                  if (s)
                        s->setRtick(len);
                  }
            mmr->removeSystemTrailer();
            }
      else {
            mmr = new Measure(this);
            mmr->setTicks(len);
            mmr->setTick(m->tick());
            undo(new ChangeMMRest(m, mmr));
            }
      mmr->setTimesig(m->timesig());
      mmr->setPageBreak(lm->pageBreak());
      mmr->setLineBreak(lm->lineBreak());
      mmr->setMMRestCount(n);
      mmr->setNo(m->no());

      Segment* ss = lm->findSegmentR(SegmentType::EndBarLine, lm->ticks());
      if (ss) {
            Segment* ds = mmr->undoGetSegmentR(SegmentType::EndBarLine, mmr->ticks());
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Element* e = ss->element(staffIdx * VOICES);
                  if (e) {
                        bool generated = e->generated();
                        if (!ds->element(staffIdx * VOICES)) {
                              Element* ee = generated ? e->clone() : e->linkedClone();
                              ee->setGenerated(generated);
                              ee->setParent(ds);
                              undoAddElement(ee);
                              }
                        else {
                              BarLine* bd = toBarLine(ds->element(staffIdx * VOICES));
                              BarLine* bs = toBarLine(e);
                              if (!generated && !bd->links())
                                    undo(new Link(bd, bs));
                              if (bd->barLineType() != bs->barLineType()) {
                                    // change directly when generating mmrests, do not change underlying measures or follow links
                                    undo(new ChangeProperty(bd, Pid::BARLINE_TYPE, QVariant::fromValue(bs->barLineType()), PropertyFlags::NOSTYLE));
                                    undo(new ChangeProperty(bd, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
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
                              Clef* mmrClef = clef->generated() ? clef->clone() : toClef(clef->linkedClone());
                              mmrClef->setParent(mmrClefSeg);
                              undoAddElement(mmrClef);
                              }
                        else {
                              Clef* mmrClef = toClef(mmrClefSeg->element(track));
                              mmrClef->setClefType(clef->clefType());
                              mmrClef->setShowCourtesy(clef->showCourtesy());
                              }
                        }
                  }
            }

      mmr->setRepeatStart(m->repeatStart() || lm->repeatStart());
      mmr->setRepeatEnd(m->repeatEnd() || lm->repeatEnd());
      mmr->setSectionBreak(lm->sectionBreak());

      ElementList oldList = mmr->takeElements();
      ElementList newList = lm->el();

      for (Element* e : m->el()) {
            if (e->isMarker())
                  newList.push_back(e);
            }
      for (Element* e : newList) {
            bool found = false;
            for (Element* ee : oldList) {
                  if (ee->type() == e->type() && ee->subtype() == e->subtype()) {
                        mmr->add(ee);
                        auto i = std::find(oldList.begin(), oldList.end(), ee);
                        if (i != oldList.end())
                              oldList.erase(i);
                        found = true;
                        break;
                        }
                  }
            if (!found)
                  mmr->add(e->clone());
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
            ns->setEnabled(cs->enabled());
            ns->setTrailer(cs->trailer());
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
      else if (ns) {
            // TODO: remove elements from ns?
            undo(new RemoveElement(ns));
            }

      //
      // check for time signature
      //
      cs = m->findSegmentR(SegmentType::TimeSig, Fraction(0,1));
      ns = mmr->findSegment(SegmentType::TimeSig, m->tick());
      if (cs) {
            if (ns == 0)
                  ns = mmr->undoGetSegmentR(SegmentType::TimeSig, Fraction(0,1));
            ns->setEnabled(cs->enabled());
            ns->setHeader(cs->header());
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  TimeSig* ts = toTimeSig(cs->element(track));
                  if (ts) {
                        TimeSig* nts = toTimeSig(ns->element(track));
                        if (!nts) {
                              nts = ts->generated() ? ts->clone() : toTimeSig(ts->linkedClone());
                              nts->setParent(ns);
                              undo(new AddElement(nts));
                              }
                        else {
                              nts->setSig(ts->sig(), ts->timeSigType());
                              nts->setNumeratorString(ts->numeratorString());
                              nts->setDenominatorString(ts->denominatorString());
                              nts->layout();
                              }
                        }
                  }
            }
      else if (ns) {
            // TODO: remove elements from ns?
            undo(new RemoveElement(ns));
            }

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
      else if (ns) {
            // TODO: remove elements from ns?
            undo(new RemoveElement(ns));
            }

      //
      // check for key signature
      //
      cs = m->findSegmentR(SegmentType::KeySig, Fraction(0,1));
      ns = mmr->findSegmentR(SegmentType::KeySig, Fraction(0,1));
      if (cs) {
            if (ns == 0)
                  ns = mmr->undoGetSegmentR(SegmentType::KeySig, Fraction(0,1));
            ns->setEnabled(cs->enabled());
            ns->setHeader(cs->header());
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  int track = staffIdx * VOICES;
                  KeySig* ks  = toKeySig(cs->element(track));
                  if (ks) {
                        KeySig* nks = toKeySig(ns->element(track));
                        if (!nks) {
                              nks = ks->generated() ? ks->clone() : toKeySig(ks->linkedClone());
                              nks->setParent(ns);
                              nks->setGenerated(true);
                              undo(new AddElement(nks));
                              }
                        else {
                              if (!(nks->keySigEvent() == ks->keySigEvent())) {
                                    bool addKey = ks->isChange();
                                    undo(new ChangeKeySig(nks, ks->keySigEvent(), nks->showCourtesy(), addKey));
                                    }
                              }
                        }
                  }
            }
      else if (ns) {
            ns->setEnabled(false);
            // TODO: remove elements from ns, then delete ns
            // previously we removed the segment if not empty,
            // but this resulted in "stale" keysig in mmrest after removed from underlying measure
            //undo(new RemoveElement(ns));
            }

      mmr->checkHeader();
      mmr->checkTrailer();

      //
      // check for rehearsal mark etc.
      //
      cs = m->findSegmentR(SegmentType::ChordRest, Fraction(0,1));
      if (cs) {
            // clone elements from underlying measure to mmr
            for (Element* e : cs->annotations()) {
                  // look at elements in underlying measure
                  if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isInstrumentChange()))
                        continue;
                  // try to find a match in mmr
                  bool found = false;
                  for (Element* ee : s->annotations()) {
                        if (e->linkList().contains(ee)) {
                              found = true;
                              break;
                              }
                        }
                  // add to mmr if no match found
                  if (!found) {
                        Element* ne = e->linkedClone();
                        ne->setParent(s);
                        undo(new AddElement(ne));
                        }
                  }

            // remove stray elements (possibly leftover from a previous layout of this mmr)
            // this should not happen since the elements are linked?
            for (Element* e : s->annotations()) {
                  // look at elements in mmr
                  if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isInstrumentChange()))
                        continue;
                  // try to find a match in underlying measure
                  bool found = false;
                  for (Element* ee : cs->annotations()) {
                        if (e->linkList().contains(ee)) {
                              found = true;
                              break;
                              }
                        }
                  // remove from mmr if no match found
                  if (!found)
                        undo(new RemoveElement(e));
                  }
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
                  if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isInstrumentChange()))
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
            // break for first measure of volta or textline and first measure *after* volta
            if ((s->isVolta() || s->isTextLine()) && (s->tick() == m->tick() || s->tick2() == m->tick()))
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

      for (Segment* s = m->first(); s; s = s->next()) {
            for (Element* e : s->annotations()) {
                  if (!e->visible())
                        continue;
                  if (e->isRehearsalMark() ||
                      e->isTempoText() ||
                      ((e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isInstrumentChange()) && (e->systemFlag() || m->score()->staff(e->staffIdx())->show())))
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
      if (m->sectionBreakElement() && m->sectionBreakElement()->startWithMeasureOne())
            measureNo = 0;
      return measureNo;
      }

//---------------------------------------------------------
//   createBeams
//    helper function
//---------------------------------------------------------

void Score::createBeams(LayoutContext& lc, Measure* measure)
      {
      bool crossMeasure = styleB(Sid::crossMeasureValues);

      for (int track = 0; track < ntracks(); ++track) {
            Staff* stf = staff(track2staff(track));

            // don’t compute beams for invisible staves and tablature without stems
            if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->stemless()))
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
                        if (mode == Beam::Mode::MID || mode == Beam::Mode::END || mode == Beam::Mode::BEGIN32 || mode == Beam::Mode::BEGIN64) {
                              ChordRest* prevCR = findCR(measure->tick() - Fraction::fromTicks(1), track);
                              if (prevCR) {
                                    const Measure* pm = prevCR->measure();
                                    if (!beamNoContinue(prevCR->beamMode())
                                        && !pm->lineBreak() && !pm->pageBreak() && !pm->sectionBreak()
                                        && lc.prevMeasure
                                        && !(prevCR->isChord() && prevCR->durationType().type() <= TDuration::DurationType::V_QUARTER)) {
                                          beam = prevCR->beam();
                                          //a1 = beam ? beam->elements().front() : prevCR;
                                          a1 = beam ? nullptr : prevCR; // when beam is found, a1 is no longer required.
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

                  if (cr->isRest() && cr->beamMode() == Beam::Mode::AUTO)
                        bm = Beam::Mode::NONE;           // do not beam rests set to Beam::Mode::AUTO
                  else
                        bm = Groups::endBeam(cr, prev);  // get defaults from time signature properties

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

                  TDuration durationType = cr->durationType();
                  if (durationType.hooks() > 0 && cr->crossMeasure() == CrossMeasure::SECOND)
                        bm = Beam::Mode::NONE;

                  // Rests of any duration can be beamed over, if required
                  bool canBeBeamed = durationType.type() > TDuration::DurationType::V_QUARTER || cr->isRest();
                  if (!canBeBeamed || (bm == Beam::Mode::NONE)) {
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
                              cr->replaceBeam(beam);
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
                                    a1->replaceBeam(beam);
                                    }
                              cr->replaceBeam(beam);
                              a1 = 0;
                              }
                        }
                  }
            if (beam)
                  beam->layout1();
            else if (a1) {
                  Fraction nextTick = a1->tick() + a1->actualTicks();
                  Measure* m = (nextTick >= measure->endTick() ? measure->nextMeasure() : measure);
                  ChordRest* nextCR = (m ? m->findChordRest(nextTick, track) : nullptr);
                  Beam* b = a1->beam();
                  if (!(b && b->elements().startsWith(a1) && nextCR && beamModeMid(nextCR->beamMode())))
                        a1->removeDeleteBeam(false);
                  }
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
      if (!fstSeg)
            return;

      for (int track = 0; track < ntracks; ++track) {
            Staff* stf = score->staff(track2staff(track));

            // don’t compute beams for invisible staves and tablature without stems
            if (!stf->show() || (stf->isTabStaff(measure->tick()) && stf->staffType(measure->tick())->stemless()))
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
//   extendedStemLenWithTwoNotesTremolo
//    Goal: To extend stem of one of the chords to make the tremolo less steep
//    Returns a modified pair of stem lengths of two chords
//---------------------------------------------------------

std::pair<qreal, qreal> extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, qreal stemLen1, qreal stemLen2)
      {
      const qreal spatium = tremolo->spatium();
      Chord* c1 = tremolo->chord1();
      Chord* c2 = tremolo->chord2();
      Stem*  s1 = c1->stem();
      Stem*  s2 = c2->stem();
      const qreal sgn1 = c1->up() ? -1.0 : 1.0;
      const qreal sgn2 = c2->up() ? -1.0 : 1.0;
      const qreal stemTipDistance = (s1 && s2) ? (s2->pagePos().y() + stemLen2) - (s1->pagePos().y() + stemLen1)
         : (c2->stemPos().y() + stemLen2) - (c1->stemPos().y() + stemLen1);

      // same staff & same direction: extend one of the stems
      if (c1->staffMove() == c2->staffMove() && c1->up() == c2->up()) {
            const bool stem1Higher = stemTipDistance > 0.0;
            if (std::abs(stemTipDistance) > 1.0 * spatium) {
                  if ((c1->up() && !stem1Higher) || (!c1->up() && stem1Higher))
                        return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - 1.0 * spatium), stemLen2 };
                  else /* if ((c1->up() && stem1Higher) || (!c1->up() && !stem1Higher)) */
                        return { stemLen1, stemLen2 + sgn2 * (std::abs(stemTipDistance) - 1.0 * spatium) };
                  }
            }

// TODO: cross-staff two-note tremolo. Currently doesn't generate the right result in some cases.
#if 0
      // cross-staff & beam between staves: extend both stems by the same length
      else if (tremolo->crossStaffBeamBetween()) {
            const qreal sw = tremolo->score()->styleS(Sid::tremoloStrokeWidth).val();
            const qreal td = tremolo->score()->styleS(Sid::tremoloDistance).val();
            const qreal tremoloMinHeight = ((tremolo->lines() - 1) * td + sw) * spatium;
            const qreal dy = c1->up() ? tremoloMinHeight - stemTipDistance : tremoloMinHeight + stemTipDistance;
            const bool tooShort = dy > 1.0 * spatium;
            const bool tooLong = dy < -1.0 * spatium;
            const qreal idealDistance = 1.0 * spatium - tremoloMinHeight;

            if (tooShort)
                  return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - idealDistance) / 2.0,
                           stemLen2 + sgn2 * (std::abs(stemTipDistance) - idealDistance) / 2.0 };
            else if (tooLong)
                  return { stemLen1 - sgn1 * (std::abs(stemTipDistance) + idealDistance) / 2.0,
                           stemLen2 - sgn2 * (std::abs(stemTipDistance) + idealDistance) / 2.0 };
            }
#endif

      return { stemLen1, stemLen2 };
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

                  while (validMMRestMeasure(nm)) {
                        MeasureBase* mb = _showVBox ? nm->next() : nm->nextMeasure();
                        if (breakMultiMeasureRest(nm) && n)
                              break;
                        if (nm != m)
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

      if (lineMode() && (measure->tick() < lc.startTick || measure->tick() > lc.endTick)) {
            // needed to reset segment widths if they can change after measure width is computed
            //for (Segment& s : measure->segments())
            //      s.createShapes();
            lc.tick += measure->ticks();
            return;
            }

      measure->connectTremolo();

      //
      // calculate accidentals and note lines,
      // create stem and set stem direction
      //
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            const Staff* staff     = Score::staff(staffIdx);
            const Drumset* drumset = staff->part()->instrument(measure->tick())->useDrumset() ? staff->part()->instrument(measure->tick())->drumset() : 0;
            AccidentalState as;      // list of already set accidentals for this measure
            as.init(staff->keySigEvent(measure->tick()), staff->clef(measure->tick()));

            for (Segment& segment : measure->segments()) {
                  // TODO? maybe we do need to process it here to make it possible to enable later
                  //if (!segment.enabled())
                  //      continue;
                  if (segment.isKeySigType()) {
                        KeySig* ks = toKeySig(segment.element(staffIdx * VOICES));
                        if (!ks)
                              continue;
                        Fraction tick = segment.tick();
                        as.init(staff->keySigEvent(tick), staff->clef(tick));
                        ks->layout();
                        }
                  else if (segment.isChordRestType()) {
                        const StaffType* st = staff->staffTypeForElement(&segment);
                        int track     = staffIdx * VOICES;
                        int endTrack  = track + VOICES;

                        for (int t = track; t < endTrack; ++t) {
                              ChordRest* cr = segment.cr(t);
                              if (!cr)
                                    continue;
                              qreal m = staff->mag(&segment);
                              if (cr->isSmall())
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

                                    // if there is a two-note tremolo attached, and it is too steep,
                                    // extend stem of one of the chords (if not cross-staff)
                                    // or extend both stems (if cross-staff)
                                    // this should be done after the stem lengths of two notes are both calculated
                                    if (chord->tremolo() && chord == chord->tremolo()->chord2()) {
                                          Stem* stem1 = chord->tremolo()->chord1()->stem();
                                          Stem* stem2 = chord->tremolo()->chord2()->stem();
                                          if (stem1 && stem2) {
                                                std::pair<qreal, qreal> extendedLen = extendedStemLenWithTwoNoteTremolo(chord->tremolo(),
                                                   stem1->p2().y(), stem2->p2().y());
                                                stem1->setLen(extendedLen.first);
                                                stem2->setLen(extendedLen.second);
                                                }
                                          }
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

      createBeams(lc, measure);

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
                                    }
                              }
                        }
                  }
            }

      measure->computeTicks();

      for (Segment& segment : measure->segments()) {
            if (segment.isBreathType()) {
                  for (Element* e : segment.elist()) {
                        if (e && e->isBreath())
                              e->layout();
                        }
                  }
            else if (segment.isChordRestType()) {
                  for (Element* e : segment.annotations()) {
                        if (e->isSymbol())
                              e->layout();
                        }
                  }
            }

      rebuildTempoAndTimeSigMaps(measure);

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
            // TODO? maybe we do need to process it here to make it possible to enable later
            //if (!s.enabled())
            //      continue;
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
//    returns true for the first CR of a beam that is not cross-staff
//---------------------------------------------------------

bool isTopBeam(ChordRest* cr)
      {
      Beam* b = cr->beam();
      if (b && b->elements().front() == cr) {
            // beam already considered cross?
            if (b->cross())
                  return false;

            // for beams not already considered cross,
            // consider them so here if any elements were moved up
            for (ChordRest* cr1 : b->elements()) {
                  // some element moved up?
                  if (cr1->staffMove() < 0)
                        return false;
                  }

            // not cross
            return true;
            }

      // no beam or not first element
      return false;
      }

//---------------------------------------------------------
//   notTopBeam
//    returns true for the first CR of a beam that is cross-staff
//---------------------------------------------------------

bool notTopBeam(ChordRest* cr)
      {
      Beam* b = cr->beam();
      if (b && b->elements().front() == cr) {
            // beam already considered cross?
            if (b->cross())
                  return true;

            // for beams not already considered cross,
            // consider them so here if any elements were moved up
            for (ChordRest* cr1 : b->elements()) {
                  // some element moved up?
                  if (cr1->staffMove() < 0)
                        return true;
                  }

            // not cross
            return false;
            }

      // no beam or not first element
      return false;
      }

//---------------------------------------------------------
//   isTopTuplet
//    returns true for the first CR of a tuplet that is not cross-staff
//---------------------------------------------------------

bool isTopTuplet(ChordRest* cr)
      {
      Tuplet* t = cr->tuplet();
      if (t && t->elements().front() == cr) {
            // find top level tuplet
            while (t->tuplet())
                  t = t->tuplet();
            // consider tuplet cross if anything moved within it
            if (t->cross())
                  return false;
            else
                  return true;
            }

      // no tuplet or not first element
      return false;
      }

//---------------------------------------------------------
//   notTopTuplet
//    returns true for the first CR of a tuplet that is cross-staff
//---------------------------------------------------------

bool notTopTuplet(ChordRest* cr)
      {
      Tuplet* t = cr->tuplet();
      if (t && t->elements().front() == cr) {
            // find top level tuplet
            while (t->tuplet())
                  t = t->tuplet();
            // consider tuplet cross if anything moved within it
            if (t->cross())
                  return true;
            else
                  return false;
            }

      // no tuplet or not first element
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
                              qreal yOff = l->offset().y();
                              QPointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                              QRectF r = l->bbox().translated(offset);
                              r.translate(0.0, -yOff);
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
                              qreal yOff = l->offset().y();
                              QRectF r = l->bbox().translated(l->pos() + cr->pos() + s.pos() + s.measure()->pos());
                              r.translate(0.0, -yOff);
                              sk.add(r.x(), r.bottom(), r.width());
                              }
                        }
                  SysStaff* ss = s.measure()->system()->staff(staffIdx);
                  for (Lyrics* l : cr->lyrics()) {
                        if (l->autoplace() && l->placeAbove()) {
                              qreal y = sk.minDistance(ss->skyline().north());
                              if (y > -lyricsMinTopDistance)
                                    yMin = qMin(yMin, -y - lyricsMinTopDistance);
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
                        if (l->autoplace() && l->placeBelow()) {
                              l->rypos() += yMax - l->propertyDefault(Pid::OFFSET).toPointF().y();
                              if (l->addToSkyline()) {
                                    QPointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                                    sk.add(l->bbox().translated(offset).adjusted(0.0, 0.0, 0.0, lyricsMinBottomDistance));
                                    }
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
            if (l->autoplace() && l->placeAbove()) {
                  l->rypos() += yMin - l->propertyDefault(Pid::OFFSET).toPointF().y();
                  if (l->addToSkyline()) {
                        QPointF offset = l->pos() + cr->pos() + cr->segment()->pos() + cr->segment()->measure()->pos();
                        sk.add(l->bbox().translated(offset));
                        }
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
                              Beam* b = cr->beam();
                              b->layout();
                              b->addSkyline(m->system()->staff(b->staffIdx())->skyline());
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
                                                // user adjusted offset can possibly change placement
                                                if (l->offsetChanged() != OffsetChange::NONE) {
                                                      Placement p = l->placement();
                                                      l->rebaseOffset();
                                                      if (l->placement() != p) {
                                                            l->undoResetProperty(Pid::AUTOPLACE);
                                                            //l->undoResetProperty(Pid::OFFSET);
                                                            //l->layout();
                                                            }
                                                      }
                                                l->setOffsetChanged(false);
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
      SysStaff* staff = system->staff(ch->staffIdx());
      if (!staff->show())
            return;
      for (Note* note : ch->notes()) {
            Tie* t = note->tieFor();
            if (t) {
                  TieSegment* ts = t->layoutFor(system);
                  if (ts && ts->addToSkyline())
                        staff->skyline().add(ts->shape().translated(ts->pos()));
                  }
            t = note->tieBack();
            if (t) {
                  if (t->startNote()->tick() < stick) {
                        TieSegment* ts = t->layoutBack(system);
                        if (ts && ts->addToSkyline())
                              staff->skyline().add(ts->shape().translated(ts->pos()));
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
                        h->autoplaceSegmentElement();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   alignHarmonies
//---------------------------------------------------------

void alignHarmonies(const System* system, const std::vector<Segment*>& sl, bool harmony, const qreal maxShiftAbove, const qreal maxShiftBelow)
      {

      // Help class.
      // Contains harmonies/fretboard per segment.
      class HarmonyList : public QList<Element*> {
            QMap<const Segment*, QList<Element*>> elements;
            QList<Element*> modified;

            Element* getReferenceElement(const Segment* s, bool above, bool visible) const
                  {
                  // Returns the reference element for aligning.
                  // When a segments contains multiple harmonies/fretboard, the lowest placed
                  // element (for placement above, otherwise the highest placed element) is
                  // used for alignment.
                  Element* element { nullptr };
                  for (Element* e : elements[s]) {
                        // Only chord symbols have styled offset, fretboards don't.
                        if (!e->autoplace() || (e->isHarmony() && !e->isStyled(Pid::OFFSET)) || (visible && !e->visible()))
                              continue;
                        if (!element) {
                              element = e;
                              }
                        else {
                              if ((e->placeAbove() &&  above && (element->y() < e->y())) ||
                                  (e->placeBelow() && !above && (element->y() > e->y())))
                                    element = e;
                              }
                         }
                  return element;
                  }

         public:
            HarmonyList()
                  {
                  elements.clear();
                  modified.clear();
                  }

            void append(const Segment* s, Element* e)
                  {
                  elements[s].append(e);
                  }

            qreal getReferenceHeight(bool above) const
                  {
                  // The reference height is the height of
                  //    the lowest element if placed above
                  // or
                  //    the highest element if placed below.
                  bool first { true };
                  qreal ref { 0.0 };
                  for (auto s : elements.keys()) {
                        Element* e { getReferenceElement(s, above, true) };
                        if (!e)
                              continue;
                        if (e->placeAbove() && above) {
                              ref = first ? e->y() : qMin(ref, e->y());
                              first = false;
                              }
                        else if (e->placeBelow() && !above) {
                              ref = first ? e->y() : qMax(ref, e->y());
                              first = false;
                              }
                        }
                  return ref;
                  }

            bool align(bool above, qreal reference, qreal maxShift)
                  {
                  // Align the elements. If a segment contains multiple elements,
                  // only the reference elements is used in the algorithm. All other
                  // elements will remain their original placement with respect to
                  // the reference element.
                  bool moved { false };
                  if (almostZero(reference))
                        return moved;

                  for (auto s : elements.keys()) {
                        QList<Element*> handled;
                        Element* be = getReferenceElement(s, above, false);
                        if (!be)
                              // If there are only invisible elements, we have to use an invisible
                              // element for alignment reference.
                              be = getReferenceElement(s, above, true);
                        if (be && ((above && (be->y() < (reference + maxShift))) || ((!above && (be->y() > (reference - maxShift)))))) {
                              qreal shift = be->rypos();
                              be->rypos() = reference - be->ryoffset();
                              shift -= be->rypos();
                              for (Element* e : elements[s]) {
                                    if ((above && e->placeBelow()) || (!above && e->placeAbove()))
                                          continue;
                                    modified.append(e);
                                    handled.append(e);
                                    moved = true;
                                    if (e != be)
                                          e->rypos() -= shift;
                                    }
                              for (auto e : handled)
                                    elements[s].removeOne(e);
                              }
                        }
                  return moved;
                  }

            void addToSkyline(const System* system)
                  {
                  for (Element* e : qAsConst(modified)) {
                        const Segment* s = toSegment(e->parent());
                        const MeasureBase* m = toMeasureBase(s->parent());
                        system->staff(e->staffIdx())->skyline().add(e->shape().translated(e->pos() + s->pos() + m->pos()));
                        if (e->isFretDiagram()) {
                              FretDiagram* fd = toFretDiagram(e);
                              Harmony* h = fd->harmony();
                              if (h)
                                    system->staff(e->staffIdx())->skyline().add(h->shape().translated(h->pos() + fd->pos() + s->pos() + m->pos()));
                              else
                                    system->staff(e->staffIdx())->skyline().add(fd->shape().translated(fd->pos() + s->pos() + m->pos()));
                              }
                        }
                  }
            };

      if (almostZero(maxShiftAbove) && almostZero(maxShiftBelow))
            return;

      // Collect all fret diagrams and chord symbol and store them per staff.
      // In the same pass, the maximum height is collected.
      QMap<int, HarmonyList> staves;
      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if ((harmony && e->isHarmony()) || (!harmony && e->isFretDiagram()))
                        staves[e->staffIdx()].append(s, e);
                  }
            }

      for (int idx: staves.keys()) {
            // Align the objects.
            // Algorithm:
            //    - Find highest placed harmony/fretdiagram.
            //    - Align all harmony/fretdiagram objects placed between height and height-maxShiftAbove.
            //    - Repeat for all harmony/fretdiagram objects below heigt-maxShiftAbove.
            bool moved { true };
            int pass { 0 };
            while (moved && (pass++ < 10)) {
                  moved = false;
                  moved |= staves[idx].align(true, staves[idx].getReferenceHeight(true), maxShiftAbove);
                  moved |= staves[idx].align(false, staves[idx].getReferenceHeight(false), maxShiftBelow);
                  }

            // Add all aligned objects to the sky line.
            staves[idx].addToSkyline(system);
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
            if (ss->autoplace())
                  segments.push_back(ss);
            }

      if (align && segments.size() > 1) {
            const int nstaves = system->staves()->size();
            constexpr qreal minY = -1000000.0;
            const qreal defaultY = segments[0]->rypos();
            std::vector<qreal> y(nstaves, minY);

            for (SpannerSegment* ss : segments) {
                  if (ss->visible()) {
                        qreal& staffY = y[ss->staffIdx()];
                        staffY = qMax(staffY, ss->rypos());
                        }
                  }
            for (SpannerSegment* ss : segments) {
                  if (!ss->isStyled(Pid::OFFSET))
                        continue;
                  const qreal staffY = y[ss->staffIdx()];
                  if (staffY > minY)
                        ss->rypos() = staffY;
                  else
                        ss->rypos() = defaultY;
                  }
            }

      //
      // add shapes to skyline
      //
      for (SpannerSegment* ss : segments) {
            if (ss->addToSkyline())
                  system->staff(ss->staffIdx())->skyline().add(ss->shape().translated(ss->pos()));
            }
      }

//---------------------------------------------------------
//   collectSystem
//---------------------------------------------------------

System* Score::collectSystem(LayoutContext& lc)
      {
      if (!lc.curMeasure)
            return 0;
      const MeasureBase* measure  = _systems.empty() ? 0 : _systems.back()->measures().back();
      if (measure)
            measure = measure->findPotentialSectionBreak();
      if (measure) {
            lc.firstSystem        = measure->sectionBreak() && _layoutMode != LayoutMode::FLOAT;
            lc.firstSystemIndent  = lc.firstSystem && measure->sectionBreakElement()->firstSystemIdentation() && styleB(Sid::enableIndentationOnFirstSystem);
            lc.startWithLongNames = lc.firstSystem && measure->sectionBreakElement()->startWithLongNames();
            }
      System* system = getNextSystem(lc);
      Fraction lcmTick = lc.curMeasure->tick();
      system->setInstrumentNames(lc.startWithLongNames, lcmTick);

      qreal minWidth    = 0;
      qreal layoutSystemMinWidth = 0;
      bool firstMeasure = true;
      bool createHeader = false;
      qreal systemWidth = styleD(Sid::pagePrintableWidth) * DPI;
      system->setWidth(systemWidth);

      // save state of measure
      qreal curWidth = lc.curMeasure->width();
      bool curHeader = lc.curMeasure->header();
      bool curTrailer = lc.curMeasure->trailer();

      while (lc.curMeasure) {    // collect measure for system
            System* oldSystem = lc.curMeasure->system();
            system->appendMeasure(lc.curMeasure);

            qreal ww  = 0;      // width of current measure

            if (lc.curMeasure->isMeasure()) {
                  Measure* m = toMeasure(lc.curMeasure);
                  if (firstMeasure) {
                        layoutSystemMinWidth = minWidth;
                        system->layoutSystem(minWidth, lc.firstSystem, lc.firstSystemIndent);
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
                  m->addSystemTrailer(m->nextMeasure());
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
                  // TODO: if lc.curMeasure is a frame, removing the trailer may be premature
                  // but merely skipping this code isn't good enough,
                  // we need to find the right time to re-enable the trailer,
                  // since it seems to be disabled somewhere else
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
                  // TODO: we actually still don't know for sure
                  // if this will be the last true measure of the system or not
                  // since the lc.curMeasure may be a frame
                  // but at this point we have no choice but to assume it isn't
                  // since we don't know yet if another true measure will fit
                  // worst that happens is we don't get the automatic double bar before a courtesy key signature
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

            // preserve state of next measure (which is about to become current measure)
            if (lc.nextMeasure) {
                  MeasureBase* nmb = lc.nextMeasure;
                  if (nmb->isMeasure() && styleB(Sid::createMultiMeasureRests)) {
                        Measure* nm = toMeasure(nmb);
                        if (nm->hasMMRest())
                              nmb = nm->mmRest();
                        }
                  curWidth = nmb->width();
                  curHeader = nmb->header();
                  curTrailer = nmb->trailer();
                  }

            getNextMeasure(lc);

            minWidth += ww;

            // ElementType nt = lc.curMeasure ? lc.curMeasure->type() : ElementType::INVALID;
            mb = lc.curMeasure;
            bool tooWide = false; // minWidth + minMeasureWidth > systemWidth;  // TODO: noBreak
            if (lineBreak || !mb || mb->isVBox() || mb->isTBox() || mb->isFBox() || tooWide)
                  break;
            }

      if (lc.endTick < lc.prevMeasure->tick()) {
            // we've processed the entire range
            // but we need to continue layout until we reach a system whose last measure is the same as previous layout
            if (lc.prevMeasure == lc.systemOldMeasure) {
                  // this system ends in the same place as the previous layout
                  // ok to stop
                  if (lc.curMeasure && lc.curMeasure->isMeasure()) {
                        // we may have previously processed first measure of next system
                        // so now we must restore it to its original state
                        Measure* m = toMeasure(lc.curMeasure);
                        if (m->repeatStart()) {
                              Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0,1));
                              if (!s->enabled())
                                    s->setEnabled(true);
                              }
                        const MeasureBase* pbmb = lc.prevMeasure->findPotentialSectionBreak();
                        bool firstSystem = pbmb->sectionBreak() && _layoutMode != LayoutMode::FLOAT;
                        if (curHeader)
                              m->addSystemHeader(firstSystem);
                        else
                              m->removeSystemHeader();
                        if (curTrailer)
                              m->addSystemTrailer(m->nextMeasure());
                        else
                              m->removeSystemTrailer();
                        m->computeMinWidth();
                        m->stretchMeasure(curWidth);
                        restoreBeams(m);
                        }
                  lc.rangeDone = true;
                  }
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
      // Relayout system decorations to reuse space properly for
      // hidden staves' instrument names or other hidden elements.
      minWidth -= system->leftMargin();
      system->layoutSystem(layoutSystemMinWidth, lc.firstSystem, lc.firstSystemIndent);
      minWidth += system->leftMargin();

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
                        int weight   = m->layoutWeight();
                        totalWeight += weight * m->basicStretch();
                        }
                  }

#ifndef NDEBUG
            if (!qFuzzyCompare(mw, minWidth))
                  qDebug("==layoutSystem %6d old %.1f new %.1f", system->measures().front()->tick().ticks(), minWidth, mw);
#endif
            rest = systemWidth - minWidth;
            //
            // don’t stretch last system of a section (or the last of the piece),
            // if accumulated minWidth is <= lastSystemFillLimit
            //
            if ((lc.curMeasure == 0  || (lm && lm->sectionBreak()))
               && ((minWidth / systemWidth) <= styleD(Sid::lastSystemFillLimit))) {
                  if (minWidth > rest)
                        rest = rest * .5;
                  else
                        rest = minWidth;
                  }
            rest /= totalWeight;
            }

      QPointF pos;
      firstMeasure = true;
      bool createBrackets = false;
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
                  int weight = m->layoutWeight();
                  ww  += rest * weight * stretch;
                  m->stretchMeasure(ww);
                  m->layoutStaffLines();
                  if (createBrackets) {
                        system->addBrackets(toMeasure(mb));
                        createBrackets = false;
                        }
                  }
            else if (mb->isHBox()) {
                  mb->setPos(pos + QPointF(toHBox(mb)->topGap(), 0.0));
                  mb->layout();
                  createBrackets = toHBox(mb)->createSystemHeader();
                  }
            else if (mb->isVBox())
                  mb->setPos(pos);
            pos.rx() += ww;
            }
      system->setWidth(pos.x());

      layoutSystemElements(system, lc);
      system->layout2();   // compute staff distances
      // TODO: now that the code at the top of this function does this same backwards search,
      // we might be able to eliminate this block
      // but, lc might be used elsewhere so we need to be careful
#if 1
      measure = system->measures().back();
      if (measure)
            measure = measure->findPotentialSectionBreak();
      if (measure) {
            lc.firstSystem        = measure->sectionBreak() && _layoutMode != LayoutMode::FLOAT;
            lc.firstSystemIndent  = lc.firstSystem && measure->sectionBreakElement()->firstSystemIdentation() && styleB(Sid::enableIndentationOnFirstSystem);
            lc.startWithLongNames = lc.firstSystem && measure->sectionBreakElement()->startWithLongNames();
            }
#endif
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
            m->layoutMMRestRange();

            // in continuous view, entire score is one system
            // but we only need to process the range
            if (lineMode() && (m->tick() < lc.startTick || m->tick() > lc.endTick))
                  continue;
            for (Segment* s = m->first(); s; s = s->next()) {
                  if (s->isChordRestType() || !s->annotations().empty())
                        sl.push_back(s);
                  }
            }

      //-------------------------------------------------------------
      // layout beams
      //  Needs to be done before creating skylines as stem lengths
      //  may change.
      //-------------------------------------------------------------

      for (Segment* s : sl) {
            for (Element* e : s->elist()) {
                  if (!e || !e->isChordRest() || !score()->staff(e->staffIdx())->show()) {
                        // the beam and its system may still be referenced when selecting all,
                        // even if the staff is invisible. The old system is invalid and does cause problems in #284012
                        if (e && e->isChordRest() && !score()->staff(e->staffIdx())->show() && toChordRest(e)->beam())
                              toChordRest(e)->beam()->setParent(nullptr);
                        continue;
                        }
                  ChordRest* cr = toChordRest(e);

                  // layout beam
                  if (isTopBeam(cr)) {
                        Beam* b = cr->beam();
                        b->layout();
                        }
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
                  MeasureNumber* mno = m->noText(staffIdx);
                  MMRestRange* mmrr  = m->mmRangeText(staffIdx);
                  // no need to build skyline outside of range in continuous view
                  if (lineMode() && (m->tick() < lc.startTick || m->tick() > lc.endTick))
                        continue;
                  if (mno && mno->addToSkyline())
                        ss->skyline().add(mno->bbox().translated(m->pos() + mno->pos()));
                  if (mmrr && mmrr->addToSkyline())
                        ss->skyline().add(mmrr->bbox().translated(m->pos() + mmrr->pos()));
                  if (m->staffLines(staffIdx)->addToSkyline())
                        ss->skyline().add(m->staffLines(staffIdx)->bbox().translated(m->pos()));
                  for (Segment& s : m->segments()) {
                        if (!s.enabled())
                              continue;
                        QPointF p(s.pos() + m->pos());
                        if (s.segmentType() & (SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine)) {
                              BarLine* bl = toBarLine(s.element(staffIdx * VOICES));
                              if (bl && bl->addToSkyline()) {
                                    QRectF r = bl->layoutRect();
                                    skyline.add(r.translated(bl->pos() + p));
                                    }
                              }
                        else if (s.segmentType() & SegmentType::TimeSig) {
                              TimeSig* ts = toTimeSig(s.element(staffIdx * VOICES));
                              if (ts && ts->addToSkyline()) {
                                    skyline.add(ts->shape().translated(ts->pos() + p));
                                    }
                              }
                        else {
                              int strack = staffIdx * VOICES;
                              int etrack = strack + VOICES;
                              for (Element* e : s.elist()) {
                                    if (!e)
                                          continue;
                                    int effectiveTrack = e->vStaffIdx() * VOICES + e->voice();
                                    if (effectiveTrack < strack || effectiveTrack >= etrack)
                                          continue;

                                    // clear layout for chord-based fingerings
                                    // do this before adding chord to skyline
                                    if (e->isChord()) {
                                          Chord* c = toChord(e);
                                          std::list<Note*> notes;
                                          for (auto gc : c->graceNotes()) {
                                                for (auto n : gc->notes())
                                                      notes.push_back(n);
                                                }
                                          for (auto n : c->notes())
                                                notes.push_back(n);
                                          for (Note* note : notes) {
                                                for (Element* en : note->el()) {
                                                      if (en->isFingering()) {
                                                            Fingering* f = toFingering(en);
                                                            if (f->layoutType() == ElementType::CHORD) {
                                                                  f->setPos(QPointF());
                                                                  f->setbbox(QRectF());
                                                                  }
                                                            }
                                                      }
                                                }
                                          }

                                    // add element to skyline
                                    if (e->addToSkyline() || e->isChord())
                                          skyline.add(e->shape().translated(e->pos() + p));

                                    // add tremolo to skyline
                                    if (e->isChord() && toChord(e)->tremolo()) {
                                          Tremolo* t = toChord(e)->tremolo();
                                          Chord* c1 = t->chord1();
                                          Chord* c2 = t->chord2();
                                          if (!t->twoNotes() || (c1 && !c1->staffMove() && c2 && !c2->staffMove())) {
                                                if (t->chord() == e && t->addToSkyline())
                                                      skyline.add(t->shape().translated(t->pos() + e->pos() + p));
                                                }
                                          }
                                    }
                              }
                        }
                  }
            }

      //-------------------------------------------------------------
      // layout fingerings, add beams to skylines
      //-------------------------------------------------------------

      for (Segment* s : sl) {
            std::set<int> recreateShapes;
            for (Element* e : s->elist()) {
                  if (!e || !e->isChordRest() || !score()->staff(e->staffIdx())->show())
                        continue;
                  ChordRest* cr = toChordRest(e);

                  // add beam to skyline
                  if (isTopBeam(cr)) {
                        Beam* b = cr->beam();
                        b->addSkyline(system->staff(b->staffIdx())->skyline());
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
                              if (f->addToSkyline()) {
                                    Note* n = f->note();
                                    QRectF r = f->bbox().translated(f->pos() + n->pos() + n->chord()->pos() + s->pos() + s->measure()->pos());
                                    system->staff(f->note()->chord()->vStaffIdx())->skyline().add(r);
                                    }
                              recreateShapes.insert(f->staffIdx());
                              }
                        }
                  }
            for (auto staffIdx : recreateShapes)
                  s->createShape(staffIdx);
            }

      //-------------------------------------------------------------
      // layout articulations
      //-------------------------------------------------------------

      for (Segment* s : sl) {
            for (Element* e : s->elist()) {
                  if (!e || !e->isChordRest() || !score()->staff(e->staffIdx())->show())
                        continue;
                  ChordRest* cr = toChordRest(e);
                  // articulations
                  if (cr->isChord()) {
                        Chord* c = toChord(cr);
                        c->layoutArticulations();
                        c->layoutArticulations2();
                        }
                  }
            }

      //-------------------------------------------------------------
      // layout tuplets
      //-------------------------------------------------------------

      for (Segment* s : sl) {
            for (Element* e : s->elist()) {
                  if (!e || !e->isChordRest() || !score()->staff(e->staffIdx())->show())
                        continue;
                  ChordRest* cr = toChordRest(e);
                  if (!isTopTuplet(cr))
                        continue;
                  DurationElement* de = cr;
                  while (de->tuplet() && de->tuplet()->elements().front() == de) {
                        Tuplet* t = de->tuplet();
                        t->layout();
                        de = t;
                        }
                  }
            }

      //-------------------------------------------------------------
      // Drumline sticking
      //-------------------------------------------------------------

      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isSticking())
                        e->layout();
                  }
            }

      //-------------------------------------------------------------
      // layout slurs
      //-------------------------------------------------------------

      bool useRange = false;  // TODO: lineMode();
      Fraction stick = useRange ? lc.startTick : system->measures().front()->tick();
      Fraction etick = useRange ? lc.endTick : system->measures().back()->endTick();
      auto spanners = score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks());

      std::vector<Spanner*> spanner;
      for (auto interval : spanners) {
            Spanner* sp = interval.value;
            sp->computeStartElement();
            sp->computeEndElement();
            lc.processedSpanners.insert(sp);
            if (sp->tick() < etick && sp->tick2() >= stick) {
                  if (sp->isSlur()) {
                        // skip cross-staff slurs
                        ChordRest* scr = sp->startCR();
                        ChordRest* ecr = sp->endCR();
                        int idx = sp->vStaffIdx();
                        if (scr && ecr && (scr->vStaffIdx() != idx || ecr->vStaffIdx() != idx))
                              continue;
                        spanner.push_back(sp);
                        }
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

                        if (d->autoplace()) {
                              d->autoplaceSegmentElement(false);
                              dynamics.push_back(d);
                              }
                        }
                  else if (e->isFiguredBass()) {
                        e->layout();
                        e->autoplaceSegmentElement();
                        }
                  }
            }

      // add dynamics shape to skyline

      for (Dynamic* d : dynamics) {
            if (!d->addToSkyline())
                  continue;
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

      if (!hasFretDiagram) {
            layoutHarmonies(sl);
            alignHarmonies(system, sl, true, styleP(Sid::maxChordShiftAbove), styleP(Sid::maxChordShiftBelow));
            }

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
            alignHarmonies(system, sl, false, styleP(Sid::maxFretShiftAbove), styleP(Sid::maxFretShiftBelow));
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
      // layout Voltas for current system
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
                        if (ss->autoplace() && ss->isStyled(Pid::OFFSET))
                              ss->rypos() = y;
                        if (ss->addToSkyline())
                              system->staff(staffIdx)->skyline().add(ss->shape().translated(ss->pos()));
                        }

                  voltaSegments.erase(voltaSegments.begin(), voltaSegments.begin() + idx);
                  }
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

      //-------------------------------------------------------------
      // Image
      //-------------------------------------------------------------

      for (const Segment* s : sl) {
            for (Element* e : s->annotations()) {
                  if (e->isImage())
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
      qreal footerExtension = page->footerExtension();
      qreal headerExtension = page->headerExtension();
      qreal headerFooterPadding = score->styleP(Sid::staffHeaderFooterPadding);
      qreal endY = page->height() - page->bm();
      qreal y = 0.0;

      System* nextSystem = 0;
      int systemIdx = -1;

      // re-calculate positions for systems before current
      // (they may have been filled on previous layout)
      int pSystems = page->systems().size();
      if (pSystems > 0) {
            page->system(0)->restoreLayout2();
            y = page->system(0)->y() + page->system(0)->height();
            }
      else {
            y = page->tm();
            }
      for (int i = 1; i < pSystems; ++i) {
            System* cs = page->system(i);
            System* ps = page->system(i - 1);
            qreal distance = ps->minDistance(cs);
            y += distance;
            cs->setPos(page->lm(), y);
            cs->restoreLayout2();
            y += cs->height();
            }

      for (;;) {
            //
            // calculate distance to previous system
            //
            qreal distance;
            if (prevSystem)
                  distance = prevSystem->minDistance(curSystem);
            else {
                  // this is the first system on page
                  if (curSystem->vbox()) {
                        // if the header exists and there is a frame, move the frame downwards
                        // to avoid collisions
                        distance = headerExtension ? headerExtension + headerFooterPadding : 0.0;
                        }
                  else {
                        distance = score->styleP(Sid::staffUpperBorder);
                        bool fixedDistance = false;
                        // TODO: curSystem->spacerDistance(true)
                        for (MeasureBase* mb : curSystem->measures()) {
                              if (mb->isMeasure()) {
                                    Measure* m = toMeasure(mb);
                                    Spacer* sp = m->vspacerUp(0);       // TODO: first visible?
                                    if (sp) {
                                          if (sp->spacerType() == SpacerType::FIXED) {
                                                distance = sp->gap();
                                                fixedDistance = true;
                                                break;
                                                }
                                          else
                                                distance = qMax(distance, sp->gap());
                                          }
                                    }
                              }
                        if (!fixedDistance) {
                              qreal top = curSystem->minTop();
                              // ensure it doesn't collide with header
                              if (headerExtension > 0.0)
                                    top += headerExtension + headerFooterPadding;
                              distance = qMax(distance, top);
                              }
                        }
                  }

            y += distance;
            curSystem->setPos(page->lm(), y);
            curSystem->restoreLayout2();
            page->appendSystem(curSystem);
            y += curSystem->height();

            //
            //  check for page break or if next system will fit on page
            //
            bool collected = false;
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
                  if (nextSystem)
                        collected = true;
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
                  if (vbox) {
                        if (footerExtension > 0)
                              dist += footerExtension;
                        }
                  else if (!prevSystem->hasFixedDownDistance()) {
                        qreal margin = qMax(curSystem->minBottom(), curSystem->spacerDistance(false));
                        // ensure it doesn't collide with footer
                        if (footerExtension > 0)
                              margin += footerExtension + headerFooterPadding;
                        dist += qMax(margin, slb);
                        }
                  breakPage = (y + dist) >= endY && breakPages;
                  }
            if (breakPage) {
                  qreal dist = qMax(prevSystem->minBottom(), prevSystem->spacerDistance(false));
                  qreal footerPadding = 0.0;
                  // ensure it doesn't collide with footer
                  if (footerExtension > 0) {
                        footerPadding = footerExtension + headerFooterPadding;
                        dist += footerPadding;
                        }
                  dist = qMax(dist, slb);
                  layoutPage(page, endY - (y + dist), footerPadding);
                  // if we collected a system we cannot fit onto this page,
                  // we need to collect next page in order to correctly set system positions
                  if (collected)
                        pageOldMeasure = nullptr;
                  break;
                  }
            }

      Fraction stick = Fraction(-1,1);
      for (System* s : page->systems()) {
            Score* currentScore = s->score();
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
                                    if (notTopBeam(cr))                 // layout cross staff beams
                                          cr->beam()->layout();
                                    if (notTopTuplet(cr)) {
                                          // fix layout of tuplets
                                          DurationElement* de = cr;
                                          while (de->tuplet() && de->tuplet()->elements().front() == de) {
                                                Tuplet* t = de->tuplet();
                                                t->layout();
                                                de = t;
                                                }
                                          }

                                    if (cr->isChord()) {
                                          Chord* c = toChord(cr);
                                          for (Chord* cc : c->graceNotes()) {
                                                if (cc->beam() && cc->beam()->elements().front() == cc)
                                                      cc->beam()->layout();
                                                cc->layoutSpanners();
                                                for (Element* element : cc->el()) {
                                                      if (element->isSlur())
                                                            element->layout();
                                                      }
                                                }
                                          c->layoutArpeggio2();
                                          c->layoutSpanners();
                                          if (c->tremolo()) {
                                                Tremolo* t = c->tremolo();
                                                Chord* c1 = t->chord1();
                                                Chord* c2 = t->chord2();
                                                if (t->twoNotes() && c1 && c2 && (c1->staffMove() || c2->staffMove()))
                                                      t->layout();
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

      if (score->systemMode()) {
            System* s = page->systems().last();
            qreal height = s ? s->pos().y() + s->height() + s->minBottom() : page->tm();
            page->bbox().setRect(0.0, 0.0, score->loWidth(), height + page->bm());
            }

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
//   CmdStateLocker
//---------------------------------------------------------

class CmdStateLocker {
      Score* score;
   public:
      CmdStateLocker(Score* s) : score(s) { score->cmdState().lock(); }
      ~CmdStateLocker() { score->cmdState().unlock(); }
      };

//---------------------------------------------------------
//   doLayoutRange
//---------------------------------------------------------

void Score::doLayoutRange(const Fraction& st, const Fraction& et)
      {
      CmdStateLocker cmdStateLocker(this);
      LayoutContext lc(this);

      Fraction stick(st);
      Fraction etick(et);
      Q_ASSERT(!(stick == Fraction(-1,1) && etick == Fraction(-1,1)));

      if (!last() || (lineMode() && !firstMeasure())) {
            qDebug("empty score");
            qDeleteAll(_systems);
            _systems.clear();
            qDeleteAll(pages());
            pages().clear();
            lc.getNextPage();
            return;
            }
//      if (!_systems.isEmpty())
//            return;
      bool layoutAll = stick <= Fraction(0,1) && (etick < Fraction(0,1) || etick >= masterScore()->last()->endTick());
      if (stick < Fraction(0,1))
            stick = Fraction(0,1);
      if (etick < Fraction(0,1))
            etick = last()->endTick();

      lc.endTick     = etick;
      _scoreFont     = ScoreFont::fontFactory(style().value(Sid::MusicalSymbolFont).toString());
      _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, spatium() / SPATIUM20);

      if (cmdState().layoutFlags & LayoutFlag::REBUILD_MIDI_MAPPING) {
            if (isMaster())
                  masterScore()->rebuildMidiMapping();
            }
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

      if (lineMode()) {
            lc.prevMeasure = 0;
            lc.nextMeasure = m;     //_showVBox ? first() : firstMeasure();
            lc.startTick   = m->tick();
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
                  const MeasureBase* mb = lc.nextMeasure->prev();
                  if (mb)
                        mb = mb->findPotentialSectionBreak();
                  LayoutBreak* sectionBreak = mb->sectionBreakElement();
                  // TODO: also use mb in else clause here?
                  // probably not, only actual measures have meaningful numbers
                  if (sectionBreak && sectionBreak->startWithMeasureOne())
                        lc.measureNo = 0;
                  else
                        lc.measureNo = lc.nextMeasure->prevMeasure()->no()                     // will be adjusted later with respect
                                       + (lc.nextMeasure->prevMeasure()->irregular() ? 0 : 1); // to the user-defined offset.
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

            for (System* s : qAsConst(_systems)) {
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
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LayoutContext::layout()
      {
      MeasureBase* lmb;
      do {
            getNextPage();
            collectPage();

            if (page && !page->systems().isEmpty())
                  lmb = page->systems().back()->measures().back();
            else
                  lmb = nullptr;

            // we can stop collecting pages when:
            // 1) we reach the end of score (curSystem is nullptr)
            // or
            // 2) we have fully processed the range and reached a point of stability:
            //    a) we have completed layout for the range (rangeDone is true)
            //    b) we haven't collected a system that will need to go on the next page
            //    c) this page ends with the same measure as the previous layout
            //    pageOldMeasure will be last measure from previous layout if range was completed on or before this page
            //    it will be nullptr if this page was never laid out or if we collected a system for next page
            } while (curSystem && !(rangeDone && lmb == pageOldMeasure));
            // && page->system(0)->measures().back()->tick() > endTick // FIXME: perhaps the first measure was meant? Or last system?

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
//   LayoutContext::LayoutContext
//---------------------------------------------------------

LayoutContext::LayoutContext(Score* s)
      : score(s)
      {
      firstSystemIndent = score && score->styleB(Sid::enableIndentationOnFirstSystem);
      }

//---------------------------------------------------------
//   LayoutContext::~LayoutContext
//---------------------------------------------------------

LayoutContext::~LayoutContext()
      {
      for (Spanner* s : processedSpanners)
            s->layoutSystemsDone();

      for (MuseScoreView* v : score->getViewer())
            v->layoutChanged();
      }

//---------------------------------------------------------
//   VerticalStretchData
//      defines a gap ABOVE the staff.
//---------------------------------------------------------

VerticalGapData::VerticalGapData(bool first, System *sys, Staff *st, SysStaff *sst, Spacer* nextSpacer, qreal y)
      : _fixedHeight(first), system(sys), sysStaff(sst), staff(st)
      {
      if (_fixedHeight) {
            _normalisedSpacing = system->score()->styleP(Sid::staffUpperBorder);
            _maxActualSpacing = _normalisedSpacing;
            }
      else {
            _normalisedSpacing = system->y() + (sysStaff ? sysStaff->bbox().y() : 0.0) - y;
            _maxActualSpacing = system->score()->styleP(Sid::maxStaffSpread);

            Spacer* spacer { staff ? system->upSpacer(staff->idx(), nextSpacer) : nullptr };

            if (spacer) {
                  _fixedSpacer = spacer->spacerType() == SpacerType::FIXED;
                  _normalisedSpacing = qMax(_normalisedSpacing, spacer->gap());
                  if (_fixedSpacer) {
                        _maxActualSpacing = _normalisedSpacing;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   updateFactor
//---------------------------------------------------------

void VerticalGapData::updateFactor(qreal factor)
      {
      if (_fixedHeight)
            return;
      qreal f = qMax(factor, _factor);
      _normalisedSpacing *= _factor / f;
      _factor = f;
      }

//---------------------------------------------------------
//   addSpaceBetweenSections
//---------------------------------------------------------

void VerticalGapData::addSpaceBetweenSections()
      {
      updateFactor(system->score()->styleD(Sid::spreadSystem));
      if (!(_fixedHeight | _fixedSpacer))
            _maxActualSpacing = system->score()->styleP(Sid::maxSystemSpread) / _factor;
      }

//---------------------------------------------------------
//   addSpaceAroundVBox
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundVBox(bool above)
      {
      _fixedHeight = true;
      _factor = 1.0;
      const Score* score { system->score() };
      _normalisedSpacing = above ? score->styleP(Sid::frameSystemDistance) : score->styleP(Sid::systemFrameDistance);
      _maxActualSpacing = _normalisedSpacing / _factor;
      }

//---------------------------------------------------------
//   addSpaceAroundNormalBracket
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundNormalBracket()
      {
      updateFactor(system->score()->styleD(Sid::spreadSquareBracket));
      }

//---------------------------------------------------------
//   addSpaceAroundCurlyBracket
//---------------------------------------------------------

void VerticalGapData::addSpaceAroundCurlyBracket()
      {
      updateFactor(system->score()->styleD(Sid::spreadCurlyBracket));
      }

//---------------------------------------------------------
//   insideCurlyBracket
//---------------------------------------------------------

void VerticalGapData::insideCurlyBracket()
      {
      _maxActualSpacing = system->score()->styleP(Sid::maxAkkoladeDistance) / _factor;
      }

//---------------------------------------------------------
//   factor
//---------------------------------------------------------

qreal VerticalGapData::factor() const
      {
      return _factor;
      }

//---------------------------------------------------------
//   spacing
//    return normalised spacing
//---------------------------------------------------------

qreal VerticalGapData::spacing() const
      {
      return _normalisedSpacing + _addedNormalisedSpace;
      }

//---------------------------------------------------------
//   addedSpace
//---------------------------------------------------------

qreal VerticalGapData::actualAddedSpace() const
      {
      return _addedNormalisedSpace * factor();
      }

//---------------------------------------------------------
//   addSpacing
//---------------------------------------------------------

qreal VerticalGapData::addSpacing(qreal step)
      {
      if (_fixedHeight | _fixedSpacer)
            return 0.0;
      if (_normalisedSpacing >= _maxActualSpacing) {
            _normalisedSpacing = _maxActualSpacing;
            step = 0.0;
            }
      else {
            qreal newSpacing { _normalisedSpacing + _addedNormalisedSpace + step };
            if ((newSpacing >= _maxActualSpacing))
                  step = _maxActualSpacing - _normalisedSpacing - _addedNormalisedSpace;
            }
      _addedNormalisedSpace += step;
      _lastStep = step;
      return step;
      }

//---------------------------------------------------------
//   isFixedHeight
//---------------------------------------------------------

bool VerticalGapData::isFixedHeight() const
      {
      return _fixedHeight || almostZero(_normalisedSpacing - _maxActualSpacing);
      }

//---------------------------------------------------------
//   undoLastAddSpacing
//---------------------------------------------------------

void VerticalGapData::undoLastAddSpacing()
      {
      _addedNormalisedSpace -= _lastStep;
      _lastStep = 0.0;
      }

//---------------------------------------------------------
//   addFillSpacing
//---------------------------------------------------------

qreal VerticalGapData::addFillSpacing(qreal step, qreal maxFill)
      {
      if (_fixedSpacer)
            return 0.0;
      qreal actStep { ((step + _fillSpacing / _factor) > maxFill) ? (maxFill - _fillSpacing / _factor) : step};
      qreal res = addSpacing(actStep);
      _fillSpacing += res * _factor;
      return res;
      }

//---------------------------------------------------------
//   deleteAll
//---------------------------------------------------------

void VerticalGapDataList::deleteAll()
      {
      for (auto vsd : *this)
            delete vsd;
      }

//---------------------------------------------------------
//   sumStretchFactor
//---------------------------------------------------------

qreal VerticalGapDataList::sumStretchFactor() const
      {
      qreal sum { 0.0 };
      for (VerticalGapData* vsd : *this) {
            if (!vsd->isFixedHeight())
                  sum += vsd->factor();
            }
      return sum;
      }

//---------------------------------------------------------
//   smallest
//---------------------------------------------------------

qreal VerticalGapDataList::smallest(qreal limit) const
      {
      VerticalGapData* vdp { nullptr };
      for (VerticalGapData* vgd : *this) {
            if (vgd->isFixedHeight())
                  continue;
            if ((qCeil(limit) == qCeil(vgd->spacing())))
                  continue;
            if (!vdp || (vgd->spacing() <  vdp->spacing()))
                  vdp = vgd;
            }
      return vdp ? vdp->spacing() : 0.0;
      }
}

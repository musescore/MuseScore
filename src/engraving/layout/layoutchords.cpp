/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "layoutchords.h"

#include "containers.h"

#include "libmscore/accidental.h"
#include "libmscore/chord.h"
#include "libmscore/hook.h"
#include "libmscore/note.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/stemslash.h"
#include "libmscore/tie.h"
#include "libmscore/glissando.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   layoutSegmentElements
//---------------------------------------------------------

static void layoutSegmentElements(Segment* segment, track_idx_t startTrack, track_idx_t endTrack)
{
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        if (EngravingItem* e = segment->element(track)) {
            e->layout();
        }
    }
}

//---------------------------------------------------------
//   layoutChords1
//    - layout upstem and downstem chords
//    - offset as necessary to avoid conflict
//---------------------------------------------------------

void LayoutChords::layoutChords1(Score* score, Segment* segment, staff_idx_t staffIdx)
{
    const Staff* staff = score->Score::staff(staffIdx);
    const track_idx_t startTrack = staffIdx * VOICES;
    const track_idx_t endTrack   = startTrack + VOICES;
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
    double nominalWidth = score->noteHeadWidth() * staff->staffMag(tick);
    double maxUpWidth   = 0.0;
    double maxDownWidth = 0.0;
    double maxUpMag     = 0.0;
    double maxDownMag   = 0.0;

    // dots and hooks can affect layout of notes as well as vice versa
    int upDots         = 0;
    int downDots       = 0;
    bool upHooks       = false;
    bool downHooks     = false;

    // also check for grace notes
    bool upGrace       = false;
    bool downGrace     = false;

    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* e = segment->element(track);
        if (e && e->isChord()) {
            Chord* chord = toChord(e);
            if (chord->beam() && chord->beam()->cross()) {
                crossBeamFound = true;
            }
            bool hasGraceBefore = false;
            for (Chord* c : chord->graceNotes()) {
                if (c->isGraceBefore()) {
                    hasGraceBefore = true;
                }
                layoutChords2(c->notes(), c->up());               // layout grace note noteheads
                layoutChords3(score->style(), c->notes(), staff, 0);              // layout grace note chords
            }
            if (chord->up()) {
                ++upVoices;
                upStemNotes.insert(upStemNotes.end(), chord->notes().begin(), chord->notes().end());
                upDots   = std::max(upDots, chord->dots());
                maxUpMag = std::max(maxUpMag, chord->mag());
                if (!upHooks) {
                    upHooks = chord->hook();
                }
                if (hasGraceBefore) {
                    upGrace = true;
                }
            } else {
                ++downVoices;
                downStemNotes.insert(downStemNotes.end(), chord->notes().begin(), chord->notes().end());
                downDots = std::max(downDots, chord->dots());
                maxDownMag = std::max(maxDownMag, chord->mag());
                if (!downHooks) {
                    downHooks = chord->hook();
                }
                if (hasGraceBefore) {
                    downGrace = true;
                }
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
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        if (upVoices) {
            double hw = layoutChords2(upStemNotes, true);
            maxUpWidth = std::max(maxUpWidth, hw);
        }

        // layout downstem noteheads
        if (downVoices > 1) {
            std::sort(downStemNotes.begin(), downStemNotes.end(),
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        if (downVoices) {
            double hw = layoutChords2(downStemNotes, false);
            maxDownWidth = std::max(maxDownWidth, hw);
        }

        double sp                 = staff->spatium(tick);
        double upOffset           = 0.0;          // offset to apply to upstem chords
        double downOffset         = 0.0;          // offset to apply to downstem chords
        double dotAdjust          = 0.0;          // additional chord offset to account for dots
        double dotAdjustThreshold = 0.0;          // if it exceeds this amount

        // centering adjustments for whole note, breve, and small chords
        double centerUp          = 0.0;          // offset to apply in order to center upstem chords
        double oversizeUp        = 0.0;          // adjustment to oversized upstem chord needed if laid out to the right
        double centerDown        = 0.0;          // offset to apply in order to center downstem chords
        double centerAdjustUp    = 0.0;          // adjustment to upstem chord needed after centering donwstem chord
        double centerAdjustDown  = 0.0;          // adjustment to downstem chord needed after centering upstem chord

        // only center chords if they differ from nominal by at least this amount
        // this avoids unnecessary centering on differences due only to floating point roundoff
        // it also allows for the possibility of disabling centering
        // for notes only "slightly" larger than nominal, like half notes
        // but this will result in them not being aligned with each other between voices
        // unless you change to left alignment as described in the comments below
        double centerThreshold   = 0.01 * sp;

        // amount by which actual width exceeds nominal, adjusted for staff mag() only
        double headDiff = maxUpWidth - nominalWidth;
        // amount by which actual width exceeds nominal, adjusted for staff & chord/note mag()
        double headDiff2 = maxUpWidth - nominalWidth * (maxUpMag / staff->staffMag(tick));
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
        } else if (-headDiff > centerThreshold) {
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
        } else if (-headDiff > centerThreshold) {
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
            if (bottomUpNote->chord()->staffMove() == topDownNote->chord()->staffMove() && !crossBeamFound) {
                separation = topDownNote->line() - bottomUpNote->line();
            } else {
                separation = 2;           // no conflict
            }
            std::vector<Note*> overlapNotes;
            overlapNotes.reserve(8);

            if (separation == 1) {
                // second
                downOffset = maxUpWidth;
                // align stems if present, leave extra room if not
                if (topDownNote->chord()->stem() && bottomUpNote->chord()->stem()) {
                    downOffset -= topDownNote->chord()->stem()->lineWidth();
                } else {
                    downOffset += 0.1 * sp;
                }
            } else if (separation < 1) {
                // overlap (possibly unison)

                // build list of overlapping notes
                for (size_t i = 0, n = upStemNotes.size(); i < n; ++i) {
                    if (upStemNotes[i]->line() >= topDownNote->line() - 1) {
                        overlapNotes.push_back(upStemNotes[i]);
                    } else {
                        break;
                    }
                }
                for (size_t i = downStemNotes.size(); i > 0; --i) {         // loop most probably needs to be in this reverse order
                    if (downStemNotes[i - 1]->line() <= bottomUpNote->line() + 1) {
                        overlapNotes.push_back(downStemNotes[i - 1]);
                    } else {
                        break;
                    }
                }
                std::sort(overlapNotes.begin(), overlapNotes.end(),
                          [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });

                // determine nature of overlap
                bool shareHeads = true;               // can all overlapping notes share heads?
                bool matchPending = false;            // looking for a unison match
                bool conflictUnison = false;          // unison found
                bool conflictSecondUpHigher = false;              // second found
                bool conflictSecondDownHigher = false;            // second found
                int lastLine = 1000;
                Note* p = overlapNotes[0];
                for (size_t i = 0, count = overlapNotes.size(); i < count; ++i) {
                    Note* n = overlapNotes[i];
                    NoteHeadType nHeadType;
                    NoteHeadType pHeadType;
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
                        nHeadType = (n->headType() == NoteHeadType::HEAD_AUTO) ? n->chord()->durationType().headType() : n->headType();
                        pHeadType = (p->headType() == NoteHeadType::HEAD_AUTO) ? p->chord()->durationType().headType() : p->headType();
                        // the most important rules for sharing noteheads on unisons between voices are
                        // that notes must be one same line with same tpc
                        // noteheads must be unmirrored and of same group
                        // and chords must be same size (or else sharing code won't work)
                        if (n->headGroup() != p->headGroup() || n->tpc() != p->tpc() || n->mirror() || p->mirror()
                            || nchord->isSmall() != pchord->isSmall()) {
                            shareHeads = false;
                        } else {
                            // noteheads are potentially shareable
                            // it is more subjective at this point
                            // current default is to require *either* of the following:
                            //    1) both chords have same number of dots, both have stems, and both noteheads are same type and are full size (automatic match)
                            // or 2) one or more of the noteheads is not of type AUTO, but is explicitly set to match the other (user-forced match)
                            // or 3) exactly one of the noteheads is invisible (user-forced match)
                            // thus user can force notes to be shared despite differing number of dots or either being stemless
                            // by setting one of the notehead types to match the other or by making one notehead invisible
                            // TODO: consider adding a style option, staff properties, or note property to control sharing
                            if ((nchord->dots() != pchord->dots() || !nchord->stem() || !pchord->stem() || nHeadType != pHeadType
                                 || n->isSmall() || p->isSmall())
                                && ((n->headType() == NoteHeadType::HEAD_AUTO && p->headType() == NoteHeadType::HEAD_AUTO)
                                    || nHeadType != pHeadType)
                                && (n->visible() == p->visible())) {
                                shareHeads = false;
                            }
                        }
                        break;
                    case 1:
                        // second
                        // trust that this won't be a problem for single unison
                        if (separation < 0) {
                            if (n->chord()->up()) {
                                conflictSecondUpHigher = true;
                            } else {
                                conflictSecondDownHigher = true;
                            }
                            shareHeads = false;
                        }
                        break;
                    default:
                        // no conflict
                        if (matchPending) {
                            shareHeads = false;
                        }
                        matchPending = true;
                    }
                    p = n;
                    lastLine = line;
                }
                if (matchPending) {
                    shareHeads = false;
                }

                // calculate offsets
                if (shareHeads) {
                    for (int i = static_cast<int>(overlapNotes.size()) - 1; i >= 1; i -= 2) {
                        Note* previousNote = overlapNotes[i - 1];
                        Note* n = overlapNotes[i];
                        if (!(previousNote->chord()->isNudged() || n->chord()->isNudged())) {
                            if (previousNote->chord()->dots() == n->chord()->dots()) {
                                // hide one set dots
                                bool onLine = !(previousNote->line() & 1);
                                if (onLine) {
                                    // hide dots for lower voice
                                    if (previousNote->voice() & 1) {
                                        previousNote->setDotsHidden(true);
                                    } else {
                                        n->setDotsHidden(true);
                                    }
                                } else {
                                    // hide dots for upper voice
                                    if (!(previousNote->voice() & 1)) {
                                        previousNote->setDotsHidden(true);
                                    } else {
                                        n->setDotsHidden(true);
                                    }
                                }
                            }
                            // formerly we hid noteheads in an effort to fix playback
                            // but this doesn't work for cases where noteheads cannot be shared
                            // so better to solve the problem elsewhere
                        }
                    }
                } else if (conflictUnison && separation == 0 && (!downGrace || upGrace)) {
                    downOffset = maxUpWidth + 0.3 * sp;
                } else if (conflictUnison) {
                    upOffset = maxDownWidth + 0.3 * sp;
                } else if (conflictSecondUpHigher) {
                    upOffset = maxDownWidth + 0.2 * sp;
                } else if ((downHooks && !upHooks) && !(upDots && !downDots)) {
                    downOffset = maxUpWidth + 0.3 * sp;
                } else if (conflictSecondDownHigher) {
                    if (downDots && !upDots) {
                        downOffset = maxUpWidth + 0.3 * sp;
                    } else {
                        upOffset = maxDownWidth - 0.2 * sp;
                        if (downHooks) {
                            upOffset += 0.3 * sp;
                        }
                    }
                } else {
                    // no direct conflict, so parts can overlap (downstem on left)
                    // just be sure that stems clear opposing noteheads
                    double clearLeft = 0.0, clearRight = 0.0;
                    if (topDownNote->chord()->stem()) {
                        clearLeft = topDownNote->chord()->stem()->lineWidth() + 0.3 * sp;
                    }
                    if (bottomUpNote->chord()->stem()) {
                        clearRight = bottomUpNote->chord()->stem()->lineWidth() + std::max(maxDownWidth - maxUpWidth, 0.0) + 0.3 * sp;
                    } else {
                        downDots = 0;             // no need to adjust for dots in this case
                    }
                    upOffset = std::max(clearLeft, clearRight);
                    if (downHooks) {
                        // we will need more space to avoid collision with hook
                        // but we won't need as much dot adjustment
                        upOffset = std::max(upOffset, maxDownWidth + 0.1 * sp);
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
                double mag;
                if (upDots) {
                    dots = upDots;
                    mag = maxUpMag;
                } else {
                    dots = downDots;
                    mag = maxDownMag;
                }
                double dotWidth = segment->symWidth(SymId::augmentationDot);
                // first dot
                dotAdjust = score->styleMM(Sid::dotNoteDistance) + dotWidth;
                // additional dots
                if (dots > 1) {
                    dotAdjust += score->styleMM(Sid::dotDotDistance).val() * (dots - 1);
                }
                dotAdjust *= mag;
                // only by amount over threshold
                dotAdjust = std::max(dotAdjust - dotAdjustThreshold, 0.0);
            }
            if (separation == 1) {
                dotAdjust += 0.1 * sp;
            }
        }

        // apply chord offsets
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* e = segment->element(track);
            if (e && e->isChord()) {
                Chord* chord = toChord(e);
                if (chord->up()) {
                    if (upOffset != 0.0) {
                        chord->movePosX(upOffset + centerAdjustUp + oversizeUp);
                        if (downDots && !upDots) {
                            chord->movePosX(dotAdjust);
                        }
                    } else {
                        chord->movePosX(centerUp);
                    }
                } else {
                    if (downOffset != 0.0) {
                        chord->movePosX(downOffset + centerAdjustDown);
                        if (upDots && !downDots) {
                            chord->movePosX(dotAdjust);
                        }
                    } else {
                        chord->movePosX(centerDown);
                    }
                }
            }
        }

        // layout chords
        std::vector<Note*> notes;
        if (upVoices) {
            notes.insert(notes.end(), upStemNotes.begin(), upStemNotes.end());
        }
        if (downVoices) {
            notes.insert(notes.end(), downStemNotes.begin(), downStemNotes.end());
        }
        if (upVoices + downVoices > 1) {
            std::sort(notes.begin(), notes.end(),
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        layoutChords3(score->style(), notes, staff, segment);
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

double LayoutChords::layoutChords2(std::vector<Note*>& notes, bool up)
{
    int startIdx, endIdx, incIdx;
    double maxWidth = 0.0;

    // loop in correct direction so that first encountered notehead wins conflict
    if (up) {
        // loop bottom up
        startIdx = 0;
        endIdx = int(notes.size());
        incIdx = 1;
    } else {
        // loop top down
        startIdx = int(notes.size()) - 1;
        endIdx = -1;
        incIdx = -1;
    }

    int ll        = 1000;           // line of previous notehead
                                    // hack: start high so first note won't show as conflict
    bool lvisible = false;          // was last note visible?
    bool mirror   = false;          // should current notehead be mirrored?
                                    // value is retained and may be used on next iteration
                                    // to track mirror status of previous note
    bool isLeft   = notes[startIdx]->chord()->up();               // is notehead on left?
    int lmove     = notes[startIdx]->chord()->staffMove();        // staff offset of last note (for cross-staff beaming)

    for (int idx = startIdx; idx != endIdx; idx += incIdx) {
        Note* note    = notes[idx];                         // current note
        int line      = note->line();                       // line of current note
        Chord* chord  = note->chord();
        int move      = chord->staffMove();                 // staff offset of current note

        // there is a conflict
        // if this is same or adjacent line as previous note (and chords are on same staff!)
        // but no need to do anything about it if either note is invisible
        bool conflict = (std::abs(ll - line) < 2) && (lmove == move) && note->visible() && lvisible;

        // this note is on opposite side of stem as previous note
        // if there is a conflict
        // or if this the first note *after* a conflict
        if (conflict || (chord->up() != isLeft)) {
            isLeft = !isLeft;
        }

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
        chord->setPosX(0.0);

        // let user mirror property override the default we calculated
        if (note->userMirror() == DirectionH::AUTO) {
            mirror = nmirror;
        } else {
            mirror = note->chord()->up();
            if (note->userMirror() == DirectionH::LEFT) {
                mirror = !mirror;
            }
        }
        note->setMirror(mirror);

        // accumulate return value
        if (!mirror) {
            maxWidth = std::max(maxWidth, note->bboxRightPos());
        }

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
    double x;            // actual x position of this accidental relative to origin
    double top;          // top of accidental bbox relative to staff
    double bottom;       // bottom of accidental bbox relative to staff
    int line;           // line of note
    int next;           // index of next accidental of same pitch class (ascending list)
    double width;        // width of accidental
    double ascent;       // amount (in sp) vertical strokes extend above body
    double descent;      // amount (in sp) vertical strokes extend below body
    double rightClear;   // amount (in sp) to right of last vertical stroke above body
    double leftClear;    // amount (in sp) to left of last vertical stroke below body
};

//---------------------------------------------------------
//   resolveAccidentals
//    lx = calculated position of rightmost edge of left accidental relative to origin
//---------------------------------------------------------

static bool resolveAccidentals(AcEl* left, AcEl* right, double& lx, double pd, double sp)
{
    AcEl* upper;
    AcEl* lower;
    if (left->line >= right->line) {
        upper = right;
        lower = left;
    } else {
        upper = left;
        lower = right;
    }

    double gap = lower->top - upper->bottom;

    // no conflict at all if there is sufficient vertical gap between accidentals
    // the arrangement of accidentals into columns assumes accidentals an octave apart *do* clear
    if (gap >= pd || lower->line - upper->line >= 7) {
        return false;
    }

    double allowableOverlap = std::max(upper->descent, lower->ascent) - pd;

    // accidentals that are "close" (small gap or even slight overlap)
    if (std::abs(gap) <= 0.33 * sp) {
        // acceptable with slight offset
        // if one of the accidentals can subsume the overlap
        // and both accidentals allow it
        if (-gap <= allowableOverlap && std::min(upper->descent, lower->ascent) > 0.0) {
            double align = std::min(left->width, right->width);
            lx = std::min(lx, right->x + align - pd);
            return true;
        }
    }

    // amount by which overlapping accidentals will be separated
    // for example, the vertical stems of two flat signs
    // these need more space than we would need between non-overlapping accidentals
    double overlapShift = pd * 1.41;

    // accidentals with more significant overlap
    // acceptable if one accidental can subsume overlap
    if (left == lower && -gap <= allowableOverlap) {
        double offset = std::max(left->rightClear, right->leftClear);
        offset = std::min(offset, left->width) - overlapShift;
        lx = std::min(lx, right->x + offset);
        return true;
    }

    // accidentals with even more overlap
    // can work if both accidentals can subsume overlap
    if (left == lower && -gap <= upper->descent + lower->ascent - pd) {
        double offset = std::min(left->rightClear, right->leftClear) - overlapShift;
        if (offset > 0.0) {
            lx = std::min(lx, right->x + offset);
            return true;
        }
    }

    // otherwise, there is real conflict
    lx = std::min(lx, right->x - pd);
    return true;
}

//---------------------------------------------------------
//   layoutAccidental
//---------------------------------------------------------

static std::pair<double, double> layoutAccidental(const MStyle& style, AcEl* me, AcEl* above, AcEl* below, double colOffset,
                                                  std::vector<Note*>& leftNotes, double pnd,
                                                  double pd, double sp)
{
    double lx = colOffset;
    Accidental* acc = me->note->accidental();
    double mag = acc->mag();
    pnd *= mag;
    pd *= mag;

    Chord* chord = me->note->chord();
    Staff* staff = chord->staff();
    Fraction tick = chord->tick();

    // extra space for ledger lines
    double ledgerAdjust = 0.0;
    double ledgerVerticalClear = 0.0;
    bool ledgerAbove = chord->upNote()->line() <= -2;
    bool ledgerBelow = chord->downNote()->line() >= staff->lines(tick) * 2;
    if (ledgerAbove || ledgerBelow) {
        // ledger lines are present
        // check for collision with lines above & below staff
        // note that on 1-line staff, both collisions are possible at once
        // TODO: account for cutouts in accidental
        double lds = staff->lineDistance(tick) * sp;
        if ((ledgerAbove && me->top + lds <= pnd) || (ledgerBelow && staff->lines(tick) * lds - me->bottom <= pnd)) {
            ledgerAdjust = -style.styleS(Sid::ledgerLineLength).val() * sp;
            ledgerVerticalClear = style.styleS(Sid::ledgerLineWidth).val() * 0.5 * sp;
            lx = std::min(lx, ledgerAdjust);
        }
    }

    // clear left notes
    size_t lns = leftNotes.size();
    for (size_t i = 0; i < lns; ++i) {
        Note* ln = leftNotes[i];
        int lnLine = ln->line();
        double lnTop = (lnLine - 1) * 0.5 * sp;
        double lnBottom = lnTop + sp;
        if (me->top - lnBottom <= pnd && lnTop - me->bottom <= pnd) {
            double lnLedgerAdjust = 0.0;
            if (lnLine <= -2 || lnLine >= staff->lines(tick) * 2) {
                // left note has a ledger line we probably need to clear horizontally as well
                // except for accidentals that clear the last extended ledger line vertically
                // in these cases, the accidental may tuck closer
                Note* lastLnNote = lnLine < 0 ? leftNotes[0] : leftNotes[lns - 1];
                int lastLnLine = lastLnNote->line();
                double ledgerY = (lastLnLine / 2) * sp;
                if (me->line < 0 && ledgerY - me->bottom < ledgerVerticalClear) {
                    lnLedgerAdjust = ledgerAdjust;
                } else if (me->line > 0 && me->top - ledgerY < ledgerVerticalClear) {
                    lnLedgerAdjust = ledgerAdjust;
                }
            }
            // undercut note above if possible
            if (lnBottom - me->top <= me->ascent - pnd) {
                lx = std::min(lx, ln->x() + ln->chord()->x() + lnLedgerAdjust + me->rightClear);
            } else {
                lx = std::min(lx, ln->x() + ln->chord()->x() + lnLedgerAdjust);
            }
        } else if (lnTop > me->bottom) {
            break;
        }
    }

    // clear other accidentals
    bool conflictAbove = false;
    bool conflictBelow = false;

    if (above) {
        conflictAbove = resolveAccidentals(me, above, lx, pd, sp);
    }
    if (below) {
        conflictBelow = resolveAccidentals(me, below, lx, pd, sp);
    }
    if (conflictAbove || conflictBelow) {
        me->x = lx - acc->width() - acc->bbox().x();
    } else if (colOffset != 0.0) {
        me->x = lx - pd - acc->width() - acc->bbox().x();
    } else {
        me->x = lx - pnd - acc->width() - acc->bbox().x();
    }

    return std::pair<double, double>(me->x, me->x + me->width);
}

//---------------------------------------------------------
//   layoutChords3
//    - calculate positions of notes, accidentals, dots
//---------------------------------------------------------

void LayoutChords::layoutChords3(const MStyle& style, std::vector<Note*>& notes, const Staff* staff, Segment* segment)
{
    //---------------------------------------------------
    //    layout accidentals
    //    find column for dots
    //---------------------------------------------------

    std::vector<Note*> leftNotes;   // notes to left of origin
    leftNotes.reserve(8);
    std::vector<AcEl> aclist;         // accidentals
    aclist.reserve(8);

    // track columns of octave-separated accidentals
    int columnBottom[7] = { -1, -1, -1, -1, -1, -1, -1 };

    Fraction tick      =  notes.front()->chord()->segment()->tick();
    double sp           = staff->spatium(tick);
    double stepDistance = sp * staff->lineDistance(tick) * .5;
    int stepOffset     = staff->staffType(tick)->stepOffset();

    double lx           = 10000.0;    // leftmost notehead position
    double upDotPosX    = 0.0;
    double downDotPosX  = 0.0;

    int nNotes = int(notes.size());
    int nAcc = 0;
    for (int i = nNotes - 1; i >= 0; --i) {
        Note* note     = notes[i];
        Accidental* ac = note->accidental();
        if (ac && !note->fixed()) {
            ac->layout();
            if (!ac->visible()) {
                ac->setPos(ac->bbox().x() - ac->width(), 0.0);
            } else {
                AcEl acel;
                acel.note   = note;
                int line    = note->line();
                acel.line   = line;
                acel.x      = 0.0;
                acel.top    = line * 0.5 * sp + ac->bbox().top();
                acel.bottom = line * 0.5 * sp + ac->bbox().bottom();
                acel.width  = ac->width();
                PointF bboxNE = ac->symBbox(ac->symbol()).topRight();
                PointF bboxSW = ac->symBbox(ac->symbol()).bottomLeft();
                PointF cutOutNE = ac->symSmuflAnchor(ac->symbol(), SmuflAnchorId::cutOutNE);
                PointF cutOutSW = ac->symSmuflAnchor(ac->symbol(), SmuflAnchorId::cutOutSW);
                if (!cutOutNE.isNull()) {
                    acel.ascent     = cutOutNE.y() - bboxNE.y();
                    acel.rightClear = bboxNE.x() - cutOutNE.x();
                } else {
                    acel.ascent     = 0.0;
                    acel.rightClear = 0.0;
                }

                if (!cutOutSW.isNull()) {
                    acel.descent   = bboxSW.y() - cutOutSW.y();
                    acel.leftClear = cutOutSW.x() - bboxSW.x();
                } else {
                    acel.descent   = 0.0;
                    acel.leftClear = 0.0;
                }

                int pitchClass = (line + 700) % 7;
                acel.next = columnBottom[pitchClass];
                columnBottom[pitchClass] = nAcc;
                aclist.push_back(acel);
                ++nAcc;
            }
        }

        Chord* chord = note->chord();
        bool _up     = chord->up();

        if (chord->stemSlash()) {
            chord->stemSlash()->layout();
        }

        double overlapMirror;
        Stem* stem = chord->stem();
        if (stem) {
            overlapMirror = stem->lineWidth();
        } else if (chord->durationType().headType() == NoteHeadType::HEAD_WHOLE) {
            overlapMirror = style.styleMM(Sid::stemWidth) * chord->mag();
        } else {
            overlapMirror = 0.0;
        }

        double x = 0.0;
        if (note->mirror()) {
            if (_up) {
                x = chord->stemPosX() - overlapMirror;
            } else {
                x = -note->headBodyWidth() + overlapMirror;
            }
        } else if (_up) {
            x = chord->stemPosX() - note->headBodyWidth();
        }

        double ny = (note->line() + stepOffset) * stepDistance;
        if (note->ypos() != ny) {
            note->setPosY(ny);
            if (chord->stem()) {
                chord->stem()->layout();
                if (chord->hook()) {
                    chord->hook()->setPosY(chord->stem()->flagPosition().y());
                }
            }
        }
        note->setPosX(x);

        // find leftmost non-mirrored note to set as X origin for accidental layout
        // a mirrored note that extends to left of segment X origin
        // will displace accidentals only if there is conflict
        double sx = x + chord->x();     // segment-relative X position of note
        if (note->mirror() && !chord->up() && sx < 0.0) {
            leftNotes.push_back(note);
        } else if (sx < lx) {
            lx = sx;
        }

        double xx = x + note->headBodyWidth() + chord->pos().x();

        DirectionV dotPosition = note->userDotPosition();
        if (chord->dots()) {
            if (chord->up()) {
                upDotPosX = std::max(upDotPosX, xx);
            } else {
                downDotPosX = std::max(downDotPosX, xx);
            }

            if (dotPosition == DirectionV::AUTO && nNotes > 1 && note->visible() && !note->dotsHidden()) {
                // resolve dot conflicts
                int line = note->line();
                Note* above = (i < nNotes - 1) ? notes[i + 1] : 0;
                if (above && (!above->visible() || above->dotsHidden())) {
                    above = 0;
                }
                int intervalAbove = above ? line - above->line() : 1000;
                Note* below = (i > 0) ? notes[i - 1] : 0;
                if (below && (!below->visible() || below->dotsHidden())) {
                    below = 0;
                }
                int intervalBelow = below ? below->line() - line : 1000;
                if ((line & 1) == 0) {
                    // line
                    if (intervalAbove == 1 && intervalBelow != 1) {
                        dotPosition = DirectionV::DOWN;
                    } else if (intervalBelow == 1 && intervalAbove != 1) {
                        dotPosition = DirectionV::UP;
                    } else if (intervalAbove == 0 && above->chord()->dots()) {
                        // unison
                        if (((above->voice() & 1) == (note->voice() & 1))) {
                            above->setDotY(DirectionV::UP);
                            dotPosition = DirectionV::DOWN;
                        }
                    }
                } else {
                    // space
                    if (intervalAbove == 0 && above->chord()->dots()) {
                        // unison
                        if (!(note->voice() & 1)) {
                            dotPosition = DirectionV::UP;
                        } else {
                            if (!(above->voice() & 1)) {
                                above->setDotY(DirectionV::UP);
                            } else {
                                dotPosition = DirectionV::DOWN;
                            }
                        }
                    }
                }
            }
        }
        note->setDotY(dotPosition);      // also removes invalid dots
    }

    // if there are no non-mirrored notes in a downstem chord,
    // then use the stem X position as X origin for accidental layout
    if (nNotes && static_cast<int>(leftNotes.size()) == nNotes) {
        lx = notes.front()->chord()->stemPosX();
    }

    if (segment) {
        // align all dots for segment/staff
        // it would be possible to dots for up & down chords separately
        // this would require space to have been allocated previously
        // when calculating chord offsets
        segment->setDotPosX(staff->idx(), std::max(upDotPosX, downDotPosX));
    }

    if (nAcc == 0) {
        return;
    }

    std::vector<int> umi;
    double pd  = style.styleMM(Sid::accidentalDistance);
    double pnd = style.styleMM(Sid::accidentalNoteDistance);
    double colOffset = 0.0;

    if (nAcc >= 2 && aclist[nAcc - 1].line - aclist[0].line >= 7) {
        // accidentals spread over an octave or more
        // set up columns for accidentals with octave matches
        // these will start at right and work to the left
        // unmatched accidentals will use zig zag approach (see below)
        // starting to the left of the octave columns

        int columnTop[7] = { -1, -1, -1, -1, -1, -1, -1 };

        // find columns of octaves
        for (int pc = 0; pc < 7; ++pc) {
            if (columnBottom[pc] == -1) {
                continue;
            }
            // calculate column height
            for (int j = columnBottom[pc]; j != -1; j = aclist[j].next) {
                columnTop[pc] = j;
            }
        }

        // compute reasonable column order
        // use zig zag
        std::vector<int> column;
        std::vector<int> unmatched;
        int n = nAcc - 1;
        for (int i = 0; i <= n; ++i, --n) {
            int pc = (aclist[i].line + 700) % 7;
            if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                if (!mu::contains(column, pc)) {
                    column.push_back(pc);
                }
            } else {
                unmatched.push_back(i);
            }
            if (i == n) {
                break;
            }
            pc = (aclist[n].line + 700) % 7;
            if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                if (!mu::contains(column, pc)) {
                    column.push_back(pc);
                }
            } else {
                unmatched.push_back(n);
            }
        }
        size_t nColumns = column.size();
        size_t nUnmatched = unmatched.size();

        // handle unmatched accidentals
        for (size_t i = 0; i < nUnmatched; ++i) {
            // first try to slot it into an existing column
            AcEl* me = &aclist[unmatched[i]];
            // find column
            bool found = false;
            for (size_t j = 0; j < nColumns; ++j) {
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
                double myPd = pd * me->note->accidental()->mag();
                bool conflict = false;
                if (above != -1 && me->top - aclist[above].bottom < myPd) {
                    conflict = true;
                } else if (below != -1 && aclist[below].top - me->bottom < myPd) {
                    conflict = true;
                }
                if (!conflict) {
                    // insert into column
                    found = true;
                    me->next = above;
                    if (above == -1) {
                        columnTop[pc] = unmatched[i];
                    }
                    if (below != -1) {
                        aclist[below].next = unmatched[i];
                    } else {
                        columnBottom[pc] = unmatched[i];
                    }
                    break;
                }
            }
            // if no slot found, then add to list of unmatched accidental indices
            if (!found) {
                umi.push_back(unmatched[i]);
            }
        }
        nAcc = static_cast<int>(umi.size());
        if (nAcc > 1) {
            std::sort(umi.begin(), umi.end());
        }

        bool alignLeft = style.styleB(Sid::alignAccidentalsLeft);

        // through columns
        for (size_t i = 0; i < nColumns; ++i) {
            // column index
            const int pc = column[i];

            double minX = 0.0;
            double maxX = 0.0;
            AcEl* below = 0;
            // through accidentals in this column
            for (int j = columnBottom[pc]; j != -1; j = aclist[j].next) {
                std::pair<double, double> x = layoutAccidental(style, &aclist[j], 0, below, colOffset, leftNotes, pnd, pd, sp);
                minX = std::min(minX, x.first);
                maxX = std::min(maxX, x.second);
                below = &aclist[j];
            }

            // align
            int next = -1;
            for (int j = columnBottom[pc]; j != -1; j = next) {
                AcEl* current = &aclist[j];
                next = current->next;
                if (next != -1 && current->line == aclist[next].line) {
                    continue;
                }

                if (alignLeft) {
                    current->x = minX;
                } else {
                    current->x = maxX - current->width;
                }
            }
            colOffset = minX;
        }
    } else {
        for (int i = 0; i < nAcc; ++i) {
            umi.push_back(i);
        }
    }

    if (nAcc) {
        // for accidentals with no octave matches, use zig zag approach
        // layout right to left in pairs, (next) highest then lowest

        AcEl* me = &aclist[umi[0]];
        AcEl* above = 0;
        AcEl* below = 0;

        // layout top accidental
        layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);

        // layout bottom accidental
        int n = nAcc - 1;
        if (n > 0) {
            above = me;
            me = &aclist[umi[n]];
            layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);
        }

        // layout middle accidentals
        if (n > 1) {
            for (int i = 1; i < n; ++i, --n) {
                // next highest
                below = me;
                me = &aclist[umi[i]];
                layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);
                if (i == n - 1) {
                    break;
                }
                // next lowest
                above = me;
                me = &aclist[umi[n - 1]];
                layoutAccidental(style, me, above, below, colOffset, leftNotes, pnd, pd, sp);
            }
        }
    }

    for (const AcEl& e : aclist) {
        // even though we initially calculate accidental position relative to segment
        // we must record pos for accidental relative to note,
        // since pos is always interpreted relative to parent
        Note* note = e.note;
        double x    = e.x + lx - (note->x() + note->chord()->x());
        note->accidental()->setPos(x, 0);
    }
}

/* updateGraceNotes()
 * Processes a full measure, making sure that all grace notes are
 * attacched to the correct segment. Has to be performed after
 * all the segments are known.
 * */
void LayoutChords::updateGraceNotes(Measure* measure)
{
    Score* score = measure->score();
    for (Segment& s : measure->segments()) { // Clean everything
        for (unsigned track = 0; track < score->staves().size() * VOICES; ++track) {
            EngravingItem* item = s.preAppendedItem(track);
            if (item && item->isGraceNotesGroup()) {
                s.clearPreAppended(track);
            }
        }
    }

    for (Segment& s : measure->segments()) { // Attach grace notes to appropriate segment
        if (!s.isChordRestType()) {
            continue;
        }
        for (auto el : s.elist()) {
            if (el && el->isChord()) {
                appendGraceNotes(toChord(el));
            }
        }
    }
}

void LayoutChords::appendGraceNotes(Chord* chord)
{
    Segment* segment = chord->segment();
    Measure* measure = chord->measure();
    track_idx_t track = chord->track();
    staff_idx_t staffIdx = chord->staffIdx();
    GraceNotesGroup& gnb = chord->graceNotesBefore();
    GraceNotesGroup& gna = chord->graceNotesAfter();

    //Attach graceNotesBefore of this chord to *this* segment
    if (!gnb.empty()) {
        // If this segment already contains grace notes in the same voice (could happen if a
        // previous chord has appended grace-notes-after here) put them in the same vector.
        EngravingItem* item = segment->preAppendedItem(static_cast<int>(track));
        if (item && item->isGraceNotesGroup()) {
            GraceNotesGroup* gng = toGraceNotesGroup(item);
            gng->insert(gng->end(), gnb.begin(), gnb.end());
        } else {
            segment->preAppend(&gnb, static_cast<int>(track));
        }
        segment->createShape(staffIdx);
    }

    //Attach graceNotesAfter of this chord to the *following* segment
    if (!gna.empty()) {
        Segment* followingSeg = measure->tick2segment(segment->tick() + chord->actualTicks(), SegmentType::All);
        while (followingSeg && !followingSeg->hasElements(staff2track(staffIdx), staff2track(staffIdx) + 3)) {
            // If there is nothing on this staff, go to next segment
            followingSeg = followingSeg->next();
        }
        if (followingSeg) {
            followingSeg->preAppend(&gna, static_cast<int>(track));
            followingSeg->createShape(staffIdx);
        }
    }
}

/* Grace-notes-after have the special property of belonging to
*  a segment but being pre-appended to another. This repositioning
*  is needed and must be called AFTER horizontal spacing is calculated. */
void LayoutChords::repositionGraceNotesAfter(Segment* segment)
{
    size_t tracks = segment->score()->staves().size() * VOICES;
    for (size_t track = 0; track < tracks; track++) {
        EngravingItem* item = segment->preAppendedItem(static_cast<int>(track));
        if (!item || !item->isGraceNotesGroup()) {
            continue;
        }
        GraceNotesGroup* gng = toGraceNotesGroup(item);
        for (Chord* chord : *gng) {
            double offset = segment->xpos() - chord->parentItem()->parentItem()->xpos();
            // Difference between the segment they "belong" and the segment they are "appended" to.
            chord->setPos(chord->pos().x() + offset, 0.0);
        }
    }
}

void LayoutChords::updateLineAttachPoints(Measure* measure)
{
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        for (EngravingItem* e : s.elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                n->lineAttachPoints().clear();
            }
            for (Chord* ch : c->graceNotes()) {
                for (Note* n : ch->notes()) {
                    n->lineAttachPoints().clear();
                }
            }
        }
    }
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        for (EngravingItem* e : s.elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            doUpdateLineAttachPoints(c);
            for (Chord* gn : c->graceNotes()) {
                doUpdateLineAttachPoints(gn);
            }
        }
    }
    for (Segment& s : measure->segments()) {
        s.createShapes();
    }
}

/* We perform a pre-layout of ties and glissandi to obtain the attach points and attach them to
 * the notes of the chord. Will be needed for spacing calculation, particularly to
 * enforce minTieLength. The true layout of ties and glissandi is done much later. */
void LayoutChords::doUpdateLineAttachPoints(Chord* chord)
{
    if (chord->endsGlissando()) {
        for (Note* note : chord->notes()) {
            for (Spanner* sp : note->spannerBack()) {
                if (sp->isGlissando()) {
                    Glissando* gliss = toGlissando(sp);
                    if (gliss->startElement() && gliss->startElement()->isNote()) {
                        Note* startNote = toNote(gliss->startElement());
                        if (startNote->findMeasure() == chord->measure()) {
                            gliss->layout(); // line attach points are updated here
                        }
                    }
                }
            }
        }
    }
    for (Note* note : chord->notes()) {
        Tie* tie = note->tieFor();
        if (tie) {
            Note* endNote = tie->endNote();
            if (endNote && endNote->findMeasure() == note->findMeasure()) {
                tie->layoutFor(note->findMeasure()->system()); // line attach points are updated here
            }
        }
    }
}

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

#include <cmath>
#include <QtMath>

#include "style/style.h"

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
#include "mmrestrange.h"
#include "mmrest.h"
#include "mscore.h"
#include "notedot.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "measurerepeat.h"
#include "score.h"
#include "scorefont.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stem.h"
#include "stemslash.h"
#include "sticking.h"
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

#include "masterscore.h"

using namespace mu;

namespace Ms {
// #define PAGE_DEBUG

#ifdef PAGE_DEBUG
#define PAGEDBG(...)  qDebug(__VA_ARGS__)
#else
#define PAGEDBG(...)  ;
#endif

//---------------------------------------------------------
//   almostZero
//---------------------------------------------------------

static bool inline almostZero(qreal value)
{
    // 1e-3 is close enough to zero to see it as zero.
    return value > -1e-3 && value < 1e-3;
}

//---------------------------------------------------------
//   rebuildBspTree
//---------------------------------------------------------

void Score::rebuildBspTree()
{
    for (Page* page : pages()) {
        page->rebuildBspTree();
    }
}

//---------------------------------------------------------
//   layoutSegmentElements
//---------------------------------------------------------

static void layoutSegmentElements(Segment* segment, int startTrack, int endTrack)
{
    for (int track = startTrack; track < endTrack; ++track) {
        if (Element* e = segment->element(track)) {
            e->layout();
        }
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
    if (!chord) {
        return true;
    } else if (!chord->beam() || !chord->beam()->cross()) {
        return chord->up();
    } else {
        // cross-staff beam: we cannot know the actual direction of this chord until the beam layout,
        // but that's too late - it won't work to lay out as if the chord is up on pass one but then down on pass two
        // so just assign a logical direction based on attributes that won't change
        // so chords can be laid out consistently on both passes
        bool up;
        if (chord->stemDirection() != Direction::AUTO) {
            up = chord->stemDirection() == Direction::UP;
        } else if (chord->staffMove()) {
            up = chord->staffMove() > 0;
        } else if (chord->track() < chord->beam()->track()) {
            up = false;
        } else if (chord->track() > chord->beam()->track()) {
            up = true;
        } else if (chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks())) {
            up = !(chord->track() % 2);
        } else {
            up = !chord->staff()->isTop();
        }
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
    qreal nominalWidth = noteHeadWidth() * staff->staffMag(tick);
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
            if (chord->beam() && chord->beam()->cross()) {
                crossBeamFound = true;
            }
            bool hasGraceBefore = false;
            for (Chord* c : chord->graceNotes()) {
                if (c->isGraceBefore()) {
                    hasGraceBefore = true;
                }
                layoutChords2(c->notes(), c->up());               // layout grace note noteheads
                layoutChords3(c->notes(), staff, 0);              // layout grace note chords
            }
            if (chord->up()) {
                ++upVoices;
                upStemNotes.insert(upStemNotes.end(), chord->notes().begin(), chord->notes().end());
                upDots   = qMax(upDots, chord->dots());
                maxUpMag = qMax(maxUpMag, chord->mag());
                if (!upHooks) {
                    upHooks = chord->hook();
                }
                if (hasGraceBefore) {
                    upGrace = true;
                }
            } else {
                ++downVoices;
                downStemNotes.insert(downStemNotes.end(), chord->notes().begin(), chord->notes().end());
                downDots = qMax(downDots, chord->dots());
                maxDownMag = qMax(maxDownMag, chord->mag());
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
            qreal hw = layoutChords2(upStemNotes, true);
            maxUpWidth = qMax(maxUpWidth, hw);
        }

        // layout downstem noteheads
        if (downVoices > 1) {
            std::sort(downStemNotes.begin(), downStemNotes.end(),
                      [](Note* n1, const Note* n2) ->bool { return n1->line() > n2->line(); });
        }
        if (downVoices) {
            qreal hw = layoutChords2(downStemNotes, false);
            maxDownWidth = qMax(maxDownWidth, hw);
        }

        qreal sp                 = staff->spatium(tick);
        qreal upOffset           = 0.0;          // offset to apply to upstem chords
        qreal downOffset         = 0.0;          // offset to apply to downstem chords
        qreal dotAdjust          = 0.0;          // additional chord offset to account for dots
        qreal dotAdjustThreshold = 0.0;          // if it exceeds this amount

        // centering adjustments for whole note, breve, and small chords
        qreal centerUp          = 0.0;          // offset to apply in order to center upstem chords
        qreal oversizeUp        = 0.0;          // adjustment to oversized upstem chord needed if laid out to the right
        qreal centerDown        = 0.0;          // offset to apply in order to center downstem chords
        qreal centerAdjustUp    = 0.0;          // adjustment to upstem chord needed after centering donwstem chord
        qreal centerAdjustDown  = 0.0;          // adjustment to downstem chord needed after centering upstem chord

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
        qreal headDiff2 = maxUpWidth - nominalWidth * (maxUpMag / staff->staffMag(tick));
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
            QVector<Note*> overlapNotes;
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
                        overlapNotes.append(upStemNotes[i]);
                    } else {
                        break;
                    }
                }
                for (size_t i = downStemNotes.size(); i > 0; --i) {         // loop most probably needs to be in this reverse order
                    if (downStemNotes[i - 1]->line() <= bottomUpNote->line() + 1) {
                        overlapNotes.append(downStemNotes[i - 1]);
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
                                && ((n->headType() == NoteHead::Type::HEAD_AUTO && p->headType() == NoteHead::Type::HEAD_AUTO)
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
                    for (int i = overlapNotes.size() - 1; i >= 1; i -= 2) {
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
                    qreal clearLeft = 0.0, clearRight = 0.0;
                    if (topDownNote->chord()->stem()) {
                        clearLeft = topDownNote->chord()->stem()->lineWidth() + 0.3 * sp;
                    }
                    if (bottomUpNote->chord()->stem()) {
                        clearRight = bottomUpNote->chord()->stem()->lineWidth() + qMax(maxDownWidth - maxUpWidth, 0.0) + 0.3 * sp;
                    } else {
                        downDots = 0;             // no need to adjust for dots in this case
                    }
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
                } else {
                    dots = downDots;
                    mag = maxDownMag;
                }
                qreal dotWidth = segment->symWidth(SymId::augmentationDot);
                // first dot
                dotAdjust = styleP(Sid::dotNoteDistance) + dotWidth;
                // additional dots
                if (dots > 1) {
                    dotAdjust += styleP(Sid::dotDotDistance) * (dots - 1);
                }
                dotAdjust *= mag;
                // only by amount over threshold
                dotAdjust = qMax(dotAdjust - dotAdjustThreshold, 0.0);
            }
            if (separation == 1) {
                dotAdjust += 0.1 * sp;
            }
        }

        // apply chord offsets
        for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (e && e->isChord()) {
                Chord* chord = toChord(e);
                if (chord->up()) {
                    if (upOffset != 0.0) {
                        chord->rxpos() += upOffset + centerAdjustUp + oversizeUp;
                        if (downDots && !upDots) {
                            chord->rxpos() += dotAdjust;
                        }
                    } else {
                        chord->rxpos() += centerUp;
                    }
                } else {
                    if (downOffset != 0.0) {
                        chord->rxpos() += downOffset + centerAdjustDown;
                        if (upDots && !downDots) {
                            chord->rxpos() += dotAdjust;
                        }
                    } else {
                        chord->rxpos() += centerDown;
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
        bool conflict = (qAbs(ll - line) < 2) && (lmove == move) && note->visible() && lvisible;

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
        chord->rxpos() = 0.0;

        // let user mirror property override the default we calculated
        if (note->userMirror() == MScore::DirectionH::AUTO) {
            mirror = nmirror;
        } else {
            mirror = note->chord()->up();
            if (note->userMirror() == MScore::DirectionH::LEFT) {
                mirror = !mirror;
            }
        }
        note->setMirror(mirror);

        // accumulate return value
        if (!mirror) {
            maxWidth = qMax(maxWidth, note->bboxRightPos());
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
    qreal x;            // actual x position of this accidental relative to origin
    qreal top;          // top of accidental bbox relative to staff
    qreal bottom;       // bottom of accidental bbox relative to staff
    int line;           // line of note
    int next;           // index of next accidental of same pitch class (ascending list)
    qreal width;        // width of accidental
    qreal ascent;       // amount (in sp) vertical strokes extend above body
    qreal descent;      // amount (in sp) vertical strokes extend below body
    qreal rightClear;   // amount (in sp) to right of last vertical stroke above body
    qreal leftClear;    // amount (in sp) to left of last vertical stroke below body
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
    } else {
        upper = left;
        lower = right;
    }

    qreal gap = lower->top - upper->bottom;

    // no conflict at all if there is sufficient vertical gap between accidentals
    // the arrangement of accidentals into columns assumes accidentals an octave apart *do* clear
    if (gap >= pd || lower->line - upper->line >= 7) {
        return false;
    }

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

static QPair<qreal, qreal> layoutAccidental(AcEl* me, AcEl* above, AcEl* below, qreal colOffset, QVector<Note*>& leftNotes, qreal pnd,
                                            qreal pd, qreal sp)
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
                Note* lastLnNote = lnLine < 0 ? leftNotes[0] : leftNotes[lns - 1];
                int lastLnLine = lastLnNote->line();
                qreal ledgerY = (lastLnLine / 2) * sp;
                if (me->line < 0 && ledgerY - me->bottom < ledgerVerticalClear) {
                    lnLedgerAdjust = ledgerAdjust;
                } else if (me->line > 0 && me->top - ledgerY < ledgerVerticalClear) {
                    lnLedgerAdjust = ledgerAdjust;
                }
            }
            // undercut note above if possible
            if (lnBottom - me->top <= me->ascent - pnd) {
                lx = qMin(lx, ln->x() + ln->chord()->x() + lnLedgerAdjust + me->rightClear);
            } else {
                lx = qMin(lx, ln->x() + ln->chord()->x() + lnLedgerAdjust);
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

    return QPair<qreal, qreal>(me->x, me->x + me->width);
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

    QVector<Note*> leftNotes;   // notes to left of origin
    leftNotes.reserve(8);
    QVector<AcEl> aclist;         // accidentals
    aclist.reserve(8);

    // track columns of octave-separated accidentals
    int columnBottom[7] = { -1, -1, -1, -1, -1, -1, -1 };

    Fraction tick      =  notes.front()->chord()->segment()->tick();
    qreal sp           = staff->spatium(tick);
    qreal stepDistance = sp * staff->lineDistance(tick) * .5;
    int stepOffset     = staff->staffType(tick)->stepOffset();

    qreal lx           = 10000.0;    // leftmost notehead position
    qreal upDotPosX    = 0.0;
    qreal downDotPosX  = 0.0;

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
                aclist.append(acel);
                ++nAcc;
            }
        }

        Chord* chord = note->chord();
        bool _up     = chord->up();

        if (chord->stemSlash()) {
            chord->stemSlash()->layout();
        }

        qreal overlapMirror;
        Stem* stem = chord->stem();
        if (stem) {
            overlapMirror = stem->lineWidth();
        } else if (chord->durationType().headType() == NoteHead::Type::HEAD_WHOLE) {
            overlapMirror = styleP(Sid::stemWidth) * chord->mag();
        } else {
            overlapMirror = 0.0;
        }

        qreal x = 0.0;
        if (note->mirror()) {
            if (_up) {
                x = chord->stemPosX() - overlapMirror;
            } else {
                x = -note->headBodyWidth() + overlapMirror;
            }
        } else if (_up) {
            x = chord->stemPosX() - note->headBodyWidth();
        }

        qreal ny = (note->line() + stepOffset) * stepDistance;
        if (note->rypos() != ny) {
            note->rypos() = ny;
            if (chord->stem()) {
                chord->stem()->layout();
                if (chord->hook()) {
                    chord->hook()->rypos() = chord->stem()->hookPos().y();
                }
            }
        }
        note->rxpos()  = x;

        // find leftmost non-mirrored note to set as X origin for accidental layout
        // a mirrored note that extends to left of segment X origin
        // will displace accidentals only if there is conflict
        qreal sx = x + chord->x();     // segment-relative X position of note
        if (note->mirror() && !chord->up() && sx < 0.0) {
            leftNotes.append(note);
        } else if (sx < lx) {
            lx = sx;
        }

        qreal xx = x + note->headBodyWidth() + chord->pos().x();

        Direction dotPosition = note->userDotPosition();
        if (chord->dots()) {
            if (chord->up()) {
                upDotPosX = qMax(upDotPosX, xx);
            } else {
                downDotPosX = qMax(downDotPosX, xx);
            }

            if (dotPosition == Direction::AUTO && nNotes > 1 && note->visible() && !note->dotsHidden()) {
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
                        dotPosition = Direction::DOWN;
                    } else if (intervalBelow == 1 && intervalAbove != 1) {
                        dotPosition = Direction::UP;
                    } else if (intervalAbove == 0 && above->chord()->dots()) {
                        // unison
                        if (((above->voice() & 1) == (note->voice() & 1))) {
                            above->setDotY(Direction::UP);
                            dotPosition = Direction::DOWN;
                        }
                    }
                } else {
                    // space
                    if (intervalAbove == 0 && above->chord()->dots()) {
                        // unison
                        if (!(note->voice() & 1)) {
                            dotPosition = Direction::UP;
                        } else {
                            if (!(above->voice() & 1)) {
                                above->setDotY(Direction::UP);
                            } else {
                                dotPosition = Direction::DOWN;
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
    if (nNotes && leftNotes.size() == nNotes) {
        lx = notes.front()->chord()->stemPosX();
    }

    if (segment) {
        // align all dots for segment/staff
        // it would be possible to dots for up & down chords separately
        // this would require space to have been allocated previously
        // when calculating chord offsets
        segment->setDotPosX(staff->idx(), qMax(upDotPosX, downDotPosX));
    }

    if (nAcc == 0) {
        return;
    }

    QVector<int> umi;
    qreal pd  = styleP(Sid::accidentalDistance);
    qreal pnd = styleP(Sid::accidentalNoteDistance);
    qreal colOffset = 0.0;

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
        QVector<int> column;
        QVector<int> unmatched;
        int n = nAcc - 1;
        for (int i = 0; i <= n; ++i, --n) {
            int pc = (aclist[i].line + 700) % 7;
            if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                if (!column.contains(pc)) {
                    column.append(pc);
                }
            } else {
                unmatched.append(i);
            }
            if (i == n) {
                break;
            }
            pc = (aclist[n].line + 700) % 7;
            if (aclist[columnTop[pc]].line != aclist[columnBottom[pc]].line) {
                if (!column.contains(pc)) {
                    column.append(pc);
                }
            } else {
                unmatched.append(n);
            }
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
        nAcc = umi.size();
        if (nAcc > 1) {
            std::sort(umi.begin(), umi.end());
        }

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
                if (i == n - 1) {
                    break;
                }
                // next lowest
                above = me;
                me = &aclist[umi[n - 1]];
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
            if (track == tracks - 1) {
                size_t n = segment->annotations().size();
                for (size_t i = 0; i < n; ++i) {
                    segment->annotations().at(i)->layout();
                }
            }
            Element* e = segment->element(track);
            if (e && e->isChord()) {
                Chord* c = toChord(segment->element(track));
                c->layoutStem();
                for (Note* n : c->notes()) {
                    Tie* tie = n->tieFor();
                    if (tie) {
                        tie->layout();
                    }
                    for (Spanner* sp : n->spannerFor()) {
                        sp->layout();
                    }
                }
            }
        }
    }
    rebuildBspTree();
}

#endif

//---------------------------------------------------------
//   connectTies
///   Rebuild tie connections.
//---------------------------------------------------------

void Score::connectTies(bool silent)
{
    int tracks = nstaves() * VOICES;
    Measure* m = firstMeasure();
    if (!m) {
        return;
    }

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = m->first(st); s; s = s->next1(st)) {
        for (int i = 0; i < tracks; ++i) {
            Element* e = s->element(i);
            if (e == 0 || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                // connect a tie without end note
                Tie* tie = n->tieFor();
                if (tie && !tie->endNote()) {
                    Note* nnote;
                    if (_mscVersion <= 114) {
                        nnote = searchTieNote114(n);
                    } else {
                        nnote = searchTieNote(n);
                    }
                    if (nnote == 0) {
                        if (!silent) {
                            qDebug("next note at %d track %d for tie not found (version %d)", s->tick().ticks(), i, _mscVersion);
                            delete tie;
                            n->setTieFor(0);
                        }
                    } else {
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
                        } else {
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
                    if (!element) {
                        continue;
                    }
                    if (!element->isChord()) {
                        qDebug("cannot connect tremolo");
                    } else {
                        Chord* nc = toChord(element);
                        nc->setTremolo(tremolo);
                        tremolo->setChords(c, nc);
                        // cross-measure tremolos are not supported
                        // but can accidentally result from copy & paste
                        // remove them now
                        if (c->measure() != nc->measure()) {
                            c->remove(tremolo);
                        }
                    }
                    break;
                }
            }
#endif
        }
    }
}

//---------------------------------------------------------
//   relayoutForStyles
///   some styles can't properly apply if score hasn't been laid out yet,
///   so temporarily disable them and then reenable after layout
///   (called during score load)
//---------------------------------------------------------

void Score::relayoutForStyles()
{
    std::vector<Sid> stylesToTemporarilyDisable;

    for (Sid sid : { Sid::createMultiMeasureRests, Sid::mrNumberSeries }) {
        // only necessary if boolean style is true
        if (styleB(sid)) {
            stylesToTemporarilyDisable.push_back(sid);
        }
    }

    if (!stylesToTemporarilyDisable.empty()) {
        for (Sid sid : stylesToTemporarilyDisable) {
            style().set(sid, false); // temporarily disable
        }
        doLayout();
        for (Sid sid : stylesToTemporarilyDisable) {
            style().set(sid, true); // and immediately reenable
        }
    }
}

//---------------------------------------------------------
//   Spring
//---------------------------------------------------------

struct Spring {
    int seg;
    qreal stretch;
    qreal fix;
    Spring(int i, qreal s, qreal f)
        : seg(i), stretch(s), fix(f) {}
};

typedef std::multimap<qreal, Spring, std::less<qreal> > SpringMap;

//---------------------------------------------------------
//   sff2
//    compute 1/Force for a given Extend
//---------------------------------------------------------

static qreal sff2(qreal width, qreal xMin, const SpringMap& springs)
{
    if (width <= xMin) {
        return 0.0;
    }
    auto i = springs.begin();
    qreal c  = i->second.stretch;
    if (c == 0.0) {           //DEBUG
        c = 1.1;
    }
    qreal f = 0.0;
    for (; i != springs.end();) {
        xMin -= i->second.fix;
        f = (width - xMin) / c;
        ++i;
        if (i == springs.end() || f <= i->first) {
            break;
        }
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
    qreal width[n - 1];
    int ticksList[n - 1];
#else
    // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
    //    heap allocation is slow, an optimization might be used.
    std::vector<qreal> width(n - 1);
    std::vector<int> ticksList(n - 1);
#endif
    int minTick = 100000;

    for (int i = 0; i < n - 1; ++i) {
        ChordRest* cr  = (*elements)[i];
        ChordRest* ncr  = (*elements)[i + 1];
        width[i]       = cr->shape().minHorizontalDistance(ncr->shape());
        ticksList[i]   = cr->ticks().ticks();
        minTick = qMin(ticksList[i], minTick);
    }

    //---------------------------------------------------
    // compute stretches
    //---------------------------------------------------

    SpringMap springs;
    qreal minimum = 0.0;
    for (int i = 0; i < n - 1; ++i) {
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
        if (stretch < i->second.fix) {
            stretch = i->second.fix;
        }
        width[i->second.seg] = stretch;
    }
    qreal x = x1;
    for (int i = 1; i < n - 1; ++i) {
        x += width[i - 1];
        ChordRest* cr = (*elements)[i];
        qreal dx = x - cr->segment()->pos().x();
        cr->rxpos() += dx;
    }
}

//---------------------------------------------------------
//   createMMRest
//    create a multimeasure rest
//    from firstMeasure to lastMeasure (inclusive)
//---------------------------------------------------------

void Score::createMMRest(Measure* firstMeasure, Measure* lastMeasure, const Fraction& len)
{
    int numMeasuresInMMRest = 1;
    if (firstMeasure != lastMeasure) {
        for (Measure* m = firstMeasure->nextMeasure(); m; m = m->nextMeasure()) {
            ++numMeasuresInMMRest;
            m->setMMRestCount(-1);
            if (m->mmRest()) {
                undo(new ChangeMMRest(m, 0));
            }
            if (m == lastMeasure) {
                break;
            }
        }
    }

    // mmrMeasure coexists with n undisplayed measures of rests
    Measure* mmrMeasure = firstMeasure->mmRest();
    if (mmrMeasure) {
        // reuse existing mmrest
        if (mmrMeasure->ticks() != len) {
            Segment* s = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
            // adjust length
            mmrMeasure->setTicks(len);
            // move existing end barline
            if (s) {
                s->setRtick(len);
            }
        }
        mmrMeasure->removeSystemTrailer();
    } else {
        mmrMeasure = new Measure(this);
        mmrMeasure->setTicks(len);
        mmrMeasure->setTick(firstMeasure->tick());
        undo(new ChangeMMRest(firstMeasure, mmrMeasure));
    }
    mmrMeasure->setTimesig(firstMeasure->timesig());
    mmrMeasure->setPageBreak(lastMeasure->pageBreak());
    mmrMeasure->setLineBreak(lastMeasure->lineBreak());
    mmrMeasure->setMMRestCount(numMeasuresInMMRest);
    mmrMeasure->setNo(firstMeasure->no());

    //
    // set mmrMeasure with same barline as last underlying measure
    //
    Segment* lastMeasureEndBarlineSeg = lastMeasure->findSegmentR(SegmentType::EndBarLine, lastMeasure->ticks());
    if (lastMeasureEndBarlineSeg) {
        Segment* mmrEndBarlineSeg = mmrMeasure->undoGetSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Element* e = lastMeasureEndBarlineSeg->element(staffIdx * VOICES);
            if (e) {
                bool generated = e->generated();
                if (!mmrEndBarlineSeg->element(staffIdx * VOICES)) {
                    Element* eClone = generated ? e->clone() : e->linkedClone();
                    eClone->setGenerated(generated);
                    eClone->setParent(mmrEndBarlineSeg);
                    undoAddElement(eClone);
                } else {
                    BarLine* mmrEndBarline = toBarLine(mmrEndBarlineSeg->element(staffIdx * VOICES));
                    BarLine* lastMeasureEndBarline = toBarLine(e);
                    if (!generated && !mmrEndBarline->links()) {
                        undo(new Link(mmrEndBarline, lastMeasureEndBarline));
                    }
                    if (mmrEndBarline->barLineType() != lastMeasureEndBarline->barLineType()) {
                        // change directly when generating mmrests, do not change underlying measures or follow links
                        undo(new ChangeProperty(mmrEndBarline, Pid::BARLINE_TYPE,
                                                QVariant::fromValue(lastMeasureEndBarline->barLineType()),
                                                PropertyFlags::NOSTYLE));
                        undo(new ChangeProperty(mmrEndBarline, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
                    }
                }
            }
        }
    }

    //
    // if last underlying measure ends with clef change, show same at end of mmrest
    //
    Segment* lastMeasureClefSeg = lastMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef,
                                                            lastMeasure->ticks());
    if (lastMeasureClefSeg) {
        Segment* mmrClefSeg = mmrMeasure->undoGetSegment(lastMeasureClefSeg->segmentType(), lastMeasure->endTick());
        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            const int track = staff2track(staffIdx);
            Element* e = lastMeasureClefSeg->element(track);
            if (e && e->isClef()) {
                Clef* lastMeasureClef = toClef(e);
                if (!mmrClefSeg->element(track)) {
                    Clef* mmrClef = lastMeasureClef->generated() ? lastMeasureClef->clone() : toClef(
                        lastMeasureClef->linkedClone());
                    mmrClef->setParent(mmrClefSeg);
                    undoAddElement(mmrClef);
                } else {
                    Clef* mmrClef = toClef(mmrClefSeg->element(track));
                    mmrClef->setClefType(lastMeasureClef->clefType());
                    mmrClef->setShowCourtesy(lastMeasureClef->showCourtesy());
                }
            }
        }
    }

    mmrMeasure->setRepeatStart(firstMeasure->repeatStart() || lastMeasure->repeatStart());
    mmrMeasure->setRepeatEnd(firstMeasure->repeatEnd() || lastMeasure->repeatEnd());
    mmrMeasure->setSectionBreak(lastMeasure->sectionBreak());

    //
    // copy markers to mmrMeasure
    //
    ElementList oldList = mmrMeasure->takeElements();
    ElementList newList = lastMeasure->el();
    for (Element* e : firstMeasure->el()) {
        if (e->isMarker()) {
            newList.push_back(e);
        }
    }
    for (Element* e : newList) {
        bool found = false;
        for (Element* ee : oldList) {
            if (ee->type() == e->type() && ee->subtype() == e->subtype()) {
                mmrMeasure->add(ee);
                auto i = std::find(oldList.begin(), oldList.end(), ee);
                if (i != oldList.end()) {
                    oldList.erase(i);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            mmrMeasure->add(e->clone());
        }
    }
    for (Element* e : oldList) {
        delete e;
    }
    Segment* s = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        int track = staffIdx * VOICES;
        if (s->element(track) == 0) {
            MMRest* mmr = new MMRest(this);
            mmr->setDurationType(TDuration::DurationType::V_MEASURE);
            mmr->setTicks(mmrMeasure->ticks());
            mmr->setTrack(track);
            mmr->setParent(s);
            undo(new AddElement(mmr));
        }
    }

    //
    // further check for clefs
    //
    Segment* underlyingSeg = lastMeasure->findSegmentR(SegmentType::Clef, lastMeasure->ticks());
    Segment* mmrSeg = mmrMeasure->findSegment(SegmentType::Clef, lastMeasure->endTick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Clef, lastMeasure->ticks());
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setTrailer(underlyingSeg->trailer());
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            Clef* clef = toClef(underlyingSeg->element(track));
            if (clef) {
                if (mmrSeg->element(track) == 0) {
                    mmrSeg->add(clef->clone());
                } else {
                    //TODO: check if same clef
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        undo(new RemoveElement(mmrSeg));
    }

    //
    // check for time signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::TimeSig, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            TimeSig* underlyingTimeSig = toTimeSig(underlyingSeg->element(track));
            if (underlyingTimeSig) {
                TimeSig* mmrTimeSig = toTimeSig(mmrSeg->element(track));
                if (!mmrTimeSig) {
                    mmrTimeSig = underlyingTimeSig->generated() ? underlyingTimeSig->clone() : toTimeSig(
                        underlyingTimeSig->linkedClone());
                    mmrTimeSig->setParent(mmrSeg);
                    undo(new AddElement(mmrTimeSig));
                } else {
                    mmrTimeSig->setSig(underlyingTimeSig->sig(), underlyingTimeSig->timeSigType());
                    mmrTimeSig->layout();
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        undo(new RemoveElement(mmrSeg));
    }

    //
    // check for ambitus
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::Ambitus, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::Ambitus, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Ambitus, Fraction(0, 1));
        }
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            Ambitus* underlyingAmbitus = toAmbitus(underlyingSeg->element(track));
            if (underlyingAmbitus) {
                Ambitus* mmrAmbitus = toAmbitus(mmrSeg->element(track));
                if (!mmrAmbitus) {
                    mmrAmbitus = underlyingAmbitus->clone();
                    mmrAmbitus->setParent(mmrSeg);
                    undo(new AddElement(mmrAmbitus));
                } else {
                    mmrAmbitus->initFrom(underlyingAmbitus);
                    mmrAmbitus->layout();
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        undo(new RemoveElement(mmrSeg));
    }

    //
    // check for key signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            KeySig* underlyingKeySig  = toKeySig(underlyingSeg->element(track));
            if (underlyingKeySig) {
                KeySig* mmrKeySig = toKeySig(mmrSeg->element(track));
                if (!mmrKeySig) {
                    mmrKeySig = underlyingKeySig->generated() ? underlyingKeySig->clone() : toKeySig(
                        underlyingKeySig->linkedClone());
                    mmrKeySig->setParent(mmrSeg);
                    mmrKeySig->setGenerated(true);
                    undo(new AddElement(mmrKeySig));
                } else {
                    if (!(mmrKeySig->keySigEvent() == underlyingKeySig->keySigEvent())) {
                        bool addKey = underlyingKeySig->isChange();
                        undo(new ChangeKeySig(mmrKeySig, underlyingKeySig->keySigEvent(), mmrKeySig->showCourtesy(),
                                              addKey));
                    }
                }
            }
        }
    } else if (mmrSeg) {
        mmrSeg->setEnabled(false);
        // TODO: remove elements from mmrSeg, then delete mmrSeg
        // previously we removed the segment if not empty,
        // but this resulted in "stale" keysig in mmrest after removed from underlying measure
        //undo(new RemoveElement(mmrSeg));
    }

    mmrMeasure->checkHeader();
    mmrMeasure->checkTrailer();

    //
    // check for rehearsal mark etc.
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    if (underlyingSeg) {
        // clone elements from underlying measure to mmr
        for (Element* e : underlyingSeg->annotations()) {
            // look at elements in underlying measure
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()
                  || e->isInstrumentChange())) {
                continue;
            }
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
                Element* eClone = e->linkedClone();
                eClone->setParent(s);
                undo(new AddElement(eClone));
            }
        }

        // remove stray elements (possibly leftover from a previous layout of this mmr)
        // this should not happen since the elements are linked?
        for (Element* e : s->annotations()) {
            // look at elements in mmr
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()
                  || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in underlying measure
            bool found = false;
            for (Element* ee : underlyingSeg->annotations()) {
                if (e->linkList().contains(ee)) {
                    found = true;
                    break;
                }
            }
            // remove from mmr if no match found
            if (!found) {
                undo(new RemoveElement(e));
            }
        }
    }

    MeasureBase* nm = _showVBox ? lastMeasure->next() : lastMeasure->nextMeasure();
    mmrMeasure->setNext(nm);
    mmrMeasure->setPrev(firstMeasure->prev());
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
        if (b->cross()) {
            return true;
        }

        // for beams not already considered cross,
        // consider them so here if any elements were moved up
        for (ChordRest* cr1 : b->elements()) {
            // some element moved up?
            if (cr1->staffMove() < 0) {
                return true;
            }
        }

        // not cross
        return false;
    }

    // no beam or not first element
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
        while (t->tuplet()) {
            t = t->tuplet();
        }
        // consider tuplet cross if anything moved within it
        if (t->cross()) {
            return true;
        } else {
            return false;
        }
    }

    // no tuplet or not first element
    return false;
}

//---------------------------------------------------------
//   doLayout
//    do a complete (re-) layout
//---------------------------------------------------------

void Score::doLayout()
{
    doLayoutRange(Fraction(0, 1), Fraction(-1, 1));
}

void Score::doLayoutRange(const Fraction& st, const Fraction& et)
{
    m_layout.doLayoutRange(st, et);
}
}

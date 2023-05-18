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
#include "tremololayout.h"

#include "libmscore/chord.h"
#include "libmscore/stem.h"
#include "libmscore/tremolo.h"
#include "libmscore/note.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"

#include "chordlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

void TremoloLayout::layout(Tremolo* item, LayoutContext&)
{
    item->path = item->basePath();

    item->_chord1 = toChord(item->explicitParent());
    if (!item->_chord1) {
        // palette
        if (!item->isBuzzRoll()) {
            const RectF box = item->path.boundingRect();
            item->addbbox(RectF(box.x(), box.bottom(), box.width(), item->spatium()));
        }
        return;
    }

    Note* anchor1 = item->_chord1->up() ? item->_chord1->upNote() : item->_chord1->downNote();
    Stem* stem    = item->_chord1->stem();
    double x, y, h;
    if (stem) {
        x = stem->pos().x() + stem->width() / 2 * (item->_chord1->up() ? -1.0 : 1.0);
        y = stem->pos().y();
        h = stem->length();
    } else {
        // center tremolo above note
        x = anchor1->x() + anchor1->headWidth() * 0.5;
        if (!item->twoNotes()) {
            bool hasMirroredNote = false;
            for (Note* n : item->_chord1->notes()) {
                if (n->mirror()) {
                    hasMirroredNote = true;
                    break;
                }
            }
            if (hasMirroredNote) {
                x = item->_chord1->stemPosX();
            }
        }
        y = anchor1->y();
        h = (item->score()->styleMM(Sid::tremoloNoteSidePadding).val() + item->bbox().height()) * item->_chord1->intrinsicMag();
    }

    if (item->twoNotes()) {
        layoutTwoNotesTremolo(item, x, y, h, item->spatium());
    } else {
        layoutOneNoteTremolo(item, x, y, h, item->spatium());
    }
}

//---------------------------------------------------------
//   layoutOneNoteTremolo
//---------------------------------------------------------

void TremoloLayout::layoutOneNoteTremolo(Tremolo* item, double x, double y, double h, double spatium)
{
    assert(!item->twoNotes());

    bool up = item->chord()->up();
    int upValue = up ? -1 : 1;
    double mag = item->chord()->intrinsicMag();
    spatium *= mag;

    double yOffset = h - item->score()->styleMM(Sid::tremoloOutSidePadding).val() * mag;

    int beams = item->chord()->beams();
    if (item->chord()->hook()) {
        // allow for space at the hook side of the stem (yOffset)
        // straight flags and traditional flags have different requirements because of their slopes
        // away from the stem. Straight flags have a shallower slope and a lot more space in general
        // so we can place the trem higher in that case
        bool straightFlags = item->score()->styleB(Sid::useStraightNoteFlags);
        if (straightFlags) {
            yOffset -= 0.75 * spatium;
        } else {
            // up-hooks and down-hooks are shaped differently
            yOffset -= up ? 1.5 * spatium : 1.0 * spatium;
        }
        // we need an additional offset for beams > 2 since those beams extend outwards and we don't want to adjust for that
        double beamOffset = straightFlags ? 0.75 : 0.5;
        yOffset -= beams >= 2 ? beamOffset * spatium : 0.0;
    } else if (beams) {
        yOffset -= (beams * (item->score()->styleB(Sid::useWideBeams) ? 1.0 : 0.75) - 0.25) * spatium;
    }
    yOffset -= item->isBuzzRoll() && up ? 0.5 * spatium : 0.0;
    yOffset -= up ? 0.0 : item->minHeight() * spatium / mag;
    yOffset *= upValue;

    y += yOffset;

    if (up) {
        double height = item->isBuzzRoll() ? 0 : item->minHeight();
        y = std::min(y, ((item->staff()->lines(item->tick()) - 1) - height) * spatium / mag);
    } else {
        y = std::max(y, 0.0);
    }
    item->setPos(x, y);
}

//---------------------------------------------------------
//   layoutTwoNotesTremolo
//---------------------------------------------------------

void TremoloLayout::layoutTwoNotesTremolo(Tremolo* item, double x, double y, double h, double spatium)
{
    UNUSED(x);
    UNUSED(y);
    UNUSED(h);
    UNUSED(spatium);

    // make sure both stems are in the same direction
    int up = 0;
    bool isUp = item->_up;
    if (item->_chord1->beam() && item->_chord1->beam() == item->_chord2->beam()) {
        Beam* beam = item->_chord1->beam();
        item->_up = beam->up();
        item->_direction = beam->beamDirection();
        // stem stuff is already taken care of by the beams
    } else if (!item->userModified()) {
        // user modified trems will be dealt with later
        bool hasVoices = item->_chord1->measure()->hasVoices(item->_chord1->staffIdx(), item->_chord1->tick(),
                                                             item->_chord2->tick() - item->_chord1->tick());
        if (item->_chord1->stemDirection() == DirectionV::AUTO
            && item->_chord2->stemDirection() == DirectionV::AUTO
            && item->_chord1->staffMove() == item->_chord2->staffMove()
            && !hasVoices) {
            std::vector<int> noteDistances;
            for (int distance : item->_chord1->noteDistances()) {
                noteDistances.push_back(distance);
            }
            for (int distance : item->_chord2->noteDistances()) {
                noteDistances.push_back(distance);
            }
            std::sort(noteDistances.begin(), noteDistances.end());
            up = Chord::computeAutoStemDirection(noteDistances);
            isUp = up > 0;
        } else if (item->_chord1->stemDirection() != DirectionV::AUTO) {
            isUp = item->_chord1->stemDirection() == DirectionV::UP;
        } else if (item->_chord2->stemDirection() != DirectionV::AUTO) {
            isUp = item->_chord2->stemDirection() == DirectionV::UP;
        } else if (item->_chord1->staffMove() > 0 || item->_chord2->staffMove() > 0) {
            isUp = false;
        } else if (item->_chord1->staffMove() < 0 || item->_chord2->staffMove() < 0) {
            isUp = true;
        } else if (hasVoices) {
            isUp = item->_chord1->track() % 2 == 0;
        }
        item->_up = isUp;
        item->_chord1->setUp(item->_chord1->staffMove() == 0 ? isUp : !isUp); // if on a different staff, flip stem dir
        item->_chord2->setUp(item->_chord2->staffMove() == 0 ? isUp : !isUp);

        LayoutContext ctx(item->score());
        ChordLayout::layoutStem(item->_chord1, ctx);
        ChordLayout::layoutStem(item->_chord2, ctx);
    }

    item->_layoutInfo = BeamTremoloLayout(item);
    item->_startAnchor = item->_layoutInfo.chordBeamAnchor(item->_chord1, BeamTremoloLayout::ChordBeamAnchorType::Start);
    item->_endAnchor = item->_layoutInfo.chordBeamAnchor(item->_chord2, BeamTremoloLayout::ChordBeamAnchorType::End);
    // deal with manual adjustments here and return
    PropertyValue val = item->getProperty(Pid::PLACEMENT);
    if (item->userModified()) {
        int idx = (item->_direction == DirectionV::AUTO || item->_direction == DirectionV::DOWN) ? 0 : 1;
        double startY = item->_beamFragment.py1[idx];
        double endY = item->_beamFragment.py2[idx];
        if (item->score()->styleB(Sid::snapCustomBeamsToGrid)) {
            const double quarterSpace = item->EngravingItem::spatium() / 4;
            startY = round(startY / quarterSpace) * quarterSpace;
            endY = round(endY / quarterSpace) * quarterSpace;
        }
        startY += item->pagePos().y();
        endY += item->pagePos().y();
        item->_startAnchor.setY(startY);
        item->_endAnchor.setY(endY);
        item->_layoutInfo.setAnchors(item->_startAnchor, item->_endAnchor);

        LayoutContext ctx(item->score());
        ChordLayout::layoutStem(item->_chord1, ctx);
        ChordLayout::layoutStem(item->_chord2, ctx);

        item->createBeamSegments();
        return;
    }
    item->setPosY(0.);
    std::vector<ChordRest*> chordRests{ item->chord1(), item->chord2() };
    std::vector<int> notes;
    double mag = 0.0;

    notes.clear();
    for (ChordRest* cr : chordRests) {
        double m = cr->isSmall() ? item->score()->styleD(Sid::smallNoteMag) : 1.0;
        mag = std::max(mag, m);
        if (cr->isChord()) {
            Chord* chord = toChord(cr);
            //int i = chord->staffMove();
            //_minMove = std::min(_minMove, i); todo: investigate this
            //_maxMove = std::max(_maxMove, i);

            for (int distance : chord->noteDistances()) {
                notes.push_back(distance);
            }
        }
    }

    std::sort(notes.begin(), notes.end());
    item->setMag(mag);
    item->_layoutInfo.calculateAnchors(chordRests, notes);
    item->_startAnchor = item->_layoutInfo.startAnchor();
    item->_endAnchor = item->_layoutInfo.endAnchor();
    int idx = (item->_direction == DirectionV::AUTO || item->_direction == DirectionV::DOWN) ? 0 : 1;
    item->_beamFragment.py1[idx] = item->_startAnchor.y() - item->pagePos().y();
    item->_beamFragment.py2[idx] = item->_endAnchor.y() - item->pagePos().y();
    item->createBeamSegments();
}

//---------------------------------------------------------
//   extendedStemLenWithTwoNotesTremolo
//    Goal: To extend stem of one of the chords to make the tremolo less steep
//    Returns a modified pair of stem lengths of two chords
//---------------------------------------------------------

std::pair<double, double> TremoloLayout::extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, double stemLen1, double stemLen2)
{
    const double spatium = tremolo->spatium();
    Chord* c1 = tremolo->chord1();
    Chord* c2 = tremolo->chord2();
    Stem* s1 = c1->stem();
    Stem* s2 = c2->stem();
    const double sgn1 = c1->up() ? -1.0 : 1.0;
    const double sgn2 = c2->up() ? -1.0 : 1.0;
    const double stemTipDistance = (s1 && s2) ? (s2->pagePos().y() + stemLen2) - (s1->pagePos().y() + stemLen1)
                                   : (c2->stemPos().y() + (sgn2 * stemLen2)) - (c1->stemPos().y() + (sgn1 * stemLen1));

    // same staff & same direction: extend one of the stems
    if (c1->staffMove() == c2->staffMove() && c1->up() == c2->up()) {
        const bool stem1Higher = stemTipDistance > 0.0;
        if (std::abs(stemTipDistance) > 1.0 * spatium) {
            if ((c1->up() && !stem1Higher) || (!c1->up() && stem1Higher)) {
                return { stemLen1 + (std::abs(stemTipDistance) - spatium), stemLen2 };
            } else {   /* if ((c1->up() && stem1Higher) || (!c1->up() && !stem1Higher)) */
                return { stemLen1, stemLen2 + (std::abs(stemTipDistance) - spatium) };
            }
        }
    }

    return { stemLen1, stemLen2 };
}

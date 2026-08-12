/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editbeam.h"

#include "../dom/chordrest.h"
#include "../dom/groups.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/utils.h"

#include "navigation.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   setBeamMode
//---------------------------------------------------------

void EditBeam::setBeamMode(Transaction&, Score* score, BeamMode mode)
{
    for (ChordRest* cr : score->getSelectedChordRests()) {
        if (cr) {
            cr->undoChangeProperty(Pid::BEAM_MODE, mode);
        }
    }
}

//---------------------------------------------------------
//   beamSelectedRange
//---------------------------------------------------------

void EditBeam::beamSelectedRange(Transaction& tx, Score* score)
{
    if (!score->selection().isRange()) {
        return;
    }

    resetBeamMode(tx, score);

    const track_idx_t startTrack = staff2track(score->selection().staffStart());
    const track_idx_t endTrack = staff2track(score->selection().staffEnd());
    const SelectionFilter filter = score->selectionFilter();

    for (staff_idx_t trackIdx = startTrack; trackIdx < endTrack; ++trackIdx) {
        if (!filter.canSelectVoice(trackIdx)) {
            continue;
        }

        ChordRest* firstChordRest = score->selection().firstChordRest(trackIdx);
        ChordRest* lastChordRest = score->selection().lastChordRest(trackIdx);

        if (!firstChordRest || !lastChordRest) {
            continue;
        }

        ChordRest* prev = Navigation::prevChordRest(firstChordRest);
        ChordRest* cr = firstChordRest;
        BeamMode actualBeamMode = Groups::actualBeamMode(cr, prev);
        if (actualBeamMode != BeamMode::BEGIN) {
            cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN);
        }

        while (cr != lastChordRest) {
            prev = cr;
            cr = Navigation::nextChordRest(cr);
            actualBeamMode = Groups::actualBeamMode(cr, prev);
            if (actualBeamMode == BeamMode::BEGIN || actualBeamMode == BeamMode::NONE) {
                cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::MID);
            }
        }

        prev = cr;
        cr = Navigation::nextChordRest(cr);
        if (cr) {
            actualBeamMode = Groups::actualBeamMode(cr, prev);
            if (actualBeamMode != BeamMode::BEGIN && actualBeamMode != BeamMode::NONE) {
                cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN);
            }
        }
    }
}

//---------------------------------------------------------
//   resetBeamMode
//---------------------------------------------------------

void EditBeam::resetBeamMode(Transaction&, Score* score)
{
    bool noSelection = score->selection().isNone();
    if (noSelection) {
        score->cmdSelectAll();
    } else if (!score->selection().isRange()) {
        LOGD("no system or staff selected");
        return;
    }

    ChordRest* firstCr = score->selection().firstChordRest();
    if (!firstCr) {
        LOGD("no chord/rest in selection");
        return;
    }

    const track_idx_t trackStart = staff2track(score->selection().staffStart());
    const track_idx_t trackEnd = staff2track(score->selection().staffEnd());
    const Fraction endTick = score->selection().tickEnd();
    const SelectionFilter filter = score->selectionFilter();

    for (Segment* seg = firstCr->segment(); seg && seg->tick() < endTick; seg = seg->next1(SegmentType::ChordRest)) {
        for (track_idx_t track = trackStart; track < trackEnd; ++track) {
            if (!filter.canSelectVoice(track)) {
                continue;
            }

            ChordRest* cr = toChordRest(seg->element(track));
            if (!cr) {
                continue;
            }
            if (cr->isChord()) {
                if (cr->beamMode() != BeamMode::AUTO) {
                    cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::AUTO);
                }
            } else if (cr->isRest()) {
                if (cr->beamMode() != BeamMode::NONE) {
                    cr->undoChangeProperty(Pid::BEAM_MODE, BeamMode::NONE);
                }
            }
        }
    }

    if (noSelection) {
        score->deselectAll();
    }
}

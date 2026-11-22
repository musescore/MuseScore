/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "midirecordingpreview.h"

#include "draw/types/color.h"
#include "draw/types/pen.h"

#include "engraving/dom/instrument.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/system.h"
#include "engraving/dom/utils.h"
#include "engraving/iengravingfont.h"
#include "engraving/style/style.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::draw;

static const Color PREVIEW_COLOR(229, 53, 43);

int MidiRecordingPreview::staffLine(int writtenPitch, Key key, ClefType clef)
{
    const int tpc = pitch2tpc(writtenPitch, key, Prefer::NEAREST);
    return relStep(writtenPitch, tpc, clef);
}

int MidiRecordingPreview::writtenPitch(const Score* score, staff_idx_t staffIdx, int soundingPitch, const Fraction& tick)
{
    const Staff* staff = score->staff(staffIdx);
    if (!staff || score->style().styleB(Sid::concertPitch)) {
        return soundingPitch;
    }

    const Instrument* instrument = staff->part()->instrument(tick);
    if (!instrument) {
        return soundingPitch;
    }

    return soundingPitch - instrument->transpose().chromatic;
}

//! NOTE The same interpolation as PlaybackCursor: the notes appear where the cursor was when they were played
std::optional<double> MidiRecordingPreview::xAtTick(const Score* score, const Fraction& tick)
{
    const Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return std::nullopt;
    }

    for (const Segment* s = measure->first(SegmentType::ChordRest); s;) {
        const Fraction t1 = s->tick();
        const double x1 = s->canvasPos().x();

        const Segment* ns = s->next(SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(SegmentType::ChordRest);
        }

        Fraction t2;
        double x2 = 0.0;

        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            const Segment* barLine = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
            x2 = barLine ? barLine->canvasPos().x() : measure->canvasPos().x() + measure->width();
        }

        if (tick >= t1 && tick < t2) {
            return x1 + (x2 - x1) * (tick - t1).ticks() / (t2 - t1).ticks();
        }

        s = ns;
    }

    return std::nullopt;
}

void MidiRecordingPreview::paint(Painter* painter, const Score* score, const playback::MidiRecordingPreviewNoteList& notes)
{
    if (!painter || !score || notes.empty() || !score->engravingFont()) {
        return;
    }

    const double spatium = score->style().spatium();
    const double symbolMag = spatium / score->style().defaultSpatium();

    painter->save();
    painter->setPen(Pen(PREVIEW_COLOR));

    for (const playback::MidiRecordingPreviewNote& note : notes) {
        const Fraction tick = Fraction::fromTicks(note.tick);
        const Staff* staff = score->staff(note.staffIdx);
        if (!staff || !staff->show() || !staff->isPitchedStaff(tick)) {
            continue;
        }

        const Measure* measure = score->tick2measureMM(tick);
        const System* system = measure ? measure->system() : nullptr;
        if (!system || !system->page() || system->staves().size() <= note.staffIdx) {
            continue;
        }

        const std::optional<double> x = xAtTick(score, tick);
        if (!x.has_value()) {
            continue;
        }

        const double mag = symbolMag * staff->staffMag(tick);
        const int line = staffLine(writtenPitch(score, note.staffIdx, note.pitch, tick), staff->key(tick), staff->clef(tick));
        // The same as Note::updateLine: (line + stepOffset * 2) * spatium * 0.5 * lineDistance
        const StaffType* staffType = staff->staffType(tick);
        const int stepOffset = staffType ? staffType->stepOffset() : 0;
        const double y = system->staffCanvasYpage(note.staffIdx)
                         + (line + stepOffset * 2.0) * 0.5 * spatium * staff->staffMag(tick) * staff->lineDistance(tick);

        const double noteheadWidth = score->engravingFont()->width(SymId::noteheadBlack, mag);

        score->engravingFont()->draw(SymId::noteheadBlack, painter, mag, PointF(x.value() - noteheadWidth / 2, y));
    }

    painter->restore();
}

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Emit chord symbols (Harmony) from Encore CHORD elements.

#include "emitters-internal.h"

#include "../parser/ticks.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

namespace mu::iex::enc {
// CHD elements often carry a MIDI timing offset from the note they annotate, so floor the CHD tick
// to the beat start and attach to the first ChordRest segment in [beatStart, chdTick]; this avoids
// near-miss snapping to a subdivision note. See ENCORE_IMPORTER.md §Chord symbol (harmony) import.
void handleChordSym(BuildCtx& ctx, const MeasEmitCtx& mc, const NoteElemCtx& ec)
{
    const EncChordSym* ecs = static_cast<const EncChordSym*>(ec.e);
    const QString raw = ecs->chordName();
    if (raw.isEmpty()) {
        return;
    }
    const int wt = kEncWholeTicks;
    const int bt = static_cast<int>(mc.encMeas->beatTicks ? mc.encMeas->beatTicks : 240);
    const int chdEncTick = static_cast<int>(ec.e->tick);
    const int beatStart  = (chdEncTick / bt) * bt;
    const Fraction beatStartFrac(beatStart, wt);
    const Fraction chdFrac(chdEncTick, wt);
    Segment* seg = nullptr;
    for (Segment* s = mc.measure->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        const Fraction sRel = s->tick() - mc.measTick;
        if (sRel < beatStartFrac) {
            continue;
        }
        if (sRel > chdFrac) {
            break;
        }
        if (!seg) {
            seg = s;
        }
    }
    if (!seg) {
        for (Segment* s = mc.measure->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            if (s->tick() - mc.measTick <= chdFrac) {
                seg = s;
            } else {
                break;
            }
        }
    }
    if (!seg) {
        seg = mc.measure->getSegment(SegmentType::ChordRest, ec.elemTick);
    }
    Harmony* h = Factory::createHarmony(ctx.score->dummy()->segment());
    h->setTrack(ec.track);
    h->setHarmony(String(raw));

    // Encore records per chord symbol whether a guitar frame is drawn above it (tipo bit 2). Only
    // then wrap the harmony in a FretDiagram (segment annotation with the Harmony as its child);
    // chords without the flag, or whose name the database cannot resolve, keep the plain text symbol.
    if (ecs->hasFretDiagram) {
        FretDiagram* fd = Factory::createFretDiagram(ctx.score->dummy()->segment());
        fd->setTrack(ec.track);
        fd->updateDiagram(h->harmonyName());
        if (!fd->isClear()) {
            seg->add(fd);
            fd->add(h);
            return;
        }
        delete fd;
    }
    seg->add(h);
}
} // namespace mu::iex::enc

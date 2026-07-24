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

// Apply per-measure BPM marks as TempoText and render tempo-text strings.

#include "emitters-internal.h"

#include <cmath>

#include "durations.h"
#include "../parser/ticks.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tempotext.h"

namespace mu::iex::enc {
// Decode the tempo mark's beat unit from the ORN `noto` field into display ticks (quarter=240).
// Returns 0 when `noto` is unset or unrecognised (older formats store unrelated bytes here) so
// the caller can fall back to the meter heuristic. See ENCORE_FORMAT.md §Ornament element.
int notoToBeatTicks(quint8 noto)
{
    if (noto == 0) {
        return 0;
    }
    const bool dotted = (noto & 0x80) != 0;
    const int baseVal = noto & 0x7F;            // 0-indexed note value
    if (baseVal > 6) {
        return 0;                                // out of range: misparsed old-format byte
    }
    int ticks = faceValue2ticks(static_cast<quint8>(baseVal + 1));   // 1=whole .. 7=64th
    if (ticks == 0) {
        return 0;
    }
    if (dotted) {
        ticks = ticks * 3 / 2;
    }
    return ticks;
}

// Render tempo text. displayBpm is the beat-unit BPM that Encore shows the user.
// beatTicks is the beat duration in display ticks (quarter=240, dotted-quarter=360,
// half=480, eighth=120, ...); a value that is a base note times 3/2 renders dotted.
String tempoXmlText(int displayBpm, int beatTicks)
{
    bool dotted = false;
    int base = beatTicks;
    if (beatTicks % 3 == 0) {
        const int b = beatTicks * 2 / 3;
        if (b == 960 || b == 480 || b == 240 || b == 120 || b == 60) {
            dotted = true;
            base = b;
        }
    }
    String sym;
    switch (base) {
    case 960: sym = u"metNoteWhole";
        break;
    case 480: sym = u"metNoteHalfUp";
        break;
    case 240: sym = u"metNoteQuarterUp";
        break;
    case 120: sym = u"metNote8thUp";
        break;
    case 60:  sym = u"metNote16thUp";
        break;
    default:                                     // unknown: fall back to quarter / dotted-quarter
        sym = u"metNoteQuarterUp";
        dotted = (beatTicks == 360);
        break;
    }
    if (dotted) {
        return String(u"<sym>%1</sym><sym>space</sym><sym>metAugmentationDot</sym> = %2").arg(sym).arg(displayBpm);
    }
    return String(u"<sym>%1</sym> = %2").arg(sym).arg(displayBpm);
}

// Apply per-measure BPM from MEAS headers as TempoText elements.
// Emits a TempoText only when BPM changes, and skips measures that already
// have an ORN TEMPO or STAFFTEXT tempo mark to avoid duplicates.
void applyMeasureBpmMarks(BuildCtx& ctx)
{
    const EncRoot& enc = ctx.enc;
    MasterScore* score = ctx.score;

    quint16 lastBpm = 0;
    for (size_t mi = 0; mi < enc.measures.size(); ++mi) {
        const quint16 bpm = enc.measures[mi].bpm;
        if (bpm == 0) {
            continue;
        }
        if (mi > 0 && bpm == lastBpm) {
            continue;
        }
        const size_t msI = (mi < ctx.encToMsIdx.size()) ? ctx.encToMsIdx[mi] : mi;
        if (msI >= ctx.measuresByIdx.size()) {
            continue;
        }
        Measure* m = ctx.measuresByIdx[msI];
        const Fraction measTick = m->tick();
        Segment* seg = m->getSegment(SegmentType::ChordRest, measTick);
        if (!seg) {
            continue;
        }
        bool hasExisting = false;
        for (Segment* s = m->first(SegmentType::ChordRest); s && !hasExisting;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    hasExisting = true;
                    break;
                }
            }
        }
        if (!hasExisting) {
            // The header BPM is a quarter-note BPM. Choose the display beat unit: prefer the
            // explicit unit on this measure's ORN tempo mark (`noto`) so a quarter=198 mark in 6/8
            // stays "quarter=198" rather than the compound default "dotted-quarter=132"; otherwise
            // fall back to the meter heuristic (compound meters display in dotted quarters).
            int displayBeatTicks = 0;
            for (const auto& el : enc.measures[mi].elements) {
                const EncOrnament* orn = dynamic_cast<const EncOrnament*>(el.get());
                if (orn && orn->ornType() == EncOrnamentType::TEMPO && orn->tempo > 0) {
                    displayBeatTicks = notoToBeatTicks(orn->noto);
                    if (displayBeatTicks != 0) {
                        break;
                    }
                }
            }
            const double bps = bpm / 60.0;
            int displayBpm;
            if (displayBeatTicks != 0) {
                // Re-express the quarter-note BPM in the chosen beat unit.
                displayBpm = static_cast<int>(std::lround(bpm * 240.0 / displayBeatTicks));
            } else {
                const bool cmpd = isCompoundBeat(enc.measures[mi].beatTicks, m->timesig());
                displayBeatTicks = cmpd ? 360 : 240;
                displayBpm = cmpd ? (bpm * 2 + 1) / 3 : static_cast<int>(bpm);
            }
            TempoText* tt = Factory::createTempoText(seg);
            tt->setTrack(0);
            tt->setTempo(BeatsPerSecond(bps));
            tt->setXmlText(tempoXmlText(displayBpm, displayBeatTicks));
            tt->setFollowText(true);
            seg->add(tt);
            score->setTempo(measTick, BeatsPerSecond(bps));
        }
        lastBpm = bpm;
    }
}
} // namespace mu::iex::enc

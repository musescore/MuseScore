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

// Post-pass resolvers that attach deferred spanners and ornaments (slurs, hairpins, ornaments,
// fingerings/bowings, ottavas) once every chord exists, plus shared track-validity and
// tick-to-chord lookup helpers.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_RESOLVERS_H
#define MU_IMPORTEXPORT_ENC_IMPORT_RESOLVERS_H

#include "ctx.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

namespace mu::iex::enc {
void resolveAll(BuildCtx& ctx);
void resolveSlurs(BuildCtx& ctx);
void resolveHairpins(BuildCtx& ctx);
void resolveOrnaments(BuildCtx& ctx);
void resolveFingeringAndBowing(BuildCtx& ctx);
void resolveOttavas(BuildCtx& ctx);

// A track derived from untrusted Encore staff/voice bytes can exceed the score's track
// count; Segment::element(track) indexes a fixed-size vector, so an out-of-range track is
// out-of-bounds. Every resolver must check this before any element(track) / spanner-track use.
inline bool validTrack(const mu::engraving::Score* score, mu::engraving::track_idx_t track)
{
    return score && track < score->ntracks();
}

// Tick → measure → ChordRest segment → Chord lookup; shared by all resolver files.
inline mu::engraving::Chord* findChordAt(mu::engraving::MasterScore* score,
                                         mu::engraving::Fraction tick,
                                         mu::engraving::track_idx_t track)
{
    using namespace mu::engraving;
    if (!validTrack(score, track)) {
        return nullptr;
    }
    Measure* m = score->tick2measure(tick);
    if (!m) {
        return nullptr;
    }
    Segment* seg = m->findSegment(SegmentType::ChordRest, tick);
    if (!seg) {
        return nullptr;
    }
    EngravingItem* el = seg->element(track);
    if (!el || !el->isChord()) {
        return nullptr;
    }
    return toChord(el);
}

// First chord on any voice of `staffIdx` within segment `seg`, scanning voices 0..VOICES-1 in order.
// Sets outTrack to the carrying voice's track and returns the chord, or nullptr if seg is null or
// holds no chord on that staff. validTrack is folded in so an out-of-range staff yields nullptr
// instead of an out-of-bounds element() access.
inline mu::engraving::Chord* firstChordVoiceAt(const mu::engraving::Score* score,
                                               const mu::engraving::Segment* seg, int staffIdx,
                                               mu::engraving::track_idx_t& outTrack)
{
    using namespace mu::engraving;
    if (!seg) {
        return nullptr;
    }
    const track_idx_t base = static_cast<track_idx_t>(staffIdx) * VOICES;
    for (track_idx_t v = 0; v < VOICES; ++v) {
        const track_idx_t t = base + v;
        if (!validTrack(score, t)) {
            break;
        }
        EngravingItem* el = seg->element(t);
        if (el && el->isChord()) {
            outTrack = t;
            return toChord(el);
        }
    }
    return nullptr;
}
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_RESOLVERS_H

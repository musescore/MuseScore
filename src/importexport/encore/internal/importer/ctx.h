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

// Shared importer state: the BuildCtx threaded through every phase, the deferred "pending" element
// structs drained in the post-pass, and the emit-phase scratch tables.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_CTX_H
#define MU_IMPORTEXPORT_ENC_IMPORT_CTX_H

#include "import-options.h"

#include <array>
#include <map>
#include <set>
#include <memory>
#include <vector>

#include <QtGlobal>

#include "../parser/elem.h"
#include "../parser/readers.h"
#include "emitters-tuplets.h"
#include "engraving/types/fraction.h"
#include "engraving/types/symid.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/ottava.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Reverse map from a raw staff byte (instrStaffIdx in the LINE block) to its LINE slot index;
// -1 = no staff with that byte. Needed when an earlier instrument owns more than one staff, so the
// raw byte no longer equals the slot index. Shared by the emitter routing and the slur resolver.
inline void buildLineSlotByRawByte(const EncRoot& enc, std::array<int, 256>& out)
{
    out.fill(-1);
    if (enc.lines.empty()) {
        return;
    }
    const auto& sd = enc.lines[0].staffData;
    for (int s = 0; s < static_cast<int>(sd.size()); ++s) {
        out[static_cast<unsigned char>(sd[s].instrStaffIdx)] = s;
    }
}

// LINE staff-data entry for the given running staff index, or nullptr when absent/out of range.
// Centralizes the "lines non-empty + index in range" guard used by the part/staff routing.
inline const EncLineStaffData* lineStaffDataAt(const EncRoot& enc, int idx)
{
    if (enc.lines.empty() || idx < 0
        || idx >= static_cast<int>(enc.lines[0].staffData.size())) {
        return nullptr;
    }
    return &enc.lines[0].staffData[static_cast<size_t>(idx)];
}

struct PendingSlur {
    Fraction startTick;
    track_idx_t track;
    int startMeasIdx;
    int endMeasIdx;
    int alMezuro;
    bool alMezuroValid { true };  // false when format cannot guarantee measure-count semantics
    int slurXoffset;
    int slurXoffset2;
    int staffIdx;
    int encVoice;
};

// Hairpin end resolved in post-pass to the next Dynamic on the same track (not the barline).
struct PendingHairpin {
    Fraction startTick;
    Fraction maxEndTick;     // end of alMezuro target measure (upper bound)
    track_idx_t track;
    HairpinType type;
    int endMeasIdx;
    int hairpinXoffset2;
    int staffIdx;
    int encVoice;
};

// Deferred: ORN precedes chord notes in MEAS order, so the chord does not exist at parse time.
struct PendingArpeggio {
    Fraction tick;
    track_idx_t track;
};

// Single-chord tremolo (tipo 0xAF/0xEF), deferred like ARPEGGIO. Post-pass falls back to latest chord before the tick.
struct PendingOrnTremolo {
    Fraction tick;
    Fraction measTick;
    int staffIdx;
    int msVoice;
    TremoloType tremType;
};

// Trill intents (tipo 0x35/0x36/0x37), deferred for the same reason as ARPEGGIO.
struct PendingTrill {
    Fraction tick;
    track_idx_t track;
    int alMezuro { 0 };           // forward measure count to trill end (0 = same measure)
    size_t measIdx  { 0 };
    int xoffset2 { 0 };           // end x-position hint for same-measure endpoint detection
    bool isAlt    { false };      // TRILL_ALT (0x37): secondary mark, always Ornament glyph
    bool isSimple { false };      // TRILL_TR/TRILL_SHORT: standalone glyph, never a spanner
    mu::engraving::SymId simpleSymId { mu::engraving::SymId::ornamentTrill };
};

// Staccato (tipo 0xC9), deferred like ARPEGGIO.
struct PendingStaccato {
    Fraction tick;
    track_idx_t track;
};

// Fermata (tipo 0xCC/0xCD), deferred like ARPEGGIO.
struct PendingFermata {
    Fraction tick;
    track_idx_t track;
    mu::engraving::SymId symId;
};

// Breath / caesura (tipo 0xA7/0xA8).
struct PendingBreath {
    Fraction tick;
    track_idx_t track;
    mu::engraving::SymId symId;
};

// Measure repeat (tipo 0xA3).
struct PendingMeasureRepeat {
    Fraction measTick;
    int staffIdx;
};

// Bowing/stroke (tipo 0xC4/0xC5), deferred like ARPEGGIO.
// v0xC4: 0xC4=stringsUpBow, 0xC5=stringsDownBow; v0xC2: 0xC4=articAccentAbove.
struct PendingBowing {
    Fraction tick;
    track_idx_t track;
    mu::engraving::SymId symId;
    int measIdx = -1;
    bool crossMeasure = false;  // no voice=0 note at same enc tick; belongs to next measure
    int ornXoffset = 0;         // absolute horizontal pixel position (for xoffset-cluster correction)
    int encTickRaw = 0;
};

// Fingering from stand-alone ORN elements (tipo 0xB9..0xBD), deferred like ARPEGGIO.
// Also used for standalone string-number ORNs (0xE6..0xEA = strings 2..6); those set
// isStringNum=true and the resolver uses TextStyleType::STRING_NUMBER.
struct PendingOrnFingering {
    Fraction tick;
    track_idx_t track;
    int fingerNum;
    int measIdx = -1;
    bool crossMeasure = false;   // ORN at last v0 tick, no v4 note there: belongs to next measure
    bool preferSibling = false;  // more ORNs than v0 notes at tick: belongs to 2nd-staff chord
    bool isStringNum  = false;   // true → render as STRING_NUMBER (circled), not FINGERING
};

// Ottava lines (tipo 0x10=8va, 0x12=8vb). Endpoint = next ottava on same staff or scoreEnd.
struct PendingOttava {
    Fraction startTick;
    track_idx_t track;
    int staffIdx;
    mu::engraving::OttavaType ottavaType;
};

// Segno/Coda markers (tipo 0xA2/0xA6).
struct PendingMarker {
    Fraction tick;
    MarkerType type;
};

// Lyric syllables queued for attachment. encTick lets us find the correct chord even
// when queue index shifts due to ORN elements.
struct PendingLyric {
    int encTick;
    int xoffset;        // horizontal anchor (EncLyric::kie); the reliable per-syllable position
    String text;
    bool hyphenBefore;
    bool hyphenAfter;
};

// A grace chord held detached until the next normal chord, paired with the EncNote it came
// from so its articulations can be applied after attachment (see pendingGraces).
struct PendingGrace {
    mu::engraving::Chord* gc { nullptr };
    const EncNote* en { nullptr };
    // Measure the grace was queued in, so a grace that never finds a principal chord (dangling at end
    // of score) can be re-placed as a cue note in its own bar instead of being discarded.
    mu::engraving::Measure* measure { nullptr };
};

// Shared importer context threaded through builders, emitters and resolvers: the target score,
// the parsed Encore model, import options, derived staff/measure tables, the resolver "pending"
// queues drained in the post-pass, and (grouped in EmitState scratch) the emitter-only state.
struct BuildCtx
{
    mu::engraving::MasterScore* score;
    const EncRoot& enc;
    EncImportOptions opts;

    // Populated by buildParts():
    int totalStaves = 0;
    std::vector<int> staffPitchOffset {};
    std::vector<ClefType> staffTemplateConcertClef {};

    // Nominal time sig of the score (differs from measures[0] when the first measure is a pickup).
    Fraction nominalTimeSig { 4, 4 };
    TimeSigType nominalTimeSigType { TimeSigType::NORMAL };

    // Tick → TimeSigType for measures with non-numeric display (e.g. common time "C").
    std::map<int, TimeSigType> measTickToTimeSigType {};

    // encToMsIdx[i] = MuseScore measure index of the first measure from enc.measures[i].
    // Accounts for multi-measure rest expansion (mrestCount > 1).
    std::vector<size_t> encToMsIdx {};

    std::vector<Measure*> measuresByIdx {};
    std::vector<PendingHairpin> pendingHairpins {};
    std::vector<PendingSlur> pendingSlurs {};
    std::vector<PendingArpeggio> pendingArpeggios {};
    std::vector<PendingOrnTremolo> pendingOrnTremolos {};
    std::vector<PendingTrill> pendingTrills {};
    // TRILL_END ticks by track; consumed by resolveOrnaments() to compute span endpoints.
    std::map<track_idx_t, std::vector<mu::engraving::Fraction> > pendingTrillEnds {};
    std::vector<PendingStaccato> pendingStaccatos {};
    std::vector<PendingFermata> pendingFermatas {};
    std::vector<PendingBreath> pendingBreaths {};
    std::vector<PendingMeasureRepeat> pendingMeasureRepeats {};
    std::vector<PendingBowing> pendingBowings {};
    // (measIdx, staffIdx) → list of (enc_tick, note.xoffset) for bowing xoffset clustering.
    std::map<std::pair<int, int>, std::vector<std::pair<int, int> > > noteXoffByMeasStaff {};
    std::vector<PendingOrnFingering> pendingOrnFingerings {};
    std::vector<PendingOttava> pendingOttavas {};
    std::vector<PendingMarker> pendingMarkers {};

    // Volta being coalesced: equal-bitmask runs collapse into one Volta.
    Volta* activeVolta { nullptr };
    quint8 activeVoltaBits { 0 };
    // Bitmask of all volta endings already labelled in earlier brackets of the current repeat
    // block; suppresses re-labelling them (e.g. after "1.-3." a {2,4} bracket shows only "4.").
    quint8 usedVoltaBits { 0 };

    // Per-measure / per-run emitter scratch, grouped so builders and resolvers don't touch it:
    // only the emitters (emitMeasures and its helpers) use ctx.scratch.*. Lives for the emit phase.
    struct EmitState {
        // Elements routeElementStaffVoice dropped for referencing a missing staff (mostly orphan
        // data from staves deleted in Encore) or an out-of-range voice. Counted, reported once.
        // mutable: routeElementStaffVoice takes a const BuildCtx& and otherwise only reads.
        mutable std::map<int, int> droppedByMissingStaff {};   // staff index -> count
        mutable int droppedByBadVoice { 0 };

        // Tuplet trackers under construction, keyed by (staffIdx, msVoice); cleared each measure.
        std::map<std::pair<int, int>, TupletTracker> tuplets {};
        // Inner (nested) TupletTrackers; cleared each measure alongside tuplets.
        std::map<std::pair<int, int>, TupletTracker> innerTuplets {};

        // Pending tie-start notes, persists across measures. key=(staffIdx, voice, pitch).
        std::map<std::tuple<int, int, int>, Note*> pendingTieNote {};

        // Accumulated written position per (staffIdx, msVoice).
        std::map<std::pair<int, int>, Fraction> cumTick {};
        // Last MIDI tick placed; same tick = same chord.
        std::map<std::pair<int, int>, int> prevMidiTick {};
        // Encore voice of last note placed; guards against chord-extension misdetection.
        std::map<std::pair<int, int>, int> prevEncVoice {};
        // Notated column (xoffset) of last note placed; near-simultaneous notes in different
        // columns are separate events (e.g. tightly played tuplet members), not one chord.
        std::map<std::pair<int, int>, int> prevXoffset {};
        std::map<std::pair<int, int>, Fraction> lastChordPos {};
        // Last enc tick at which a REST was placed; absorbs duplicate rests when multiple
        // Encore voices route to the same MuseScore voice.
        std::map<std::pair<int, int>, int> prevRestTick {};

        // Grace chords held detached, attached to the next normal chord. The source EncNote is kept
        // so articulations/fermatas are applied once the grace has a real segment (fermatas cannot
        // anchor to the dummy segment of a detached grace).
        std::map<std::pair<int, int>, std::vector<PendingGrace> > pendingGraces {};
        // Ticks borrowed by grace notes; suppresses spurious gap-snap rests after a grace group.
        std::map<std::pair<int, int>, int> graceStolenTicks {};
        // Most recent grace chord per track and its enc tick. Encore stores each chord member as a
        // note at the same tick, so a same-tick grace member merges here instead of spawning a new
        // grace. Covers both before- and after-graces.
        std::map<std::pair<int, int>, mu::engraving::Chord*> lastGraceChord {};
        std::map<std::pair<int, int>, int> lastGraceTick {};

        // Lyric syllables queued for attachment; drained each measure by attachPendingLyrics.
        std::map<track_idx_t, std::vector<PendingLyric> > pendingLyrics {};
        // True when the next syllable follows a hyphen; reset at measure boundary.
        std::map<track_idx_t, bool> nextLyricHyphenBefore {};
        // Last lyric attached on each track. Lets a hyphen that opens the next measure
        // promote the previous measure's final syllable so the word stays hyphenated
        // across the barline.
        std::map<track_idx_t, mu::engraving::Lyrics*> lastAttachedLyric {};
    } scratch {};
};
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_CTX_H

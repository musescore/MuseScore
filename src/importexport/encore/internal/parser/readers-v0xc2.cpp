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

// Encore 3.x/4.x (v0xC2) reader: pitch/tuplet slot swap, tie-sender flag, dotted-eighth and
// implied-tuplet fixups, and the two TEMPO/slur layout quirks that diverge from v0xC4.

#include "readers-v0xc2.h"
#include "readers-v0xc4-base.h"

#include <QDataStream>

#include "elem.h"
#include "ticks.h"

namespace mu::iex::enc {
// v0xC2 stores the eighth of a dotted-eighth+sixteenth group as plain (rdur 120) instead of dotted
// (180), placing the sixteenth at tick+120. Force the dot only when the measure is short by exactly
// 60t (the amount the anomaly steals), otherwise a genuine 8th+16th would get a spurious dot.
static void fixDottedEighthPattern(std::vector<EncMeasureElem*>& elems, qint16 durTicks)
{
    int faceSum = 0;
    for (const EncMeasureElem* e : elems) {
        quint8 fv = 0;
        if (const auto* en = dynamic_cast<const EncNote*>(e)) {
            fv = en->faceValue & 0x0F;
        } else if (const auto* er = dynamic_cast<const EncRest*>(e)) {
            fv = er->faceValue & 0x0F;
        }
        faceSum += faceValue2ticks(fv);
    }
    if (faceSum + 60 != static_cast<int>(durTicks)) {
        return;
    }

    for (size_t i = 0; i < elems.size(); ++i) {
        EncNote* en = dynamic_cast<EncNote*>(elems[i]);
        if (!en || (en->faceValue & 0x0F) != 4 || en->realDuration != 120) {
            continue;
        }
        const qint16 targetTick = static_cast<qint16>(elems[i]->tick + 120);
        for (size_t j = i + 1; j < elems.size(); ++j) {
            if (elems[j]->tick > targetTick) {
                break;
            }
            if (elems[j]->tick == targetTick) {
                const EncNote* enNext = dynamic_cast<const EncNote*>(elems[j]);
                if (enNext
                    && (enNext->faceValue & 0x0F) == 5
                    && enNext->realDuration == 60) {
                    en->dotControl |= 1;   // kept for documentation; dot is forced via forceDotted
                    en->forceDotted = true;
                    break;
                }
            }
        }
    }
}

// v0xC2: mark consecutive notes/rests whose rdur/faceValue ratio identifies an implied tuplet.
// Groups same-tick elements as chords before scanning, matching the grouping in
// computeImpliedTupletMembers so the two passes agree on group boundaries.
static void markImpliedTupletMembers(std::vector<EncMeasureElem*>& elems)
{
    std::vector<std::vector<EncMeasureElem*> > chords;
    for (EncMeasureElem* e : elems) {
        if (!chords.empty() && chords.back()[0]->tick == e->tick) {
            chords.back().push_back(e);
        } else {
            chords.push_back({ e });
        }
    }
    int n = static_cast<int>(chords.size());
    int i = 0;
    while (i < n) {
        EncMeasureElem* first = chords[i][0];
        quint8 fv = 0;
        if (auto* en = dynamic_cast<EncNote*>(first)) {
            fv = en->faceValue & 0x0F;
        } else if (auto* er = dynamic_cast<EncRest*>(first)) {
            fv = er->faceValue & 0x0F;
        }
        if (fv < 4) {
            ++i;
            continue;
        }
        int normalN = 0;
        int actualN = detectImpliedTuplet(first->realDuration, fv, normalN);
        if (actualN < 2 || i + actualN > n) {
            ++i;
            continue;
        }
        bool allMatch = true;
        for (int k = 1; k < actualN; ++k) {
            EncMeasureElem* ek = chords[i + k][0];
            quint8 fvk = 0;
            if (auto* en = dynamic_cast<EncNote*>(ek)) {
                fvk = en->faceValue & 0x0F;
            } else if (auto* er = dynamic_cast<EncRest*>(ek)) {
                fvk = er->faceValue & 0x0F;
            }
            int nk = 0;
            if (fvk < 4 || detectImpliedTuplet(ek->realDuration, fvk, nk) != actualN || nk != normalN) {
                allMatch = false;
                break;
            }
        }
        if (allMatch) {
            for (int k = 0; k < actualN; ++k) {
                for (EncMeasureElem* e : chords[i + k]) {
                    if (auto* en = dynamic_cast<EncNote*>(e)) {
                        en->isImpliedTupletMember = true;
                    } else if (auto* er = dynamic_cast<EncRest*>(e)) {
                        er->isImpliedTupletMember = true;
                    }
                }
            }
            i += actualN;
        } else {
            ++i;
        }
    }
}

// Encore 3.x / 4.x (v0xC2) format reader.
// Differences from v0xC4:
//   - ORN 0xC4 is an accent, not an up-bow
//   - grace1 low nibble encodes the tie-sender flag
//   - alMezuro field in ornaments is unreliable
//   - Lyric text starts at element offset +18 (not +20)
//   - NOTE: MIDI pitch is in tuplet slot; semiTonePitch is 0 (swap them in postProcess)
//   - Instrument metadata: names only (no TK-based MIDI/key tables)
struct EncFormatReader_V0xC2 final : EncFormatReader_V0xC4Base
{
    const char* formatName() const override { return "v0xC2"; }
    quint8 lyricTextGapAfterKie() const override { return 7; }

    // v0xC2 slur xoffset2 lives in a stale ornament-coordinate origin; anchor endpoints
    // explicitly (forward measure-count / next note) instead of by coordinate search.
    bool slurXoffset2Stale() const override { return true; }

    bool postProcessElement(EncMeasureElem* elem, QDataStream& ds, qint64 rawElemStart) const override
    {
        if (EncOrnament* orn = dynamic_cast<EncOrnament*>(elem)) {
            // v0xC2: tipo 0xC4 (UPBOW in v0xC4) encodes accent above in this format.
            if (orn->tipo == static_cast<quint8>(EncOrnamentType::UPBOW)) {
                orn->tipo = static_cast<quint8>(EncOrnamentType::ACCENT);
            }
            // v0xC2 keeps the reliable forward slur span at +16 (altMezuro), not +18 (garbage here):
            // 0 = within measure, N = ends N bars later. Marking it valid lets the post-pass anchor
            // by measure count instead of the unreliable xoffset2 coordinate. See ENCORE_FORMAT.md §Slur.
            if (orn->tipo == static_cast<quint8>(EncOrnamentType::SLURSTART)) {
                orn->alMezuro = orn->altMezuro;
                orn->alMezuroValid = true;
            } else {
                orn->alMezuroValid = false;
            }
            // v0xC2 has two TEMPO layouts. New (v0xC4-style): beat-unit code at +28, BPM at +30.
            // Old: BPM at +28 (read into noto) with a constant in the +30 slot. Discriminate by
            // whether +28 holds a valid beat-unit code (low 7 bits 0..6). See ENCORE_FORMAT.md §Ornament subtypes.
            if (orn->tipo == static_cast<quint8>(EncOrnamentType::TEMPO)) {
                const quint8 beatUnitCode = orn->noto & 0x7F;
                const bool validBeatUnit = (orn->noto != 0) && (beatUnitCode <= 6);
                if (!validBeatUnit) {
                    orn->tempo = orn->noto;   // old layout: +28 is the BPM
                    // Old layout keeps the per-mark beat unit at +26; recover it so the mark shows
                    // the composer's unit instead of the compound-meter dotted-quarter default.
                    orn->noto = 0;
                    const qint64 save = ds.device()->pos();
                    if (ds.device()->seek(rawElemStart + 26)) {
                        quint8 beatUnit = 0;
                        ds >> beatUnit;
                        if ((beatUnit & 0x7F) <= 6) {
                            orn->noto = beatUnit;
                        }
                    }
                    ds.device()->seek(save);
                }
            }
            return false;
        }

        EncNote* en = dynamic_cast<EncNote*>(elem);
        if (!en) {
            return false;
        }
        // v0xC2 usually keeps the MIDI pitch in the tuplet slot (+13) with +15 empty; swap it
        // across. Some files instead store a real pitch at +15 and a genuine tuplet ratio at +13.
        // Discriminate by whether +15 is a plausible pitch (>= C0), not merely nonzero: a stray
        // small flag there must not be read as MIDI 1. See ENCORE_FORMAT.md §Note element.
        static constexpr quint8 kMinPlausiblePitch = 12; // C0; below this is not a MIDI note
        if (en->tuplet > 0 && en->semiTonePitch < kMinPlausiblePitch) {
            en->semiTonePitch = en->tuplet;
            en->tuplet = 0;
        }
        // Decode tie-sender flag from grace1 low nibble (v0xC2 only).
        en->isTieSender = ((en->grace1 & 0x0F) == 1);
        // size=24 notes carry an articulation byte at +22 (2 bytes after alterGlyph at +21).
        if (en->size == 24 && ds.device()->seek(rawElemStart + 22)) {
            ds >> en->articulationUp;
            en->articulationDown = 0;
        } else {
            en->articulationUp   = 0;
            en->articulationDown = 0;
        }
        return false;
    }

    void postProcessVoiceGroup(std::vector<EncMeasureElem*>& elems, qint16 durTicks) const override
    {
        fixDottedEighthPattern(elems, durTicks);
        markImpliedTupletMembers(elems);
    }
};

std::unique_ptr<EncFormatReader> makeFormatReader_V0xC2()
{
    return std::make_unique<EncFormatReader_V0xC2>();
}
} // namespace mu::iex::enc

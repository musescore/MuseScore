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

// Encore 3.x/4.x (v0xC2) reader: the older articulation numbering, implied-tuplet marking, and
// the two TEMPO/slur layout quirks that diverge from v0xC4.

#include "readers-v0xc2.h"
#include "readers-v0xc4-base.h"

#include <QDataStream>

#include "elem.h"
#include "ticks.h"

namespace mu::iex::enc {
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
        const quint8 fv = first->faceValue4();
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
            const quint8 fvk = ek->faceValue4();
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
//   - grace1 low nibble encodes the tie-sender flag
//   - alMezuro field in ornaments is unreliable
//   - Lyric text starts at element offset +18 (not +20)
//   - NOTE: MIDI pitch is in tuplet slot; semiTonePitch is 0 (swap them in postProcess)
//   - Instrument metadata: names only (no TK-based MIDI/key tables)
struct EncFormatReader_V0xC2 final : EncFormatReader_V0xC4Base
{
    explicit EncFormatReader_V0xC2(quint16 formatVersion)
        : m_formatVersion(formatVersion) {}

    const char* formatName() const override { return "v0xC2"; }
    quint8 lyricTextGapAfterKie() const override { return 7; }

    // Format 3.07 inserted two bytes into every element body at offset +8, so a file written by an
    // earlier build keeps those fields two bytes lower. The format version is what separates the
    // two generations: the version byte is 0xC2 for both, and element sizes overlap between them.
    // See ENCORE_FORMAT.md §1.4 What changed at each boundary.
    int elementBodyShift() const override { return m_formatVersion < ENC_FORMAT_3_07 ? -2 : 0; }

    // v0xC2 instrument entries end two bytes earlier than v0xC4 ones, so their MIDI program
    // table sits 44 bytes from the end rather than 46.
    qint64 midiProgramFromEntryEnd() const override { return 44; }

    // v0xC2 slur xoffset2 lives in a stale ornament-coordinate origin; anchor endpoints
    // explicitly (forward measure-count / next note) instead of by coordinate search.
    bool slurXoffset2Stale() const override { return true; }

    // Format 3.07 moved five articulations down by six; the rest of the vocabulary stayed put.
    // See ENCORE_FORMAT.md §7.6 Articulation bytes.
    quint8 normalizeOrnamentSubtype(quint8 subtype) const override
    {
        if (m_formatVersion >= ENC_FORMAT_3_07) {
            return subtype;
        }
        switch (subtype) {
        case 0xC4: return static_cast<quint8>(EncOrnamentType::ACCENT);
        case 0xCE: return static_cast<quint8>(EncOrnamentType::TENUTO);
        case 0xCF: return static_cast<quint8>(EncOrnamentType::STACCATO);
        case 0xD2: return static_cast<quint8>(EncOrnamentType::FERMATA_ABOVE);
        case 0xD3: return static_cast<quint8>(EncOrnamentType::FERMATA_BELOW);
        default:   return subtype;
        }
    }

    bool postProcessElement(EncMeasureElem* elem, QDataStream& ds, qint64 rawElemStart) const override
    {
        EncFormatReader::postProcessElement(elem, ds, rawElemStart);
        if (EncOrnament* orn = dynamic_cast<EncOrnament*>(elem)) {
            orn->tipo = normalizeOrnamentSubtype(orn->tipo);
            // The forward slur span (0 = within measure, N = ends N bars later) is what anchors the
            // endpoint, since the xoffset2 coordinate is stale in this format. Marking it valid lets
            // the post-pass anchor by measure count instead. See ENCORE_FORMAT.md §6.8 Ornament.
            //
            // elementBodyShift() has already pointed alMezuro at the right byte for the file's
            // generation, so the value read inline is the span in both. Only the trust flag differs
            // by subtype: outside a slur the field is stale in this format.
            orn->alMezuroValid = (orn->tipo == static_cast<quint8>(EncOrnamentType::SLURSTART));
            // v0xC2 has two TEMPO layouts. New (v0xC4-style): beat-unit code at +28, BPM at +30.
            // Old: BPM at +28 (read into noto) with a constant in the +30 slot. Discriminate by
            // whether +28 holds a valid beat-unit code (low 7 bits 0..6). See ENCORE_FORMAT.md §8.2 Ornament subtypes.
            if (orn->tipo == static_cast<quint8>(EncOrnamentType::TEMPO)) {
                const quint8 beatUnitCode = orn->noto & 0x7F;
                const bool validBeatUnit = (orn->noto != 0) && (beatUnitCode <= 6);
                if (!validBeatUnit) {
                    orn->tempo = orn->noto;   // old layout: +28 is the BPM
                    // Old layout keeps the per-mark beat unit at +26; recover it so the mark shows
                    // the composer's unit instead of the compound-meter dotted-quarter default.
                    const quint8 beatUnit = byteAt(ds, rawElemStart + 26);
                    orn->noto = (beatUnit & 0x7F) <= 6 ? beatUnit : 0;
                }
            }
            return false;
        }

        return false;
    }

    void postProcessVoiceGroup(std::vector<EncMeasureElem*>& elems, qint16) const override
    {
        markImpliedTupletMembers(elems);
    }

private:
    quint16 m_formatVersion { 0 };
};

std::unique_ptr<EncFormatReader> makeFormatReader_V0xC2(quint16 formatVersion)
{
    return std::make_unique<EncFormatReader_V0xC2>(formatVersion);
}
} // namespace mu::iex::enc

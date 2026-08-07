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

// Apply articulations, ornaments, tremolos and string numbers to an emitted note/chord.

#include "emitters-internal.h"
#include "mappers.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/tremolosinglechord.h"

namespace mu::iex::enc {
using namespace mu::engraving;

// See ENCORE_IMPORTER.md §Articulations, technical markings, tremolos.
void applyNoteArticulations(BuildCtx& ctx, Note* note, Chord* chord, const EncNote* en,
                            track_idx_t track, const MeasEmitCtx& mc)
{
    auto isOrnamentSymId = [](SymId s) {
        return s == SymId::ornamentTrill
               || s == SymId::ornamentShortTrill
               || s == SymId::ornamentTremblement
               || s == SymId::ornamentMordent
               || s == SymId::ornamentTurn;
    };
    auto isFermataSymId = [](SymId s) {
        return s == SymId::fermataAbove
               || s == SymId::fermataBelow
               || s == SymId::fermataShortAbove
               || s == SymId::fermataShortBelow
               || s == SymId::fermataLongAbove
               || s == SymId::fermataLongBelow;
    };

    // Artic bytes that Encore uses for marks with no MuseScore equivalent.
    static const std::map<quint8, const char*> WARN_BYTES = {
        { 0x01, "flat mark" }, { 0x02, "sharp/natural mark" },
        { 0x09, "wave mark" },
        { 0x47, "stick technique" },
        { 0x48, "brush" }, { 0x49, "soft mallet" }, { 0x4A, "hard mallet" },
    };
    Segment* chordSegForText = chord->segment();
    for (int slot = 0; slot < 2; ++slot) {
        const quint8 check = slot == 0 ? en->articulationUp : en->articulationDown;
        auto it = WARN_BYTES.find(check);
        if (it == WARN_BYTES.end()) {
            continue;
        }
        if (ctx.opts.importUnsupportedArticulationsAsText) {
            StaffText* st = Factory::createStaffText(chordSegForText);
            st->setTrack(track);
            st->setXmlText(String::fromAscii(it->second));
            chordSegForText->add(st);
        } else {
            LOGW() << QString("Encore: artic byte 0x%1 (%2) not imported"
                              " (measure %3 staff %4 tick %5)")
                .arg(check, 2, 16, QChar('0'))
                .arg(it->second)
                .arg(mc.measIdx).arg(static_cast<int>(en->staffIdx))
                .arg(static_cast<int>(en->tick));
        }
    }

    Segment* chordSeg = chord->segment();
    for (int slot = 0; slot < 2; ++slot) {
        const quint8 ab = slot == 0 ? en->articulationUp : en->articulationDown;
        const bool isAbove = slot == 0;
        for (SymId sid : encArticulation2SymIds(ab)) {
            if (sid == SymId::noSym) {
                continue;
            }
            // Fermatas anchor on the segment, not the chord.
            if (isFermataSymId(sid) && chordSeg) {
                // 0x20/0x21 doubles as a "tuplet bracket placement" flag on the last
                // note of a tuplet group; never export as a <fermata> in that context.
                if ((ab == 0x20 || ab == 0x21) && en->tuplet != 0) {
                    continue;
                }
                Fermata* ferm = Factory::createFermata(chordSeg);
                ferm->setTrack(track);
                SymId resolved = sid;
                if (sid == SymId::fermataAbove || sid == SymId::fermataBelow) {
                    resolved = isAbove ? SymId::fermataAbove : SymId::fermataBelow;
                } else if (sid == SymId::fermataShortAbove || sid == SymId::fermataShortBelow) {
                    resolved = isAbove ? SymId::fermataShortAbove : SymId::fermataShortBelow;
                }
                ferm->setSymId(resolved);
                ferm->setPlacement(isAbove ? PlacementV::ABOVE : PlacementV::BELOW);
                ferm->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                chordSeg->add(ferm);
                continue;
            }
            // Ornaments need an Ornament wrapper for MusicXML <ornaments>.
            // Dedup: skip if the chord already has this symbol.
            if (isOrnamentSymId(sid)) {
                bool alreadyHas = false;
                for (Articulation* a : chord->articulations()) {
                    if (a->isOrnament() && toOrnament(a)->symId() == sid) {
                        alreadyHas = true;
                        break;
                    }
                }
                if (!alreadyHas) {
                    Ornament* orn = Factory::createOrnament(chord);
                    orn->setSymId(sid);
                    if (sid == SymId::ornamentTrill) {
                        const auto interval = encArticByteToTrillInterval(ab);
                        if (interval.type != IntervalType::AUTO) {
                            orn->setIntervalAbove(interval);
                        }
                    }
                    chord->add(orn);
                }
            } else {
                bool alreadyHas = false;
                for (Articulation* a : chord->articulations()) {
                    if (a->symId() == sid) {
                        alreadyHas = true;
                        break;
                    }
                }
                if (!alreadyHas) {
                    Articulation* art = Factory::createArticulation(chord);
                    art->setSymId(sid);
                    chord->add(art);
                }
            }
        }
    }

    // Single-note tremolos: stroke count encoded in artic byte low nibble.
    // 0x41/0x42/0x43 = 1/2/3 strokes; 0x03 also = 3 strokes.
    auto tremoloStrokeFromByte = [](quint8 b) -> int {
        if (b == 0x41 || b == 0x42 || b == 0x43) {
            return b & 0x0F;
        }
        if (b == 0x03) {
            return 3;
        }
        return 0;
    };
    int strokes = std::max(tremoloStrokeFromByte(en->articulationUp),
                           tremoloStrokeFromByte(en->articulationDown));
    if (strokes > 0 && !chord->tremoloSingleChord()) {
        TremoloSingleChord* trem = Factory::createTremoloSingleChord(chord);
        TremoloType type = TremoloType::R8;
        switch (strokes) {
        case 1: type = TremoloType::R8;
            break;
        case 2: type = TremoloType::R16;
            break;
        case 3: type = TremoloType::R32;
            break;
        case 4: type = TremoloType::R64;
            break;
        default: break;
        }
        trem->setTremoloType(type);
        chord->add(trem);
    }

    // Scale string numbers (0x39..0x40 = string 1..8, "au" articulation byte).
    // Fallback: options bit 0 + position field when the measure has at least one
    // explicit anchor (mc.hasScaleStringAnchors).
    const int scaleSn = encArticByteToScaleStringNumber(en->articulationUp);
    if (scaleSn > 0) {
        Fingering* fg = Factory::createFingering(note, TextStyleType::STRING_NUMBER);
        fg->setTrack(track);
        fg->setXmlText(String::number(scaleSn));
        note->add(fg);
    } else if (mc.hasScaleStringAnchors
               && (en->options & 0x01)
               && en->position >= 0 && en->position <= 7
               && en->articulationUp == 0
               && en->articulationDown == 0) {
        Fingering* fg = Factory::createFingering(note, TextStyleType::STRING_NUMBER);
        fg->setTrack(track);
        fg->setXmlText(String::number(static_cast<int>(en->position) + 1));
        note->add(fg);
    }
}
} // namespace mu::iex::enc

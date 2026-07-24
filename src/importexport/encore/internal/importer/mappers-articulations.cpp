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

// Map Encore articulation/ornament bytes to MuseScore SymIds, finger numbers and string numbers.

#include "mappers.h"

using namespace mu::engraving;

namespace mu::iex::enc {
std::vector<mu::engraving::SymId> encArticulation2SymIds(quint8 articByte)
{
    using mu::engraving::SymId;
    // Byte encodes one or two glyphs (e.g. 0x24=tenuto+staccato). See ENCORE_FORMAT.md §Articulation bytes.
    switch (articByte) {
    // Trill/mordent: 0x04..0x07=trill, 0x08=turn, 0x09=wave(no MS equivalent),
    // 0x0A=inv-mordent(short),
    // 0x0C=inv-mordent(long/double)=tremblement, 0x0B=mordent(simple), 0x2F=mordent(double/long)=prallMordent.
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07: return { SymId::ornamentTrill };
    case 0x08: return { SymId::ornamentTurn };
    case 0x09: return {};  // wave mark, no MuseScore equivalent, skip
    case 0x0A: return { SymId::ornamentShortTrill };    // <inverted-mordent>
    case 0x0C: return { SymId::ornamentTremblement };   // <inverted-mordent long="yes">
    case 0x0B: return { SymId::ornamentMordent };       // simple lower mordent
    case 0x2F: return { SymId::ornamentPrallMordent };  // double/long lower mordent (parallel to 0x0C = ornamentTremblement for inverted)
    case 0x2E: return { SymId::ornamentTurnInverted };  // inverted turn
    case 0x12: return { SymId::articAccentAbove };
    case 0x13: return { SymId::articMarcatoAbove };
    case 0x14: return { SymId::articMarcatoStaccatoBelow };   // staccato + heavy accent (∨) as single glyph
    case 0x15: return { SymId::articMarcatoStaccatoAbove };   // marcato + staccato as single glyph
    case 0x16: return { SymId::articAccentAbove, SymId::articStaccatissimoAbove };
    case 0x17: return { SymId::articAccentStaccatoAbove };    // accent + staccato as single glyph
    case 0x18: return { SymId::stringsUpBow };
    case 0x19: return { SymId::stringsDownBow };
    case 0x1A: return { SymId::articMarcatoAbove };
    case 0x1C: return { SymId::articTenutoAbove };
    case 0x1D: return { SymId::articStaccatoAbove };
    case 0x20:
    case 0x21: return { SymId::fermataAbove };
    // Combined-articulation bytes come in (below, above) pairs, one pair per glyph.
    // Placement is taken from the note articulation slot; subtype() normalises Below->Above.
    case 0x22:                                                // tenuto + accent (below)
    case 0x23: return { SymId::articTenutoAccentAbove };      // tenuto + accent (above)
    case 0x24:                                                // tenuto + staccato (portato, below)
    case 0x25: return { SymId::articTenutoStaccatoAbove };    // tenuto + staccato (portato, above)
    case 0x26: return { SymId::articMarcatoTenutoBelow };     // tenuto + heavy accent (∨) below
    case 0x27: return { SymId::articMarcatoTenutoAbove };     // tenuto + heavy accent (above)
    case 0x28:
    case 0x29: return { SymId::articStaccatissimoAbove };
    case 0x2A:                                                                       // heavy accent (∨) + staccatissimo (below)
    case 0x2B: return { SymId::articMarcatoAbove, SymId::articStaccatissimoAbove };   // heavy accent + staccatissimo (above)
    case 0x2C:                                                                       // tenuto + staccatissimo (below)
    case 0x2D: return { SymId::articTenutoAbove, SymId::articStaccatissimoAbove };    // tenuto + staccatissimo (above)
    case 0x1B: return { SymId::brassMuteClosed };        // technical/stopped (+)
    case 0x30: return { SymId::brassMuteHalfClosed };   // technical/stopped (tick/half stopped)
    // String markings (m3, m4, m18): 0x1E/0x1F=harmonic, 0x44/0x45=thumb-position.
    // 0x46=open-string: handled in encArticByteIsOpenString() (no SymId; uses Fingering "0").
    // 0x47=stick (drumstick technique): not a string number, left unmapped.
    case 0x1E:
    case 0x1F: return { SymId::stringsHarmonic };
    case 0x44:
    case 0x45: return { SymId::stringsThumbPosition };
    default:
        return {};
    }
}

int encArticByteToFingerNumber(quint8 articByte)
{
    // Finger 1..5 map to bytes 0x0D..0x11.
    switch (articByte) {
    case 0x0D: return 1;
    case 0x0E: return 2;
    case 0x0F: return 3;
    case 0x10: return 4;
    case 0x11: return 5;
    default:   return 0;
    }
}

bool encArticByteIsOpenString(quint8 articByte)
{
    // 0x46=open-string; emitted as Fingering "0" (STRING_NUMBER style).
    return articByte == 0x46;
}

// Scale string-number artic bytes: 0x39-0x40 encode string numbers 1-8 as (byte - kStringArticBase).
static constexpr quint8 kStringArticBase = 0x38;
static constexpr quint8 kStringArticMin  = 0x39;   // string 1
static constexpr quint8 kStringArticMax  = 0x40;   // string 8

int encArticByteToScaleStringNumber(quint8 articByte)
{
    // These appear as explicit anchors in scale exercises; their presence in a
    // measure enables options-bit-0 string number display on all other notes.
    if (articByte >= kStringArticMin && articByte <= kStringArticMax) {
        return static_cast<int>(articByte) - static_cast<int>(kStringArticBase);
    }
    return 0;
}

mu::engraving::OrnamentInterval encArticByteToTrillInterval(quint8 articByte)
{
    using mu::engraving::IntervalStep;
    using mu::engraving::IntervalType;
    // Trill artic bytes 0x04..0x07 share the trill glyph but carry an accidental:
    //   0x04: no accidental (AUTO = use key context)
    //   0x05: flat  → minor second above (MINOR)
    //   0x06: sharp → augmented second above (AUGMENTED)
    //   0x07: natural → major second above (MAJOR)
    switch (articByte) {
    case 0x05: return { IntervalStep::SECOND, IntervalType::MINOR };
    case 0x06: return { IntervalStep::SECOND, IntervalType::AUGMENTED };
    case 0x07: return { IntervalStep::SECOND, IntervalType::MAJOR };
    default:   return {};   // AUTO (default)
    }
}

DynamicType encOrnType2DynamicType(EncOrnamentType ot)
{
    switch (ot) {
    case EncOrnamentType::DYN_PPP:  return DynamicType::PPP;
    case EncOrnamentType::DYN_PP:   return DynamicType::PP;
    case EncOrnamentType::DYN_P:    return DynamicType::P;
    case EncOrnamentType::DYN_MP:   return DynamicType::MP;
    case EncOrnamentType::DYN_MF:   return DynamicType::MF;
    case EncOrnamentType::DYN_F:    return DynamicType::F;
    case EncOrnamentType::DYN_FF:   return DynamicType::FF;
    case EncOrnamentType::DYN_FFF:  return DynamicType::FFF;
    case EncOrnamentType::DYN_SFZ:  return DynamicType::SFZ;
    case EncOrnamentType::DYN_SFFZ: return DynamicType::SFFZ;
    case EncOrnamentType::DYN_FP:   return DynamicType::FP;
    case EncOrnamentType::DYN_FZ:   return DynamicType::FZ;
    case EncOrnamentType::DYN_SF:   return DynamicType::SF;
    default:                        return DynamicType::OTHER;
    }
}
} // namespace mu::iex::enc

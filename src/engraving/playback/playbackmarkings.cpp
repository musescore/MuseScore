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

#include "playbackmarkings.h"

#include <map>
#include <utility>

#include <QLatin1StringView>
#include <QString>

#include "translation.h"

#include "../dom/arpeggio.h"
#include "../dom/articulation.h"
#include "../dom/chord.h"
#include "../dom/glissando.h"
#include "../dom/tremolosinglechord.h"
#include "../dom/tremolotwochord.h"
#include "../dom/trill.h"
#include "../types/symnames.h"

namespace mu::engraving {
//! The panel's strings for each marking. Symbols that come in "Above" and "Below" versions are listed by their
//! "Above" version (see aboveVersion).
namespace markingstrings {
//! The line's glyph without its name
constexpr MarkingLine glyphOnly(const MarkingLine& line)
{
    return { line.glyph, nullptr };
}

// --- Articulations
constexpr MarkingLine ACCENT { SymId::articAccentAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent") };
constexpr MarkingLine ACCENT_STACCATO { SymId::articAccentStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Staccato") };
constexpr MarkingLine LAISSEZ_VIBRER { SymId::articLaissezVibrerAbove, QT_TRANSLATE_NOOP("propertiespanel", "Laissez Vibrer (l.v.)") };
constexpr MarkingLine MARCATO { SymId::articMarcatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Marcato") };
constexpr MarkingLine MARCATO_STACCATO { SymId::articMarcatoStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Marcato-Staccato") };
constexpr MarkingLine MARCATO_TENUTO { SymId::articMarcatoTenutoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Marcato-Tenuto") };
constexpr MarkingLine SOFT_ACCENT { SymId::articSoftAccentAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent (soft)") };
constexpr MarkingLine SOFT_ACCENT_STACCATO {
    SymId::articSoftAccentStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Staccato (soft)")
};
constexpr MarkingLine SOFT_ACCENT_TENUTO {
    SymId::articSoftAccentTenutoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Tenuto (soft)")
};
constexpr MarkingLine SOFT_ACCENT_TENUTO_STACCATO {
    SymId::articSoftAccentTenutoStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Tenuto-Staccato (soft)")
};
constexpr MarkingLine STACCATISSIMO { SymId::articStaccatissimoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccatissimo") };
constexpr MarkingLine STACCATISSIMO_STROKE {
    SymId::articStaccatissimoStrokeAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccatissimo Stroke")
};
constexpr MarkingLine STACCATISSIMO_WEDGE {
    SymId::articStaccatissimoWedgeAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccatissimo Wedge")
};
constexpr MarkingLine STACCATO { SymId::articStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccato") };
constexpr MarkingLine TENUTO { SymId::articTenutoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Tenuto") };
constexpr MarkingLine TENUTO_STACCATO { SymId::articTenutoStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Tenuto-Staccato") };
constexpr MarkingLine TENUTO_ACCENT { SymId::articTenutoAccentAbove, QT_TRANSLATE_NOOP("propertiespanel", "Tenuto-Accent") };

// --- Dynamics inside symbols
constexpr MarkingLine SFORZANDO { SymId::dynamicSforzando, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando (s)") };
constexpr MarkingLine SFORZANDO_1 { SymId::dynamicSforzando1, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando (sf)") };
constexpr MarkingLine SFORZANDO_PIANISSIMO {
    SymId::dynamicSforzandoPianissimo, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando-Pianissimo (sfpp)")
};
constexpr MarkingLine SFORZANDO_PIANO { SymId::dynamicSforzandoPiano, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando-Piano (sfp)") };
constexpr MarkingLine SFORZATO { SymId::dynamicSforzato, QT_TRANSLATE_NOOP("propertiespanel", "Sforzato (sfz)") };
constexpr MarkingLine SFORZATO_FF { SymId::dynamicSforzatoFF, QT_TRANSLATE_NOOP("propertiespanel", "Sforzato-FF (sffz)") };
constexpr MarkingLine SFORZATO_PIANO { SymId::dynamicSforzatoPiano, QT_TRANSLATE_NOOP("propertiespanel", "Sforzato-Piano (sfzp)") };
constexpr MarkingLine CRESC_HAIRPIN { SymId::dynamicCrescendoHairpin, QT_TRANSLATE_NOOP("propertiespanel", "Cresc. Hairpin") };
constexpr MarkingLine DIM_HAIRPIN { SymId::dynamicDiminuendoHairpin, QT_TRANSLATE_NOOP("propertiespanel", "Dim. Hairpin") };

// --- Notes under a marking's row
constexpr MarkingLine STRINGS_ONLY { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Strings only)") };
constexpr MarkingLine BOWED_ONLY { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Bowed only)") };
constexpr MarkingLine DRUMS_ONLY { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Drums only)") };
//! Playback caps tremolos at 64ths (TremoloMetaParser)
constexpr MarkingLine PLAYS_AS_64THS { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Plays as 64ths)") };

// --- Spanners
constexpr MarkingLine LEGATO { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Legato") };
constexpr MarkingLine PEDAL { SymId::keyboardPedalPed, QT_TRANSLATE_NOOP("propertiespanel", "Pedal") };

//! Shown in the Other section, with note (if any) under it
MarkingDescription inOther(const MarkingLine& line, const MarkingLine& note = {})
{
    if (!note.name) {
        return { .other = { line } };
    }

    return { .other = { line, note } };
}

//! A symbol's entry: shown in the Other section, with note (if any) under it
std::pair<const SymId, MarkingDescription> symbolInOther(const MarkingLine& line, const MarkingLine& note = {})
{
    return { line.glyph, inOther(line, note) };
}

//! A symbol's entry: its glyph alone in the Dynamic section
std::pair<const SymId, MarkingDescription> symbolInDynamic(const MarkingLine& line)
{
    return { line.glyph, { .dynamic = { glyphOnly(line) } } };
}

const std::map<SymId, MarkingDescription>& symbols()
{
    static const std::map<SymId, MarkingDescription> table {
        // --- Articulations
        { ACCENT.glyph, { .dynamic = { ACCENT } } },
        { ACCENT_STACCATO.glyph, { .tempo = { ACCENT_STACCATO, STACCATO }, .dynamic = { ACCENT_STACCATO, ACCENT } } },
        { LAISSEZ_VIBRER.glyph, { .tempo = { LAISSEZ_VIBRER } } },
        { MARCATO.glyph, { .dynamic = { MARCATO } } },
        { MARCATO_STACCATO.glyph, { .tempo = { MARCATO_STACCATO, STACCATO }, .dynamic = { MARCATO_STACCATO, MARCATO } } },
        { MARCATO_TENUTO.glyph, { .tempo = { MARCATO_TENUTO, TENUTO }, .dynamic = { MARCATO_TENUTO, MARCATO } } },
        { SOFT_ACCENT.glyph, { .dynamic = { SOFT_ACCENT } } },
        {
            SOFT_ACCENT_STACCATO.glyph,
            { .tempo = { SOFT_ACCENT_STACCATO, STACCATO }, .dynamic = { SOFT_ACCENT_STACCATO, SOFT_ACCENT } }
        },
        {
            SOFT_ACCENT_TENUTO.glyph,
            { .tempo = { SOFT_ACCENT_TENUTO, TENUTO }, .dynamic = { SOFT_ACCENT_TENUTO, SOFT_ACCENT } }
        },
        {
            SOFT_ACCENT_TENUTO_STACCATO.glyph,
            {
                .tempo = { SOFT_ACCENT_TENUTO_STACCATO, TENUTO, STACCATO },
                .dynamic = { SOFT_ACCENT_TENUTO_STACCATO, SOFT_ACCENT }
            }
        },
        { STACCATISSIMO.glyph, { .tempo = { STACCATISSIMO } } },
        { STACCATISSIMO_STROKE.glyph, { .tempo = { STACCATISSIMO_STROKE } } },
        { STACCATISSIMO_WEDGE.glyph, { .tempo = { STACCATISSIMO_WEDGE } } },
        { STACCATO.glyph, { .tempo = { STACCATO } } },
        { TENUTO.glyph, { .tempo = { TENUTO } } },
        { TENUTO_STACCATO.glyph, { .tempo = { TENUTO_STACCATO, TENUTO, STACCATO } } },
        { TENUTO_ACCENT.glyph, { .tempo = { TENUTO_ACCENT, TENUTO }, .dynamic = { TENUTO_ACCENT, ACCENT } } },

        // --- Dynamics inside symbols
        symbolInDynamic(SFORZANDO),
        symbolInDynamic(SFORZANDO_1),
        symbolInDynamic(SFORZANDO_PIANISSIMO),
        symbolInDynamic(SFORZANDO_PIANO),
        symbolInDynamic(SFORZATO),
        symbolInDynamic(SFORZATO_FF),
        symbolInDynamic(SFORZATO_PIANO),
        symbolInDynamic(CRESC_HAIRPIN),
        symbolInDynamic(DIM_HAIRPIN),

        // --- Other symbols
        symbolInOther({ SymId::pluckedSnapPizzicatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Snap Pizzicato") }, STRINGS_ONLY),
        symbolInOther({ SymId::stringsDownBow, QT_TRANSLATE_NOOP("propertiespanel", "Down Bow") }, BOWED_ONLY),
        symbolInOther({ SymId::stringsHarmonic, QT_TRANSLATE_NOOP("propertiespanel", "Harmonic") }, STRINGS_ONLY),
        symbolInOther({ SymId::stringsUpBow, QT_TRANSLATE_NOOP("propertiespanel", "Up Bow") }, BOWED_ONLY),

        // --- Ornaments
        symbolInOther({ SymId::ornamentDownMordent, QT_TRANSLATE_NOOP("propertiespanel", "Down Mordent") }),
        symbolInOther({ SymId::ornamentHaydn, QT_TRANSLATE_NOOP("propertiespanel", "Haydn Ornament") }),
        symbolInOther({ SymId::ornamentLinePrall, QT_TRANSLATE_NOOP("propertiespanel", "Line Prall") }),
        symbolInOther({ SymId::ornamentMordent, QT_TRANSLATE_NOOP("propertiespanel", "Mordent") }),
        symbolInOther({ SymId::ornamentPinceCouperin, QT_TRANSLATE_NOOP("propertiespanel", "Pincé (Couperin)") }),
        symbolInOther({ SymId::ornamentPrallDown, QT_TRANSLATE_NOOP("propertiespanel", "Prall Down") }),
        symbolInOther({ SymId::ornamentPrallMordent, QT_TRANSLATE_NOOP("propertiespanel", "Prall Mordent") }),
        symbolInOther({ SymId::ornamentPrallUp, QT_TRANSLATE_NOOP("propertiespanel", "Prall Up") }),
        symbolInOther({ SymId::ornamentShake3, QT_TRANSLATE_NOOP("propertiespanel", "Shake") }),
        symbolInOther({ SymId::ornamentShakeMuffat1, QT_TRANSLATE_NOOP("propertiespanel", "Shake (Muffat)") }),
        symbolInOther({ SymId::ornamentShortTrill, QT_TRANSLATE_NOOP("propertiespanel", "Trill (short)") }),
        symbolInOther({ SymId::ornamentTremblement, QT_TRANSLATE_NOOP("propertiespanel", "Tremblement") }),
        symbolInOther({ SymId::ornamentTremblementCouperin, QT_TRANSLATE_NOOP("propertiespanel", "Tremblement appuyé (Couperin)") }),
        symbolInOther({ SymId::ornamentTrill, QT_TRANSLATE_NOOP("propertiespanel", "Trill") }),
        symbolInOther({ SymId::ornamentTurn, QT_TRANSLATE_NOOP("propertiespanel", "Turn") }),
        symbolInOther({ SymId::ornamentTurnInverted, QT_TRANSLATE_NOOP("propertiespanel", "Turn (inverted)") }),
        symbolInOther({ SymId::ornamentTurnSlash, QT_TRANSLATE_NOOP("propertiespanel", "Turn (w/ slash)") }),
        symbolInOther({ SymId::ornamentTurnUp, QT_TRANSLATE_NOOP("propertiespanel", "Turn (up)") }),
        symbolInOther({ SymId::ornamentTurnUpS, QT_TRANSLATE_NOOP("propertiespanel", "Turn (inverted, up)") }),
        symbolInOther({ SymId::ornamentUpMordent, QT_TRANSLATE_NOOP("propertiespanel", "Up Mordent") }),
        symbolInOther({ SymId::ornamentUpPrall, QT_TRANSLATE_NOOP("propertiespanel", "Up Prall") }),
    };

    return table;
}

//! SMuFL's strokes stop at 5 (128ths), so 256ths share them
const std::map<TremoloType, MarkingDescription>& tremolos()
{
    static const std::map<TremoloType, MarkingDescription> table {
        { TremoloType::BUZZ_ROLL, inOther({ SymId::buzzRoll, QT_TRANSLATE_NOOP("propertiespanel", "Buzz Roll") }, DRUMS_ONLY) },
        { TremoloType::R8, inOther({ SymId::tremolo1, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (8ths, 1 note)") }) },
        { TremoloType::R16, inOther({ SymId::tremolo2, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (16ths, 1 note)") }) },
        { TremoloType::R32, inOther({ SymId::tremolo3, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (32nds, 1 note)") }) },
        { TremoloType::R64, inOther({ SymId::tremolo4, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (64ths, 1 note)") }) },
        {
            TremoloType::R128,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (128ths, 1 note)") }, PLAYS_AS_64THS)
        },
        {
            TremoloType::R256,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (256ths, 1 note)") }, PLAYS_AS_64THS)
        },
        { TremoloType::C8, inOther({ SymId::tremolo1, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (8ths, 2 notes)") }) },
        { TremoloType::C16, inOther({ SymId::tremolo2, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (16ths, 2 notes)") }) },
        { TremoloType::C32, inOther({ SymId::tremolo3, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (32nds, 2 notes)") }) },
        { TremoloType::C64, inOther({ SymId::tremolo4, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (64ths, 2 notes)") }) },
        {
            TremoloType::C128,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (128ths, 2 notes)") }, PLAYS_AS_64THS)
        },
        {
            TremoloType::C256,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (256ths, 2 notes)") }, PLAYS_AS_64THS)
        },
    };

    return table;
}

//! The plain arpeggio has no glyph of its own (the score draws it from wiggle segments turned on their side)
const std::map<ArpeggioType, MarkingDescription>& arpeggios()
{
    static const std::map<ArpeggioType, MarkingDescription> table {
        { ArpeggioType::NORMAL, inOther({ SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio") }) },
        { ArpeggioType::UP, inOther({ SymId::arpeggiatoUp, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (up)") }) },
        { ArpeggioType::DOWN, inOther({ SymId::arpeggiatoDown, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (down)") }) },
        {
            ArpeggioType::UP_STRAIGHT,
            inOther({ SymId::arrowBlackUp, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (up, straight)") })
        },
        {
            ArpeggioType::DOWN_STRAIGHT,
            inOther({ SymId::arrowBlackDown, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (down, straight)") })
        },
    };

    return table;
}

const std::map<TrillType, MarkingDescription>& trills()
{
    static const std::map<TrillType, MarkingDescription> table {
        { TrillType::TRILL_LINE, inOther({ SymId::ornamentTrill, QT_TRANSLATE_NOOP("propertiespanel", "Trill Line") }) },
        { TrillType::UPPRALL_LINE, inOther({ SymId::ornamentUpPrall, QT_TRANSLATE_NOOP("propertiespanel", "Up Prall") }) },
        {
            TrillType::DOWNPRALL_LINE,
            inOther({ SymId::ornamentPrecompMordentUpperPrefix, QT_TRANSLATE_NOOP("propertiespanel", "Down Prall") })
        },
        { TrillType::PRALLPRALL_LINE, inOther({ SymId::ornamentLinePrall, QT_TRANSLATE_NOOP("propertiespanel", "Line Prall") }) },
    };

    return table;
}

//! Neither kind has a glyph that reads as a glissando
const std::map<GlissandoType, MarkingDescription>& glissandos()
{
    static const std::map<GlissandoType, MarkingDescription> table {
        { GlissandoType::WAVY, inOther({ SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Glissando") }) },
        { GlissandoType::STRAIGHT, inOther({ SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Glissando (straight)") }) },
    };

    return table;
}

const std::map<NoteType, MarkingDescription>& graceNotes()
{
    constexpr SymId APPOGGIATURA = SymId::graceNoteAppoggiaturaStemUp;

    static const std::map<NoteType, MarkingDescription> table {
        {
            NoteType::ACCIACCATURA,
            inOther({ SymId::graceNoteAcciaccaturaStemUp, QT_TRANSLATE_NOOP("propertiespanel", "Acciaccatura") })
        },
        { NoteType::APPOGGIATURA, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura") }) },
        { NoteType::GRACE4, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (4th, leading)") }) },
        { NoteType::GRACE16, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (16th, leading)") }) },
        { NoteType::GRACE32, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (32nd, leading)") }) },
        { NoteType::GRACE8_AFTER, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (8th, trailing)") }) },
        {
            NoteType::GRACE16_AFTER,
            inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (16th, trailing)") })
        },
        {
            NoteType::GRACE32_AFTER,
            inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (32nd, trailing)") })
        },
    };

    return table;
}

//! For kinds of items the table covers but doesn't list
const MarkingDescription HIDDEN;

template<typename Key>
const MarkingDescription* lookUp(const std::map<Key, MarkingDescription>& table, Key key)
{
    const auto it = table.find(key);

    return (it != table.end())
           ? &it->second
           : &HIDDEN;
}

//! The "Above" version of a "…Below" symbol: the panel shows both the same, as some "Below" glyphs sit badly in a line
SymId aboveVersion(SymId id)
{
    static const QLatin1StringView below("Below");

    const QLatin1StringView name = SymNames::nameForSymId(id).toQLatin1String();

    if (!name.endsWith(below)) {
        return id;
    }

    const QString aboveName = QString(name.chopped(below.size())) + QStringLiteral("Above");

    return SymNames::symIdByName(String::fromQString(aboveName), id);
}
} // namespace markingstrings

const MarkingDescription* markingDescription(const EngravingItem* item)
{
    if (item->isArticulationFamily()) {
        const SymId symbol = markingstrings::aboveVersion(static_cast<const Articulation*>(item)->symId());

        return markingstrings::lookUp(markingstrings::symbols(), symbol);
    }

    if (item->type() == ElementType::TREMOLO_SINGLECHORD) {
        return markingstrings::lookUp(markingstrings::tremolos(), item_cast<const TremoloSingleChord*>(item)->tremoloType());
    }

    if (item->type() == ElementType::TREMOLO_TWOCHORD) {
        return markingstrings::lookUp(markingstrings::tremolos(), item_cast<const TremoloTwoChord*>(item)->tremoloType());
    }

    if (item->isArpeggio()) {
        return markingstrings::lookUp(markingstrings::arpeggios(), toArpeggio(item)->arpeggioType());
    }

    if (item->isTrill()) {
        return markingstrings::lookUp(markingstrings::trills(), toTrill(item)->trillType());
    }

    if (item->isGlissando()) {
        return markingstrings::lookUp(markingstrings::glissandos(), toGlissando(item)->glissandoType());
    }

    if (item->isChord() && toChord(item)->isGrace()) {
        return markingstrings::lookUp(markingstrings::graceNotes(), toChord(item)->noteType());
    }

    if (item->isSlur()) {
        static const MarkingDescription slur { .tempo = { markingstrings::LEGATO } };
        return &slur;
    }

    if (item->isPedal()) {
        static const MarkingDescription pedal = markingstrings::inOther(markingstrings::PEDAL);
        return &pedal;
    }

    const bool isHidden = item->isNote() || item->isChordLine() || item->isVibrato() || item->isLetRing()
                          || item->isHammerOnPullOff() || item->isPalmMute() || item->isGuitarBend();

    return isHidden
           ? &markingstrings::HIDDEN
           : nullptr;
}

bool isListedMarking(const EngravingItem* item)
{
    const MarkingDescription* description = markingDescription(item);

    return !description || !description->tempo.empty() || !description->dynamic.empty() || !description->other.empty();
}
} // namespace mu::engraving

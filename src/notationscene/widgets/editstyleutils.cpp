/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "editstyleutils.h"

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/types/types.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::notation;

// Keep in sync with ALL_PAGE_CODES in editstyle.cpp
QString EditStyleUtils::pageCodeForElement(const EngravingItem* element)
{
    IF_ASSERT_FAILED(element) {
        return QString();
    }

    switch (element->type()) {
    case ElementType::SCORE:
        return "score";

    case ElementType::PAGE:
        return "spacing";

    case ElementType::INSTRUMENT_NAME:
    case ElementType::TEXT:
    case ElementType::HARP_DIAGRAM: {
        if (element->isText()) {
            if (toText(element)->textStyleType() == TextStyleType::FOOTER
                || toText(element)->textStyleType() == TextStyleType::HEADER) {
                return "header-and-footer";
            }
        }
        return "text-styles";
    }

    case ElementType::MEASURE_NUMBER:
    case ElementType::MMREST_RANGE:
        return "measure-number";

    case ElementType::BRACKET:
    case ElementType::BRACKET_ITEM:
    case ElementType::SYSTEM_DIVIDER:
        return "system";

    case ElementType::CLEF:
    case ElementType::KEYSIG:
    case ElementType::TIMESIG:
        return "clefs-key-and-time-signatures";

    case ElementType::ACCIDENTAL:
        return "accidentals";

    case ElementType::MEASURE:
        return "spacing";

    case ElementType::BAR_LINE:
        return "barlines";

    case ElementType::NOTE:
    case ElementType::CHORD:
    case ElementType::STEM:
    case ElementType::STEM_SLASH:
    case ElementType::LEDGER_LINE:
    case ElementType::NOTEDOT:
        return "notes";

    case ElementType::REST:
    case ElementType::MMREST:
        return "rests";

    case ElementType::MEASURE_REPEAT:
    case ElementType::PLAY_COUNT_TEXT:
        return "repeats";

    case ElementType::BEAM:
        return "beams";

    case ElementType::TUPLET:
        return "tuplets";

    case ElementType::ARPEGGIO:
        return "arpeggios";

    case ElementType::SLUR:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE:
    case ElementType::TIE_SEGMENT:
    case ElementType::LAISSEZ_VIB:
    case ElementType::LAISSEZ_VIB_SEGMENT:
    case ElementType::PARTIAL_TIE:
    case ElementType::PARTIAL_TIE_SEGMENT:
        return "slurs-and-ties";

    case ElementType::HAMMER_ON_PULL_OFF:
    case ElementType::HAMMER_ON_PULL_OFF_SEGMENT:
    case ElementType::HAMMER_ON_PULL_OFF_TEXT:
        return "hammer-ons-pull-offs-and-tapping";

    case ElementType::HAIRPIN:
    case ElementType::HAIRPIN_SEGMENT:
        return "dynamics-hairpins";

    case ElementType::VOLTA:
    case ElementType::VOLTA_SEGMENT:
        return "volta";

    case ElementType::OTTAVA:
    case ElementType::OTTAVA_SEGMENT:
        return "ottava";

    case ElementType::PEDAL:
    case ElementType::PEDAL_SEGMENT:
        return "pedal";

    case ElementType::TRILL:
    case ElementType::TRILL_SEGMENT:
        return "trill";

    case ElementType::VIBRATO:
    case ElementType::VIBRATO_SEGMENT:
        return "vibrato";

    case ElementType::BEND:
    case ElementType::GUITAR_BEND:
    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GUITAR_BEND_HOLD:
    case ElementType::GUITAR_BEND_HOLD_SEGMENT:
    case ElementType::GUITAR_BEND_TEXT:
        return "bend";

    case ElementType::TEXTLINE:
        return element->isTextLine() && toTextLine(element)->systemFlag() ? "system-text-line" : "text-line";
    case ElementType::TEXTLINE_SEGMENT:
        return element->isTextLineSegment() && toTextLineSegment(element)->systemFlag()
               ? "system-text-line" : "text-line";

    case ElementType::GLISSANDO:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::NOTELINE:
    case ElementType::NOTELINE_SEGMENT:
        return "glissando-note-line";

    case ElementType::ARTICULATION:
        return "articulations-and-ornaments";

    case ElementType::FERMATA:
        return "fermatas";

    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::STAFF_TEXT:
        return "staff-text";

    case ElementType::TEMPO_TEXT:
        return "tempo-text";

    case ElementType::LYRICS:
    case ElementType::LYRICSLINE:
    case ElementType::PARTIAL_LYRICSLINE:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT:
        return "lyrics";

    case ElementType::EXPRESSION:
        return "expression";

    case ElementType::DYNAMIC:
        return "dynamics-hairpins";

    case ElementType::REHEARSAL_MARK:
        return "rehearsal-marks";

    case ElementType::FIGURED_BASS:
        return "figured-bass";

    case ElementType::HARMONY:
        return "chord-symbols";

    case ElementType::FRET_DIAGRAM:
        return "fretboard-diagrams";

    default: return QString();
    }
}

// Keep in sync with ALL_TEXT_STYLE_SUBPAGE_CODES in editstyle.cpp
QString EditStyleUtils::subPageCodeForElement(const EngravingItem* element)
{
    IF_ASSERT_FAILED(element) {
        return QString();
    }

    if (pageCodeForElement(element) == "text-styles" && element->isTextBase()) {
        switch (toTextBase(element)->textStyleType()) {
        case TextStyleType::TITLE:
            return "title";

        case TextStyleType::SUBTITLE:
            return "subtitle";

        case TextStyleType::COMPOSER:
            return "composer";

        case TextStyleType::LYRICIST:
            return "poet";

        case TextStyleType::TRANSLATOR:
            return "translator";

        case TextStyleType::FRAME:
            return "frame";

        case TextStyleType::INSTRUMENT_EXCERPT:
            return "instrument-name-part";

        case TextStyleType::INSTRUMENT_LONG:
            return "instrument-name-long";

        case TextStyleType::INSTRUMENT_SHORT:
            return "instrument-name-short";

        case TextStyleType::INSTRUMENT_CHANGE:
            return "instrument-change";

        case TextStyleType::HEADER:
            return "header";

        case TextStyleType::FOOTER:
            return "footer";

        case TextStyleType::COPYRIGHT:
            return "copyright";

        case TextStyleType::PAGE_NUMBER:
            return "page-number";

        case TextStyleType::MEASURE_NUMBER:
            return "measure-number";

        case TextStyleType::MEASURE_NUMBER_ALTERNATE:
            return "measure-number-alternate";

        case TextStyleType::MMREST_RANGE:
            return "multimeasure-rest-range";

        case TextStyleType::TEMPO:
            return "tempo";

        case TextStyleType::TEMPO_CHANGE:
            return "tempo-change";

        case TextStyleType::METRONOME:
            return "metronome";

        case TextStyleType::REPEAT_PLAY_COUNT:
            return "repeat-play-count";

        case TextStyleType::REPEAT_LEFT:
            return "repeat-text-left";

        case TextStyleType::REPEAT_RIGHT:
            return "repeat-text-right";

        case TextStyleType::REHEARSAL_MARK:
            return "rehearsal-mark";

        case TextStyleType::SYSTEM:
            return "system";

        case TextStyleType::STAFF:
            return "staff";

        case TextStyleType::EXPRESSION:
            return "expression";

        case TextStyleType::HAIRPIN:
            return "hairpin";

        case TextStyleType::LYRICS_ODD:
            return "lyrics-odd-lines";

        case TextStyleType::LYRICS_EVEN:
            return "lyrics-even-lines";

        case TextStyleType::HARMONY_A:
            return "chord-symbols";

        case TextStyleType::HARMONY_B:
            return "chord-symbols-alternate";

        case TextStyleType::HARMONY_ROMAN:
            return "roman-numeral-analysis";

        case TextStyleType::HARMONY_NASHVILLE:
            return "nashville-number";

        case TextStyleType::TUPLET:
            return "tuplet";

        case TextStyleType::ARTICULATION:
            return "articulation";

        case TextStyleType::STICKING:
            return "sticking";

        case TextStyleType::FINGERING:
            return "fingering";

        case TextStyleType::TAB_FRET_NUMBER:
            return "tab-fret-number";

        case TextStyleType::LH_GUITAR_FINGERING:
            return "lh-guitar-fingering";

        case TextStyleType::RH_GUITAR_FINGERING:
            return "rh-guitar-fingering";

        case TextStyleType::HAMMER_ON_PULL_OFF:
            return "hammer-ons-pull-offs-and-tapping";

        case TextStyleType::STRING_NUMBER:
            return "string-number";

        case TextStyleType::STRING_TUNINGS:
            return "string-tunings";

        case TextStyleType::FRET_DIAGRAM_FINGERING:
            return "fretboard-diagram-fingering";

        case TextStyleType::FRET_DIAGRAM_FRET_NUMBER:
            return "fretboard-diagram-fret-number";

        case TextStyleType::HARP_PEDAL_DIAGRAM:
            return "harp-pedal-diagram";

        case TextStyleType::HARP_PEDAL_TEXT_DIAGRAM:
            return "harp-pedal-text-diagram";

        case TextStyleType::TEXTLINE:
            return "text-line";

        case TextStyleType::SYSTEM_TEXTLINE:
            return "system-text-line";

        case TextStyleType::NOTELINE:
            return "note-line";

        case TextStyleType::VOLTA:
            return "volta";

        case TextStyleType::OTTAVA:
            return "ottava";

        case TextStyleType::GLISSANDO:
            return "glissando";

        case TextStyleType::PEDAL:
            return "pedal";

        case TextStyleType::BEND:
            return "bend";

        case TextStyleType::LET_RING:
            return "let-ring";

        case TextStyleType::WHAMMY_BAR:
            return "whammy-bar";

        case TextStyleType::PALM_MUTE:
            return "palm-mute";

        case TextStyleType::USER1:
            return "user1";

        case TextStyleType::USER2:
            return "user2";

        case TextStyleType::USER3:
            return "user3";

        case TextStyleType::USER4:
            return "user4";

        case TextStyleType::USER5:
            return "user5";

        case TextStyleType::USER6:
            return "user6";

        case TextStyleType::USER7:
            return "user7";

        case TextStyleType::USER8:
            return "user8";

        case TextStyleType::USER9:
            return "user9";

        case TextStyleType::USER10:
            return "user10";

        case TextStyleType::USER11:
            return "user11";

        case TextStyleType::USER12:
            return "user12";

        case TextStyleType::DYNAMICS:
        case TextStyleType::DEFAULT:
        case TextStyleType::TEXT_TYPES:
        case TextStyleType::IGNORED_TYPES:
            return QString();
        }
    }
    return QString();
}

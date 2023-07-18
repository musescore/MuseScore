/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "paddingtable.h"

#include "style/style.h"

using namespace mu::engraving;

void PaddingTable::createTable(const MStyle& style)
{
    PaddingTable& table = *this;

    for (size_t i=0; i < TOT_ELEMENT_TYPES; ++i) {
        for (size_t j=0; j < TOT_ELEMENT_TYPES; ++j) {
            table[i][j] = m_minimumPaddingUnit;
        }
    }

    double spatium = style.spatium();

    const double ledgerPad = 0.25 * spatium;
    const double ledgerLength = style.styleMM(Sid::ledgerLineLength);

    /* NOTE: the padding value for note->note is NOT minNoteDistance, because minNoteDistance
     * should only apply to notes of the same voice. Notes from different voices should be
     * allowed to get much closer. So we set the general padding at minimumPaddingUnit,
     * but we introduce an appropriate exception for same-voice cases in Shape::minHorizontalDistance().
     */
    table[ElementType::NOTE][ElementType::NOTE] = m_minimumPaddingUnit;
    table[ElementType::NOTE][ElementType::LEDGER_LINE] = 0.35 * spatium;
    table[ElementType::NOTE][ElementType::ACCIDENTAL]
        = std::max(static_cast<double>(style.styleMM(Sid::accidentalNoteDistance)), 0.35 * spatium);
    table[ElementType::NOTE][ElementType::REST] = style.styleMM(Sid::minNoteDistance);
    table[ElementType::NOTE][ElementType::CLEF] = 1.0 * spatium;
    table[ElementType::NOTE][ElementType::ARPEGGIO] = 0.6 * spatium;
    table[ElementType::NOTE][ElementType::BAR_LINE] = style.styleMM(Sid::noteBarDistance);
    table[ElementType::NOTE][ElementType::KEYSIG] = 0.75 * spatium;
    table[ElementType::NOTE][ElementType::TIMESIG] = 0.75 * spatium;

    table[ElementType::LEDGER_LINE][ElementType::NOTE] = table[ElementType::NOTE][ElementType::LEDGER_LINE];
    table[ElementType::LEDGER_LINE][ElementType::LEDGER_LINE] = ledgerPad;
    table[ElementType::LEDGER_LINE][ElementType::ACCIDENTAL]
        = std::max(static_cast<double>(style.styleMM(
                                           Sid::accidentalNoteDistance)),
                   table[ElementType::NOTE][ElementType::ACCIDENTAL] - ledgerLength / 2);
    table[ElementType::LEDGER_LINE][ElementType::REST] = table[ElementType::LEDGER_LINE][ElementType::NOTE];
    table[ElementType::LEDGER_LINE][ElementType::CLEF]
        = std::max(table[ElementType::NOTE][ElementType::CLEF] - ledgerLength / 2, ledgerPad);
    table[ElementType::LEDGER_LINE][ElementType::ARPEGGIO] = 0.5 * spatium;
    table[ElementType::LEDGER_LINE][ElementType::BAR_LINE]
        = std::max(table[ElementType::NOTE][ElementType::BAR_LINE] - ledgerLength, ledgerPad);
    table[ElementType::LEDGER_LINE][ElementType::KEYSIG]
        = std::max(table[ElementType::NOTE][ElementType::KEYSIG] - ledgerLength / 2, ledgerPad);
    table[ElementType::LEDGER_LINE][ElementType::TIMESIG]
        = std::max(table[ElementType::NOTE][ElementType::TIMESIG] - ledgerLength / 2, ledgerPad);

    table[ElementType::HOOK][ElementType::NOTE] = 0.5 * spatium;
    table[ElementType::HOOK][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::HOOK][ElementType::NOTE] - ledgerLength, ledgerPad);
    table[ElementType::HOOK][ElementType::ACCIDENTAL] = 0.35 * spatium;
    table[ElementType::HOOK][ElementType::REST] = table[ElementType::HOOK][ElementType::NOTE];
    table[ElementType::HOOK][ElementType::CLEF] = 0.5 * spatium;
    table[ElementType::HOOK][ElementType::ARPEGGIO] = 0.35 * spatium;
    table[ElementType::HOOK][ElementType::BAR_LINE] = 1 * spatium;
    table[ElementType::HOOK][ElementType::KEYSIG] = 1.15 * spatium;
    table[ElementType::HOOK][ElementType::TIMESIG] = 1.15 * spatium;

    table[ElementType::NOTEDOT][ElementType::NOTE] = std::max(style.styleMM(Sid::dotNoteDistance), style.styleMM(
                                                                  Sid::dotDotDistance));
    table[ElementType::NOTEDOT][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::NOTEDOT][ElementType::NOTE] - ledgerLength, ledgerPad);
    table[ElementType::NOTEDOT][ElementType::ACCIDENTAL] = table[ElementType::NOTEDOT][ElementType::NOTE];
    table[ElementType::NOTEDOT][ElementType::REST] = table[ElementType::NOTEDOT][ElementType::NOTE];
    table[ElementType::NOTEDOT][ElementType::CLEF] = 1.0 * spatium;
    table[ElementType::NOTEDOT][ElementType::ARPEGGIO] = 0.5 * spatium;
    table[ElementType::NOTEDOT][ElementType::BAR_LINE] = 0.8 * spatium;
    table[ElementType::NOTEDOT][ElementType::KEYSIG] = 1.35 * spatium;
    table[ElementType::NOTEDOT][ElementType::TIMESIG] = 1.35 * spatium;

    table[ElementType::REST][ElementType::NOTE] = table[ElementType::NOTE][ElementType::REST];
    table[ElementType::REST][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::REST][ElementType::NOTE] - ledgerLength / 2, ledgerPad);
    table[ElementType::REST][ElementType::ACCIDENTAL] = 0.45 * spatium;
    table[ElementType::REST][ElementType::REST] = table[ElementType::REST][ElementType::NOTE];
    table[ElementType::REST][ElementType::CLEF] = table[ElementType::NOTE][ElementType::CLEF];
    table[ElementType::REST][ElementType::BAR_LINE] = 1.65 * spatium;
    table[ElementType::REST][ElementType::KEYSIG] = 1.5 * spatium;
    table[ElementType::REST][ElementType::TIMESIG] = 1.5 * spatium;

    table[ElementType::CLEF][ElementType::NOTE] = style.styleMM(Sid::clefKeyRightMargin);
    table[ElementType::CLEF][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::CLEF][ElementType::NOTE] - ledgerLength / 2, ledgerPad);
    table[ElementType::CLEF][ElementType::ACCIDENTAL] = 0.75 * spatium;
    table[ElementType::CLEF][ElementType::REST] = 1.35 * spatium;
    table[ElementType::CLEF][ElementType::CLEF] = 0.75 * spatium;
    table[ElementType::CLEF][ElementType::ARPEGGIO] = 1.15 * spatium;
    table[ElementType::CLEF][ElementType::BAR_LINE] = style.styleMM(Sid::clefBarlineDistance);
    table[ElementType::CLEF][ElementType::KEYSIG] = style.styleMM(Sid::clefKeyDistance);
    table[ElementType::CLEF][ElementType::TIMESIG] = style.styleMM(Sid::clefTimesigDistance);

    table[ElementType::BAR_LINE][ElementType::NOTE] = style.styleMM(Sid::barNoteDistance);
    table[ElementType::BAR_LINE][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::BAR_LINE][ElementType::NOTE] - ledgerLength, ledgerPad);
    table[ElementType::BAR_LINE][ElementType::ACCIDENTAL] = style.styleMM(Sid::barAccidentalDistance);
    table[ElementType::BAR_LINE][ElementType::REST] = style.styleMM(Sid::barNoteDistance);
    table[ElementType::BAR_LINE][ElementType::CLEF] = style.styleMM(Sid::clefLeftMargin);
    table[ElementType::BAR_LINE][ElementType::ARPEGGIO] = 0.65 * spatium;
    table[ElementType::BAR_LINE][ElementType::BAR_LINE] = 1.35 * spatium;
    table[ElementType::BAR_LINE][ElementType::KEYSIG] = style.styleMM(Sid::keysigLeftMargin);
    table[ElementType::BAR_LINE][ElementType::TIMESIG] = style.styleMM(Sid::timesigLeftMargin);

    table[ElementType::KEYSIG][ElementType::NOTE] = 1.75 * spatium;
    table[ElementType::KEYSIG][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::KEYSIG][ElementType::NOTE] - ledgerLength, ledgerPad);
    table[ElementType::KEYSIG][ElementType::ACCIDENTAL] = 1.6 * spatium;
    table[ElementType::KEYSIG][ElementType::REST] = table[ElementType::KEYSIG][ElementType::NOTE];
    table[ElementType::KEYSIG][ElementType::CLEF] = 1.0 * spatium;
    table[ElementType::KEYSIG][ElementType::ARPEGGIO] = 1.35 * spatium;
    table[ElementType::KEYSIG][ElementType::BAR_LINE] = style.styleMM(Sid::keyBarlineDistance);
    table[ElementType::KEYSIG][ElementType::KEYSIG] = 1 * spatium;
    table[ElementType::KEYSIG][ElementType::TIMESIG] = style.styleMM(Sid::keyTimesigDistance);

    table[ElementType::TIMESIG][ElementType::NOTE] = 1.35 * spatium;
    table[ElementType::TIMESIG][ElementType::LEDGER_LINE]
        = std::max(table[ElementType::TIMESIG][ElementType::NOTE] - ledgerLength, ledgerPad);
    table[ElementType::TIMESIG][ElementType::ACCIDENTAL] = 0.8 * spatium;
    table[ElementType::TIMESIG][ElementType::REST] = table[ElementType::TIMESIG][ElementType::NOTE];
    table[ElementType::TIMESIG][ElementType::CLEF] = 1.0 * spatium;
    table[ElementType::TIMESIG][ElementType::ARPEGGIO] = 1.35 * spatium;
    table[ElementType::TIMESIG][ElementType::BAR_LINE] = style.styleMM(Sid::timesigBarlineDistance);
    table[ElementType::TIMESIG][ElementType::KEYSIG] = style.styleMM(Sid::keyTimesigDistance);
    table[ElementType::TIMESIG][ElementType::TIMESIG] = 1.0 * spatium;

    // Obtain the Stem -> * and * -> Stem values from the note equivalents
    table[ElementType::STEM] = table[ElementType::NOTE];
    for (auto& elem: table) {
        elem[ElementType::STEM] = elem[ElementType::NOTE];
    }

    table[ElementType::STEM][ElementType::NOTE] = style.styleMM(Sid::minNoteDistance);
    table[ElementType::STEM][ElementType::STEM] = 0.85 * spatium;
    table[ElementType::STEM][ElementType::ACCIDENTAL] = 0.35 * spatium;
    table[ElementType::STEM][ElementType::LEDGER_LINE] = 0.35 * spatium;
    table[ElementType::LEDGER_LINE][ElementType::STEM] = 0.35 * spatium;

    // Ambitus
    table[ElementType::AMBITUS].fill(style.styleMM(Sid::ambitusMargin));
    for (auto& elem: table) {
        elem[ElementType::AMBITUS] = style.styleMM(Sid::ambitusMargin);
    }

    // Breath
    table[ElementType::BREATH].fill(1.0 * spatium);
    for (auto& elem: table) {
        elem[ElementType::BREATH] = 1.0 * spatium;
    }

    // Temporary hack, because some padding is already constructed inside the lyrics themselves.
    table[ElementType::BAR_LINE][ElementType::LYRICS] = 0.0 * spatium;

    // Harmony
    table[ElementType::BAR_LINE][ElementType::HARMONY] = 0.5 * style.styleMM(Sid::minHarmonyDistance);
    table[ElementType::HARMONY][ElementType::HARMONY] = style.styleMM(Sid::minHarmonyDistance);

    // Chordlines
    table[ElementType::CHORDLINE].fill(0.35 * spatium);
    for (auto& elem: table) {
        elem[ElementType::CHORDLINE] = 0.35 * spatium;
    }
    table[ElementType::BAR_LINE][ElementType::CHORDLINE] = 0.65 * spatium;
    table[ElementType::CHORDLINE][ElementType::BAR_LINE] = 0.65 * spatium;

    // For the x -> fingering padding use the same values as x -> accidental
    for (auto& elem : table) {
        elem[ElementType::FINGERING] = elem[ElementType::ACCIDENTAL];
    }

    // This is needed for beamlets, not beams themselves
    table[ElementType::BEAM][ElementType::BEAM] = 0.4 * spatium;

    // Symbols (semi-hack: the only symbol for which
    // this is relevant is noteHead parenthesis)
    table[ElementType::SYMBOL] = table[ElementType::NOTE];
    table[ElementType::SYMBOL][ElementType::NOTE] = 0.35 * spatium;
    for (auto& elem : table) {
        elem[ElementType::SYMBOL] = elem[ElementType::ACCIDENTAL];
    }
    table[ElementType::NOTEDOT][ElementType::SYMBOL] = 0.2 * spatium;
}

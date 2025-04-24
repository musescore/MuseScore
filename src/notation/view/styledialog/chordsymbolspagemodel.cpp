/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "chordsymbolspagemodel.h"
#include "engraving/types/types.h"

using namespace mu::notation;

ChordSymbolsPageModel::ChordSymbolsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::chordExtensionMag,
    StyleId::chordExtensionAdjust,
    StyleId::chordModifierMag,
    StyleId::chordModifierAdjust,
    StyleId::useStandardNoteNames,
    StyleId::useGermanNoteNames,
    StyleId::useFullGermanNoteNames,
    StyleId::useSolfeggioNoteNames,
    StyleId::useFrenchNoteNames,
    StyleId::automaticCapitalization,
    StyleId::lowerCaseMinorChords,
    StyleId::lowerCaseBassNotes,
    StyleId::allCapsNoteNames,
    StyleId::harmonyFretDist,
    StyleId::minHarmonyDistance,
    StyleId::maxHarmonyBarDistance,
    StyleId::maxChordShiftAbove,
    StyleId::maxChordShiftBelow,
    StyleId::harmonyVoiceLiteral,
    StyleId::harmonyVoicing,
    StyleId::harmonyDuration,
    StyleId::chordsXmlFile,
    StyleId::capoPosition,
})
{
}

StyleItem* ChordSymbolsPageModel::extensionMag() const { return styleItem(StyleId::chordExtensionMag); }
StyleItem* ChordSymbolsPageModel::extensionAdjust() const { return styleItem(StyleId::chordExtensionAdjust); }
StyleItem* ChordSymbolsPageModel::modifierMag() const { return styleItem(StyleId::chordModifierMag); }
StyleItem* ChordSymbolsPageModel::modifierAdjust() const { return styleItem(StyleId::chordModifierAdjust); }
StyleItem* ChordSymbolsPageModel::useStandardNoteNames() const { return styleItem(StyleId::useStandardNoteNames); }
StyleItem* ChordSymbolsPageModel::useGermanNoteNames() const { return styleItem(StyleId::useGermanNoteNames); }
StyleItem* ChordSymbolsPageModel::useFullGermanNoteNames() const { return styleItem(StyleId::useFullGermanNoteNames); }
StyleItem* ChordSymbolsPageModel::useSolfeggioNoteNames() const { return styleItem(StyleId::useSolfeggioNoteNames); }
StyleItem* ChordSymbolsPageModel::useFrenchNoteNames() const { return styleItem(StyleId::useFrenchNoteNames); }
StyleItem* ChordSymbolsPageModel::automaticCapitalization() const { return styleItem(StyleId::automaticCapitalization); }
StyleItem* ChordSymbolsPageModel::lowerCaseMinorChords() const { return styleItem(StyleId::lowerCaseMinorChords); }
StyleItem* ChordSymbolsPageModel::lowerCaseBassNotes() const { return styleItem(StyleId::lowerCaseBassNotes); }
StyleItem* ChordSymbolsPageModel::allCapsNoteNames() const { return styleItem(StyleId::allCapsNoteNames); }
StyleItem* ChordSymbolsPageModel::harmonyFretDist() const { return styleItem(StyleId::harmonyFretDist); }
StyleItem* ChordSymbolsPageModel::minHarmonyDist() const { return styleItem(StyleId::minHarmonyDistance); }
StyleItem* ChordSymbolsPageModel::maxHarmonyBarDistance() const { return styleItem(StyleId::maxHarmonyBarDistance); }
StyleItem* ChordSymbolsPageModel::maxChordShiftAbove() const { return styleItem(StyleId::maxChordShiftAbove); }
StyleItem* ChordSymbolsPageModel::maxChordShiftBelow() const { return styleItem(StyleId::maxChordShiftBelow); }

QVariantList ChordSymbolsPageModel::possibleHarmonyVoiceLiteralOptions() const
{
    QVariantList options {
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Jazz") },
            { "value", static_cast<int>(false) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Literal") },
            { "value", static_cast<int>(true) } },
    };

    return options;
}

QVariantList ChordSymbolsPageModel::possibleHarmonyVoicingOptions() const
{
    QVariantList options {
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Automatic") },
            { "value", static_cast<int>(Voicing::AUTO) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Root only") },
            { "value", static_cast<int>(Voicing::ROOT_ONLY) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Close") },
            { "value", static_cast<int>(Voicing::CLOSE) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Drop two") },
            { "value", static_cast<int>(Voicing::DROP_2) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Six note") },
            { "value", static_cast<int>(Voicing::SIX_NOTE) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Four note") },
            { "value", static_cast<int>(Voicing::FOUR_NOTE) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Three note") },
            { "value", static_cast<int>(Voicing::THREE_NOTE) } },
    };

    return options;
}

QVariantList ChordSymbolsPageModel::possibleHarmonyDurationOptions() const
{
    QVariantList options {
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Until next chord symbol") },
            { "value", static_cast<int>(mu::engraving::HDuration::UNTIL_NEXT_CHORD_SYMBOL) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Until end of measure") },
            { "value", static_cast<int>(mu::engraving::HDuration::STOP_AT_MEASURE_END) } },
        QVariantMap{
            { "text", muse::qtrc("notation/editstyle/chordsymbols", "Chord/rest duration") },
            { "value", static_cast<int>(mu::engraving::HDuration::SEGMENT_DURATION) } },
    };

    return options;
}

StyleItem* ChordSymbolsPageModel::harmonyVoiceLiteral() const { return styleItem(StyleId::harmonyVoiceLiteral); }
StyleItem* ChordSymbolsPageModel::harmonyVoicing() const { return styleItem(StyleId::harmonyVoicing); }
StyleItem* ChordSymbolsPageModel::harmonyDuration() const { return styleItem(StyleId::harmonyDuration); }
StyleItem* ChordSymbolsPageModel::capoPosition() const { return styleItem(StyleId::capoPosition); }
StyleItem* ChordSymbolsPageModel::chordsXmlFile() const { return styleItem(StyleId::chordsXmlFile); }

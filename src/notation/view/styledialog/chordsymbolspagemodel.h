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

#pragma once
#include "abstractstyledialogmodel.h"

namespace mu::notation {
class ChordSymbolsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * extensionMag READ extensionMag CONSTANT)
    Q_PROPERTY(StyleItem * extensionAdjust READ extensionAdjust CONSTANT)
    Q_PROPERTY(StyleItem * modifierMag READ modifierMag CONSTANT)
    Q_PROPERTY(StyleItem * modifierAdjust READ modifierAdjust CONSTANT)
    Q_PROPERTY(StyleItem * useStandardNoteNames READ useStandardNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * useGermanNoteNames READ useGermanNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * useFullGermanNoteNames READ useFullGermanNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * useSolfeggioNoteNames READ useSolfeggioNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * useFrenchNoteNames READ useFrenchNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * automaticCapitalization READ automaticCapitalization CONSTANT)
    Q_PROPERTY(StyleItem * lowerCaseMinorChords READ lowerCaseMinorChords CONSTANT)
    Q_PROPERTY(StyleItem * lowerCaseBassNotes READ lowerCaseBassNotes CONSTANT)
    Q_PROPERTY(StyleItem * allCapsNoteNames READ allCapsNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * harmonyFretDist READ harmonyFretDist CONSTANT)
    Q_PROPERTY(StyleItem * minHarmonyDist READ minHarmonyDist CONSTANT)
    Q_PROPERTY(StyleItem * maxHarmonyBarDistance READ maxHarmonyBarDistance CONSTANT)
    Q_PROPERTY(StyleItem * maxChordShiftAbove READ maxChordShiftAbove CONSTANT)
    Q_PROPERTY(StyleItem * maxChordShiftBelow READ maxChordShiftBelow CONSTANT)
    Q_PROPERTY(StyleItem * harmonyVoiceLiteral READ harmonyVoiceLiteral CONSTANT)
    Q_PROPERTY(StyleItem * harmonyVoicing READ harmonyVoicing CONSTANT)
    Q_PROPERTY(StyleItem * harmonyDuration READ harmonyDuration CONSTANT)
    Q_PROPERTY(StyleItem * capoPosition READ capoPosition CONSTANT)
    Q_PROPERTY(StyleItem * chordsXmlFile READ chordsXmlFile CONSTANT)
public:
    explicit ChordSymbolsPageModel(QObject* parent = nullptr);

    StyleItem* extensionMag() const;
    StyleItem* extensionAdjust() const;
    StyleItem* modifierMag() const;
    StyleItem* modifierAdjust() const;

    StyleItem* useStandardNoteNames() const;
    StyleItem* useGermanNoteNames() const;
    StyleItem* useFullGermanNoteNames() const;
    StyleItem* useSolfeggioNoteNames() const;
    StyleItem* useFrenchNoteNames() const;

    StyleItem* automaticCapitalization() const;
    StyleItem* lowerCaseMinorChords() const;
    StyleItem* lowerCaseBassNotes() const;
    StyleItem* allCapsNoteNames() const;

    StyleItem* harmonyFretDist() const;
    StyleItem* minHarmonyDist() const;
    StyleItem* maxHarmonyBarDistance() const;
    StyleItem* maxChordShiftAbove() const;
    StyleItem* maxChordShiftBelow() const;

    Q_INVOKABLE QVariantList possibleHarmonyVoiceLiteralOptions() const;
    Q_INVOKABLE QVariantList possibleHarmonyVoicingOptions() const;
    Q_INVOKABLE QVariantList possibleHarmonyDurationOptions() const;

    StyleItem* harmonyVoiceLiteral() const;
    StyleItem* harmonyVoicing() const;
    StyleItem* harmonyDuration() const;

    StyleItem* capoPosition() const;

    StyleItem* chordsXmlFile() const;
};
}

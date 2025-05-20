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
#include "iinteractive.h"
#include "inotationconfiguration.h"

namespace mu::notation {
class ChordSymbolsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * chordStylePreset READ chordStylePreset CONSTANT)
    Q_PROPERTY(StyleItem * chordDescriptionFile READ chordDescriptionFile CONSTANT)

    Q_PROPERTY(StyleItem * extensionMag READ extensionMag CONSTANT)
    Q_PROPERTY(StyleItem * extensionAdjust READ extensionAdjust CONSTANT)
    Q_PROPERTY(StyleItem * modifierMag READ modifierMag CONSTANT)
    Q_PROPERTY(StyleItem * modifierAdjust READ modifierAdjust CONSTANT)
    Q_PROPERTY(StyleItem * verticallyStackModifiers READ verticallyStackModifiers CONSTANT)

    Q_PROPERTY(StyleItem * chordBassNoteStagger READ chordBassNoteStagger CONSTANT)
    Q_PROPERTY(StyleItem * chordBassNoteScale READ chordBassNoteScale CONSTANT)
    Q_PROPERTY(StyleItem * polychordDividerThickness READ polychordDividerThickness CONSTANT)
    Q_PROPERTY(StyleItem * polychordDividerSpacing READ polychordDividerSpacing CONSTANT)

    Q_PROPERTY(StyleItem * verticallyAlignChordSymbols READ verticallyAlignChordSymbols CONSTANT)
    Q_PROPERTY(StyleItem * chordAlignmentToNotehead READ chordAlignmentToNotehead CONSTANT)
    Q_PROPERTY(StyleItem * chordAlignmentToFretboard READ chordAlignmentToFretboard CONSTANT)
    Q_PROPERTY(StyleItem * chordAlignmentExcludeModifiers READ chordAlignmentExcludeModifiers CONSTANT)

    Q_PROPERTY(StyleItem * chordSymbolSpelling READ chordSymbolSpelling CONSTANT)
    Q_PROPERTY(StyleItem * automaticCapitalization READ automaticCapitalization CONSTANT)
    Q_PROPERTY(StyleItem * lowerCaseMinorChords READ lowerCaseMinorChords CONSTANT)
    Q_PROPERTY(StyleItem * lowerCaseBassNotes READ lowerCaseBassNotes CONSTANT)
    Q_PROPERTY(StyleItem * allCapsNoteNames READ allCapsNoteNames CONSTANT)
    Q_PROPERTY(StyleItem * harmonyFretDist READ harmonyFretDist CONSTANT)
    Q_PROPERTY(StyleItem * minHarmonyDist READ minHarmonyDist CONSTANT)
    Q_PROPERTY(StyleItem * harmonyVoiceLiteral READ harmonyVoiceLiteral CONSTANT)
    Q_PROPERTY(StyleItem * harmonyVoicing READ harmonyVoicing CONSTANT)
    Q_PROPERTY(StyleItem * harmonyDuration READ harmonyDuration CONSTANT)
    Q_PROPERTY(StyleItem * capoPosition READ capoPosition CONSTANT)

    Q_PROPERTY(bool isCustomXml READ isCustomXml CONSTANT NOTIFY changePreset)

    muse::Inject<mu::notation::INotationConfiguration> configuration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };

public:

    explicit ChordSymbolsPageModel(QObject* parent = nullptr);

    StyleItem* chordStylePreset() const;
    StyleItem* chordDescriptionFile() const;
    Q_INVOKABLE QVariantList possiblePresetOptions() const;
    bool isCustomXml() const;

    StyleItem* extensionMag() const;
    StyleItem* extensionAdjust() const;

    StyleItem* modifierMag() const;
    StyleItem* modifierAdjust() const;
    StyleItem* verticallyStackModifiers() const;

    StyleItem* chordBassNoteStagger() const;
    StyleItem* chordBassNoteScale() const;

    StyleItem* polychordDividerThickness() const;
    StyleItem* polychordDividerSpacing() const;

    StyleItem* verticallyAlignChordSymbols() const;
    StyleItem* chordAlignmentToNotehead() const;
    StyleItem* chordAlignmentToFretboard() const;
    StyleItem* chordAlignmentExcludeModifiers() const;

    StyleItem* chordSymbolSpelling() const;
    Q_INVOKABLE QVariantList possibleChordSymbolSpellings() const;

    StyleItem* automaticCapitalization() const;
    StyleItem* lowerCaseMinorChords() const;
    StyleItem* lowerCaseBassNotes() const;
    StyleItem* allCapsNoteNames() const;

    StyleItem* harmonyFretDist() const;
    StyleItem* minHarmonyDist() const;

    Q_INVOKABLE QVariantList possibleHarmonyVoiceLiteralOptions() const;
    Q_INVOKABLE QVariantList possibleHarmonyVoicingOptions() const;
    Q_INVOKABLE QVariantList possibleHarmonyDurationOptions() const;

    StyleItem* harmonyVoiceLiteral() const;
    StyleItem* harmonyVoicing() const;
    StyleItem* harmonyDuration() const;
    StyleItem* capoPosition() const;

signals:
    void changePreset();

public slots:
    void setChordStyle(mu::engraving::ChordStylePreset);
    void selectChordDescriptionFile();
};
}

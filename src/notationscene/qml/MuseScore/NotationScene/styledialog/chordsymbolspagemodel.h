/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "notation/inotationconfiguration.h"

namespace mu::notation {
class ChordSymbolsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(mu::notation::StyleItem * chordStylePreset READ chordStylePreset CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * chordDescriptionFile READ chordDescriptionFile CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * extensionMag READ extensionMag CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * extensionAdjust READ extensionAdjust CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * modifierMag READ modifierMag CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * modifierAdjust READ modifierAdjust CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * verticallyStackModifiers READ verticallyStackModifiers CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * chordBassNoteStagger READ chordBassNoteStagger CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * chordBassNoteScale READ chordBassNoteScale CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * polychordDividerThickness READ polychordDividerThickness CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * polychordDividerSpacing READ polychordDividerSpacing CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * verticallyAlignChordSymbols READ verticallyAlignChordSymbols CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * chordAlignmentToNotehead READ chordAlignmentToNotehead CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * chordAlignmentToFretboard READ chordAlignmentToFretboard CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * chordAlignmentExcludeModifiers READ chordAlignmentExcludeModifiers CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * chordSymbolSpelling READ chordSymbolSpelling CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * automaticCapitalization READ automaticCapitalization CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * lowerCaseMinorChords READ lowerCaseMinorChords CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * lowerCaseBassNotes READ lowerCaseBassNotes CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * allCapsNoteNames READ allCapsNoteNames CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * harmonyFretDist READ harmonyFretDist CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * minHarmonyDist READ minHarmonyDist CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * harmonyHarmonyDist READ harmonyHarmonyDist CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * harmonyVoiceLiteral READ harmonyVoiceLiteral CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * harmonyVoicing READ harmonyVoicing CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * harmonyDuration READ harmonyDuration CONSTANT)

    Q_PROPERTY(mu::notation::StyleItem * displayCapoChords READ displayCapoChords CONSTANT)
    Q_PROPERTY(mu::notation::StyleItem * capoPosition READ capoPosition CONSTANT)

    Q_PROPERTY(bool isCustomXml READ isCustomXml NOTIFY changePreset)
    Q_PROPERTY(bool isLegacyXml READ isLegacyXml NOTIFY changePreset)

    QML_ELEMENT;

    muse::Inject<mu::notation::INotationConfiguration> configuration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };

public:

    explicit ChordSymbolsPageModel(QObject* parent = nullptr);

    StyleItem* chordStylePreset() const;
    StyleItem* chordDescriptionFile() const;
    Q_INVOKABLE QVariantList possiblePresetOptions() const;
    bool isCustomXml() const;
    bool isLegacyXml() const;

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
    StyleItem* harmonyHarmonyDist() const;

    Q_INVOKABLE QVariantList possibleHarmonyVoiceLiteralOptions() const;
    Q_INVOKABLE QVariantList possibleHarmonyVoicingOptions() const;
    Q_INVOKABLE QVariantList possibleHarmonyDurationOptions() const;

    StyleItem* harmonyVoiceLiteral() const;
    StyleItem* harmonyVoicing() const;
    StyleItem* harmonyDuration() const;

    StyleItem* displayCapoChords() const;
    Q_INVOKABLE QVariantList possibleCapoDisplayOptions() const;
    StyleItem* capoPosition() const;

signals:
    void changePreset();

public slots:
    void setChordStyle(mu::engraving::ChordStylePreset);
    void selectChordDescriptionFile();
};
}

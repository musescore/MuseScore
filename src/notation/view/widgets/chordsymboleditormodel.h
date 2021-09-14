/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_NOTATION_CHORDSYMBOLEDITORMODEL_H
#define MU_NOTATION_CHORDSYMBOLEDITORMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "engraving/style/style.h"
#include "notation/internal/chordsymbolstylemanager.h"
#include "framework/ui/view/iconcodes.h"

namespace mu::notation {
class ChordSymbolEditorModel : public QAbstractListModel
{
    INJECT(notation, mu::context::IGlobalContext, globalContext)

    Q_OBJECT

    Q_PROPERTY(QStringList chordSpellingList READ chordSpellingList NOTIFY chordSpellingListChanged)
    Q_PROPERTY(QStringList majorSeventhList READ majorSeventhList NOTIFY majorSeventhListChanged)
    Q_PROPERTY(QStringList halfDiminishedList READ halfDiminishedList NOTIFY halfDiminishedListChanged)
    Q_PROPERTY(QStringList minorList READ minorList NOTIFY minorListChanged)
    Q_PROPERTY(QStringList augmentedList READ augmentedList NOTIFY augmentedListChanged)
    Q_PROPERTY(QStringList diminishedList READ diminishedList NOTIFY diminishedListChanged)
    Q_PROPERTY(QStringList sixNineList READ sixNineList NOTIFY sixNineListChanged)
    Q_PROPERTY(QStringList omitList READ omitList NOTIFY omitListChanged)
    Q_PROPERTY(QStringList suspensionList READ suspensionList NOTIFY suspensionListChanged)
    Q_PROPERTY(QStringList bassNoteList READ bassNoteList NOTIFY bassNoteListChanged)

    Q_PROPERTY(int chordSpellingIndex READ chordSpellingIndex NOTIFY chordSpellingIndexChanged)
    Q_PROPERTY(int currentStyleIndex READ currentStyleIndex NOTIFY currentStyleIndexChanged)
    Q_PROPERTY(QString styleDescription READ styleDescription NOTIFY styleDescriptionChanged)
    Q_PROPERTY(bool usePresets READ usePresets NOTIFY usePresetsChanged)

    Q_PROPERTY(int majorSeventhIndex READ majorSeventhIndex NOTIFY majorSeventhIndexChanged)
    Q_PROPERTY(int halfDiminishedIndex READ halfDiminishedIndex NOTIFY halfDiminishedIndexChanged)
    Q_PROPERTY(int minorIndex READ minorIndex NOTIFY minorIndexChanged)
    Q_PROPERTY(int augmentedIndex READ augmentedIndex NOTIFY augmentedIndexChanged)
    Q_PROPERTY(int diminishedIndex READ diminishedIndex NOTIFY diminishedIndexChanged)
    Q_PROPERTY(int sixNineIndex READ sixNineIndex NOTIFY sixNineIndexChanged)
    Q_PROPERTY(int omitIndex READ omitIndex NOTIFY omitIndexChanged)
    Q_PROPERTY(int suspensionIndex READ suspensionIndex NOTIFY suspensionIndexChanged)
    Q_PROPERTY(int bassNoteIndex READ bassNoteIndex NOTIFY bassNoteIndexChanged)

    Q_PROPERTY(qreal qualityMag READ qualityMag NOTIFY qualityMagChanged)
    Q_PROPERTY(qreal qualityAdjust READ qualityAdjust NOTIFY qualityAdjustChanged)
    Q_PROPERTY(qreal extensionMag READ extensionMag NOTIFY extensionMagChanged)
    Q_PROPERTY(qreal extensionAdjust READ extensionAdjust NOTIFY extensionAdjustChanged)
    Q_PROPERTY(qreal modifierMag READ modifierMag NOTIFY modifierMagChanged)
    Q_PROPERTY(qreal modifierAdjust READ modifierAdjust NOTIFY modifierAdjustChanged)

    Q_PROPERTY(qreal harmonyFretDistance READ harmonyFretDistance NOTIFY harmonyFretDistanceChanged)
    Q_PROPERTY(qreal minHarmonyDistance READ minHarmonyDistance NOTIFY minHarmonyDistanceChanged)
    Q_PROPERTY(qreal maxHarmonyBarDistance READ maxHarmonyBarDistance NOTIFY maxHarmonyBarDistanceChanged)
    Q_PROPERTY(qreal maxChordShiftAbove READ maxChordShiftAbove NOTIFY maxChordShiftAboveChanged)
    Q_PROPERTY(qreal maxChordShiftBelow READ maxChordShiftBelow NOTIFY maxChordShiftBelowChanged)
    Q_PROPERTY(qreal capoFretPosition READ capoFretPosition NOTIFY capoFretPositionChanged)

    Q_PROPERTY(qreal stackModifiers READ stackModifiers NOTIFY stackModifiersChanged)

    Q_PROPERTY(qreal autoCapitalization READ autoCapitalization NOTIFY autoCapitalizationChanged)
    Q_PROPERTY(qreal minorRootCapitalization READ minorRootCapitalization NOTIFY minorRootCapitalizationChanged)
    Q_PROPERTY(qreal qualityMajorCapitalization READ qualityMajorCapitalization NOTIFY qualityMajorCapitalizationChanged)
    Q_PROPERTY(qreal qualityMinorCapitalization READ qualityMinorCapitalization NOTIFY qualityMinorCapitalizationChanged)
    Q_PROPERTY(qreal bassNotesCapitalization READ bassNotesCapitalization NOTIFY bassNotesCapitalizationChanged)
    Q_PROPERTY(qreal solfegeNotesCapitalization READ solfegeNotesCapitalization NOTIFY solfegeNotesCapitalizationChanged)

    Q_PROPERTY(qreal alterationsParentheses READ alterationsParentheses NOTIFY alterationsParenthesesChanged)
    Q_PROPERTY(qreal suspensionsParentheses READ suspensionsParentheses NOTIFY suspensionsParenthesesChanged)
    Q_PROPERTY(qreal minMajParentheses READ minMajParentheses NOTIFY minMajParenthesesChanged)
    Q_PROPERTY(qreal addOmitParentheses READ addOmitParentheses NOTIFY addOmitParenthesesChanged)

    Q_PROPERTY(qreal chordSymbolScaling READ chordSymbolScaling NOTIFY chordSymbolScalingChanged)

public:
    ChordSymbolEditorModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList chordSpellingList() const;
    QStringList majorSeventhList() const;
    QStringList halfDiminishedList() const;
    QStringList minorList() const;
    QStringList augmentedList() const;
    QStringList diminishedList() const;
    QStringList sixNineList() const;
    QStringList omitList() const;
    QStringList suspensionList() const;
    QStringList bassNoteList() const;

    int chordSpellingIndex() const;
    int currentStyleIndex() const;
    QString styleDescription() const;
    bool usePresets() const;

    int majorSeventhIndex() const;
    int halfDiminishedIndex() const;
    int minorIndex() const;
    int augmentedIndex() const;
    int diminishedIndex() const;
    int sixNineIndex() const;
    int omitIndex() const;
    int suspensionIndex() const;
    int bassNoteIndex() const;

    qreal qualityMag() const;
    qreal qualityAdjust() const;
    qreal extensionMag() const;
    qreal extensionAdjust() const;
    qreal modifierMag() const;
    qreal modifierAdjust() const;

    qreal harmonyFretDistance() const;
    qreal minHarmonyDistance() const;
    qreal maxHarmonyBarDistance() const;
    qreal maxChordShiftAbove() const;
    qreal maxChordShiftBelow() const;
    qreal capoFretPosition() const;

    qreal stackModifiers() const;

    qreal autoCapitalization() const;
    qreal minorRootCapitalization() const;
    qreal qualityMajorCapitalization() const;
    qreal qualityMinorCapitalization() const;
    qreal bassNotesCapitalization() const;
    qreal solfegeNotesCapitalization() const;

    qreal alterationsParentheses() const;
    qreal suspensionsParentheses() const;
    qreal minMajParentheses() const;
    qreal addOmitParentheses() const;

    qreal chordSymbolScaling() const;

    void initCurrentStyleIndex();
    void setQualitySymbolsOnStyleChange();
    void setPropertiesOnStyleChange();
    void setQualitySymbolsLists();
    void setPropertiesOfQualitySymbol(QualitySymbol qS);
    void stringifyAndSaveSelectionHistory();
    void extractSelectionHistory(QString selectionHistory);
    void updateSelectionHistory(QString currentStyle);
    void setStyle(Ms::Sid id, QVariant val);
    qreal getDefValR(Ms::Sid id);

    Q_INVOKABLE void setChordStyle(int index);
    Q_INVOKABLE void setChordSpelling(QString spelling);
    Q_INVOKABLE void setQualitySymbol(QString quality, int index);
    Q_INVOKABLE void setProperty(QString property, qreal val);
    Q_INVOKABLE void resetProperties();
    Q_INVOKABLE int getIconFromText(QString qualSym);

signals:
    void chordSpellingListChanged();
    void majorSeventhListChanged();
    void halfDiminishedListChanged();
    void minorListChanged();
    void augmentedListChanged();
    void diminishedListChanged();
    void sixNineListChanged();
    void omitListChanged();
    void suspensionListChanged();
    void bassNoteListChanged();

    void chordSpellingIndexChanged();
    void currentStyleIndexChanged();
    void styleDescriptionChanged();
    void usePresetsChanged();

    void majorSeventhIndexChanged();
    void halfDiminishedIndexChanged();
    void minorIndexChanged();
    void augmentedIndexChanged();
    void diminishedIndexChanged();
    void sixNineIndexChanged();
    void omitIndexChanged();
    void suspensionIndexChanged();
    void bassNoteIndexChanged();

    void qualityMagChanged();
    void qualityAdjustChanged();
    void extensionMagChanged();
    void extensionAdjustChanged();
    void modifierMagChanged();
    void modifierAdjustChanged();

    void harmonyFretDistanceChanged();
    void minHarmonyDistanceChanged();
    void maxHarmonyBarDistanceChanged();
    void maxChordShiftAboveChanged();
    void maxChordShiftBelowChanged();
    void capoFretPositionChanged();

    void stackModifiersChanged();

    void autoCapitalizationChanged();
    void minorRootCapitalizationChanged();
    void qualityMajorCapitalizationChanged();
    void qualityMinorCapitalizationChanged();
    void bassNotesCapitalizationChanged();
    void solfegeNotesCapitalizationChanged();

    void alterationsParenthesesChanged();
    void suspensionsParenthesesChanged();
    void minMajParenthesesChanged();
    void addOmitParenthesesChanged();

    void chordSymbolScalingChanged();

private:
    enum RoleNames {
        StyleNameRole = Qt::UserRole + 1,
        FileRole,
        UsePresetsRole
    };

    QList<ChordSymbolStyle> m_styles;
    ChordSymbolStyleManager* styleManager;
    QHash<QString, QList<QualitySymbol> > m_qualitySymbols;
    QHash<QString, QHash<QString, QVariant> > m_selectionHistory;

    QStringList m_chordSpellingList;
    QList<QualitySymbol> m_majorSeventhList;
    QList<QualitySymbol> m_halfDiminishedList;
    QList<QualitySymbol> m_minorList;
    QList<QualitySymbol> m_augmentedList;
    QList<QualitySymbol> m_diminishedList;
    QList<QualitySymbol> m_sixNineList;
    QList<QualitySymbol> m_omitList;
    QList<QualitySymbol> m_suspensionList;
    QList<QualitySymbol> m_bassNoteList;

    int m_chordSpellingIndex;
    int m_currentStyleIndex;
    QString m_styleDescription;
    bool m_usePresets;

    int m_majorSeventhIndex;
    int m_halfDiminishedIndex;
    int m_minorIndex;
    int m_augmentedIndex;
    int m_diminishedIndex;
    int m_sixNineIndex;
    int m_omitIndex;
    int m_suspensionIndex;
    int m_bassNoteIndex;

    qreal m_qualityMag;
    qreal m_qualityAdjust;
    qreal m_extensionMag;
    qreal m_extensionAdjust;
    qreal m_modifierMag;
    qreal m_modifierAdjust;

    qreal m_harmonyFretDistance;
    qreal m_minHarmonyDistance;
    qreal m_maxHarmonyBarDistance;
    qreal m_maxChordShiftAbove;
    qreal m_maxChordShiftBelow;
    qreal m_capoFretPosition;

    qreal m_stackModifiers;

    qreal m_autoCapitalization;
    qreal m_minorRootCapitalization;
    qreal m_qualityMajorCapitalization;
    qreal m_qualityMinorCapitalization;
    qreal m_bassNotesCapitalization;
    qreal m_solfegeNotesCapitalization;

    qreal m_alterationsParentheses;
    qreal m_suspensionsParentheses;
    qreal m_minMajParentheses;
    qreal m_addOmitParentheses;

    qreal m_chordSymbolScaling;

    QHash<QString, Ms::Sid> chordSpellingMap = {
        { "Standard", Ms::Sid::useStandardNoteNames },
        { "German", Ms::Sid::useGermanNoteNames },
        { "German Full", Ms::Sid::useFullGermanNoteNames },
        { "Solfege", Ms::Sid::useSolfeggioNoteNames },
        { "French", Ms::Sid::useFrenchNoteNames }
    };

    QStringList chordSymbolStyles = { "Pop/Contemporary", "Jazz", "Symbols", "No preset style" };
};
}
#endif // CHORDSYMBOLEDITORMODEL_H

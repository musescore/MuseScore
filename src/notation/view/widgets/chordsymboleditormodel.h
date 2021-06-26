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

    Q_PROPERTY(int chordSpellingIndex READ chordSpellingIndex NOTIFY chordSpellingIndexChanged)
    Q_PROPERTY(int currentStyleIndex READ currentStyleIndex NOTIFY currentStyleIndexChanged)
    Q_PROPERTY(int majorSeventhIndex READ majorSeventhIndex NOTIFY majorSeventhIndexChanged)
    Q_PROPERTY(int halfDiminishedIndex READ halfDiminishedIndex NOTIFY halfDiminishedIndexChanged)
    Q_PROPERTY(int minorIndex READ minorIndex NOTIFY minorIndexChanged)
    Q_PROPERTY(int augmentedIndex READ augmentedIndex NOTIFY augmentedIndexChanged)
    Q_PROPERTY(int diminishedIndex READ diminishedIndex NOTIFY diminishedIndexChanged)

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

    int chordSpellingIndex() const;
    int currentStyleIndex() const;
    int majorSeventhIndex() const;
    int halfDiminishedIndex() const;
    int minorIndex() const;
    int augmentedIndex() const;
    int diminishedIndex() const;

    void initChordSpellingList();
    void initCurrentStyleIndex();
    void updatePropertyIndices();
    void updateQualitySymbolsIndices();
    void setQualitySymbolsLists();

    Q_INVOKABLE void setChordStyle(QString styleName);
    Q_INVOKABLE void setChordSpelling(QString spelling);
    Q_INVOKABLE void setQualitySymbol(QString quality, QString symbol);

signals:
    void chordSpellingListChanged();
    void majorSeventhListChanged();
    void halfDiminishedListChanged();
    void minorListChanged();
    void augmentedListChanged();
    void diminishedListChanged();

    void chordSpellingIndexChanged();
    void currentStyleIndexChanged();
    void majorSeventhIndexChanged();
    void halfDiminishedIndexChanged();
    void minorIndexChanged();
    void augmentedIndexChanged();
    void diminishedIndexChanged();

private:
    enum RoleNames {
        StyleNameRole = Qt::UserRole + 1,
        FileRole
    };

    QList<ChordSymbolStyle> m_styles;
    ChordSymbolStyleManager* styleManager;
    QHash<QString, QStringList> m_qualitySymbols;

    QStringList m_chordSpellingList;
    QStringList m_majorSeventhList;
    QStringList m_halfDiminishedList;
    QStringList m_minorList;
    QStringList m_augmentedList;
    QStringList m_diminishedList;

    int m_chordSpellingIndex;
    int m_currentStyleIndex;
    int m_majorSeventhIndex;
    int m_halfDiminishedIndex;
    int m_minorIndex;
    int m_augmentedIndex;
    int m_diminishedIndex;

    QHash<QString, Ms::Sid> chordSpellingMap = {
        { "Standard", Ms::Sid::useStandardNoteNames },
        { "German", Ms::Sid::useGermanNoteNames },
        { "German Full", Ms::Sid::useFullGermanNoteNames },
        { "Solfege", Ms::Sid::useSolfeggioNoteNames },
        { "French", Ms::Sid::useFrenchNoteNames }
    };
};
}
#endif // CHORDSYMBOLEDITORMODEL_H

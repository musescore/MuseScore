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

    void initChordSpellingList();
    void setQualityRepresentationsLists();

    Q_INVOKABLE void setChordStyle(QString styleName);
    Q_INVOKABLE void setChordSpelling(QString spelling);

signals:
    void chordSpellingListChanged();
    void majorSeventhListChanged();
    void halfDiminishedListChanged();
    void minorListChanged();
    void augmentedListChanged();
    void diminishedListChanged();

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
};
}
#endif // CHORDSYMBOLEDITORMODEL_H

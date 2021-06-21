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

#ifndef CHORDSYMBOLEDITORMODEL_H
#define CHORDSYMBOLEDITORMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "engraving/libmscore/style.h"
#include "notation/internal/chordsymbolstylemanager.h"

class ChordSymbolEditorModel : public QAbstractListModel
{
    INJECT(notation, mu::context::IGlobalContext, globalContext)

    Q_OBJECT

    Q_PROPERTY(QList<QString> chordSpellingList READ chordSpellingList NOTIFY chordSpellingListChanged)
    Q_PROPERTY(QList<QString> majorSeventhList READ majorSeventhList NOTIFY majorSeventhListChanged)
    Q_PROPERTY(QList<QString> halfDiminishedList READ halfDiminishedList NOTIFY halfDiminishedListChanged)
    Q_PROPERTY(QList<QString> minorList READ minorList NOTIFY minorListChanged)
    Q_PROPERTY(QList<QString> augmentedList READ augmentedList NOTIFY augmentedListChanged)
    Q_PROPERTY(QList<QString> diminishedList READ diminishedList NOTIFY diminishedListChanged)

public:
    ChordSymbolEditorModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QList<QString> chordSpellingList() const;
    QList<QString> majorSeventhList() const;
    QList<QString> halfDiminishedList() const;
    QList<QString> minorList() const;
    QList<QString> augmentedList() const;
    QList<QString> diminishedList() const;

    Q_INVOKABLE void setChordStyle(QString styleName) const;

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

    QList<Ms::ChordSymbolStyle> m_styles;
    ChordSymbolStyleManager* styleManager;

    QList<QString> m_chordSpellingList;
    QList<QString> m_majorSeventhList;
    QList<QString> m_halfDiminishedList;
    QList<QString> m_minorList;
    QList<QString> m_augmentedList;
    QList<QString> m_diminishedList;

};

#endif // CHORDSYMBOLEDITORMODEL_H

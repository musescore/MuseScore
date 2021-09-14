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
#ifndef MU_INSPECTOR_CHORDSYMBOLSTYLESMODEL_H
#define MU_INSPECTOR_CHORDSYMBOLSTYLESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "notation/internal/chordsymbolstylemanager.h"

namespace mu::inspector {
class ChordSymbolStylesModel : public QAbstractListModel, async::Asyncable
{
    INJECT(inspector, context::IGlobalContext, globalContext)

    Q_OBJECT

    Q_PROPERTY(int currentStyleIndex READ currentStyleIndex NOTIFY currentStyleIndexChanged)

public:
    explicit ChordSymbolStylesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentStyleIndex() const;

    void initCurrentStyleIndex();
    void setQualitySymbolsOnStyleChange();
    void setPropertiesOnStyleChange();
    void setChordSpelling(QString newSpelling);
    void extractSelectionHistory(QString selectionHistory);
    void setStyle(Ms::Sid id, QVariant val);
    QVariant getDefVal(Ms::Sid id);

    Q_INVOKABLE void setChordStyle(int index);

signals:
    void currentStyleIndexChanged();

private:
    enum RoleNames {
        StyleNameRole = Qt::UserRole + 1,
        FileRole
    };

    QList<notation::ChordSymbolStyle> m_styles;
    notation::ChordSymbolStyleManager* styleManager;
    QHash<QString, QHash<QString, QVariant> > m_selectionHistory;

    QStringList m_chordSpellingList;

    int m_currentStyleIndex;
};
}
#endif // CHORDSYMBOLSTYLESMODEL_H

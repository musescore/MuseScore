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

#ifndef MU_MPE_ARTICULATIONPATTERNSSCOPEITEM_H
#define MU_MPE_ARTICULATIONPATTERNSSCOPEITEM_H

#include <QAbstractListModel>
#include <QList>

#include "mpetypes.h"
#include "articulationpatternitem.h"

namespace mu::mpe {
class ArticulationPatternsScopeItem : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(ArticulationPatternItem * currentPattern READ currentPattern WRITE setCurrentPattern NOTIFY currentPatternChanged)

public:
    explicit ArticulationPatternsScopeItem(QObject* parent, const ArticulationType type,
                                           const ArticulationPatternsScope& patterns = ArticulationPatternsScope());

    enum Roles {
        PatternItem = Qt::UserRole + 1,
    };

    const QString& title() const;
    void setTitle(const QString& newTitle);

    ArticulationPatternItem* currentPattern() const;
    void setCurrentPattern(ArticulationPatternItem* newCurrentPattern);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:

    void titleChanged();
    void currentPatternChanged();

private:
    QString m_title;
    ArticulationPatternItem* m_currentPattern = nullptr;

    QList<ArticulationPatternItem*> m_items;
};
}

#endif // MU_MPE_ARTICULATIONPATTERNSSCOPEITEM_H

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
#ifndef MU_DIAGNOSTICS_DIAGNOSTICENGRAVINGELEMENTSMODEL_H
#define MU_DIAGNOSTICS_DIAGNOSTICENGRAVINGELEMENTSMODEL_H

#include <QAbstractItemModel>
#include <QHash>

#include "modularity/ioc.h"
#include "idiagnosticengravingregister.h"

namespace mu::diagnostics {
class DiagnosticEngravingElementsModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString info READ info NOTIFY infoChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)

    INJECT(diagnostics, IDiagnosticEngravingRegister, engravingRegister)

public:
    DiagnosticEngravingElementsModel(QObject* parent = 0);

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString info() const;
    QString summary() const;

    Q_INVOKABLE void init();
    Q_INVOKABLE void reload();

signals:
    void infoChanged();
    void summaryChanged();

private:

    enum Roles {
        rItemData = Qt::UserRole + 1,
    };

    struct Item
    {
        Item(Item* parent);
        ~Item();

        quintptr key() const { return reinterpret_cast<quintptr>(this); }
        int row() const;
        Item* parent() const { return m_parent; }

        void addChild(Item* child) { m_children.append(child); }
        Item* child(int row) const { return m_children.at(row); }
        int childCount() const { return m_children.count(); }

        void setElement(const Ms::ScoreElement* el) { m_element = el; }
        const Ms::ScoreElement* element() const { return m_element; }

        void setData(const QVariant& d) { m_data = d; }
        QVariant data() const { return m_data; }

    private:
        const Ms::ScoreElement* m_element = nullptr;
        Item* m_parent = nullptr;
        QList<Item*> m_children;
        QVariant m_data;
    };

    Item* createItem(Item* parent);
    Item* itemByModelIndex(const QModelIndex& index) const;
    QVariant makeData(const Ms::ScoreElement* el) const;

    void load(const std::list<const Ms::ScoreElement*>& elements, Item* root);
    void findAndAddLoss(const std::list<const Ms::ScoreElement*>& elements, Item* lossRoot, const Item* root);
    const Item* findItem(const Ms::ScoreElement* el, const Item* root) const;

    void updateInfo();

    Item* m_rootItem = nullptr;
    QHash<quintptr, Item*> m_allItems;

    QString m_info;
    QString m_summary;
};
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICENGRAVINGELEMENTSMODEL_H

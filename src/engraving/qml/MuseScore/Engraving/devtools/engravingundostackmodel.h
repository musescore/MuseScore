/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QAbstractItemModel>
#include <QHash>
#include <qqmlintegration.h>

#include "async/asyncable.h"

#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

namespace mu::engraving {
class UndoCommand;

class EngravingUndoStackModel : public QAbstractItemModel, public muse::async::Asyncable, public muse::Injectable
{
    Q_OBJECT
    QML_ELEMENT;

    muse::Inject<context::IGlobalContext> context = { this };

public:
    EngravingUndoStackModel(QObject* parent = 0);

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void init();
    Q_INVOKABLE void reload();

private:
    enum Roles {
        rItemData = Qt::UserRole + 1,
    };

    struct Item {
        Item(Item* parent);
        ~Item();

        quintptr key() const { return reinterpret_cast<quintptr>(this); }
        int row() const;
        Item* parent() const { return m_parent; }

        void addChild(Item* child) { m_children.append(child); }
        Item* child(int row) const { return m_children.at(row); }
        int childCount() const { return m_children.count(); }

        void setData(const QVariantMap& data) { m_data = data; }
        const QVariantMap& data() const { return m_data; }

    private:
        Item* m_parent = nullptr;
        QList<Item*> m_children;
        QVariantMap m_data;
    };

    void onNotationChanged();

    Item* createItem(Item* parent, const engraving::UndoCommand* undoCommand, bool isCurrent = false);
    Item* itemByModelIndex(const QModelIndex& index) const;
    QVariantMap makeData(const mu::engraving::EngravingObject* el) const;

    void load(const engraving::UndoCommand* undoCommand, Item* parent);

    Item* m_rootItem = nullptr;
    QHash<quintptr, Item*> m_allItems;
};
}

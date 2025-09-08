/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <memory>
#include <vector>

#include "context/iglobalcontext.h"
#include "engraving/types/types.h"
#include "modularity/ioc.h"
#include "notation/inotation.h"

namespace mu::engraving {
class System;
}

namespace mu::notation {
class EmptyStavesVisibilityModel : public QAbstractItemModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(bool canResetAll READ canResetAll NOTIFY canResetAllChanged)

    muse::Inject<context::IGlobalContext> globalContext { this };

public:
    explicit EmptyStavesVisibilityModel(QObject* parent = nullptr);
    ~EmptyStavesVisibilityModel() override;

    void load(INotationPtr notation, const std::vector<engraving::System*>& systems);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Q_INVOKABLE void resetVisibility(const QModelIndex& index);
    Q_INVOKABLE void resetAllVisibility();
    QHash<int, QByteArray> roleNames() const override;

    int startSystemIndex() const;
    int systemCount() const { return static_cast<int>(m_systems.size()); }
    const std::vector<engraving::System*>& systems() const { return m_systems; }

signals:
    void canResetAllChanged();

private:
    enum Roles {
        Name = Qt::UserRole + 1,
        IsVisible,
        CanChangeVisibility,
        CanReset
    };

    struct Item;
    struct PartItem;
    struct StaffItem;

    void reload();

    bool canResetAll() const;

    void setPartVisibility(PartItem* partItem, engraving::AutoOnOff value);
    void setStaffVisibility(StaffItem* staffItem, engraving::AutoOnOff value);

    void updateData(PartItem* partItem);

    int partIndex(const PartItem* partItem) const;

    INotationPtr m_notation;
    std::vector<engraving::System*> m_systems;

    std::vector<std::unique_ptr<PartItem> > m_parts;
};
}

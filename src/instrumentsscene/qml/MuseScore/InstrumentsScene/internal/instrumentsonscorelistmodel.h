/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include <map>
#include <optional>
#include <vector>

#include "uicomponents/qml/Muse/UiComponents/selectableitemlistmodel.h"

#include "modularity/ioc.h"
#include "project/iprojectsession.h"
#include "notation/iinstrumentsrepository.h"

namespace mu::instrumentsscene {
class InstrumentsOnScoreListModel : public muse::uicomponents::SelectableItemListModel, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(QStringList orders READ orders NOTIFY ordersChanged)
    Q_PROPERTY(int currentOrderIndex READ currentOrderIndex WRITE setCurrentOrderIndex NOTIFY currentOrderChanged)

    QML_ELEMENT

public:
    muse::GlobalInject<notation::IInstrumentsRepository> repository;
    muse::ContextInject<project::IProjectSession> context = { this };

    //! Groups are listed only while shared staves are enabled
    enum SharedGroupRole {
        NotInGroup = 0,
        GroupHeader,
        GroupMember,
        GroupLastMember, // the group's last instrument: the group ends below it
    };
    Q_ENUM(SharedGroupRole)

    InstrumentsOnScoreListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList orders() const;
    int currentOrderIndex() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void addInstruments(const QStringList& instrumentIdList);

    Q_INVOKABLE void moveSelectedBlocksUp();
    Q_INVOKABLE void moveSelectedBlocksDown();

    Q_INVOKABLE QVariant currentOrder() const;
    Q_INVOKABLE QVariantList instruments() const;

    Q_INVOKABLE bool isSharedGroupSelected(const QString& sharedGroupId) const;

public slots:
    void setCurrentOrderIndex(int index);

signals:
    void ordersChanged();
    void currentOrderChanged();

private:
    class InstrumentItem;

    enum Roles {
        RoleName = SelectableItemListModel::UserRole + 1,
        RoleDescription,
        RoleIsSoloist,
        RoleSharedGroupRole,
        RoleSharedGroupId // the group's shared part id, or empty
    };

    struct RowRange {
        int first = 0;
        int last = 0;

        int count() const { return last - first + 1; }
    };

    void loadOrders();

    SharedGroupRole sharedGroupRole(int row) const;
    bool isGroupMember(const InstrumentItem* instrument) const;

    std::vector<RowRange> blocks(const ItemList& instruments) const;
    int sortingIndex(const ItemList& instruments, const RowRange& block) const;

    std::optional<RowRange> movingRows() const;

    void moveBlock(const RowRange& block, int destinationRow);

    void updateInstrumentsOrder();
    void sortInstruments(ItemList& instruments);
    void insertInstrument(ItemList& instruments, InstrumentItem* newInstrument);

    InstrumentItem* modelIndexToItem(const QModelIndex& index) const;
    const notation::ScoreOrder& currentScoreOrder() const;

    void onUpdateOperationsAvailability() override;
    void onRowsMoved() override;
    void onRowsRemoved() override;
    void doSetCurrentOrderIndex(int index);
    bool matchesScoreOrder() const;
    void verifyScoreOrder();
    int createCustomizedScoreOrder(const notation::ScoreOrder& order);
    void removeCustomizedScoreOrder(const notation::ScoreOrder& order);

    notation::ScoreOrderList m_scoreOrders;
    int m_currentOrderIndex = 0;

    bool m_showsSharedGroups = false;

    std::map<muse::ID, QString> m_sharedParts; // part id -> instrument id
};
}

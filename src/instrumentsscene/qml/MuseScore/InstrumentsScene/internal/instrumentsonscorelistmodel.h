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

    // Make soloist: available while instruments are selected; on while all of them are soloists
    Q_PROPERTY(bool isSoloistAvailable READ isSoloistAvailable NOTIFY soloistStateChanged)
    Q_PROPERTY(bool isSelectionSoloist READ isSelectionSoloist NOTIFY soloistStateChanged)

    QML_ELEMENT

    muse::GlobalInject<notation::IInstrumentsRepository> repository;
    muse::ContextInject<project::IProjectSession> context = { this };

public:
    InstrumentsOnScoreListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList orders() const;
    int currentOrderIndex() const;

    bool isSoloistAvailable() const;
    bool isSelectionSoloist() const;

    // Makes all selected instruments soloists, or (if they all are already) none of them
    Q_INVOKABLE void toggleSoloistForSelection();

    Q_INVOKABLE void load();
    // Returns the row of the first (topmost) instrument added, or -1 if none was
    Q_INVOKABLE int addInstruments(const QStringList& instrumentIdList);

    Q_INVOKABLE QVariant currentOrder() const;
    Q_INVOKABLE QVariantList instruments() const;

public slots:
    void setCurrentOrderIndex(int index);

signals:
    void ordersChanged();
    void currentOrderChanged();
    void soloistStateChanged();

private:
    class InstrumentItem;

    enum Roles {
        RoleName = SelectableItemListModel::UserRole + 1,
        RoleDescription,
        RoleIsSoloist,
        RoleSharedGroupRole,
        RoleSharedGroupId,
        RoleIsNew,
        RoleTablatureOption
    };

    void loadOrders();

    QList<InstrumentItem*> selectedInstruments() const;

    // Shared-staves groups (shown while the score combines instruments onto shared staves) are a header row followed
    // by their instruments' rows. A group always moves (and is sorted) as a whole; its instruments can't be moved
    int sharedGroupRole(int row) const;
    int lastRowOfBlock(int row) const;
    int firstRowOfBlock(int row) const;
    QList<ItemList> blocks(const ItemList& instruments) const;
    int sortingIndex(const ItemList& block) const;

    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                  int destinationChild) override;
    void onUpdateOperationsAvailability() override;

    int resolveInstrumentSequenceNumber(const muse::String& instrumentId) const;
    void updateInstrumentsOrder();
    void sortInstruments(ItemList& instruments);
    void insertInstrument(ItemList& instruments, InstrumentItem* newInstrument);

    InstrumentItem* modelIndexToItem(const QModelIndex& index) const;
    const notation::ScoreOrder& currentScoreOrder() const;

    void onRowsMoved() override;
    void onRowsRemoved() override;
    void doSetCurrentOrderIndex(int index);
    bool matchesScoreOrder() const;
    void verifyScoreOrder();
    int createCustomizedScoreOrder(const notation::ScoreOrder& order);
    void removeCustomizedScoreOrder(const notation::ScoreOrder& order);

    notation::ScoreOrderList m_scoreOrders;
    int m_currentOrderIndex = 0;
};
}

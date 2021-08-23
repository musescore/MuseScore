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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTSONSCORELISTMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTSONSCORELISTMODEL_H

#include "uicomponents/view/selectableitemlistmodel.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/iinstrumentsrepository.h"

namespace mu::instrumentsscene {
class InstrumentsOnScoreListModel : public uicomponents::SelectableItemListModel
{
    Q_OBJECT

    INJECT(instrumentsscene, context::IGlobalContext, context)
    INJECT(instrumentsscene, notation::IInstrumentsRepository, repository)

    Q_PROPERTY(QStringList orders READ orders NOTIFY ordersChanged)
    Q_PROPERTY(int currentOrderIndex READ currentOrderIndex WRITE setCurrentOrderIndex NOTIFY currentOrderChanged)

public:
    InstrumentsOnScoreListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList orders() const;
    int currentOrderIndex() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void addInstruments(const QVariantList& instruments);

    Q_INVOKABLE QVariant currentOrder() const;
    Q_INVOKABLE QVariantList instruments() const;

public slots:
    void setCurrentOrderIndex(int index);

signals:
    void ordersChanged();
    void currentOrderChanged();

private:
    class InstrumentItem;

    enum Roles {
        RoleName = SelectableItemListModel::UserRole + 1,
        RoleIsSoloist
    };

    void loadInstrumentsMeta();

    void updateInstrumentsOrder();
    void sortInstruments(ItemList& instruments);
    int sortInstrumentsIndex(const notation::ScoreOrder& order, const InstrumentItem& instrument) const;

    InstrumentItem* modelIndexToItem(const QModelIndex& index) const;
    const notation::ScoreOrder& currentScoreOrder() const;

    void onRowsMoved() override;
    void doSetCurrentOrderIndex(int index);

    notation::InstrumentTemplateList m_instrumentTemplates;
    notation::ScoreOrderList m_scoreOrders;
    int m_currentOrderIndex = 0;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTSONSCORELISTMODEL_H

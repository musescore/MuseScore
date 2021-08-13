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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTSONSCOREMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTSONSCOREMODEL_H

#include <QAbstractListModel>

#include "uicomponents/view/itemmultiselectionmodel.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/iinstrumentsrepository.h"

namespace mu::instrumentsscene {
class InstrumentsOnScoreModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(instrumentsscene, context::IGlobalContext, context)
    INJECT(instrumentsscene, notation::IInstrumentsRepository, repository)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

    Q_PROPERTY(QStringList orders READ orders NOTIFY ordersChanged)
    Q_PROPERTY(int currentOrderIndex READ currentOrderIndex WRITE setCurrentOrderIndex NOTIFY currentOrderChanged)

    Q_PROPERTY(bool isMovingUpAvailable READ isMovingUpAvailable NOTIFY selectionChanged)
    Q_PROPERTY(bool isMovingDownAvailable READ isMovingDownAvailable NOTIFY selectionChanged)
    Q_PROPERTY(bool isRemovingAvailable READ isRemovingAvailable NOTIFY selectionChanged)

public:
    InstrumentsOnScoreModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList orders() const;
    int currentOrderIndex() const;

    bool isMovingUpAvailable() const;
    bool isMovingDownAvailable() const;
    bool isRemovingAvailable() const;

    Q_INVOKABLE void load();

    Q_INVOKABLE void addInstruments(const QVariantList& instruments);

    Q_INVOKABLE void selectInstrument(int instrumentIndex);
    Q_INVOKABLE void toggleSoloist(int instrumentIndex);

    Q_INVOKABLE void removeSelection();
    Q_INVOKABLE void moveSelectionUp();
    Q_INVOKABLE void moveSelectionDown();

    Q_INVOKABLE QVariantMap scoreContent() const;

public slots:
    void setCurrentOrderIndex(int index);

signals:
    void countChanged();
    void ordersChanged();
    void currentOrderChanged();
    void selectionChanged();

private:
    void loadInstrumentsMeta();

    enum Roles {
        RoleName = Qt::UserRole + 1,
        RoleIsSelected,
        RoleIsSoloist
    };

    struct InstrumentOnScore
    {
        QString id;
        QString name;
        QString familyId;
        bool isSoloist = false;
        bool isExistingPart = false;
        notation::InstrumentTemplate instrumentTemplate;

        bool operator==(const InstrumentOnScore& info) const
        {
            return id == info.id;
        }
    };

    void sortInstruments();
    int sortInstrumentsIndex(const InstrumentOnScore& instrument) const;

    bool isIndexValid(int index) const;

    uicomponents::ItemMultiSelectionModel* m_selection = nullptr;
    QList<InstrumentOnScore> m_instruments;
    notation::InstrumentTemplateList m_instrumentTemplates;
    notation::ScoreOrderList m_scoreOrders;
    int m_currentOrderIndex = 0;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTSONSCOREMODEL_H

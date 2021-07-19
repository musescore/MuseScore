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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTLISTMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTLISTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "notation/iinstrumentsrepository.h"
#include "context/iglobalcontext.h"

namespace mu::instrumentsscene {
class InstrumentListModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(instruments, notation::IInstrumentsRepository, repository)
    INJECT(instruments, context::IGlobalContext, globalContext)

    Q_PROPERTY(QVariantList families READ families NOTIFY dataChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY dataChanged)
    Q_PROPERTY(QVariantList instruments READ instruments NOTIFY dataChanged)
    Q_PROPERTY(QVariantList scoreOrders READ scoreOrders NOTIFY scoreOrdersChanged)
    Q_PROPERTY(QVariant selectedScoreOrderIndex READ selectedScoreOrderIndex WRITE setSelectedScoreOrderIndex NOTIFY scoreOrdersChanged)
    Q_PROPERTY(QVariantList selectedInstruments READ selectedInstruments NOTIFY selectedInstrumentsChanged)

public:
    InstrumentListModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(bool canSelectMultipleInstruments, const QString& currentInstrumentId, const QString& currentScoreOrderId,
                          const QString& selectedPartIds);

    QVariantList families() const;
    QVariantList groups() const;
    QVariantList instruments() const;
    QVariantList scoreOrders() const;
    QVariant selectedScoreOrderIndex() const;
    void setSelectedScoreOrderIndex(const QVariant& index);
    QVariantList selectedInstruments() const;

    Q_INVOKABLE QString selectedGroupId() const;

    Q_INVOKABLE void selectFamily(const QString& familyId);
    Q_INVOKABLE void selectGroup(const QString& groupId);

    Q_INVOKABLE void selectInstrument(const QString& instrumentName, const QString& traitName = QString());
    Q_INVOKABLE void unselectInstrument(int instrumentIndex);
    Q_INVOKABLE void swapSelectedInstruments(int firstInstrumentIndex, int secondInstrumentIndex);
    Q_INVOKABLE void toggleSoloist(int instrumentIndex);

    Q_INVOKABLE void setSearchText(const QString& text);

    Q_INVOKABLE void selectScoreOrder(const QString& orderId);

signals:
    void dataChanged();

    void selectedFamilyChanged(QString familyId);
    void selectedGroupChanged(QString groupId);

    void searchStringChanged(QString searchString);
    void selectedInstrumentsChanged();

    void selectedOrderChanged(QString familyId);

    void scoreOrdersChanged();

private:
    struct SelectedInstrumentInfo
    {
        QString id;
        QString name;
        QString familyId;
        bool isSoloist = false;
        bool isExistingPart = false;
        notation::Instrument config;

        bool operator==(const SelectedInstrumentInfo& info) const { return id == info.id; }
    };

    struct ScoreOrderInfo
    {
        QString id;
        bool customized = false;
        notation::ScoreOrder info;

        bool operator==(const ScoreOrderInfo& info) const { return id == info.id; }
    };

    void initSelectedInstruments(const notation::IDList& selectedPartIds);
    notation::INotationPartsPtr notationParts() const;
    void initScoreOrders(const QString& currentId);

    void sortInstruments(QVariantList& instruments) const;

    bool isSearching() const;

    void setInstrumentsMeta(const notation::InstrumentsMeta& meta);
    QVariantMap allInstrumentsItem() const;

    void updateFamilyStateBySearch();

    bool isInstrumentAccepted(const notation::Instrument& instrument, bool compareWithSelectedGroup = true) const;

    notation::Instrument instrumentById(const QString& instrumentId) const;

    int indexOfScoreOrderId(const QString& id) const;
    void sortSelectedInstruments();
    int sortInstrumentsIndex(const SelectedInstrumentInfo& info) const;
    int instrumentInsertIndex(const SelectedInstrumentInfo& info) const;
    bool matchesScoreOrder() const;
    void checkScoreOrderMatching(bool block);
    void makeCustomizedScoreOrder(const ScoreOrderInfo& order);

    bool isInstrumentIndexValid(int index) const;

    bool m_canSelectMultipleInstruments = false;

    bool m_blockSortingInstruments = false;

    QString m_selectedFamilyId;
    QString m_selectedGroupId;
    QString m_savedFamilyId;
    int m_selectedScoreOrderIndex = -1;

    notation::InstrumentsMeta m_instrumentsMeta;
    QString m_searchText;

    QList<SelectedInstrumentInfo> m_selectedInstruments;
    QList<ScoreOrderInfo> m_scoreOrders;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTLISTMODEL_H

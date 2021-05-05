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
#ifndef MU_INSTRUMENTS_INSTRUMENTLISTMODEL_H
#define MU_INSTRUMENTS_INSTRUMENTLISTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iinstrumentsrepository.h"
#include "context/iglobalcontext.h"

namespace mu::instruments {
class InstrumentListModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(instruments, IInstrumentsRepository, repository)
    INJECT(instruments, context::IGlobalContext, globalContext)

    Q_PROPERTY(QVariantList families READ families NOTIFY dataChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY dataChanged)
    Q_PROPERTY(QVariantList instruments READ instruments NOTIFY dataChanged)
    Q_PROPERTY(QVariantList selectedInstruments READ selectedInstruments NOTIFY selectedInstrumentsChanged)

public:
    InstrumentListModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(bool canSelectMultipleInstruments, const QString& currentInstrumentId, const QString& selectedPartIds);

    QVariantList families() const;
    QVariantList groups() const;
    QVariantList instruments() const;
    QVariantList selectedInstruments() const;

    Q_INVOKABLE QString selectedGroupId() const;

    Q_INVOKABLE void selectFamily(const QString& familyId);
    Q_INVOKABLE void selectGroup(const QString& groupId);

    Q_INVOKABLE void selectInstrument(const QString& instrumentId, const QString& transpositionId = QString());
    Q_INVOKABLE void unselectInstrument(const QString& instrumentId);
    Q_INVOKABLE void swapSelectedInstruments(int firstIndex, int secondIndex);
    Q_INVOKABLE void makeSoloist(const QString& instrumentId);

    Q_INVOKABLE void setSearchText(const QString& text);

    Q_INVOKABLE QVariantList instrumentOrderTypes() const;
    Q_INVOKABLE void selectOrderType(const QString& id);

    Q_INVOKABLE QString findInstrument(const QString& instrumentId) const;

signals:
    void dataChanged();

    void selectedFamilyChanged(QString familyId);
    void selectedGroupChanged(QString groupId);

    void searchStringChanged(QString searchString);
    void selectedInstrumentsChanged();

private:
    void initSelectedInstruments(const notation::IDList& selectedPartIds);
    notation::INotationPartsPtr notationParts() const;

    void sortInstruments(QVariantList& instruments) const;

    bool isSearching() const;

    void setInstrumentsMeta(const InstrumentsMeta& meta);
    QVariantMap allInstrumentsItem() const;
    InstrumentGroupList sortedGroupList() const;

    QVariantMap defaultInstrumentTranspositionItem() const;

    void updateFamilyStateBySearch();

    bool isInstrumentAccepted(const Instrument& instrument, bool compareWithSelectedGroup = true) const;

    InstrumentTemplate instrumentTemplate(const QString& instrumentId) const;

    bool m_canSelectMultipleInstruments = false;

    QString m_selectedFamilyId;
    QString m_selectedGroupId;
    QString m_savedFamilyId;

    InstrumentsMeta m_instrumentsMeta;
    QString m_searchText;

    struct SelectedInstrumentInfo
    {
        QString id;
        QString partId;
        QString partName;
        Transposition transposition;
        Instrument config;

        bool operator==(const SelectedInstrumentInfo& info) const { return id == info.id; }
    };

    QList<SelectedInstrumentInfo> m_selectedInstruments;
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTLISTMODEL_H

//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_INSTRUMENTS_INSTRUMENTLISTMODEL_H
#define MU_INSTRUMENTS_INSTRUMENTLISTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iinstrumentsrepository.h"
#include "context/iglobalcontext.h"

namespace mu {
namespace instruments {
class InstrumentListModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(instruments, IInstrumentsRepository, repository)
    INJECT(instruments, context::IGlobalContext, context)

    Q_PROPERTY(QVariantList families READ families NOTIFY dataChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY dataChanged)
    Q_PROPERTY(QVariantList instruments READ instruments NOTIFY dataChanged)
    Q_PROPERTY(QVariantList selectedInstruments READ selectedInstruments NOTIFY selectedInstrumentsChanged)

public:
    InstrumentListModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QVariantList families() const;
    QVariantList groups() const;
    QVariantList instruments() const;
    QVariantList selectedInstruments() const;

    Q_INVOKABLE void selectFamily(const QString& family);
    Q_INVOKABLE void selectGroup(const QString& group);

    Q_INVOKABLE void selectInstrument(const QString& instrumentId, const QString& transpositionId);
    Q_INVOKABLE void unselectInstrument(const QString& instrumentId);
    Q_INVOKABLE void swapSelectedInstruments(int firstIndex, int secondIndex);
    Q_INVOKABLE void makeSoloist(const QString& instrumentId);

    Q_INVOKABLE void setSearchText(const QString& text);

    Q_INVOKABLE QVariantList instrumentOrderTypes() const;
    Q_INVOKABLE void selectOrderType(const QString& id);

signals:
    void dataChanged();

    void selectedFamilyChanged(QString family);

    void searchStringChanged(QString searchString);
    void selectedInstrumentsChanged();

private:
    void initSelectedInstruments();

    bool isSearching() const;

    void setInstrumentsMeta(const InstrumentsMeta& meta);
    QVariantList allInstrumentsGroupList() const;
    QVariantMap allInstrumentsItem() const;

    QVariantMap defaultInstrumentTranspositionItem() const;

    void updateFamilyStateBySearch();

    bool isInstrumentAccepted(const Instrument& instrument) const;

    QString m_selectedFamilyId;
    QString m_savedSelectedFamilyId;

    QString m_selectedGroupId;

    InstrumentsMeta m_instrumentsMeta;
    QString m_searchText;

    struct SelectedInstrumentInfo
    {
        QString id;
        Transposition transposition;
        Instrument config;

        bool operator==(const SelectedInstrumentInfo& info) const { return id == info.id; }
    };

    QList<SelectedInstrumentInfo> m_selectedInstruments;
};
}
}

#endif // MU_INSTRUMENTS_INSTRUMENTLISTMODEL_H

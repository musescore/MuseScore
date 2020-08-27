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
#include "instrumentlistmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::instruments;

static const QString ALL_INSTRUMENTS_ID("ALL_INSTRUMENTS");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_ID("EMPTY_KEY");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_NAME("--");

static const QString ID_KEY("id");
static const QString NAME_KEY("name");
static const QString TRANSPOSITIONS_KEY("transpositions");

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentListModel::load()
{
    RetValCh<InstrumentsMeta> instrumentsMeta = repository()->instrumentsMeta();
    if (!instrumentsMeta.ret) {
        LOGE() << instrumentsMeta.ret.toString();
        return;
    }

    instrumentsMeta.ch.onReceive(this, [this](const InstrumentsMeta& newInstrumentsMeta) {
        setInstrumentsMeta(newInstrumentsMeta);
    });

    m_selectedFamilyId = ALL_INSTRUMENTS_ID;
    setInstrumentsMeta(instrumentsMeta.val);
}

QVariantList InstrumentListModel::families()
{
    QVariantList result;
    result << allInstrumentsItem();

    for (const InstrumentGenre& genre: m_instrumentsMeta.genres) {
        QVariantMap obj;
        obj[ID_KEY] = genre.id;
        obj[NAME_KEY] = genre.name;

        result << obj;
    }

    return result;
}

QVariantList InstrumentListModel::groups()
{
    QVariantList result;

    bool allInstrumentsSelected = m_selectedFamilyId.isEmpty() || m_selectedFamilyId == ALL_INSTRUMENTS_ID;
    if (!isSearching() && allInstrumentsSelected) {
        return allInstrumentsGroupList();
    }

    QStringList availableGroups;
    for (const InstrumentTemplate& templ: m_instrumentsMeta.instrumentTemplates) {
        const Instrument& instrument = templ.instrument;

        if (!isInstrumentAccepted(instrument)) {
            continue;
        }

        if (!availableGroups.contains(instrument.groupId)) {
            availableGroups << instrument.groupId;
        }
    }

    for (const QString& groupId: availableGroups) {
        InstrumentGroup group = m_instrumentsMeta.groups[groupId];

        QVariantMap obj;
        obj[ID_KEY] = group.id;
        obj[NAME_KEY] = group.name;

        result << obj;
    }

    return result;
}

QVariantList InstrumentListModel::instruments()
{
    QVariantHash availableInstruments;

    for (const InstrumentTemplate& templ: m_instrumentsMeta.instrumentTemplates) {
        const Instrument& instrument = templ.instrument;

        if (instrument.groupId != m_selectedGroupId) {
            continue;
        }

        if (!isInstrumentAccepted(instrument)) {
            continue;
        }

        QVariantList instrumentTranspositions;
        QString instrumentId = templ.id;
        QString instrumentName = instrument.trackName;

        Transposition _transposition = templ.transposition;
        if (_transposition.isValid()) {
            instrumentId = instrumentId.replace(_transposition.id, "");
            instrumentName = instrumentName.replace(_transposition.name + " ", "")
                             .replace(" in " + _transposition.name, "");

            if (availableInstruments.contains(instrumentId)) {
                instrumentTranspositions = availableInstruments[instrumentId].toMap().value(TRANSPOSITIONS_KEY).toList();
            }

            QVariantMap obj;
            obj[ID_KEY] = _transposition.id;
            obj[NAME_KEY] = _transposition.name;

            instrumentTranspositions << obj;
        } else {
            instrumentTranspositions = availableInstruments[instrumentId].toMap().value(TRANSPOSITIONS_KEY).toList();
            instrumentTranspositions << defaultInstrumentTranspositionItem();
        }

        QVariantMap instrumentObj;
        instrumentObj[ID_KEY] = instrumentId;
        instrumentObj[NAME_KEY] = instrumentName;
        instrumentObj[TRANSPOSITIONS_KEY] = instrumentTranspositions;
        availableInstruments.insert(instrumentId, instrumentObj);
    }

    return availableInstruments.values();
}

void InstrumentListModel::selectFamily(const QString& family)
{
    if (m_selectedFamilyId == family) {
        return;
    }

    m_selectedFamilyId = family;
    emit dataChanged();
    emit selectedFamilyChanged(m_selectedFamilyId);
}

void InstrumentListModel::selectGroup(const QString& group)
{
    if (m_selectedGroupId == group) {
        return;
    }

    m_selectedGroupId = group;
    emit dataChanged();
}

void InstrumentListModel::selectInstrument(const QString& id, const QString& transpositionId)
{
    QString instrumentId = id;
    if (transpositionId != INSTRUMENT_EMPTY_TRANSPOSITION_ID) {
        instrumentId = transpositionId + id;
    }

    for (const InstrumentTemplate& instrument: m_selectedInstruments) {
        if (instrument.id == instrumentId) {
            return;
        }
    }

    m_selectedInstruments << m_instrumentsMeta.instrumentTemplates[instrumentId];
    emit selectedInstrumentsChanged();
}

void InstrumentListModel::makeSoloist(const QString&)
{
    NOT_IMPLEMENTED;
}

void InstrumentListModel::unselectInstrument(const QString& id)
{
    for (int i = 0; i < m_selectedInstruments.count(); ++i) {
        if (m_selectedInstruments[i].id == id) {
            m_selectedInstruments.removeAt(i);
            emit selectedInstrumentsChanged();
            return;
        }
    }
}

void InstrumentListModel::swapSelectedInstruments(int firstIndex, int secondIndex)
{
    m_selectedInstruments.swap(firstIndex, secondIndex);
    emit selectedInstrumentsChanged();
}

void InstrumentListModel::setSearchText(const QString& text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    emit dataChanged();

    updateFamilyStateBySearch();
}

QVariantList InstrumentListModel::instrumentOrderTypes() const
{
    // TODO: just for test, needed to implement
    QVariantList result;
    result << QVariantMap { { ID_KEY, 1 }, { NAME_KEY, qtrc("instruments", "Custom") } };

    return result;
}

void InstrumentListModel::selectOrderType(const QString&)
{
    NOT_IMPLEMENTED;
}

QStringList InstrumentListModel::selectedInstrumentIds()
{
    QStringList result;
    for (const InstrumentTemplate& instrument: m_selectedInstruments) {
        result << instrument.id;
    }

    return result;
}

QVariantList InstrumentListModel::selectedInstruments() const
{
    QVariantList result;

    for (const InstrumentTemplate& templ: m_selectedInstruments) {
        QString instrumentId = templ.id;
        QString instrumentName = templ.instrument.trackName;

        Transposition _transposition = templ.transposition;
        if (_transposition.isValid()) {
            instrumentName = instrumentName.replace(_transposition.name + " ", "")
                             .replace(" in " + _transposition.name, "");

            instrumentName = QString("%1 (%2)").arg(instrumentName, _transposition.name);
        }

        QVariantMap obj;
        obj[ID_KEY] = instrumentId;
        obj[NAME_KEY] = instrumentName;
        result << obj;
    }

    return result;
}

bool InstrumentListModel::isSearching() const
{
    return !m_searchText.isEmpty();
}

void InstrumentListModel::setInstrumentsMeta(const InstrumentsMeta& meta)
{
    m_instrumentsMeta = meta;
    emit dataChanged();
}

QVariantList InstrumentListModel::allInstrumentsGroupList() const
{
    QVariantList result;
    for (const InstrumentGroup& group: m_instrumentsMeta.groups) {
        QVariantMap obj;
        obj[ID_KEY] = group.id;
        obj[NAME_KEY] = group.name;

        result << obj;
    }

    return result;
}

QVariantMap InstrumentListModel::allInstrumentsItem() const
{
    QVariantMap obj;
    obj[ID_KEY] = ALL_INSTRUMENTS_ID;
    obj[NAME_KEY] = qtrc("instruments", "All instruments");
    return obj;
}

QVariantMap InstrumentListModel::defaultInstrumentTranspositionItem() const
{
    QVariantMap obj;
    obj[ID_KEY] = INSTRUMENT_EMPTY_TRANSPOSITION_ID;
    obj[NAME_KEY] = INSTRUMENT_EMPTY_TRANSPOSITION_NAME;

    return obj;
}

void InstrumentListModel::updateFamilyStateBySearch()
{
    if (m_searchText.isEmpty()) {
        if (!m_savedSelectedFamilyId.isEmpty()) {
            selectFamily(m_savedSelectedFamilyId);
            m_savedSelectedFamilyId.clear();
        }
    } else {
        if (m_selectedFamilyId != ALL_INSTRUMENTS_ID) {
            m_savedSelectedFamilyId = m_selectedFamilyId;
            selectFamily(ALL_INSTRUMENTS_ID);
        }
    }
}

bool InstrumentListModel::isInstrumentAccepted(const Instrument& instrument) const
{
    if (isSearching()) {
        bool acceptedByGroup = m_instrumentsMeta.groups[instrument.groupId].name.contains(m_searchText, Qt::CaseInsensitive);
        bool acceptedByName = instrument.trackName.contains(m_searchText, Qt::CaseInsensitive);
        return acceptedByGroup || acceptedByName;
    }

    return m_selectedFamilyId == ALL_INSTRUMENTS_ID || instrument.genreIds.contains(m_selectedFamilyId);
}

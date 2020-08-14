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

using namespace mu::scene::instruments;

static const QString ALL_INSTRUMENTS_ID("ALL_INSTRUMENTS");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_ID("EMPTY_KEY");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_NAME("--");

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
        obj["id"] = genre.id;
        obj["name"] = genre.name;

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
    for (const InstrumentTemplate& instrument: m_instrumentsMeta.instrumentTemplates) {
        if (isSearching()) {
            bool acceptedByGroup = m_instrumentsMeta.groups[instrument.groupId].name.contains(m_searchText, Qt::CaseInsensitive);
            bool acceptedByName = instrument.trackName.contains(m_searchText, Qt::CaseInsensitive);
            if (!acceptedByGroup && !acceptedByName) {
                continue;
            }
        } else {
            bool acceptedByFamily = instrument.genreIds.contains(m_selectedFamilyId);
            if (!acceptedByFamily) {
                continue;
            }
        }

        if (!availableGroups.contains(instrument.groupId)) {
            availableGroups << instrument.groupId;
        }
    }

    for (const QString& groupId: availableGroups) {
        InstrumentGroup group = m_instrumentsMeta.groups[groupId];

        QVariantMap obj;
        obj["id"] = group.id;
        obj["name"] = group.name;

        result << obj;
    }

    return result;
}

QVariantList InstrumentListModel::instruments()
{
    QVariantHash availableInstruments;

    for (const InstrumentTemplate& instrument: m_instrumentsMeta.instrumentTemplates) {
        if (instrument.groupId != m_selectedGroupId) {
            continue;
        }

        if (isSearching()) {
            bool acceptedByGroup = m_instrumentsMeta.groups[instrument.groupId].name.contains(m_searchText, Qt::CaseInsensitive);
            bool acceptedByName = instrument.trackName.contains(m_searchText, Qt::CaseInsensitive);
            if (!acceptedByGroup && !acceptedByName) {
                continue;
            }
        } else {
            bool acceptedByFamily = m_selectedFamilyId == ALL_INSTRUMENTS_ID || instrument.genreIds.contains(m_selectedFamilyId);
            if (!acceptedByFamily) {
                continue;
            }
        }

        QVariantList instrumentTranspositions;
        QString instrumentId = instrument.id;
        QString instrumentName = instrument.trackName;

        Transposition _transposition = transposition(instrument.id);
        if (_transposition.isValid()) {
            instrumentId = instrumentId.replace(_transposition.id, "");
            instrumentName = instrumentName.replace(_transposition.name + " ", "")
                                 .replace(" in " + _transposition.name, "");

            if (availableInstruments.contains(instrumentId)) {
                instrumentTranspositions = availableInstruments[instrumentId].toMap().value("transpositions").toList();
            }

            QVariantMap obj;
            obj["id"] = _transposition.id;
            obj["name"] = _transposition.name;

            instrumentTranspositions << obj;
        } else {
            instrumentTranspositions = availableInstruments[instrumentId].toMap().value("transpositions").toList();

            QVariantMap obj;
            obj["id"] = INSTRUMENT_EMPTY_TRANSPOSITION_ID;
            obj["name"] = INSTRUMENT_EMPTY_TRANSPOSITION_NAME;
            instrumentTranspositions.append(obj);
        }

        QVariantMap instrumentObj;
        instrumentObj["id"] = instrumentId;
        instrumentObj["name"] = instrumentName;
        instrumentObj["transpositions"] = instrumentTranspositions;
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

void InstrumentListModel::setSearchText(const QString &text)
{
    if (m_searchText == text) {
        return;
    }

    m_searchText = text;
    emit dataChanged();

    updateFamilyStateBySearch();
}

InstrumentListModel::Transposition InstrumentListModel::transposition(const QString &instrumentId) const
{
    static Transposition transpositions[] = {
        {"a-", "A"},
        {"ab-", "A♭"},
        {"b-", "B"},
        {"bb-", "B♭"},
        {"c-", "C"},
        {"d-", "D"},
        {"db-", "D♭"},
        {"e-", "E"},
        {"eb-", "E♭"},
        {"f-", "F"},
        {"g-", "G"}
    };

    for (const Transposition& transposition: transpositions) {
        if (instrumentId.startsWith(transposition.id)) {
            return transposition;
        }
    }
    return Transposition();
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
        obj["id"] = group.id;
        obj["name"] = group.name;

        result << obj;
    }

    return result;
}

QVariantMap InstrumentListModel::allInstrumentsItem() const
{
    QVariantMap obj;
    obj["id"] = ALL_INSTRUMENTS_ID;
    obj["name"] = qtrc("instruments", "All instruments");
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

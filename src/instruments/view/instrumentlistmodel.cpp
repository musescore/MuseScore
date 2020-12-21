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
using namespace mu::notation;

static const QString ALL_INSTRUMENTS_ID("ALL_INSTRUMENTS");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_ID("EMPTY_KEY");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_NAME("--");

static const QString ID_KEY("id");
static const QString NAME_KEY("name");
static const QString TRANSPOSITIONS_KEY("transpositions");
static const QString CONFIG_KEY("config");

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentListModel::load(bool canSelectMultipleInstruments, const QString& currentInstrumentId,
                               const QString& selectedInstrumentIds)
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
    m_canSelectMultipleInstruments = canSelectMultipleInstruments;
    setInstrumentsMeta(instrumentsMeta.val);

    initSelectedInstruments(selectedInstrumentIds.split(','));

    if (!currentInstrumentId.isEmpty()) {
        InstrumentTemplate instrumentTemplate = this->instrumentTemplate(currentInstrumentId);
        selectGroup(instrumentTemplate.instrument.groupId);
    } else {
        QVariantList groups = this->groups();
        QString firstGroupId = !groups.isEmpty() ? groups.first().toMap().value("id").toString() : "";
        selectGroup(firstGroupId);
    }
}

void InstrumentListModel::initSelectedInstruments(const IDList& selectedInstrumentIds)
{
    for (const ID& instrumentId: selectedInstrumentIds) {
        if (instrumentId.isEmpty()) {
            continue;
        }

        InstrumentTemplate templ = instrumentTemplate(instrumentId);

        SelectedInstrumentInfo info;
        info.id = templ.id;
        info.config = templ.instrument;
        m_selectedInstruments << info;
    }

    emit selectedInstrumentsChanged();
}

QVariantList InstrumentListModel::families() const
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

QVariantList InstrumentListModel::groups() const
{
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

    QVariantList result;

    for (const InstrumentGroup& group: sortedGroupList()) {
        if (!availableGroups.contains(group.id)) {
            continue;
        }

        QVariantMap obj;
        obj[ID_KEY] = group.id;
        obj[NAME_KEY] = group.name;

        result << obj;
    }

    return result;
}

QVariantList InstrumentListModel::instruments() const
{
    QVariantMap availableInstruments;

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
        QString instrumentName = instrument.name;

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
            instrumentTranspositions.prepend(defaultInstrumentTranspositionItem());
        }

        QVariantMap instrumentObj;
        instrumentObj[ID_KEY] = instrumentId;
        instrumentObj[NAME_KEY] = instrumentName;
        instrumentObj[TRANSPOSITIONS_KEY] = instrumentTranspositions;
        availableInstruments.insert(instrumentId, instrumentObj);
    }

    QVariantList result = availableInstruments.values();

    std::sort(result.begin(), result.end(), [](const QVariant& instrument1, const QVariant& instrument2) {
        QString instrumentName1 = instrument1.toMap()[NAME_KEY].toString().toLower();
        QString instrumentName2 = instrument2.toMap()[NAME_KEY].toString().toLower();

        return instrumentName1 < instrumentName2;
    });

    return result;
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
    emit selectedGroupChanged(group);
}

void InstrumentListModel::selectInstrument(const QString& instrumentId, const QString& transpositionId)
{
    QString codeKey = instrumentId;
    if (transpositionId != INSTRUMENT_EMPTY_TRANSPOSITION_ID) {
        codeKey = transpositionId + instrumentId;
    }

    InstrumentTemplate templ = m_instrumentsMeta.instrumentTemplates[codeKey];
    if (!templ.isValid()) {
        LOGW() << QString("Instrument template %1 does not exist").arg(codeKey);
    }

    SelectedInstrumentInfo info;
    info.id = codeKey;
    info.transposition = templ.transposition;
    info.config = templ.instrument;

    if (!m_canSelectMultipleInstruments) {
        m_selectedInstruments.clear();
    }

    m_selectedInstruments << info;
    emit selectedInstrumentsChanged();
}

void InstrumentListModel::makeSoloist(const QString&)
{
    NOT_IMPLEMENTED;
}

void InstrumentListModel::unselectInstrument(const QString& instrumentId)
{
    for (int i = 0; i < m_selectedInstruments.count(); ++i) {
        if (m_selectedInstruments[i].id == instrumentId) {
            m_selectedInstruments.removeAt(i);
            emit selectedInstrumentsChanged();
            return;
        }
    }
}

void InstrumentListModel::swapSelectedInstruments(int firstIndex, int secondIndex)
{
    m_selectedInstruments.swapItemsAt(firstIndex, secondIndex);
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

QString InstrumentListModel::findInstrument(const QString& instrumentId) const
{
    return instrumentTemplate(instrumentId).id;
}

QVariantList InstrumentListModel::selectedInstruments() const
{
    QVariantList result;

    for (const SelectedInstrumentInfo& instrument: m_selectedInstruments) {
        QString instrumentId = instrument.id;
        QString instrumentName = instrument.config.name;

        Transposition _transposition = instrument.transposition;
        if (_transposition.isValid()) {
            instrumentName = instrumentName.replace(_transposition.name + " ", "")
                             .replace(" in " + _transposition.name, "");

            instrumentName = QString("%1 (%2)").arg(instrumentName, _transposition.name);
        }

        QVariantMap obj;
        obj[ID_KEY] = instrumentId;
        obj[NAME_KEY] = instrumentName;
        obj[CONFIG_KEY] = QVariant::fromValue(instrument.config);

        result << obj;
    }

    return result;
}

QString InstrumentListModel::selectedGroup() const
{
    return m_selectedGroupId;
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

    for (const InstrumentGroup& group: sortedGroupList()) {
        QVariantMap obj;
        obj[ID_KEY] = group.id;
        obj[NAME_KEY] = group.name;

        result << obj;
    }

    return result;
}

InstrumentGroupList InstrumentListModel::sortedGroupList() const
{
    InstrumentGroupList result = m_instrumentsMeta.groups.values();

    std::sort(result.begin(), result.end(), [](const InstrumentGroup& group1, const InstrumentGroup& group2) {
        return group1.sequenceOrder < group2.sequenceOrder;
    });

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
        bool acceptedByName = instrument.name.contains(m_searchText, Qt::CaseInsensitive);
        return acceptedByGroup || acceptedByName;
    }

    return m_selectedFamilyId == ALL_INSTRUMENTS_ID || instrument.genreIds.contains(m_selectedFamilyId);
}

InstrumentTemplate InstrumentListModel::instrumentTemplate(const QString& instrumentId) const
{
    for (const InstrumentTemplate& instrumentTemplate: m_instrumentsMeta.instrumentTemplates) {
        if (instrumentTemplate.instrument.id == instrumentId) {
            return instrumentTemplate;
        }
    }

    return InstrumentTemplate();
}

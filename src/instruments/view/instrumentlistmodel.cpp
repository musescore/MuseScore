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

static const QString ALL_INSTRUMENTS_GENRE_ID("ALL_INSTRUMENTS");

static const QString ID_KEY("id");
static const QString NAME_KEY("name");
static const QString TRANSPOSITIONS_KEY("transpositions");
static const QString GROUP_ID("groupId");
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

    m_selectedFamilyId = COMMON_GENRE_ID;
    m_canSelectMultipleInstruments = canSelectMultipleInstruments;
    setInstrumentsMeta(instrumentsMeta.val);

    initSelectedInstruments(selectedInstrumentIds.split(','));

    if (!currentInstrumentId.isEmpty()) {
        InstrumentTemplate instrumentTemplate = this->instrumentTemplate(currentInstrumentId);
        selectGroup(instrumentTemplate.instrument.groupId);
    }
}

void InstrumentListModel::initSelectedInstruments(const IDList& selectedInstrumentIds)
{
    for (const ID& instrumentId: selectedInstrumentIds) {
        if (instrumentId.isEmpty()) {
            continue;
        }

        m_selectedInstruments << instrumentTemplate(instrumentId);
    }

    emit selectedInstrumentsChanged();
}

QVariantList InstrumentListModel::families() const
{
    auto toMap = [](const InstrumentGenre& genre) {
        return QVariantMap {
            { ID_KEY, genre.id },
            { NAME_KEY, genre.name }
        };
    };

    QVariantList result;
    result << allInstrumentsItem();

    for (const InstrumentGenre& genre: m_instrumentsMeta.genres) {
        if (genre.id == COMMON_GENRE_ID) {
            result.prepend(toMap(genre));
            continue;
        }

        result << toMap(genre);
    }

    return result;
}

QVariantList InstrumentListModel::groups() const
{
    QStringList availableGroups;

    for (const InstrumentTemplate& templ: m_instrumentsMeta.templates) {
        const Instrument& instrument = templ.instrument;

        constexpr bool compareWithSelectedGroup = false;
        if (!isInstrumentAccepted(instrument, compareWithSelectedGroup)) {
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
    QHash<QString, QStringList> transpositions;
    QVariantMap availableInstruments;

    for (const InstrumentTemplate& templ: m_instrumentsMeta.templates) {
        const Instrument& instrument = templ.instrument;

        if (!isInstrumentAccepted(instrument)) {
            continue;
        }

        if (!templ.transpositionName.isEmpty()) {
            transpositions[instrument.name] << templ.transpositionName;
        }

        QVariantMap instrumentObj;
        instrumentObj[NAME_KEY] = instrument.name;
        instrumentObj[GROUP_ID] = instrument.groupId;

        if (transpositions.contains(instrument.name)) {
            instrumentObj[TRANSPOSITIONS_KEY] = transpositions[instrument.name];
        }

        availableInstruments[instrument.name] = instrumentObj;
    }

    QVariantList result = availableInstruments.values();
    sortInstruments(result);

    return result;
}

void InstrumentListModel::sortInstruments(QVariantList& instruments) const
{
    std::sort(instruments.begin(), instruments.end(), [this](const QVariant& instrument1, const QVariant& instrument2) {
        QString instrumentName1 = instrument1.toMap()[NAME_KEY].toString().toLower();
        QString instrumentName2 = instrument2.toMap()[NAME_KEY].toString().toLower();
        QString searchText = m_searchText.toLower();

        int searchTextPosition1 = instrumentName1.indexOf(searchText);
        int searchTextPosition2 = instrumentName2.indexOf(searchText);

        if (searchTextPosition1 == searchTextPosition2) {
            return instrumentName1 < instrumentName2;
        }

        return searchTextPosition1 < searchTextPosition2;
    });
}

void InstrumentListModel::selectFamily(const QString& familyId)
{
    if (m_selectedFamilyId == familyId) {
        return;
    }

    m_selectedFamilyId = familyId;

    emit dataChanged();
    emit selectedFamilyChanged(m_selectedFamilyId);
}

void InstrumentListModel::selectGroup(const QString& groupId)
{
    if (m_selectedGroupId == groupId) {
        return;
    }

    m_selectedGroupId = groupId;

    emit dataChanged();
    emit selectedGroupChanged(groupId);
}

void InstrumentListModel::selectInstrument(const QString& instrumentName, const QString& transpositionName)
{
    InstrumentTemplate suitedTemplate;

    for (const InstrumentTemplate& templ : m_instrumentsMeta.templates) {
        if (templ.instrument.name == instrumentName && templ.transpositionName == transpositionName) {
            suitedTemplate = templ;
            break;
        }
    }

    if (!suitedTemplate.isValid()) {
        LOGE() << QString("Template of instrument %1 with transposition %2 does not exist")
                  .arg(instrumentName)
                  .arg(transpositionName);
        return;
    }

    if (!m_canSelectMultipleInstruments) {
        m_selectedInstruments.clear();
    }

    m_selectedInstruments << suitedTemplate;
    emit selectedInstrumentsChanged();
}

void InstrumentListModel::makeSoloist(int)
{
    NOT_IMPLEMENTED;
}

void InstrumentListModel::unselectInstrument(int index)
{
    if (index < 0 || index >= m_selectedInstruments.size()) {
        return;
    }

    m_selectedInstruments.removeAt(index);
    emit selectedInstrumentsChanged();
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

    for (const InstrumentTemplate& templ: m_selectedInstruments) {
        QString instrumentName = templ.instrument.name;

        if (!templ.transpositionName.isEmpty()) {
            instrumentName += " " + templ.transpositionName;
        }

        QVariantMap obj;
        obj[ID_KEY] = templ.instrument.id;
        obj[NAME_KEY] = instrumentName;
        obj[CONFIG_KEY] = QVariant::fromValue(templ.instrument);

        result << obj;
    }

    return result;
}

QString InstrumentListModel::selectedGroupId() const
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

InstrumentGroupList InstrumentListModel::sortedGroupList() const
{
    InstrumentGroupList result = m_instrumentsMeta.groups;

    std::sort(result.begin(), result.end(), [](const InstrumentGroup& group1, const InstrumentGroup& group2) {
        return group1.sequenceOrder < group2.sequenceOrder;
    });

    return result;
}

QVariantMap InstrumentListModel::allInstrumentsItem() const
{
    QVariantMap obj;
    obj[ID_KEY] = ALL_INSTRUMENTS_GENRE_ID;
    obj[NAME_KEY] = qtrc("instruments", "All instruments");
    return obj;
}

void InstrumentListModel::updateFamilyStateBySearch()
{
    bool familySaved = !m_savedFamilyId.isEmpty();

    if (isSearching() && !familySaved) {
        m_savedFamilyId = m_selectedFamilyId;
        selectFamily(ALL_INSTRUMENTS_GENRE_ID);
        selectGroup(QString());
    } else if (!isSearching() && familySaved) {
        selectFamily(m_savedFamilyId);
        m_savedFamilyId.clear();
    }
}

bool InstrumentListModel::isInstrumentAccepted(const Instrument& instrument, bool compareWithSelectedGroup) const
{
    if (isSearching()) {
        return instrument.name.contains(m_searchText, Qt::CaseInsensitive);
    }

    if (instrument.groupId != m_selectedGroupId && compareWithSelectedGroup) {
        return false;
    }

    if (m_selectedFamilyId == ALL_INSTRUMENTS_GENRE_ID) {
        return true;
    }

    if (instrument.genreIds.contains(m_selectedFamilyId)) {
        return true;
    }

    return false;
}

InstrumentTemplate InstrumentListModel::instrumentTemplate(const QString& instrumentId) const
{
    for (const InstrumentTemplate& instrumentTemplate: m_instrumentsMeta.templates) {
        if (instrumentTemplate.instrument.id == instrumentId) {
            return instrumentTemplate;
        }
    }

    return InstrumentTemplate();
}

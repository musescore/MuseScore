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
#include "instrumentlistmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::instruments;
using namespace mu::notation;

static const QString ALL_INSTRUMENTS_GENRE_ID("ALL_INSTRUMENTS");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_ID("EMPTY_KEY");
static const QString INSTRUMENT_EMPTY_TRANSPOSITION_NAME("--");

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
                               const QString& selectedPartIds)
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

    initSelectedInstruments(selectedPartIds.split(','));

    if (!currentInstrumentId.isEmpty()) {
        InstrumentTemplate instrumentTemplate = this->instrumentTemplate(currentInstrumentId);
        selectGroup(instrumentTemplate.instrument.groupId);
    }
}

void InstrumentListModel::initSelectedInstruments(const IDList& selectedPartIds)
{
    auto _notationParts = notationParts();
    if (!_notationParts) {
        return;
    }

    QMap<QString, const Part*> partMap;
    for (const Part* part: _notationParts->partList()) {
        if (selectedPartIds.contains(part->id())) {
            partMap.insert(part->id(), part);
        }
    }

    for (const ID& partId: selectedPartIds) {
        if (!partMap.contains(partId)) {
            continue;
        }
        for (auto instrument: _notationParts->instrumentList(partId)) {
            if (partMap[partId]->isDoublingInstrument(instrument.id)) {
                continue;
            }
            InstrumentTemplate templ = instrumentTemplate(instrument.id);

            SelectedInstrumentInfo info;
            info.id = templ.id;
            info.partId = partId;
            info.partName = partMap[partId]->partName();
            info.config = templ.instrument;
            m_selectedInstruments << info;
        }
    }

    emit selectedInstrumentsChanged();
}

INotationPartsPtr InstrumentListModel::notationParts() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->parts();
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
    result << toMap(m_instrumentsMeta.genres[COMMON_GENRE_ID]);
    result << allInstrumentsItem();

    for (const InstrumentGenre& genre: m_instrumentsMeta.genres) {
        if (genre.id == COMMON_GENRE_ID) {
            continue;
        }

        result << toMap(genre);
    }

    return result;
}

QVariantList InstrumentListModel::groups() const
{
    QStringList availableGroups;

    for (const InstrumentTemplate& templ: m_instrumentsMeta.instrumentTemplates) {
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
    QVariantMap availableInstruments;

    for (const InstrumentTemplate& templ: m_instrumentsMeta.instrumentTemplates) {
        const Instrument& instrument = templ.instrument;

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
        instrumentObj[GROUP_ID] = instrument.groupId;
        availableInstruments.insert(instrumentId, instrumentObj);
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
    info.partId = QString();
    info.partName = QString();
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
        QString instrumentName = instrument.partName;

        if (instrumentName.isEmpty()) {
            instrumentName = instrument.config.name;
            Transposition _transposition = instrument.transposition;
            if (_transposition.isValid()) {
                instrumentName = instrumentName.replace(_transposition.name + " ", "")
                                 .replace(" in " + _transposition.name, "");

                instrumentName = QString("%1 (%2)").arg(instrumentName, _transposition.name);
            }
        }

        QVariantMap obj;
        obj[ID_KEY] = instrumentId;
        obj[NAME_KEY] = instrumentName;
        obj[CONFIG_KEY] = QVariant::fromValue(instrument.config);

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
    InstrumentGroupList result = m_instrumentsMeta.groups.values();

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

QVariantMap InstrumentListModel::defaultInstrumentTranspositionItem() const
{
    QVariantMap obj;
    obj[ID_KEY] = INSTRUMENT_EMPTY_TRANSPOSITION_ID;
    obj[NAME_KEY] = INSTRUMENT_EMPTY_TRANSPOSITION_NAME;

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
    for (const InstrumentTemplate& instrumentTemplate: m_instrumentsMeta.instrumentTemplates) {
        if (instrumentTemplate.instrument.id == instrumentId) {
            return instrumentTemplate;
        }
    }

    return InstrumentTemplate();
}

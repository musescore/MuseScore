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

using namespace mu::instrumentsscene;
using namespace mu::notation;

static const QString ALL_INSTRUMENTS_GENRE_ID("ALL_INSTRUMENTS");

static const QString ID_KEY("id");
static const QString NAME_KEY("name");
static const QString TRAITS_KEY("traits");
static const QString GROUP_ID("groupId");
static const QString CONFIG_KEY("config");
static const QString SOLOIST_KEY("isSoloist");
static const QString IS_EXISTING_PART_KEY("isExistingPart");

static mu::IDList parseIdList(const QString& str)
{
    mu::IDList result;

    for (const QString& idStr : str.split(',')) {
        result.push_back(mu::ID(idStr));
    }

    return result;
}

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentListModel::load(bool canSelectMultipleInstruments, const QString& currentInstrumentId,
                               const QString& currentScoreOrderId, const QString& selectedPartIds)
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

    initSelectedInstruments(parseIdList(selectedPartIds));

    if (!currentInstrumentId.isEmpty()) {
        InstrumentTemplate templ = instrumentTemplateById(currentInstrumentId);
        selectGroup(templ.groupId);
    }

    initScoreOrders(currentScoreOrderId);
}

void InstrumentListModel::initSelectedInstruments(const IDList& selectedPartIds)
{
    auto _notationParts = notationParts();
    if (!_notationParts) {
        return;
    }

    auto parts = _notationParts->partList();
    for (const ID& partId: selectedPartIds) {
        auto compareId = [partId](const Part* part) {
            return ID(part->id()) == partId;
        };

        auto pi = find_if(begin(parts), end(parts), compareId);
        if ((pi == end(parts)) || !(*pi)) {
            continue;
        }

        const Part* part = *pi;

        SelectedInstrumentInfo info;

        info.id = partId.toQString();
        info.isExistingPart = true;
        info.name = part->partName();
        info.isSoloist = part->soloist();
        info.familyCode = part->familyId();

        m_selectedInstruments << info;
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

void InstrumentListModel::InstrumentListModel::initScoreOrders(const QString& currentId)
{
    auto toList = [](const ScoreOrder& order) {
        ScoreOrderInfo info;
        info.id = order.id;
        info.customized = false;
        info.info = order;
        return info;
    };

    for (const ScoreOrder& order: m_instrumentsMeta.scoreOrders) {
        m_scoreOrders << toList(order);
    }

    emit scoreOrdersChanged();

    if (currentId.isEmpty()) {
        m_selectedScoreOrderIndex = 0;
    } else {
        m_selectedScoreOrderIndex = indexOfScoreOrderId(currentId);
        m_blockSortingInstruments = !matchesScoreOrder();
        if (m_blockSortingInstruments) {
            ScoreOrderInfo order = m_scoreOrders[m_selectedScoreOrderIndex];
            makeCustomizedScoreOrder(order);
        }
    }
    m_blockSortingInstruments = false;

    emit scoreOrdersChanged();
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

    for (const InstrumentTemplate* templ: m_instrumentsMeta.instrumentTemplates) {
        constexpr bool compareWithSelectedGroup = false;
        if (!isInstrumentAccepted(*templ, compareWithSelectedGroup)) {
            continue;
        }

        if (!availableGroups.contains(templ->groupId)) {
            availableGroups << templ->groupId;
        }
    }

    QVariantList result;

    for (const InstrumentGroup& group: m_instrumentsMeta.groups) {
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
    QHash<QString, QStringList> traits;
    QVariantMap availableInstruments;

    for (const InstrumentTemplate* templ: m_instrumentsMeta.instrumentTemplates) {
        const Trait& trait = templ->trait;
        const QString& instrumentName = templ->trackName;

        if (!isInstrumentAccepted(*templ)) {
            continue;
        }

        if (!trait.name.isEmpty()) {
            if (trait.isDefault) {
                traits[instrumentName].prepend(trait.name);
            } else {
                traits[instrumentName] << trait.name;
            }
        }

        QVariantMap instrumentObj;
        instrumentObj[ID_KEY] = templ->id;
        instrumentObj[NAME_KEY] = instrumentName;
        instrumentObj[GROUP_ID] = templ->groupId;

        if (traits.contains(instrumentName)) {
            instrumentObj[TRAITS_KEY] = traits[instrumentName];
        }

        availableInstruments[instrumentName] = instrumentObj;
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

void InstrumentListModel::selectInstrument(const QString& instrumentName, const QString& traitName)
{
    const InstrumentTemplate* suitedTemplate = nullptr;

    for (const InstrumentTemplate* templ : m_instrumentsMeta.instrumentTemplates) {
        if (templ->trackName == instrumentName && templ->trait.name == traitName) {
            suitedTemplate = templ;
            break;
        }
    }

    if (!suitedTemplate) {
        LOGE() << QString("Instrument %1 with trait %2 does not exist")
            .arg(instrumentName)
            .arg(traitName);
        return;
    }

    SelectedInstrumentInfo info;
    info.isExistingPart = false;
    info.isSoloist = false;
    info.id = suitedTemplate->id;
    info.name = formatInstrumentTitle(suitedTemplate->trackName, suitedTemplate->trait);
    info.familyCode = suitedTemplate->familyId();
    info.config = *suitedTemplate;

    if (!m_canSelectMultipleInstruments) {
        m_selectedInstruments.clear();
        m_selectedInstruments << info;
    } else if (!m_scoreOrders[m_selectedScoreOrderIndex].info.isValid()) {
        m_selectedInstruments << info;
    } else {
        m_selectedInstruments.insert(instrumentInsertIndex(info), info);
    }

    emit selectedInstrumentsChanged();
}

void InstrumentListModel::unselectInstrument(int instrumentIndex)
{
    if (!isInstrumentIndexValid(instrumentIndex)) {
        return;
    }

    m_selectedInstruments.removeAt(instrumentIndex);
    emit selectedInstrumentsChanged();
}

void InstrumentListModel::swapSelectedInstruments(int firstInstrumentIndex, int secondInstrumentIndex)
{
    m_selectedInstruments.swapItemsAt(firstInstrumentIndex, secondInstrumentIndex);
    emit selectedInstrumentsChanged();
    checkScoreOrderMatching(true);
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

QVariantList InstrumentListModel::scoreOrders() const
{
    auto toMap = [](const ScoreOrderInfo& order) {
        QString name = qtrc("instruments", "Order: ") + order.info.name;
        if (order.customized) {
            name += qtrc("instruments", " (Customized)");
        }
        QVariantMap obj;
        obj[ID_KEY] = order.id;
        obj[NAME_KEY] = name;
        obj[CONFIG_KEY] = QVariant::fromValue(order.info);
        return obj;
    };

    QVariantList result;
    for (const ScoreOrderInfo& order: m_scoreOrders) {
        result << toMap(order);
    }

    return result;
}

QVariant InstrumentListModel::selectedScoreOrderIndex() const
{
    return QVariant(m_selectedScoreOrderIndex);
}

void InstrumentListModel::setSelectedScoreOrderIndex(const QVariant& index)
{
    m_selectedScoreOrderIndex = index.toInt();

    emit scoreOrdersChanged();
}

void InstrumentListModel::toggleSoloist(int instrumentIndex)
{
    if (!isInstrumentIndexValid(instrumentIndex)) {
        return;
    }

    SelectedInstrumentInfo sio = m_selectedInstruments.takeAt(instrumentIndex);
    sio.isSoloist = !sio.isSoloist;
    m_selectedInstruments.insert(instrumentInsertIndex(sio), sio);
    emit selectedInstrumentsChanged();
    checkScoreOrderMatching(false);
}

void InstrumentListModel::selectScoreOrder(const QString& orderId)
{
    int index = indexOfScoreOrderId(orderId);

    if (m_selectedScoreOrderIndex == index) {
        return;
    }

    if (!m_blockSortingInstruments) {
        m_selectedScoreOrderIndex = index;
        if (!m_scoreOrders[m_selectedScoreOrderIndex].customized) {
            sortSelectedInstruments();
        }
    }
    emit selectedInstrumentsChanged();
}

int InstrumentListModel::indexOfScoreOrderId(const QString& id) const
{
    int index = 0;
    int custom = 0;
    for (const auto& order: m_scoreOrders) {
        if (order.id == id) {
            return index;
        }
        if (!order.info.groups.size()) {
            custom = index;
        }
        ++index;
    }
    return custom;
}

void InstrumentListModel::sortSelectedInstruments()
{
    std::sort(m_selectedInstruments.begin(), m_selectedInstruments.end(),
              [this](const SelectedInstrumentInfo& info1, const SelectedInstrumentInfo& info2) {
        int index1 = sortInstrumentsIndex(info1);
        int index2 = sortInstrumentsIndex(info2);
        return index1 < index2;
    });
}

int InstrumentListModel::instrumentInsertIndex(const SelectedInstrumentInfo& info) const
{
    int order = sortInstrumentsIndex(info);
    int index = 0;
    while (index < m_selectedInstruments.size()) {
        if (sortInstrumentsIndex(m_selectedInstruments[index]) > order) {
            break;
        }
        ++index;
    }
    return index;
}

int InstrumentListModel::sortInstrumentsIndex(const SelectedInstrumentInfo& info) const
{
    static const QString SoloistsGroup("<soloists>");
    static const QString UnsortedGroup("<unsorted>");

    enum class Priority {
        Undefined,
        Unsorted,
        UnsortedGroup,
        Family,
        Soloist
    };

    auto calculateIndex = [this, info](int index) {
        return index * m_instrumentsMeta.instrumentTemplates.size() + info.config.sequenceOrder;
    };

    ScoreOrder order = m_scoreOrders[m_selectedScoreOrderIndex].info;
    QString family = order.instrumentMap.contains(info.id) ? order.instrumentMap[info.id].id : info.familyCode;

    int index = order.groups.size();
    Priority priority = Priority::Undefined;

    for (int i = 0; i < order.groups.size(); ++i) {
        const ScoreOrderGroup& sg = order.groups[i];
        if ((sg.family == SoloistsGroup) && info.isSoloist) {
            return calculateIndex(i);
        } else if ((priority < Priority::Family) && (sg.family == family)) {
            index = i;
            priority = Priority::Family;
        } else if ((priority < Priority::UnsortedGroup) && (sg.family == UnsortedGroup) && (sg.unsorted == info.config.groupId)) {
            index = i;
            priority = Priority::UnsortedGroup;
        } else if ((priority < Priority::Unsorted) && (sg.family == UnsortedGroup)) {
            index = i;
            priority = Priority::Unsorted;
        }
    }

    return calculateIndex(index);
}

bool InstrumentListModel::matchesScoreOrder() const
{
    ScoreOrderInfo order = m_scoreOrders[m_selectedScoreOrderIndex];
    if (!order.info.isValid()) {
        return true;
    }

    int prvIndex = -1;
    for (const SelectedInstrumentInfo& info: m_selectedInstruments) {
        int index = sortInstrumentsIndex(info);
        if (prvIndex > index) {
            order.customized = true;
            return false;
        }
        prvIndex = index;
    }

    return true;
}

QVariantList InstrumentListModel::selectedInstruments() const
{
    QVariantList result;

    for (const SelectedInstrumentInfo& instrument: m_selectedInstruments) {
        QVariantMap obj;
        obj[ID_KEY] = instrument.id;
        obj[NAME_KEY] = instrument.name;
        obj[IS_EXISTING_PART_KEY] = instrument.isExistingPart;
        obj[SOLOIST_KEY] = instrument.isSoloist;
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

bool InstrumentListModel::isInstrumentAccepted(const InstrumentTemplate& instrument, bool compareWithSelectedGroup) const
{
    if (isSearching()) {
        return instrument.trackName.contains(m_searchText, Qt::CaseInsensitive);
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

InstrumentTemplate InstrumentListModel::instrumentTemplateById(const QString& instrumentTemplateId) const
{
    for (const InstrumentTemplate* templ: m_instrumentsMeta.instrumentTemplates) {
        if (templ->id == instrumentTemplateId) {
            return *templ;
        }
    }

    return InstrumentTemplate();
}

void InstrumentListModel::checkScoreOrderMatching(bool block)
{
    bool matches = matchesScoreOrder();

    ScoreOrderInfo order = m_scoreOrders[m_selectedScoreOrderIndex];
    if (order.customized != matches) {
        return;
    }

    if (matches) {
        m_scoreOrders.removeAt(m_selectedScoreOrderIndex);
    } else {
        makeCustomizedScoreOrder(order);
    }

    m_blockSortingInstruments = block;
    emit scoreOrdersChanged();
    m_blockSortingInstruments = false;
}

void InstrumentListModel::makeCustomizedScoreOrder(const ScoreOrderInfo& order)
{
    ScoreOrderInfo customizedOrder = ScoreOrderInfo(order);
    customizedOrder.customized = true;
    customizedOrder.id += QString("_customized");

    m_selectedScoreOrderIndex = indexOfScoreOrderId(order.id);
    m_scoreOrders.insert(m_selectedScoreOrderIndex, customizedOrder);
}

bool InstrumentListModel::isInstrumentIndexValid(int index) const
{
    return index >= 0 && index < m_selectedInstruments.size();
}

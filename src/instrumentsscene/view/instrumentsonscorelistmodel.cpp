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

#include "instrumentsonscorelistmodel.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::uicomponents;
using namespace mu::async;
using namespace mu::notation;

static const QString PART_ID_KEY("partId");
static const QString IS_SOLOIST_KEY("isSoloist");
static const QString IS_EXISTING_PART_KEY("isExistingPart");
static const QString INSTRUMENT_TEMPLATE_KEY("instrumentTemplate");

namespace mu::instrumentsscene {
class InstrumentsOnScoreListModel::InstrumentItem : public SelectableItemListModel::Item
{
public:
    InstrumentItem(QObject* parent)
        : Item(parent)
    {
    }

    QString id;
    ID partId;
    QString name;
    QString familyId;
    bool isSoloist = false;
    bool isExistingPart = false;
    notation::InstrumentTemplate instrumentTemplate;
};
}

InstrumentsOnScoreListModel::InstrumentsOnScoreListModel(QObject* parent)
    : SelectableItemListModel(parent)
{
}

QVariant InstrumentsOnScoreListModel::data(const QModelIndex& index, int role) const
{
    const InstrumentItem* instrument = modelIndexToItem(index);
    if (!instrument) {
        return QVariant();
    }

    switch (role) {
    case RoleName:
        return instrument->name;
    case RoleIsSoloist:
        return instrument->isSoloist;
    default:
        break;
    }

    return SelectableItemListModel::data(index, role);
}

bool InstrumentsOnScoreListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    InstrumentItem* instrument = modelIndexToItem(index);
    if (!instrument) {
        return false;
    }

    switch (role) {
    case RoleIsSoloist:
        instrument->isSoloist = value.toBool();
        {
            ItemList items = this->items();
            items.removeAll(instrument);
            insertInstrument(items, instrument);
            setItems(items);
        }
        verifyScoreOrder();
        return true;
    case RoleName:
        break;
    default:
        return SelectableItemListModel::setData(index, role);
    }

    return false;
}

QHash<int, QByteArray> InstrumentsOnScoreListModel::roleNames() const
{
    QHash<int, QByteArray> roles = SelectableItemListModel::roleNames();
    roles[RoleName] = "name";
    roles[RoleIsSoloist] = "isSoloist";

    return roles;
}

QStringList InstrumentsOnScoreListModel::orders() const
{
    QStringList result;

    for (const ScoreOrder& order: m_scoreOrders) {
        result << order.getName();
    }

    return result;
}

int InstrumentsOnScoreListModel::currentOrderIndex() const
{
    return m_currentOrderIndex;
}

void InstrumentsOnScoreListModel::load()
{
    TRACEFUNC;

    IMasterNotationPtr master = context()->currentMasterNotation();
    auto parts = master ? master->parts()->partList() : NotifyList<const Part*>();

    QList<Item*> instruments;

    for (const Part* part : parts) {
        InstrumentItem* instrument = new InstrumentItem(this);
        instrument->id = part->instrument()->id();
        instrument->partId = part->id();
        instrument->isExistingPart = true;
        instrument->name = part->partName();
        instrument->isSoloist = part->soloist();
        instrument->familyId = part->familyId();
        instrument->instrumentTemplate.sequenceOrder = resolveInstrumentSequenceNumber(instrument->id);

        instruments << instrument;
    }

    setItems(instruments);

    loadOrders();
}

void InstrumentsOnScoreListModel::loadOrders()
{
    TRACEFUNC;

    m_scoreOrders = repository()->orders();
    if (m_scoreOrders.isEmpty() || !m_scoreOrders.contains(customOrder())) {
        m_scoreOrders.append(customOrder());
    }

    INotationPtr notation = context()->currentNotation();
    ScoreOrder currentOrder = notation ? notation->parts()->scoreOrder() : m_scoreOrders[0];
    bool orderCustomized = currentOrder.customized;

    if (!m_scoreOrders.contains(currentOrder)) {
        currentOrder.customized = false;
        m_scoreOrders.append(currentOrder);
    }

    emit ordersChanged();

    for (int i = 0; i < m_scoreOrders.size(); ++i) {
        if (m_scoreOrders[i].id == currentOrder.id) {
            doSetCurrentOrderIndex(i);
            break;
        }
    }

    if (orderCustomized) {
        doSetCurrentOrderIndex(createCustomizedScoreOrder(currentOrder));
    }
}

int InstrumentsOnScoreListModel::resolveInstrumentSequenceNumber(const QString& instrumentId) const
{
    const InstrumentTemplateList& templates = repository()->instrumentTemplates();
    for (const InstrumentTemplate* templ : templates) {
        if (templ->id == instrumentId) {
            return templ->sequenceOrder;
        }
    }
    return templates.size();
}

void InstrumentsOnScoreListModel::addInstruments(const QVariantList& instruments)
{
    TRACEFUNC;

    ItemList items = this->items();

    for (const QVariant& obj : instruments) {
        if (obj.isNull()) {
            continue;
        }

        InstrumentTemplate templ = obj.toMap()[INSTRUMENT_TEMPLATE_KEY].value<InstrumentTemplate>();
        InstrumentItem* instrument = new InstrumentItem(this);
        instrument->id = templ.id;
        instrument->name = formatInstrumentTitle(templ.trackName, templ.trait);
        instrument->familyId = templ.familyId();
        instrument->instrumentTemplate = templ;

        insertInstrument(items, instrument);
    }

    setItems(items);
    verifyScoreOrder();
}

QVariant InstrumentsOnScoreListModel::currentOrder() const
{
    return QVariant::fromValue(currentScoreOrder());
}

QVariantList InstrumentsOnScoreListModel::instruments() const
{
    TRACEFUNC;

    QVariantList result;

    for (const Item* item : items()) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (!instrument) {
            continue;
        }

        QVariantMap obj;
        obj[PART_ID_KEY] = instrument->partId.toQString();
        obj[IS_SOLOIST_KEY] = instrument->isSoloist;
        obj[IS_EXISTING_PART_KEY] = instrument->isExistingPart;
        obj[INSTRUMENT_TEMPLATE_KEY] = QVariant::fromValue(instrument->instrumentTemplate);

        result << obj;
    }

    return result;
}

void InstrumentsOnScoreListModel::setCurrentOrderIndex(int index)
{
    if (m_currentOrderIndex == index) {
        return;
    }

    doSetCurrentOrderIndex(index);
    updateInstrumentsOrder();
}

void InstrumentsOnScoreListModel::doSetCurrentOrderIndex(int index)
{
    m_currentOrderIndex = index;

    emit currentOrderChanged();
}

void InstrumentsOnScoreListModel::updateInstrumentsOrder()
{
    ItemList items = this->items();
    sortInstruments(items);
    setItems(items);
}

void InstrumentsOnScoreListModel::sortInstruments(ItemList& instruments)
{
    TRACEFUNC;

    const ScoreOrder& currentOrder = currentScoreOrder();

    std::sort(instruments.begin(), instruments.end(), [currentOrder](const Item* item1, const Item* item2) {
        auto instrument1 = dynamic_cast<const InstrumentItem*>(item1);
        auto instrument2 = dynamic_cast<const InstrumentItem*>(item2);

        int index1 = currentOrder.instrumentSortingIndex(instrument1->id, instrument1->isSoloist);
        int index2 = currentOrder.instrumentSortingIndex(instrument2->id, instrument2->isSoloist);

        return index1 < index2;
    });
}

void InstrumentsOnScoreListModel::insertInstrument(ItemList& instruments, InstrumentItem* newInstrument)
{
    const ScoreOrder& currentOrder = currentScoreOrder();
    const int newIndex = currentOrder.instrumentSortingIndex(newInstrument->id, newInstrument->isSoloist);

    for (int index = 0; index < instruments.size(); ++index) {
        auto instrument = dynamic_cast<const InstrumentItem*>(instruments[index]);

        if (currentOrder.instrumentSortingIndex(instrument->id, instrument->isSoloist) > newIndex) {
            instruments.insert(index, newInstrument);
            return;
        }
    }
    instruments << newInstrument;
}

InstrumentsOnScoreListModel::InstrumentItem* InstrumentsOnScoreListModel::modelIndexToItem(const QModelIndex& index) const
{
    return dynamic_cast<InstrumentItem*>(item(index));
}

const ScoreOrder& InstrumentsOnScoreListModel::currentScoreOrder() const
{
    if (m_currentOrderIndex < 0 || m_currentOrderIndex >= m_scoreOrders.size()) {
        static ScoreOrder dummy;
        return dummy;
    }

    return m_scoreOrders[m_currentOrderIndex];
}

void InstrumentsOnScoreListModel::onRowsMoved()
{
    verifyScoreOrder();
}

void InstrumentsOnScoreListModel::onRowsRemoved()
{
    verifyScoreOrder();
}

bool InstrumentsOnScoreListModel::matchesScoreOrder() const
{
    const ScoreOrder currentOrder = currentScoreOrder();

    QList<int> instrumentIndices;
    for (const Item* item : items()) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (!instrument) {
            continue;
        }
        instrumentIndices << currentOrder.instrumentSortingIndex(instrument->id, instrument->isSoloist);
    }

    return currentOrder.isScoreOrder(instrumentIndices);
}

void InstrumentsOnScoreListModel::verifyScoreOrder()
{
    const ScoreOrder currentOrder = currentScoreOrder();
    bool matchingScoreOrder = matchesScoreOrder();

    if (matchingScoreOrder == currentOrder.customized) {
        if (matchingScoreOrder) {
            removeCustomizedScoreOrder(currentOrder);
        } else {
            doSetCurrentOrderIndex(createCustomizedScoreOrder(currentOrder));
        }
    }
}

int InstrumentsOnScoreListModel::createCustomizedScoreOrder(const ScoreOrder& order)
{
    ScoreOrder customizedOrder = order.clone();
    customizedOrder.customized = true;
    int customizedIndex = m_currentOrderIndex + 1;
    m_scoreOrders.insert(customizedIndex, customizedOrder);

    emit ordersChanged();

    return customizedIndex;
}

void InstrumentsOnScoreListModel::removeCustomizedScoreOrder(const ScoreOrder& order)
{
    if (!order.customized) {
        return;
    }

    for (int i = 0; i < m_scoreOrders.size(); ++i) {
        if (!m_scoreOrders[i].customized && (m_scoreOrders[i].id == order.id)) {
            doSetCurrentOrderIndex(i);
            m_scoreOrders.removeAt(i + 1);

            emit ordersChanged();

            break;
        }
    }
}

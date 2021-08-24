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
        updateInstrumentsOrder();
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

    for (const ScoreOrder* order: m_scoreOrders) {
        result << order->name;
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
        instrument->id = part->id();
        instrument->partId = part->id();
        instrument->isExistingPart = true;
        instrument->name = part->partName();
        instrument->isSoloist = part->soloist();
        instrument->familyId = part->familyId();

        instruments << instrument;
    }

    setItems(instruments);
    loadInstrumentsMeta();
}

void InstrumentsOnScoreListModel::loadInstrumentsMeta()
{
    TRACEFUNC;

    RetValCh<InstrumentsMeta> meta = repository()->instrumentsMeta();
    if (!meta.ret) {
        LOGE() << meta.ret.toString();
    }

    m_scoreOrders = meta.val.scoreOrders;
    emit ordersChanged();

    m_instrumentTemplates = meta.val.instrumentTemplates;

    INotationPtr notation = context()->currentNotation();
    QString currentOrderId = notation ? notation->scoreOrder().id : QString();

    for (int i = 0; i < m_scoreOrders.size(); ++i) {
        if (m_scoreOrders[i]->id == currentOrderId) {
            doSetCurrentOrderIndex(i);
            break;
        }
    }
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

        items << instrument;
    }

    sortInstruments(items);
    setItems(items);
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

    std::sort(instruments.begin(), instruments.end(), [this, currentOrder](const Item* item1, const Item* item2) {
        auto instrument1 = dynamic_cast<const InstrumentItem*>(item1);
        auto instrument2 = dynamic_cast<const InstrumentItem*>(item2);

        int index1 = sortInstrumentsIndex(currentOrder, *instrument1);
        int index2 = sortInstrumentsIndex(currentOrder, *instrument2);

        return index1 < index2;
    });
}

int InstrumentsOnScoreListModel::sortInstrumentsIndex(const ScoreOrder& order, const InstrumentItem& instrument) const
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

    auto calculateIndex = [this, &instrument](int index) {
        return index * m_instrumentTemplates.size() + instrument.instrumentTemplate.sequenceOrder;
    };

    QString family = order.instrumentMap.contains(instrument.id) ? order.instrumentMap[instrument.id].id : instrument.familyId;
    int index = order.groups.size();
    Priority priority = Priority::Undefined;

    for (int i = 0; i < order.groups.size(); ++i) {
        const ScoreOrderGroup& sg = order.groups[i];

        if ((sg.family == SoloistsGroup) && instrument.isSoloist) {
            return calculateIndex(i);
        } else if ((priority < Priority::Family) && (sg.family == family)) {
            index = i;
            priority = Priority::Family;
        } else if ((priority < Priority::UnsortedGroup) && (sg.family == UnsortedGroup)
                   && (sg.unsorted == instrument.instrumentTemplate.groupId)) {
            index = i;
            priority = Priority::UnsortedGroup;
        } else if ((priority < Priority::Unsorted) && (sg.family == UnsortedGroup)) {
            index = i;
            priority = Priority::Unsorted;
        }
    }

    return calculateIndex(index);
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

    return *m_scoreOrders[m_currentOrderIndex];
}

void InstrumentsOnScoreListModel::onRowsMoved()
{
    static ScoreOrder customOrder = makeCustomOrder();
    bool containsCustomOrder = false;

    for (const ScoreOrder* order : m_scoreOrders) {
        if (order->id == customOrder.id) {
            containsCustomOrder = true;
            break;
        }
    }

    if (!containsCustomOrder) {
        m_scoreOrders << &customOrder;
        emit ordersChanged();
    }

    int customOrderIndex = m_scoreOrders.size() - 1;

    if (m_currentOrderIndex != customOrderIndex) {
        doSetCurrentOrderIndex(customOrderIndex);
    }
}

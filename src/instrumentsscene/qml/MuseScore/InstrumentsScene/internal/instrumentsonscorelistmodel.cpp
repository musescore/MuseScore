/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "async/notifylist.h"

#include "log.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/stafftype.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationparts.h"
#include "notation/inotationstyle.h"

#include "layoutpanelitemtype.h"
#include "sharedparttreeitem.h"

using namespace muse;
using namespace muse::uicomponents;
using namespace muse::async;
using namespace mu::instrumentsscene;
using namespace mu::notation;

namespace mu::instrumentsscene {
class InstrumentsOnScoreListModel::InstrumentItem : public QObject
{
public:
    InstrumentItem(QObject* parent)
        : QObject(parent)
    {
    }

    QString id;
    ID partId;
    QString name;
    QString familyId;
    bool isSoloist = false;
    bool isExistingPart = false;
    bool isSharedGroup = false; // the header of a shared-staves group
    ID sharedGroupId; // the group this is the header of, or an instrument in
    TablatureOption tablatureOption = TablatureOption::None; // for new instruments that have a tablature version
    notation::InstrumentTemplate instrumentTemplate;
};
}

InstrumentsOnScoreListModel::InstrumentsOnScoreListModel(QObject* parent)
    : SelectableItemListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
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
    case RoleDescription:
        return instrument->instrumentTemplate.description.toQString();
    case RoleIsSoloist:
        return instrument->isSoloist;
    case RoleSharedGroupRole:
        return sharedGroupRole(index.row());
    case RoleSharedGroupId:
        return instrument->sharedGroupId.isValid() ? instrument->sharedGroupId.toQString() : QString();
    case RoleIsNew:
        return !instrument->isExistingPart;
    case RoleTablatureOption:
        return static_cast<int>(instrument->tablatureOption);
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
        if (instrument->isSharedGroup) {
            return false;
        }

        instrument->isSoloist = value.toBool();

        // Instruments in a group stay where they are
        if (instrument->sharedGroupId.isValid()) {
            emit dataChanged(index, index, { RoleIsSoloist });
        } else {
            ItemList items = this->items();
            items.removeAll(instrument);
            insertInstrument(items, instrument);
            setItems(items);
        }
        verifyScoreOrder();
        return true;
    case RoleTablatureOption:
        if (instrument->tablatureOption == TablatureOption::None || value.toInt() < 0) {
            return false;
        }

        instrument->tablatureOption = static_cast<TablatureOption>(value.toInt());
        emit dataChanged(index, index, { RoleTablatureOption });
        return true;
    case RoleName:
    case RoleDescription:
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
    roles[RoleDescription] = "description";
    roles[RoleIsSoloist] = "isSoloist";
    roles[RoleSharedGroupRole] = "sharedGroupRole";
    roles[RoleSharedGroupId] = "sharedGroupId";
    roles[RoleIsNew] = "isNew";
    roles[RoleTablatureOption] = "tablatureOption";

    return roles;
}

QStringList InstrumentsOnScoreListModel::orders() const
{
    QStringList result;

    for (const ScoreOrder& order: m_scoreOrders) {
        result << order.getName().qTranslated();
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

    IMasterNotationPtr master = context()->masterNotation();
    auto parts = master ? master->parts()->partList() : NotifyList<const Part*>();

    INotationPtr notation = master ? master->notation() : nullptr;
    const bool showsSharedGroups = notation && notation->style()->styleValue(StyleId::enableStaveSharing).toBool();

    ItemList instruments;

    auto appendInstrument = [this, &instruments](const Part* part, const ID& sharedGroupId) {
        InstrumentItem* instrument = new InstrumentItem(this);
        instrument->id = part->instrument()->id();
        instrument->partId = part->id();
        instrument->isExistingPart = true;
        instrument->name = part->partName();
        instrument->isSoloist = part->soloist();
        instrument->familyId = part->familyId();
        instrument->instrumentTemplate.sequenceOrder = resolveInstrumentSequenceNumber(instrument->id);
        instrument->sharedGroupId = sharedGroupId;

        instruments << instrument;
    };

    for (const Part* part : parts) {
        // Shared staves are only shown (as groups) while the score combines instruments onto them; the group's
        // instruments are listed right after it
        if (part->isSharedPart()) {
            if (!showsSharedGroups) {
                continue;
            }

            InstrumentItem* group = new InstrumentItem(this);
            group->id = part->instrument()->id();
            group->partId = part->id();
            group->isExistingPart = true;
            group->isSharedGroup = true;
            group->sharedGroupId = part->id();
            group->name = SharedPartTreeItem::groupTitle(mu::engraving::toSharedPart(part));

            instruments << group;

            for (const Part* member : parts) {
                if (member->sharedPart() == part) {
                    appendInstrument(member, part->id());
                }
            }
            continue;
        }

        if (showsSharedGroups && part->sharedPart()) {
            continue;
        }

        appendInstrument(part, ID());
    }

    setItems(instruments);

    loadOrders();
}

void InstrumentsOnScoreListModel::loadOrders()
{
    TRACEFUNC;

    m_scoreOrders = repository()->orders();

    const ScoreOrder& custom = customOrder();
    if (m_scoreOrders.empty() || !muse::contains(m_scoreOrders, custom)) {
        m_scoreOrders.push_back(custom);
    }

    INotationPtr notation = context()->currentNotation();
    ScoreOrder currentOrder = notation ? notation->parts()->scoreOrder() : m_scoreOrders[0];
    bool orderCustomized = currentOrder.customized;

    if (!muse::contains(m_scoreOrders, currentOrder)) {
        currentOrder.customized = false;
        m_scoreOrders.push_back(currentOrder);
    }

    emit ordersChanged();

    for (size_t i = 0; i < m_scoreOrders.size(); ++i) {
        if (m_scoreOrders[i].id == currentOrder.id) {
            doSetCurrentOrderIndex(static_cast<int>(i));
            break;
        }
    }

    if (orderCustomized) {
        doSetCurrentOrderIndex(createCustomizedScoreOrder(currentOrder));
    }
}

int InstrumentsOnScoreListModel::resolveInstrumentSequenceNumber(const String& instrumentId) const
{
    const InstrumentTemplate& templ = repository()->instrumentTemplate(instrumentId);
    if (templ.isValid()) {
        return templ.sequenceOrder;
    }

    const InstrumentTemplateList& allTemplates = repository()->instrumentTemplates();
    return static_cast<int>(allTemplates.size());
}

int InstrumentsOnScoreListModel::addInstruments(const QStringList& instrumentIdList)
{
    TRACEFUNC;

    ItemList items = this->items();
    ItemList addedItems;

    for (const QString& id : instrumentIdList) {
        const InstrumentTemplate& templ = repository()->instrumentTemplate(id);
        if (!templ.isValid()) {
            continue;
        }

        InstrumentItem* instrument = new InstrumentItem(this);

        instrument->id = templ.id;
        instrument->name = formatInstrumentTitle(templ.trackName, templ.trait);
        instrument->familyId = templ.familyId();
        instrument->instrumentTemplate = templ;

        // Instruments with a tablature version get a notation and a linked tablature staff, of which notation is shown
        if (mu::engraving::searchTablatureTemplate(templ.id)) {
            instrument->tablatureOption = TablatureOption::Notation;
        }

        insertInstrument(items, instrument);
        addedItems << instrument;
    }

    setItems(items);
    verifyScoreOrder();

    int firstAddedRow = -1;
    for (QObject* item : addedItems) {
        const int row = static_cast<int>(items.indexOf(item));
        firstAddedRow = firstAddedRow < 0 ? row : std::min(firstAddedRow, row);
    }

    return firstAddedRow;
}

QVariant InstrumentsOnScoreListModel::currentOrder() const
{
    const ScoreOrder& order = currentScoreOrder();

    QVariantMap map;
    map["id"] = order.id.toQString();
    map["customized"] = order.customized;

    return map;
}

QVariantList InstrumentsOnScoreListModel::instruments() const
{
    TRACEFUNC;

    QVariantList result;

    for (const QObject* item : items()) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (!instrument) {
            continue;
        }

        QVariantMap obj;
        obj["partId"] = instrument->partId.toQString();
        obj["isSoloist"] = instrument->isSoloist;
        obj["isExistingPart"] = instrument->isExistingPart;
        obj["instrumentId"] = instrument->isExistingPart ? instrument->id : instrument->instrumentTemplate.id.toQString();
        obj["tablatureOption"] = static_cast<int>(instrument->tablatureOption);

        result << obj;
    }

    return result;
}

QList<InstrumentsOnScoreListModel::InstrumentItem*> InstrumentsOnScoreListModel::selectedInstruments() const
{
    QList<int> rows = selection()->selectedRows();
    std::sort(rows.begin(), rows.end());

    // Groups themselves can't be soloists (their instruments can)
    QList<InstrumentItem*> result;
    for (int row : rows) {
        InstrumentItem* instrument = row < items().size() ? dynamic_cast<InstrumentItem*>(items()[row]) : nullptr;
        if (instrument && !instrument->isSharedGroup) {
            result << instrument;
        }
    }

    return result;
}

bool InstrumentsOnScoreListModel::isSoloistAvailable() const
{
    return !selectedInstruments().isEmpty();
}

bool InstrumentsOnScoreListModel::isSelectionSoloist() const
{
    const QList<InstrumentItem*> instruments = selectedInstruments();
    if (instruments.isEmpty()) {
        return false;
    }

    for (const InstrumentItem* instrument : instruments) {
        if (!instrument->isSoloist) {
            return false;
        }
    }

    return true;
}

void InstrumentsOnScoreListModel::toggleSoloistForSelection()
{
    const QList<InstrumentItem*> instruments = selectedInstruments();
    if (instruments.isEmpty()) {
        return;
    }

    ItemList selectedItems;
    for (int row : selection()->selectedRows()) {
        selectedItems << items()[row];
    }

    // In a mixed selection, all of them become soloists
    const bool makeSoloists = !isSelectionSoloist();

    ItemList items = this->items();
    for (InstrumentItem* instrument : instruments) {
        instrument->isSoloist = makeSoloists;

        // Soloists are sorted separately, but instruments in a group stay where they are
        if (!instrument->sharedGroupId.isValid()) {
            items.removeAll(instrument);
            insertInstrument(items, instrument);
        }
    }

    setItems(items);

    for (QObject* item : selectedItems) {
        selection()->QItemSelectionModel::select(index(static_cast<int>(items.indexOf(item))), QItemSelectionModel::Select);
    }

    verifyScoreOrder();

    emit soloistStateChanged();
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

    // Groups are sorted as a whole, by their first instrument
    QList<ItemList> blocks = this->blocks(instruments);

    std::stable_sort(blocks.begin(), blocks.end(), [this](const ItemList& block1, const ItemList& block2) {
        return sortingIndex(block1) < sortingIndex(block2);
    });

    instruments.clear();
    for (const ItemList& block : blocks) {
        instruments << block;
    }
}

void InstrumentsOnScoreListModel::insertInstrument(ItemList& instruments, InstrumentItem* newInstrument)
{
    const ScoreOrder& currentOrder = currentScoreOrder();
    const int newIndex = currentOrder.instrumentSortingIndex(newInstrument->id, newInstrument->isSoloist);

    // Never in among a group's instruments
    int row = 0;
    for (const ItemList& block : blocks(instruments)) {
        if (sortingIndex(block) > newIndex) {
            instruments.insert(row, newInstrument);
            return;
        }
        row += block.size();
    }
    instruments << newInstrument;
}

int InstrumentsOnScoreListModel::sharedGroupRole(int row) const
{
    const InstrumentItem* instrument = row >= 0 && row < items().size() ? dynamic_cast<const InstrumentItem*>(items()[row]) : nullptr;
    if (!instrument || !instrument->sharedGroupId.isValid()) {
        return LayoutPanelItemType::NOT_IN_GROUP;
    }

    if (instrument->isSharedGroup) {
        return LayoutPanelItemType::GROUP_HEADER;
    }

    const InstrumentItem* next = row + 1 < items().size() ? dynamic_cast<const InstrumentItem*>(items()[row + 1]) : nullptr;
    const bool nextIsInGroup = next && !next->isSharedGroup && next->sharedGroupId == instrument->sharedGroupId;

    return nextIsInGroup ? LayoutPanelItemType::GROUP_MEMBER : LayoutPanelItemType::GROUP_LAST_MEMBER;
}

int InstrumentsOnScoreListModel::lastRowOfBlock(int row) const
{
    if (sharedGroupRole(row) != LayoutPanelItemType::GROUP_HEADER) {
        return row;
    }

    while (sharedGroupRole(row + 1) == LayoutPanelItemType::GROUP_MEMBER
           || sharedGroupRole(row + 1) == LayoutPanelItemType::GROUP_LAST_MEMBER) {
        ++row;
        if (sharedGroupRole(row) == LayoutPanelItemType::GROUP_LAST_MEMBER) {
            break;
        }
    }

    return row;
}

int InstrumentsOnScoreListModel::firstRowOfBlock(int row) const
{
    while (row > 0 && (sharedGroupRole(row) == LayoutPanelItemType::GROUP_MEMBER
                       || sharedGroupRole(row) == LayoutPanelItemType::GROUP_LAST_MEMBER)) {
        --row;
    }

    return row;
}

QList<SelectableItemListModel::ItemList> InstrumentsOnScoreListModel::blocks(const ItemList& instruments) const
{
    QList<ItemList> result;

    for (QObject* item : instruments) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        auto previous = result.isEmpty() ? nullptr : dynamic_cast<const InstrumentItem*>(result.last().last());

        const bool continuesGroup = instrument && previous && !instrument->isSharedGroup
                                    && instrument->sharedGroupId.isValid() && instrument->sharedGroupId == previous->sharedGroupId;
        if (continuesGroup) {
            result.last() << item;
        } else {
            result << ItemList { item };
        }
    }

    return result;
}

int InstrumentsOnScoreListModel::sortingIndex(const ItemList& block) const
{
    const ScoreOrder& currentOrder = currentScoreOrder();

    for (const QObject* item : block) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (instrument && !instrument->isSharedGroup) {
            return currentOrder.instrumentSortingIndex(instrument->id, instrument->isSoloist);
        }
    }

    return 0;
}

bool InstrumentsOnScoreListModel::moveRows(const QModelIndex&, int sourceRow, int, const QModelIndex&, int destinationChild)
{
    TRACEFUNC;

    QList<int> selectedRows = selection()->selectedRows();
    if (selectedRows.isEmpty()) {
        return false;
    }

    std::sort(selectedRows.begin(), selectedRows.end());

    // Selected groups move with their instruments
    ItemList moving;
    for (int row : selectedRows) {
        for (int r = row; r <= lastRowOfBlock(row); ++r) {
            if (!moving.contains(items()[r])) {
                moving << items()[r];
            }
        }
    }

    const ItemList selectedItems = [this, &selectedRows]() {
        ItemList result;
        for (int row : selectedRows) {
            result << items()[row];
        }
        return result;
    }();

    ItemList items = this->items();
    const bool moveUp = destinationChild < sourceRow;

    // Past the whole neighbouring group (or instrument), never in among a group's instruments
    QObject* neighbour = nullptr;
    if (moveUp) {
        const int row = items.indexOf(moving.first()) - 1;
        if (row < 0) {
            return false;
        }
        neighbour = items[firstRowOfBlock(row)];
    } else {
        const int row = items.indexOf(moving.last()) + 1;
        if (row >= items.size()) {
            return false;
        }
        neighbour = items[lastRowOfBlock(row)];
    }

    for (QObject* item : moving) {
        items.removeOne(item);
    }

    int insertRow = items.indexOf(neighbour) + (moveUp ? 0 : 1);
    for (QObject* item : moving) {
        items.insert(insertRow++, item);
    }

    setItems(items);

    for (QObject* item : selectedItems) {
        selection()->QItemSelectionModel::select(index(items.indexOf(item)), QItemSelectionModel::Select);
    }

    onRowsMoved();

    return true;
}

void InstrumentsOnScoreListModel::onUpdateOperationsAvailability()
{
    SelectableItemListModel::onUpdateOperationsAvailability();

    emit soloistStateChanged();

    const QList<int> rows = selection()->selectedRows();
    if (rows.isEmpty()) {
        return;
    }

    // A group's instruments can't be moved (within it, or out of it)
    int firstRow = rows.first();
    int lastRow = rows.first();
    bool selectsGroupInstrument = false;
    for (int row : rows) {
        const int role = sharedGroupRole(row);
        selectsGroupInstrument = selectsGroupInstrument || role == LayoutPanelItemType::GROUP_MEMBER
                                 || role == LayoutPanelItemType::GROUP_LAST_MEMBER;
        firstRow = std::min(firstRow, row);
        lastRow = std::max(lastRow, lastRowOfBlock(row));
    }

    setIsMovingUpAvailable(!selectsGroupInstrument && firstRow > 0);
    setIsMovingDownAvailable(!selectsGroupInstrument && lastRow < items().size() - 1);
}

InstrumentsOnScoreListModel::InstrumentItem* InstrumentsOnScoreListModel::modelIndexToItem(const QModelIndex& index) const
{
    return dynamic_cast<InstrumentItem*>(item(index));
}

const ScoreOrder& InstrumentsOnScoreListModel::currentScoreOrder() const
{
    if (m_currentOrderIndex < 0 || m_currentOrderIndex >= static_cast<int>(m_scoreOrders.size())) {
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
    // Removing a group removes its instruments; removing all of a group's instruments removes the group
    QSet<QString> groupsWithHeader;
    QSet<QString> groupsWithInstruments;
    for (const QObject* item : items()) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (instrument && instrument->sharedGroupId.isValid()) {
            (instrument->isSharedGroup ? groupsWithHeader : groupsWithInstruments).insert(instrument->sharedGroupId.toQString());
        }
    }

    ItemList items = this->items();
    const qsizetype count = items.size();
    items.removeIf([&](const QObject* item) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (!instrument || !instrument->sharedGroupId.isValid()) {
            return false;
        }
        const QString groupId = instrument->sharedGroupId.toQString();
        return instrument->isSharedGroup ? !groupsWithInstruments.contains(groupId) : !groupsWithHeader.contains(groupId);
    });

    if (items.size() != count) {
        setItems(items);
    }

    verifyScoreOrder();
}

bool InstrumentsOnScoreListModel::matchesScoreOrder() const
{
    const ScoreOrder currentOrder = currentScoreOrder();

    std::vector<int> instrumentIndices;
    for (const QObject* item : items()) {
        auto instrument = dynamic_cast<const InstrumentItem*>(item);
        if (!instrument || instrument->isSharedGroup) {
            continue;
        }
        instrumentIndices.push_back(currentOrder.instrumentSortingIndex(instrument->id, instrument->isSoloist));
    }

    return currentOrder.isScoreOrder(instrumentIndices);
}

void InstrumentsOnScoreListModel::verifyScoreOrder()
{
    const ScoreOrder& currentOrder = currentScoreOrder();
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
    m_scoreOrders.insert(m_scoreOrders.begin() + customizedIndex, customizedOrder);

    emit ordersChanged();

    return customizedIndex;
}

void InstrumentsOnScoreListModel::removeCustomizedScoreOrder(const ScoreOrder& order)
{
    if (!order.customized) {
        return;
    }

    for (size_t i = 0; i < m_scoreOrders.size(); ++i) {
        if (!m_scoreOrders[i].customized && (m_scoreOrders[i].id == order.id)) {
            doSetCurrentOrderIndex(static_cast<int>(i));

            m_scoreOrders.erase(m_scoreOrders.begin() + i + 1);

            emit ordersChanged();
            break;
        }
    }
}

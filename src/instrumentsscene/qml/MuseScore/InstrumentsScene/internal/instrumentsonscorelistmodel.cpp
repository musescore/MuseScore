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

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include "async/notifylist.h"
#include "containers.h"

#include "log.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/sharedpart.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationparts.h"
#include "notation/inotationstyle.h"

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

    QString id; // the instrument (template) id
    ID partId; // existing instruments only
    ID sharedPartId; // the shared part the instrument is on, or (group header) the shared part itself
    QString name;
    QString description; // new instruments only
    bool isSoloist = false;
    bool isExistingPart = false;
    bool isSharedGroupHeader = false; // the header of a shared-staves group
    bool hasTablatureVersion = false; // new instruments only: they can be added as tablature
    TablatureOption tablatureOption = TablatureOption::Notation;
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
        return instrument->description;
    case RoleIsSoloist:
        return instrument->isSoloist;
    case RoleSharedGroupRole:
        return sharedGroupRole(index.row());
    case RoleSharedGroupId:
        return m_showsSharedGroups && instrument->sharedPartId.isValid() ? instrument->sharedPartId.toQString() : QString();
    case RoleIsNew:
        return !instrument->isExistingPart;
    case RoleTablatureOption:
        return instrument->hasTablatureVersion ? static_cast<int>(instrument->tablatureOption) : -1;
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
        // Groups themselves can't be soloists (their instruments can)
        if (instrument->isSharedGroupHeader) {
            return false;
        }

        instrument->isSoloist = value.toBool();

        // A group's instruments stay in their group
        if (isGroupMember(instrument)) {
            emit dataChanged(index, index, { RoleIsSoloist });
        } else {
            ItemList items = this->items();
            items.removeAll(instrument);
            insertInstrument(items, instrument);
            setItems(items);
        }
        verifyScoreOrder();
        emit soloistStateChanged();
        return true;
    case RoleTablatureOption: {
        const int option = value.toInt();
        if (!instrument->hasTablatureVersion || option < static_cast<int>(TablatureOption::Notation)
            || option > static_cast<int>(TablatureOption::TablatureAndNotation)) {
            return false;
        }

        instrument->tablatureOption = static_cast<TablatureOption>(option);
        emit dataChanged(index, index, { RoleTablatureOption });
        return true;
    }
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
    result.reserve(static_cast<int>(m_scoreOrders.size()));

    for (const ScoreOrder& order: m_scoreOrders) {
        result << order.getName().qTranslated();
    }

    return result;
}

int InstrumentsOnScoreListModel::currentOrderIndex() const
{
    return m_currentOrderIndex;
}

bool InstrumentsOnScoreListModel::isSoloistAvailable() const
{
    return !selectedInstruments().isEmpty();
}

bool InstrumentsOnScoreListModel::isSelectionSoloist() const
{
    const QList<InstrumentItem*> instruments = selectedInstruments();

    return !instruments.isEmpty() && std::all_of(instruments.cbegin(), instruments.cend(), [](const InstrumentItem* instrument) {
        return instrument->isSoloist;
    });
}

void InstrumentsOnScoreListModel::load()
{
    TRACEFUNC;

    IMasterNotationPtr master = context()->masterNotation();
    auto parts = master ? master->parts()->partList() : NotifyList<const Part*>();

    INotationPtr notation = master ? master->notation() : nullptr;
    m_showsSharedGroups = notation && notation->style()->styleValue(StyleId::enableStaveSharing).toBool();

    ItemList instruments;
    instruments.reserve(static_cast<int>(parts.size()));
    m_sharedParts.clear();

    for (const Part* part : parts) {
        const mu::engraving::SharedPart* sharedPart = part->isSharedPart() ? mu::engraving::toSharedPart(part) : part->sharedPart();
        if (!sharedPart) {
            instruments << createItem(part);
            continue;
        }

        // A group's instruments follow its header, in score order, even if the score has them apart
        if (!m_sharedParts.try_emplace(sharedPart->id(), sharedPart->instrument()->id()).second) {
            continue;
        }

        if (m_showsSharedGroups) {
            InstrumentItem* header = createItem(sharedPart);
            header->isSharedGroupHeader = true;
            instruments << header;
        }

        for (const Part* member : sharedPart->originParts()) {
            instruments << createItem(member);
        }
    }

    setItems(instruments);

    loadOrders();
}

InstrumentsOnScoreListModel::InstrumentItem* InstrumentsOnScoreListModel::createItem(const Part* part)
{
    InstrumentItem* instrument = new InstrumentItem(this);
    instrument->id = part->instrument()->id();
    instrument->partId = part->id();
    instrument->isExistingPart = true;
    instrument->name = part->partName();
    instrument->isSoloist = part->soloist();

    if (part->isSharedPart()) {
        instrument->sharedPartId = part->id();
    } else if (const mu::engraving::SharedPart* sharedPart = part->sharedPart()) {
        instrument->sharedPartId = sharedPart->id();
    }

    return instrument;
}

void InstrumentsOnScoreListModel::loadOrders()
{
    TRACEFUNC;

    m_scoreOrders = repository()->orders();

    const ScoreOrder& custom = customOrder();
    if (!muse::contains(m_scoreOrders, custom)) {
        m_scoreOrders.push_back(custom);
    }

    INotationPtr notation = context()->currentNotation();
    ScoreOrder currentOrder = notation ? notation->parts()->scoreOrder() : m_scoreOrders.front();
    bool orderCustomized = currentOrder.customized;

    if (!muse::contains(m_scoreOrders, currentOrder)) {
        currentOrder.customized = false;
        m_scoreOrders.push_back(currentOrder);
    }

    emit ordersChanged();

    for (size_t i = 0; i < m_scoreOrders.size(); ++i) {
        if (m_scoreOrders.at(i).id == currentOrder.id) {
            doSetCurrentOrderIndex(static_cast<int>(i));
            break;
        }
    }

    if (orderCustomized) {
        doSetCurrentOrderIndex(createCustomizedScoreOrder(currentOrder));
    }
}

int InstrumentsOnScoreListModel::addInstruments(const QStringList& instrumentIdList)
{
    TRACEFUNC;

    ItemList items = this->items();
    ItemList added;

    for (const QString& id : instrumentIdList) {
        const InstrumentTemplate& templ = repository()->instrumentTemplate(id);
        if (!templ.isValid()) {
            continue;
        }

        InstrumentItem* instrument = new InstrumentItem(this);

        instrument->id = templ.id;
        instrument->name = formatInstrumentTitle(templ.trackName, templ.trait);
        instrument->description = templ.description.toQString();
        instrument->hasTablatureVersion = mu::engraving::searchTablatureTemplate(templ.id) != nullptr;

        insertInstrument(items, instrument);
        added << instrument;
    }

    setItems(items);
    verifyScoreOrder();

    // A later instrument can be inserted above an earlier one
    int firstRow = -1;
    for (const QObject* item : added) {
        const int row = static_cast<int>(items.indexOf(item));
        firstRow = firstRow < 0 ? row : std::min(firstRow, row);
    }

    return firstRow;
}

void InstrumentsOnScoreListModel::moveSelectedBlocksUp()
{
    const std::optional<RowRange> moving = movingRows();
    if (!moving || moving->first == 0) {
        return;
    }

    // The block above (an instrument, or a whole group) goes below the moving rows
    for (const RowRange& block : blocks(items())) {
        if (block.last == moving->first - 1) {
            moveBlock(block, moving->last);
            return;
        }
    }
}

void InstrumentsOnScoreListModel::moveSelectedBlocksDown()
{
    const std::optional<RowRange> moving = movingRows();
    if (!moving || moving->last == rowCount() - 1) {
        return;
    }

    // The block below goes above the moving rows
    for (const RowRange& block : blocks(items())) {
        if (block.first == moving->last + 1) {
            moveBlock(block, moving->first);
            return;
        }
    }
}

void InstrumentsOnScoreListModel::removeSelectedBlocks()
{
    const ItemList& items = this->items();

    // removing a group's header, or all its instruments, removes both
    for (const RowRange& block : blocks(items)) {
        if (!static_cast<const InstrumentItem*>(items.at(block.first))->isSharedGroupHeader) {
            continue;
        }

        bool allInstrumentsSelected = true;
        for (int row = block.first + 1; row <= block.last && allInstrumentsSelected; ++row) {
            allInstrumentsSelected = selection()->isSelected(index(row));
        }

        if (allInstrumentsSelected || selection()->isSelected(index(block.first))) {
            selection()->select(QItemSelection(index(block.first), index(block.last)), QItemSelectionModel::Select);
        }
    }

    SelectableItemListModel::removeSelection();
}

void InstrumentsOnScoreListModel::toggleSoloistForSelection()
{
    const QList<InstrumentItem*> instruments = selectedInstruments();
    if (instruments.isEmpty()) {
        return;
    }

    // In a mixed selection, all of them become soloists
    const bool makeSoloists = !isSelectionSoloist();

    ItemList items = this->items();

    // setItems() clears the selection: it's restored afterwards, so that the selection can be toggled back
    ItemList selectedItems;
    for (int row : selection()->selectedRows()) {
        selectedItems << items.at(row);
    }

    bool reordered = false;
    for (InstrumentItem* instrument : instruments) {
        instrument->isSoloist = makeSoloists;

        // Soloists are sorted separately, but a group's instruments stay in their group
        if (isGroupMember(instrument)) {
            const QModelIndex changed = index(static_cast<int>(this->items().indexOf(instrument)));
            emit dataChanged(changed, changed, { RoleIsSoloist });
        } else {
            items.removeAll(instrument);
            insertInstrument(items, instrument);
            reordered = true;
        }
    }

    if (reordered) {
        setItems(items);

        QItemSelection restored;
        for (const QObject* item : selectedItems) {
            const QModelIndex selected = index(static_cast<int>(items.indexOf(item)));
            restored.select(selected, selected);
        }
        selection()->select(restored, QItemSelectionModel::Select);
    }

    verifyScoreOrder();

    // Also when only the soloist flags changed, not the selection
    emit soloistStateChanged();
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
    result.reserve(items().size() + static_cast<int>(m_sharedParts.size()));

    auto appendPart = [&result](const ID& partId, const QString& instrumentId, bool isExistingPart, bool isSoloist,
                                TablatureOption tablatureOption = TablatureOption::Notation) {
        QVariantMap obj;
        obj["partId"] = partId.toQString();
        obj["instrumentId"] = instrumentId;
        obj["isExistingPart"] = isExistingPart;
        obj["isSoloist"] = isSoloist;
        obj["tablatureOption"] = static_cast<int>(tablatureOption);

        result << obj;
    };

    //! NOTE Parts that aren't listed are removed from the score: shared parts are listed before their first
    //! instrument, so they stay. A shared part none of whose instruments are left is removed
    std::set<ID> listedSharedParts;

    for (const QObject* item : items()) {
        auto instrument = static_cast<const InstrumentItem*>(item);
        if (instrument->isSharedGroupHeader) {
            continue;
        }

        const ID& sharedPartId = instrument->sharedPartId;
        if (sharedPartId.isValid() && listedSharedParts.insert(sharedPartId).second) {
            if (auto it = m_sharedParts.find(sharedPartId); it != m_sharedParts.end()) {
                appendPart(sharedPartId, it->second, true, false);
            }
        }

        appendPart(instrument->partId, instrument->id, instrument->isExistingPart, instrument->isSoloist,
                   instrument->tablatureOption);
    }

    return result;
}

bool InstrumentsOnScoreListModel::isSharedGroupSelected(const QString& sharedGroupId) const
{
    if (!m_showsSharedGroups || sharedGroupId.isEmpty()) {
        return false;
    }

    const ID groupId(sharedGroupId);
    const ItemList& items = this->items();

    for (int row = 0; row < items.size(); ++row) {
        auto instrument = static_cast<const InstrumentItem*>(items.at(row));
        if (instrument->isSharedGroupHeader && instrument->sharedPartId == groupId) {
            return selection()->isSelected(index(row));
        }
    }

    return false;
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

QList<InstrumentsOnScoreListModel::InstrumentItem*> InstrumentsOnScoreListModel::selectedInstruments() const
{
    QList<int> rows = selection()->selectedRows();
    std::sort(rows.begin(), rows.end());

    // Groups themselves can't be soloists (their instruments can)
    QList<InstrumentItem*> result;
    result.reserve(rows.size());

    for (int row : rows) {
        auto instrument = static_cast<InstrumentItem*>(items().at(row));
        if (!instrument->isSharedGroupHeader) {
            result << instrument;
        }
    }

    return result;
}

InstrumentsOnScoreListModel::SharedGroupRole InstrumentsOnScoreListModel::sharedGroupRole(int row) const
{
    const ItemList& items = this->items();
    if (!m_showsSharedGroups || row < 0 || row >= items.size()) {
        return NotInGroup;
    }

    auto instrument = static_cast<const InstrumentItem*>(items.at(row));
    if (!instrument->sharedPartId.isValid()) {
        return NotInGroup;
    }

    if (instrument->isSharedGroupHeader) {
        return GroupHeader;
    }

    auto next = row + 1 < items.size() ? static_cast<const InstrumentItem*>(items.at(row + 1)) : nullptr;
    const bool nextIsMember = next && !next->isSharedGroupHeader && next->sharedPartId == instrument->sharedPartId;

    return nextIsMember ? GroupMember : GroupLastMember;
}

bool InstrumentsOnScoreListModel::isGroupMember(const InstrumentItem* instrument) const
{
    return !instrument->isSharedGroupHeader && instrument->sharedPartId.isValid();
}

std::vector<InstrumentsOnScoreListModel::RowRange> InstrumentsOnScoreListModel::blocks(const ItemList& instruments) const
{
    // a group's header (sharing on only) with its instruments, or a single row
    std::vector<RowRange> result;

    for (int row = 0; row < instruments.size(); ++row) {
        auto instrument = static_cast<const InstrumentItem*>(instruments.at(row));

        auto previous = result.empty() ? nullptr : static_cast<const InstrumentItem*>(instruments.at(result.back().first));
        const bool continuesGroup = previous && isGroupMember(instrument) && instrument->sharedPartId == previous->sharedPartId;

        if (continuesGroup) {
            result.back().last = row;
        } else {
            result.push_back({ row, row });
        }
    }

    return result;
}

int InstrumentsOnScoreListModel::sortingIndex(const ItemList& instruments, const RowRange& block) const
{
    // a group is sorted by its first instrument, not its header
    auto first = static_cast<const InstrumentItem*>(instruments.at(block.first));
    const int row = first->isSharedGroupHeader && block.count() > 1 ? block.first + 1 : block.first;
    auto instrument = static_cast<const InstrumentItem*>(instruments.at(row));

    return currentScoreOrder().instrumentSortingIndex(instrument->id, instrument->isSoloist);
}

std::optional<InstrumentsOnScoreListModel::RowRange> InstrumentsOnScoreListModel::movingRows() const
{
    const ItemList& items = this->items();
    std::optional<RowRange> result;

    // a group's instruments can't be moved, unless the group isn't shown
    if (m_showsSharedGroups) {
        for (int row : selection()->selectedRows()) {
            if (isGroupMember(static_cast<const InstrumentItem*>(items.at(row)))) {
                return std::nullopt;
            }
        }
    }

    // A selected header, or (group not shown) instrument, brings the whole group
    for (const RowRange& block : blocks(items)) {
        bool isBlockSelected = false;
        for (int row = block.first; row <= block.last && !isBlockSelected; ++row) {
            isBlockSelected = selection()->isSelected(index(row));
        }

        if (!isBlockSelected) {
            continue;
        }

        // an unselected block in between
        if (result && result->last + 1 != block.first) {
            return std::nullopt;
        }

        if (result) {
            result->last = block.last;
        } else {
            result = block;
        }
    }

    return result;
}

void InstrumentsOnScoreListModel::moveBlock(const RowRange& block, int destinationRow)
{
    // destinationRow: where the block's first row (moving up) / last row (moving down) ends up
    // the base's moveRows() is private
    QAbstractItemModel* model = this;
    model->moveRows(QModelIndex(), block.first, block.count(), QModelIndex(), destinationRow);
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

    //! NOTE Groups are sorted as a whole. The sorting index is worked out once per block: it searches all
    //! instrument templates
    std::vector<std::pair<int, RowRange> > keyed;
    for (const RowRange& block : blocks(instruments)) {
        keyed.emplace_back(sortingIndex(instruments, block), block);
    }

    //! NOTE Stable: instruments with the same index (e.g. two of the same instrument) keep their order
    std::stable_sort(keyed.begin(), keyed.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    ItemList sorted;
    sorted.reserve(instruments.size());
    for (const auto& entry : keyed) {
        sorted << instruments.mid(entry.second.first, entry.second.count());
    }

    instruments = std::move(sorted);
}

void InstrumentsOnScoreListModel::insertInstrument(ItemList& instruments, InstrumentItem* newInstrument)
{
    const int newIndex = currentScoreOrder().instrumentSortingIndex(newInstrument->id, newInstrument->isSoloist);

    // Never in among a group's instruments
    for (const RowRange& block : blocks(instruments)) {
        if (sortingIndex(instruments, block) > newIndex) {
            instruments.insert(block.first, newInstrument);
            return;
        }
    }

    instruments << newInstrument;
}

InstrumentsOnScoreListModel::InstrumentItem* InstrumentsOnScoreListModel::modelIndexToItem(const QModelIndex& index) const
{
    //! NOTE All items are InstrumentItems; item() returns null for an invalid index
    return static_cast<InstrumentItem*>(item(index));
}

const ScoreOrder& InstrumentsOnScoreListModel::currentScoreOrder() const
{
    if (m_currentOrderIndex < 0 || m_currentOrderIndex >= static_cast<int>(m_scoreOrders.size())) {
        static ScoreOrder dummy;
        return dummy;
    }

    return m_scoreOrders.at(m_currentOrderIndex);
}

void InstrumentsOnScoreListModel::onUpdateOperationsAvailability()
{
    SelectableItemListModel::onUpdateOperationsAvailability();

    const std::optional<RowRange> moving = movingRows();
    setIsMovingUpAvailable(moving && moving->first > 0);
    setIsMovingDownAvailable(moving && moving->last < rowCount() - 1);

    emit soloistStateChanged();
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
    const ItemList& instruments = items();
    const std::vector<RowRange> instrumentBlocks = blocks(instruments);

    std::vector<int> blockIndices;
    blockIndices.reserve(instrumentBlocks.size());

    for (const RowRange& block : instrumentBlocks) {
        blockIndices.push_back(sortingIndex(instruments, block));
    }

    return currentScoreOrder().isScoreOrder(blockIndices);
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
        if (!m_scoreOrders.at(i).customized && (m_scoreOrders.at(i).id == order.id)) {
            doSetCurrentOrderIndex(static_cast<int>(i));

            m_scoreOrders.erase(m_scoreOrders.begin() + i + 1);

            emit ordersChanged();
            break;
        }
    }
}

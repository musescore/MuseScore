/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "layoutpaneltreemodel.h"

#include <algorithm>
#include <functional>
#include <set>

#include "async/notifylist.h"

#include "translation.h"
#include "defer.h"

#include "engraving/dom/score.h"
#include "engraving/dom/sharedpart.h"

#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationselection.h" // IWYU pragma: keep
#include "notation/inotationstyle.h" // IWYU pragma: keep
#include "notation/inotationundostack.h" // IWYU pragma: keep

#include "roottreeitem.h"
#include "parttreeitem.h"
#include "sharedparttreeitem.h"
#include "stafftreeitem.h"
#include "staffcontroltreeitem.h"
#include "systemobjectslayertreeitem.h"
#include "instrumentscommands.h"

#include "uicomponents/qml/Muse/UiComponents/itemmultiselectionmodel.h"

#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;
using namespace muse::uicomponents;

// Ids of empty shared-staves groups (which only exist in the panel), out of the range of score element ids
static muse::ID emptySharedGroupId(int number)
{
    return muse::ID(uint64_t(0xFFFF000000000000ULL) + uint64_t(number));
}

LayoutPanelTreeModel::LayoutPanelTreeModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void LayoutPanelTreeModel::classBegin()
{
    init();
}

void LayoutPanelTreeModel::init()
{
    m_partsNotifyReceiver = std::make_shared<muse::async::Asyncable>();

    m_selectionModel = new ItemMultiSelectionModel(this);
    m_selectionModel->setAllowedModifiers(Qt::ShiftModifier);

    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged,
            this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
        setItemsSelected(deselected.indexes(), false);
        setItemsSelected(selected.indexes(), true);
        syncItemsSelection();

        updateRearrangementAvailability();
        updateRemovingAvailability();
        updateSelectedItemsType();
        updateIsAddingSystemMarkingsAvailable();
    });

    connect(this, &LayoutPanelTreeModel::rowsInserted, this, [this]() {
        updateRemovingAvailability();
    });

    onMasterNotationChanged();
    context()->masterNotationChanged().onNotify(this, [this]() {
        onMasterNotationChanged();
    });

    onNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    commandShortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        emit addInstrumentsKeyboardShortcutChanged();
    });
}

void LayoutPanelTreeModel::onMasterNotationChanged()
{
    m_masterNotation = context()->masterNotation();
}

void LayoutPanelTreeModel::onNotationChanged()
{
    m_partsNotifyReceiver->async_disconnectAll();

    if (m_isLoadingBlocked) {
        m_notationChangedWhileLoadingWasBlocked = true;
        return;
    }

    onBeforeChangeNotation();
    m_notation = context()->currentNotation();

    if (m_notation) {
        load();
    } else {
        clear();
    }

    m_notationChangedWhileLoadingWasBlocked = false;
}

bool LayoutPanelTreeModel::isMasterScore() const
{
    return m_notation && m_notation->isMaster();
}

LayoutPanelTreeModel::~LayoutPanelTreeModel()
{
    deleteItems();
}

bool LayoutPanelTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!m_isRemovingAvailable) {
        return false;
    }

    AbstractLayoutPanelTreeItem* parentItem = modelIndexToItem(parent);

    if (!parentItem) {
        parentItem = m_rootItem;
    }

    if (parentItem == m_rootItem && needWarnOnRemoveRows(row, count)) {
        // When removing instruments, the user needs to be warned in some cases
        if (!warnAboutRemovingInstrumentsIfNecessary(count)) {
            return false;
        }
    }

    setLoadingBlocked(true);
    beginRemoveRows(parent, row, row + count - 1);

    parentItem->removeChildren(row, count, true);

    endRemoveRows();
    setLoadingBlocked(false);

    emit isEmptyChanged();

    updateSystemObjectLayers();

    return true;
}

bool LayoutPanelTreeModel::shouldShowSystemObjectLayers() const
{
    // Only show system object staves in master score
    // TODO: extend system object staves logic to parts
    return isMasterScore();
}

void LayoutPanelTreeModel::onBeforeChangeNotation()
{
    if (!m_notation || !m_rootItem) {
        return;
    }

    QList<muse::ID> partIdList;

    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->type() == LayoutPanelItemType::ItemType::PART) {
            partIdList << item->id();
        }
    }
}

void LayoutPanelTreeModel::setLoadingBlocked(bool blocked)
{
    m_isLoadingBlocked = blocked;

    if (!m_isLoadingBlocked && m_notationChangedWhileLoadingWasBlocked) {
        onNotationChanged();
    }
}

void LayoutPanelTreeModel::setupPartsConnections()
{
    async::NotifyList<const Part*> notationParts = m_notation->parts()->partList();

    notationParts.onChanged(m_partsNotifyReceiver.get(), [this]() {
        load();
    }, Mode::SetReplace);

    auto updateMasterPartItem = [this](const muse::ID& partId) {
        auto partItem = dynamic_cast<PartTreeItem*>(m_rootItem->childAtId(partId, LayoutPanelItemType::PART));
        if (!partItem) {
            return;
        }

        partItem->init(m_masterNotation->parts()->part(partId));
        updateRemovingAvailability();
    };

    notationParts.onItemAdded(m_partsNotifyReceiver.get(), [updateMasterPartItem](const Part* part) {
        updateMasterPartItem(part->id());
    }, Mode::SetReplace);

    notationParts.onItemChanged(m_partsNotifyReceiver.get(), [updateMasterPartItem](const Part* part) {
        updateMasterPartItem(part->id());
    }, Mode::SetReplace);

    m_notation->parts()->systemObjectStavesChanged().onNotify(this, [this]() {
        m_shouldUpdateSystemObjectLayers = true;
        if (!m_isLoadingBlocked) {
            load();
        }
    }, Mode::SetReplace);

    m_notation->parts()->sharedPartsChanged().onNotify(this, [this]() {
        updateIsStaveSharingEnabled();
        updateSharedGroupRoles();
    }, Mode::SetReplace);
}

void LayoutPanelTreeModel::setupStavesConnections(const muse::ID& partId)
{
    async::NotifyList<const Staff*> notationStaves = m_notation->parts()->staffList(partId);

    notationStaves.onItemChanged(m_partsNotifyReceiver.get(), [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId, LayoutPanelItemType::PART);
        if (!partItem) {
            return;
        }

        auto staffItem = dynamic_cast<StaffTreeItem*>(partItem->childAtId(staff->id(), LayoutPanelItemType::STAFF));
        if (!staffItem) {
            return;
        }

        staffItem->init(m_masterNotation->parts()->staff(staff->id()));
    }, Mode::SetReplace);

    notationStaves.onItemRemoved(m_partsNotifyReceiver.get(), [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId, LayoutPanelItemType::PART);
        if (!partItem) {
            return;
        }

        auto staffItem = partItem->childAtId(staff->id(), LayoutPanelItemType::STAFF);
        if (!staffItem) {
            return;
        }

        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());
        int staffRow = staffItem->row();

        beginRemoveRows(partIndex, staffRow, staffRow);
        partItem->removeChildren(staffRow, 1, false);
        endRemoveRows();
    }, Mode::SetReplace);

    notationStaves.onItemAdded(m_partsNotifyReceiver.get(), [this, partId](const Staff* staff) {
        auto partItem = m_rootItem->childAtId(partId, LayoutPanelItemType::PART);
        if (!partItem) {
            return;
        }

        if (partItem->childAtId(staff->id(), LayoutPanelItemType::STAFF)) {
            return; // this staff item already exists in the part
        }

        const Staff* masterStaff = m_masterNotation->parts()->staff(staff->id());
        size_t staffIdx = muse::indexOf(masterStaff->part()->staves(), const_cast<Staff*>(masterStaff));
        if (staffIdx == muse::nidx) {
            return;
        }

        auto staffItem = buildMasterStaffItem(masterStaff, partItem);
        QModelIndex partIndex = index(partItem->row(), 0, QModelIndex());
        int dstRow = static_cast<int>(staffIdx);

        beginInsertRows(partIndex, dstRow, dstRow);
        partItem->insertChild(staffItem, dstRow);
        endInsertRows();
    }, Mode::SetReplace);
}

void LayoutPanelTreeModel::setupNotationConnections()
{
    m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
        updateSelectedRows();
    }, Mode::SetReplace);

    m_notation->undoStack()->changesChannel().onReceive(this, [this](const mu::engraving::ScoreChanges& changes) {
        if (changes.isTextEditing) {
            return;
        }

        if (!m_layoutPanelVisible) {
            m_scoreChanged = true;
            return;
        }

        onScoreChanged(changes);
    }, Mode::SetReplace /*because this channel is from MasterScore*/);
}

void LayoutPanelTreeModel::updateSelectedRows()
{
    if (!m_layoutPanelVisible || !m_notation) {
        return;
    }

    m_selectionModel->clear();

    const std::vector<EngravingItem*>& selectedElements = m_notation->interaction()->selection()->elements();
    if (selectedElements.empty()) {
        return;
    }

    std::vector<muse::ID> selectedPartIds;
    for (const EngravingItem* element : selectedElements) {
        if (!element->part()) {
            continue;
        }

        selectedPartIds.push_back(element->part()->id());
    }

    for (const muse::ID& selectedPartId : selectedPartIds) {
        AbstractLayoutPanelTreeItem* item = m_rootItem->childAtId(selectedPartId, LayoutPanelItemType::PART);

        if (item) {
            m_selectionModel->select(createIndex(item->row(), 0, item), QItemSelectionModel::Select);
        }
    }
}

void LayoutPanelTreeModel::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    if (!m_rootItem) {
        return;
    }

    updateIsStaveSharingEnabled();
    updateIsHideEmptyStavesEnabled();

    for (AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        item->onScoreChanged(changes);
    }

    updateSharedGroupRoles();

    m_scoreChanged = false;
}

void LayoutPanelTreeModel::clear()
{
    TRACEFUNC;

    beginResetModel();
    deleteItems();
    endResetModel();

    emit isEmptyChanged();
    emit isAddingAvailableChanged(false);
}

void LayoutPanelTreeModel::deleteItems()
{
    m_selectionModel->clear();
    delete m_rootItem;
    m_rootItem = nullptr;
}

void LayoutPanelTreeModel::load()
{
    if (m_isLoadingBlocked) {
        return;
    }

    TRACEFUNC;

    beginResetModel();
    deleteItems();

    m_rootItem = new RootTreeItem(m_masterNotation, m_notation, this);

    async::NotifyList<const Part*> masterParts = m_masterNotation->parts()->partList();
    async::NotifyList<const Part*> excerptParts = m_notation->parts()->partList();
    // For master score we should use score's order, for others - custom
    if (!isMasterScore()) {
        sortParts(masterParts, excerptParts);
    }

    const bool showSystemObjectLayers = shouldShowSystemObjectLayers();
    const std::vector<Staff*>& systemObjectStaves = m_masterNotation->notation()->parts()->systemObjectStaves();

    SystemObjectGroupsByStaff systemObjects;
    if (showSystemObjectLayers) {
        systemObjects = collectSystemObjectGroups(systemObjectStaves);
    }

    for (const Part* part : masterParts) {
        if (showSystemObjectLayers) {
            for (Staff* staff : part->staves()) {
                if (!staff->hasSystemObjectsBelowBottomStaff() && muse::contains(systemObjectStaves, staff)) {
                    m_rootItem->appendChild(buildSystemObjectsLayerItem(staff, systemObjects[staff]));
                }
            }
        }

        if (part->isSharedPart()) {
            // Shared staves aren't shown in parts: there, instruments are shown as they are without combining
            if (isMasterScore()) {
                m_rootItem->appendChild(buildSharedPartItem(part));
            }
        } else {
            m_rootItem->appendChild(buildMasterPartItem(part));
        }

        if (showSystemObjectLayers) {
            for (Staff* staff : part->staves()) {
                if (staff->hasSystemObjectsBelowBottomStaff()) {
                    m_rootItem->appendChild(buildSystemObjectsLayerItem(staff, systemObjects[staff]));
                }
            }
        }
    }

    // Empty shared-staves groups: those added via the Add menu go after the last instrument (above system markings
    // at the bottom); those left by their instruments stay where the group was (after the instrument above it).
    // One whose shared part exists again (e.g. after undo) isn't shown
    if (isMasterScore() && !m_emptySharedGroups.empty()) {
        for (const EmptySharedGroup& group : m_emptySharedGroups) {
            if (group.originSharedPartId.isValid() && m_masterNotation->parts()->partExists(group.originSharedPartId)) {
                continue;
            }

            int row = 0;
            if (group.atBottom) {
                row = m_rootItem->childCount();
                while (row > 0 && m_rootItem->childType(row - 1) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                    --row;
                }
            } else if (const AbstractLayoutPanelTreeItem* anchor = findEmptyGroupAnchor(group.anchorPartId)) {
                row = anchor->row() + 1;
                // After the anchor's whole group, if it's in one (or is one)
                const Part* anchorPart = static_cast<const PartTreeItem*>(anchor)->part();
                const Part* anchorGroup = anchorPart && anchorPart->isSharedPart() ? anchorPart
                                          : anchorPart ? anchorPart->sharedPart() : nullptr;
                while (anchorGroup && row < m_rootItem->childCount() && m_rootItem->childType(row) == LayoutPanelItemType::PART
                       && static_cast<const PartTreeItem*>(m_rootItem->childAtRow(row))->part()->sharedPart() == anchorGroup) {
                    ++row;
                }
            } else {
                // At the top (below system markings there)
                while (row < m_rootItem->childCount() && m_rootItem->childType(row) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                    ++row;
                }
            }

            auto item = new AbstractLayoutPanelTreeItem(LayoutPanelItemType::EMPTY_SHARED_GROUP, m_masterNotation, m_notation,
                                                        m_rootItem);
            item->setId(emptySharedGroupId(group.number));
            item->setTitle(group.title);
            item->setIsVisible(true);
            item->setIsExpandable(true);
            item->setSettingsAvailable(true);
            item->setSettingsEnabled(true);
            item->setIsSelectable(true); // to be moved
            m_rootItem->insertChild(item, row);
        }
    }

    endResetModel();

    setupPartsConnections();
    setupNotationConnections();

    updateIsAddingSystemMarkingsAvailable();

    updateIsStaveSharingEnabled();
    updateIsHideEmptyStavesEnabled();
    updateSharedGroupRoles();

    emit scoreChanged();
    emit isEmptyChanged();
    emit isAddingAvailableChanged(true);
}

void LayoutPanelTreeModel::addEmptySharedGroup()
{
    if (!isMasterScore()) {
        return;
    }

    EmptySharedGroup group;
    group.number = ++m_lastEmptySharedGroupNumber;
    //: The name of a new shared-staves group, before any instruments are added to it; %1 is a number
    group.title = muse::qtrc("layoutpanel", "Group %1").arg(group.number);
    group.atBottom = true;
    m_emptySharedGroups.push_back(group);

    // A group only makes sense while instruments are combined
    if (!isStaveSharingEnabled()) {
        toggleStaveSharing(true); // reloads
    } else {
        load();
    }
}

const AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::findEmptyGroupAnchor(const muse::ID& anchorPartId) const
{
    if (!anchorPartId.isValid() || !m_rootItem) {
        return nullptr;
    }
    if (const AbstractLayoutPanelTreeItem* item = m_rootItem->childAtId(anchorPartId, LayoutPanelItemType::PART)) {
        return item;
    }
    return m_rootItem->childAtId(anchorPartId, LayoutPanelItemType::SHARED_PART);
}

void LayoutPanelTreeModel::updateEmptySharedGroupPositions()
{
    if (!m_rootItem) {
        return;
    }

    // Each empty group's position is the instrument (or group) above it in the panel
    ID anchorPartId;
    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->type() == LayoutPanelItemType::PART || item->type() == LayoutPanelItemType::SHARED_PART) {
            anchorPartId = item->id();
        } else if (item->type() == LayoutPanelItemType::EMPTY_SHARED_GROUP) {
            if (EmptySharedGroup* group = findEmptySharedGroup(item->idStr())) {
                group->anchorPartId = anchorPartId;
                group->atBottom = false;
            }
        }
    }
}

LayoutPanelTreeModel::EmptySharedGroup* LayoutPanelTreeModel::findEmptySharedGroup(const QString& emptyGroupId)
{
    for (EmptySharedGroup& group : m_emptySharedGroups) {
        if (emptySharedGroupId(group.number).toQString() == emptyGroupId) {
            return &group;
        }
    }
    return nullptr;
}

void LayoutPanelTreeModel::keepEmptiedSharedGroups(const QStringList& leavingPartIds, const QString& destinationGroupId)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts || !m_rootItem) {
        return;
    }

    std::set<const SharedPart*> leftGroups;
    for (const QString& partId : leavingPartIds) {
        const Part* part = parts->part(ID(partId));
        if (part && part->sharedPart() && part->sharedPart()->id().toQString() != destinationGroupId) {
            leftGroups.insert(part->sharedPart());
        }
    }

    for (const SharedPart* sharedPart : leftGroups) {
        bool allLeaving = true;
        for (const Part* originPart : sharedPart->originParts()) {
            if (!leavingPartIds.contains(originPart->id().toQString())) {
                allLeaving = false;
            }
        }
        if (!allLeaving) {
            continue;
        }

        // The group stays where it is, as an empty group
        EmptySharedGroup group;
        group.number = ++m_lastEmptySharedGroupNumber;
        group.originSharedPartId = sharedPart->id();

        const AbstractLayoutPanelTreeItem* header = m_rootItem->childAtId(sharedPart->id(), LayoutPanelItemType::SHARED_PART);
        group.title = header ? header->title() : muse::qtrc("layoutpanel", "Group %1").arg(group.number);

        // Anchor: the nearest instrument above the group (if none, the group goes at the top)
        for (int row = header ? header->row() - 1 : -1; row >= 0; --row) {
            const AbstractLayoutPanelTreeItem* item = m_rootItem->childAtRow(row);
            if (item->type() == LayoutPanelItemType::PART || item->type() == LayoutPanelItemType::SHARED_PART) {
                group.anchorPartId = item->id();
                break;
            }
        }

        m_emptySharedGroups.push_back(group);
    }
}

QString LayoutPanelTreeModel::fillEmptySharedGroup(const QString& emptyGroupId, const QString& partId)
{
    return movePartsToSharedGroup({ partId }, emptyGroupId, true);
}

void LayoutPanelTreeModel::addPartToSharedGroup(const QString& partId, const QString& sharedPartId)
{
    movePartsToSharedGroup({ partId }, sharedPartId, false);
}

void LayoutPanelTreeModel::ungroupSharedGroup(const QString& sharedPartId)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts) {
        return;
    }

    // Ungrouping deliberately removes the group (also any empty group it left behind earlier)
    m_emptySharedGroups.removeIf([&](const EmptySharedGroup& group) {
        return group.originSharedPartId == ID(sharedPartId);
    });

    parts->ungroupSharedPart(ID(sharedPartId));
}

void LayoutPanelTreeModel::removePartFromSharedGroup(const QString& partId)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts) {
        return;
    }

    keepEmptiedSharedGroups({ partId }, QString());
    parts->removePartFromSharedPart(ID(partId));
}

void LayoutPanelTreeModel::duplicateParts(const QStringList& partIds)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts) {
        return;
    }

    IDList ids;
    for (const QString& id : partIds) {
        ids.push_back(ID(id));
    }

    parts->duplicateParts(ids);
}

QString LayoutPanelTreeModel::movePartsToSharedGroup(const QStringList& partIds, const QString& groupId, bool isEmptyGroup)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts || partIds.empty()) {
        return QString();
    }

    IDList ids;
    for (const QString& id : partIds) {
        ids.push_back(ID(id));
    }

    if (!isEmptyGroup) {
        keepEmptiedSharedGroups(partIds, groupId);
        parts->addPartsToSharedPart(ids, ID(groupId));
        return groupId;
    }

    EmptySharedGroup* emptyGroup = findEmptySharedGroup(groupId);
    if (!emptyGroup) {
        return QString();
    }

    // Where the empty group is: after its anchor, at the bottom (after the last instrument), or at the top
    ID destinationPartId = emptyGroup->anchorPartId;
    INotationParts::InsertMode mode = INotationParts::InsertMode::After;
    if (emptyGroup->atBottom || !destinationPartId.isValid()) {
        for (const Part* p : parts->partList()) {
            if (emptyGroup->atBottom) {
                if (!p->isSharedPart()) {
                    destinationPartId = p->id(); // the last instrument
                }
            } else if (!destinationPartId.isValid()) {
                destinationPartId = p->id(); // the first instrument
                mode = INotationParts::InsertMode::Before;
            }
        }
    } else if (const Part* anchor = parts->part(destinationPartId); anchor&& anchor->sharedPart()) {
        // After the anchor's whole group
        for (const Part* p : parts->partList()) {
            if (p->sharedPart() == anchor->sharedPart()) {
                destinationPartId = p->id();
            }
        }
    }

    const int number = emptyGroup->number;
    keepEmptiedSharedGroups(partIds, QString());
    m_emptySharedGroups.removeIf([number](const EmptySharedGroup& group) { return group.number == number; });

    const QString sharedPartId = parts->createSharedPart(ids, destinationPartId, mode).toQString();
    return sharedPartId == ID().toQString() ? QString() : sharedPartId;
}

QString LayoutPanelTreeModel::createSharedGroupWithParts(const QStringList& partIds)
{
    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts || partIds.empty()) {
        return QString();
    }

    if (!isStaveSharingEnabled()) {
        toggleStaveSharing(true);
    }

    IDList ids;
    for (const QString& id : partIds) {
        ids.push_back(ID(id));
    }

    keepEmptiedSharedGroups(partIds, QString());

    const QString sharedPartId = parts->createSharedPart(ids).toQString();
    return sharedPartId == ID().toQString() ? QString() : sharedPartId;
}

void LayoutPanelTreeModel::deleteEmptySharedGroup(const QString& emptyGroupId)
{
    if (EmptySharedGroup* group = findEmptySharedGroup(emptyGroupId)) {
        const int number = group->number;
        m_emptySharedGroups.removeIf([number](const EmptySharedGroup& g) { return g.number == number; });
        load();
    }
}

void LayoutPanelTreeModel::toggleStaveSharing(bool on)
{
    m_notation->parts()->toggleStaveSharing(on);
    updateIsStaveSharingEnabled();
}

void LayoutPanelTreeModel::toggleHideEmptyStaves(bool on)
{
    if (!m_notation || isHideEmptyStavesEnabled() == on) {
        return;
    }

    m_notation->undoStack()->prepareChanges(TranslatableString("undoableAction", "Hide empty staves"));
    m_notation->style()->setStyleValue(StyleId::hideEmptyStaves, on);
    m_notation->undoStack()->commitChanges();

    updateIsHideEmptyStavesEnabled();
}

void LayoutPanelTreeModel::sortParts(PartList& parts, const PartList& referenceParts)
{
    // First collect ids of referenceParts to use in sorting further

    std::vector<ID> referenceIdOrder;
    referenceIdOrder.reserve(referenceParts.size());

    for (const Part* part : referenceParts) {
        referenceIdOrder.push_back(part->id());
    }

    // Now we can define ordering function based on reference order
    auto comparator = [&referenceIdOrder] (const Part* partA, const Part* partB) {
        const ID& idA = partA->id();
        const ID& idB = partB->id();

        auto itA = std::find(referenceIdOrder.begin(), referenceIdOrder.end(), idA);
        auto itB = std::find(referenceIdOrder.begin(), referenceIdOrder.end(), idB);

        auto idxA = std::distance(referenceIdOrder.begin(), itA);
        auto idxB = std::distance(referenceIdOrder.begin(), itB);

        return idxA < idxB;
    };

    std::stable_sort(parts.begin(), parts.end(), comparator);
}

void LayoutPanelTreeModel::setLayoutPanelVisible(bool visible)
{
    if (m_layoutPanelVisible == visible) {
        return;
    }

    m_layoutPanelVisible = visible;

    if (visible) {
        updateSelectedRows();

        if (m_scoreChanged) {
            onScoreChanged({});
            m_shouldUpdateSystemObjectLayers = true;
            updateSystemObjectLayers();
        }
    }
}

void LayoutPanelTreeModel::selectRow(const QModelIndex& rowIndex)
{
    m_selectionModel->select(rowIndex);
}

void LayoutPanelTreeModel::clearSelection()
{
    m_selectionModel->clear();
}

void LayoutPanelTreeModel::addInstruments()
{
    commandDispatcher()->dispatch(INSTRUMENTS_SELECT_COMMAND);
}

void LayoutPanelTreeModel::addSystemMarkings()
{
    if (const Staff* staff = resolveNewSystemObjectStaff()) {
        m_systemStaffToSelect = staff->id();
        m_masterNotation->parts()->addSystemObjects({ staff->id() });
    }
}

void LayoutPanelTreeModel::moveSelectedRowsUp()
{
    if (!m_isMovingUpAvailable) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();
    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](QModelIndex f, QModelIndex s) -> bool {
        return f.row() < s.row();
    });

    const QModelIndex& sourceRowFirst = selectedIndexList.first();
    const int count = movedRowCount(selectedIndexList);
    int destinationRow = skipOtherSharedGroup(sourceRowFirst, sourceRowFirst.row() - 1, /*up*/ true);

    // Nothing goes above the first system markings row
    if (!sourceRowFirst.parent().isValid() && m_rootItem && m_rootItem->childType(0) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        destinationRow = std::max(destinationRow, 1);
    }
    if (destinationRow >= sourceRowFirst.row()) {
        return;
    }

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), count, sourceRowFirst.parent(), destinationRow);
}

void LayoutPanelTreeModel::moveSelectedRowsDown()
{
    if (!m_isMovingDownAvailable) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();
    if (selectedIndexList.isEmpty()) {
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](const QModelIndex& f, const QModelIndex& s) -> bool {
        return f.row() < s.row();
    });

    const QModelIndex& sourceRowFirst = selectedIndexList.first();
    const int count = movedRowCount(selectedIndexList);
    const int destinationRow = skipOtherSharedGroup(sourceRowFirst, sourceRowFirst.row() + count, /*up*/ false);

    moveRows(sourceRowFirst.parent(), sourceRowFirst.row(), count, sourceRowFirst.parent(), destinationRow);
}

int LayoutPanelTreeModel::movedRowCount(const QModelIndexList& sortedSelectedIndexes) const
{
    const int selectedCount = sortedSelectedIndexes.count();
    const QModelIndex& last = sortedSelectedIndexes.last();
    if (last.parent().isValid() || !m_rootItem) {
        return selectedCount;
    }

    // A selected group header (at the end of the selection) brings its instruments along
    const AbstractLayoutPanelTreeItem* lastItem = m_rootItem->childAtRow(last.row());
    if (!lastItem || lastItem->sharedGroupRole() != LayoutPanelItemType::GROUP_HEADER) {
        return selectedCount;
    }

    int lastRow = last.row();
    while (lastRow + 1 < m_rootItem->childCount()) {
        const AbstractLayoutPanelTreeItem* next = m_rootItem->childAtRow(lastRow + 1);
        const bool isMember = next->sharedGroupRole() == LayoutPanelItemType::GROUP_MEMBER
                              || next->sharedGroupRole() == LayoutPanelItemType::GROUP_LAST_MEMBER;
        if (!isMember || next->sharedGroupId() != lastItem->sharedGroupId()) {
            break;
        }
        ++lastRow;
    }

    return lastRow - sortedSelectedIndexes.first().row() + 1;
}

int LayoutPanelTreeModel::skipOtherSharedGroup(const QModelIndex& movedIndex, int destinationRow, bool up) const
{
    // Only top level rows (instruments, groups, system markings) can be moved past groups
    if (movedIndex.parent().isValid() || !m_rootItem) {
        return destinationRow;
    }

    auto isMember = [](const AbstractLayoutPanelTreeItem* item) {
        return item->sharedGroupRole() == LayoutPanelItemType::GROUP_MEMBER
               || item->sharedGroupRole() == LayoutPanelItemType::GROUP_LAST_MEMBER;
    };

    const AbstractLayoutPanelTreeItem* moved = m_rootItem->childAtRow(movedIndex.row());
    const QString movedGroupId = moved && isMember(moved) ? moved->sharedGroupId() : QString();

    const AbstractLayoutPanelTreeItem* destination = m_rootItem->childAtRow(destinationRow);
    if (!destination || destination->sharedGroupId().isEmpty() || destination->sharedGroupId() == movedGroupId) {
        return destinationRow;
    }

    const QString groupId = destination->sharedGroupId();

    if (up) {
        // Moving up from below another group: go above its header, rather than in among its instruments
        if (!isMember(destination)) {
            return destinationRow;
        }
        while (destinationRow > 0 && m_rootItem->childAtRow(destinationRow)->sharedGroupId() == groupId
               && isMember(m_rootItem->childAtRow(destinationRow))) {
            --destinationRow;
        }
        return destinationRow; // the group's header
    }

    // Moving down onto another group's header: go after its last instrument
    if (destination->sharedGroupRole() != LayoutPanelItemType::GROUP_HEADER) {
        return destinationRow;
    }
    while (destinationRow + 1 < m_rootItem->childCount()) {
        const AbstractLayoutPanelTreeItem* next = m_rootItem->childAtRow(destinationRow + 1);
        if (next->sharedGroupId() != groupId || !isMember(next)) {
            break;
        }
        ++destinationRow;
    }
    return destinationRow;
}

void LayoutPanelTreeModel::removeSelectedRows()
{
    if (!m_isRemovingAvailable) {
        return;
    }

    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();
    if (selectedIndexList.empty()) {
        return;
    }

    QModelIndex firstIndex = *std::min_element(selectedIndexList.cbegin(), selectedIndexList.cend(),
                                               [](const QModelIndex& f, const QModelIndex& s) {
        return f.row() < s.row();
    });

    removeRows(firstIndex.row(), selectedIndexList.size(), firstIndex.parent());
}

bool LayoutPanelTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                                    int destinationChild)
{
    setLoadingBlocked(true);
    DEFER {
        setLoadingBlocked(false);
    };

    AbstractLayoutPanelTreeItem* sourceParentItem = modelIndexToItem(sourceParent);
    AbstractLayoutPanelTreeItem* destinationParentItem = modelIndexToItem(destinationParent);

    if (!sourceParentItem) {
        sourceParentItem = m_rootItem;
    }

    if (!destinationParentItem) {
        destinationParentItem = m_rootItem;
    }

    int sourceFirstRow = sourceRow;
    int sourceLastRow = sourceRow + count - 1;

    if (sourceRow < 0 || count <= 0 || destinationChild < 0) {
        return false;
    }

    if (sourceRow == destinationChild) {
        return false;
    }

    int destinationRow = (sourceLastRow > destinationChild) ? destinationChild : destinationChild + 1;

    if (sourceParentItem && destinationParentItem) {
        int maxRow = sourceParentItem->childCount() - 1;

        if (sourceLastRow > maxRow || destinationRow > maxRow + 1) {
            return false;
        }
    }

    if (!sourceParentItem || !destinationParentItem) {
        return false;
    }

    if (sourceFirstRow >= sourceParentItem->childCount()) {
        return false;
    }

    // Empty groups only exist in the panel: moving them, or moving rows past nothing but them, doesn't change the score
    bool panelOnly = false;
    if (sourceParentItem == m_rootItem && destinationParentItem == m_rootItem) {
        auto isEmptyGroup = [this](int row) {
            return m_rootItem->childType(row) == LayoutPanelItemType::EMPTY_SHARED_GROUP;
        };

        bool movingEmptyGroups = true;
        for (int row = sourceFirstRow; row <= sourceLastRow; ++row) {
            movingEmptyGroups = movingEmptyGroups && isEmptyGroup(row);
        }

        bool passingEmptyGroupsOnly = true;
        const int passedFirst = destinationRow < sourceFirstRow ? destinationRow : sourceLastRow + 1;
        const int passedLast = destinationRow < sourceFirstRow ? sourceFirstRow - 1 : destinationRow - 1;
        for (int row = passedFirst; row <= passedLast; ++row) {
            passingEmptyGroupsOnly = passingEmptyGroupsOnly && isEmptyGroup(row);
        }

        panelOnly = movingEmptyGroups || passingEmptyGroupsOnly;
    }

    if (m_dragInProgress && !panelOnly) {
        MoveParams params = sourceParentItem->buildMoveParams(sourceRow, count, destinationParentItem, destinationRow);
        if (!params.isValid()) {
            return false;
        }

        m_dragSourceParentItem = sourceParentItem;
        m_activeDragMoveParams = params;
    }

    beginMoveRows(sourceParent, sourceFirstRow, sourceLastRow, destinationParent, destinationRow);
    sourceParentItem->moveChildren(sourceFirstRow, count, destinationParentItem, destinationRow, !m_dragInProgress && !panelOnly);
    endMoveRows();

    updateEmptySharedGroupPositions();

    // The selection follows the moved rows, but the items' selected state must be brought in line
    syncItemsSelection();

    if (!m_dragInProgress) {
        updateSystemObjectLayers();
    }

    updateSharedGroupRoles();

    updateRearrangementAvailability();

    return true;
}

void LayoutPanelTreeModel::startActiveDrag()
{
    m_dragInProgress = true;
}

void LayoutPanelTreeModel::cancelActiveDrag()
{
    m_dragSourceParentItem = nullptr;
    m_activeDragMoveParams = MoveParams();
    m_dragInProgress = false;
}

void LayoutPanelTreeModel::endActiveDragLeavingSharedGroup(const QString& partId)
{
    const MoveParams params = m_activeDragMoveParams;
    cancelActiveDrag();

    INotationPartsPtr parts = m_masterNotation ? m_masterNotation->parts() : nullptr;
    if (!parts) {
        return;
    }

    const bool moved = params.isValid() && params.objectsType == LayoutPanelItemType::PART;

    keepEmptiedSharedGroups({ partId }, QString());
    parts->removePartFromSharedPart(ID(partId), moved ? params.destinationObjectId : ID(), params.insertMode);
}

void LayoutPanelTreeModel::endActiveDrag()
{
    setLoadingBlocked(true);

    if (m_dragSourceParentItem) {
        m_dragSourceParentItem->moveChildrenOnScore(m_activeDragMoveParams);
    }

    m_dragSourceParentItem = nullptr;
    m_activeDragMoveParams = MoveParams();
    m_dragInProgress = false;

    setLoadingBlocked(false);

    updateSystemObjectLayers();
}

void LayoutPanelTreeModel::changeVisibilityOfSelectedRows(bool visible)
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    for (const QModelIndex& index : selectedIndexes) {
        changeVisibility(index, visible);
    }
}

void LayoutPanelTreeModel::changeVisibility(const QModelIndex& index, bool visible)
{
    setLoadingBlocked(true);

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    item->setIsVisible(visible);

    setLoadingBlocked(false);
}

void LayoutPanelTreeModel::changeEnabledOfSelectedRows(bool enable)
{
    const QModelIndexList selectedIndices = m_selectionModel->selectedIndexes();
    for (const QModelIndex& index : selectedIndices) {
        changeEnabled(index, enable);
    }
}

void LayoutPanelTreeModel::changeEnabled(const QModelIndex& index, bool enable)
{
    setLoadingBlocked(true);

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    item->setIsEnabled(enable);

    setLoadingBlocked(false);
}

QItemSelectionModel* LayoutPanelTreeModel::selectionModel() const
{
    return m_selectionModel;
}

QModelIndex LayoutPanelTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    const AbstractLayoutPanelTreeItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    AbstractLayoutPanelTreeItem* childItem = parentItem->childAtRow(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex LayoutPanelTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    const AbstractLayoutPanelTreeItem* childItem = modelIndexToItem(child);
    AbstractLayoutPanelTreeItem* parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int LayoutPanelTreeModel::rowCount(const QModelIndex& parent) const
{
    const AbstractLayoutPanelTreeItem* parentItem = m_rootItem;

    if (parent.isValid()) {
        parentItem = modelIndexToItem(parent);
    }

    return parentItem ? parentItem->childCount() : 0;
}

int LayoutPanelTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant LayoutPanelTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != ItemRole) {
        return QVariant();
    }

    AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
    return item ? QVariant::fromValue<AbstractLayoutPanelTreeItem*>(item) : QVariant();
}

QHash<int, QByteArray> LayoutPanelTreeModel::roleNames() const
{
    return { { ItemRole, "item" } };
}

void LayoutPanelTreeModel::setIsMovingUpAvailable(bool isMovingUpAvailable)
{
    if (m_isMovingUpAvailable == isMovingUpAvailable) {
        return;
    }

    m_isMovingUpAvailable = isMovingUpAvailable;
    emit isMovingUpAvailableChanged(m_isMovingUpAvailable);
}

void LayoutPanelTreeModel::setIsMovingDownAvailable(bool isMovingDownAvailable)
{
    if (m_isMovingDownAvailable == isMovingDownAvailable) {
        return;
    }

    m_isMovingDownAvailable = isMovingDownAvailable;
    emit isMovingDownAvailableChanged(m_isMovingDownAvailable);
}

bool LayoutPanelTreeModel::isMovingUpAvailable() const
{
    return m_isMovingUpAvailable;
}

bool LayoutPanelTreeModel::isMovingDownAvailable() const
{
    return m_isMovingDownAvailable;
}

bool LayoutPanelTreeModel::isRemovingAvailable() const
{
    return m_isRemovingAvailable;
}

bool LayoutPanelTreeModel::isAddingAvailable() const
{
    return m_notation != nullptr;
}

bool LayoutPanelTreeModel::isAddingSystemMarkingsAvailable() const
{
    return m_isAddingSystemMarkingsAvailable;
}

bool LayoutPanelTreeModel::isEmpty() const
{
    return m_rootItem ? m_rootItem->isEmpty() : true;
}

int LayoutPanelTreeModel::selectedItemsType() const
{
    return static_cast<int>(m_selectedItemsType);
}

QString LayoutPanelTreeModel::scoreName() const
{
    return m_notation ? m_notation->name() : QString();
}

bool LayoutPanelTreeModel::isHideEmptyStavesEnabled() const
{
    return m_isHideEmptyStavesEnabled;
}

bool LayoutPanelTreeModel::isStaveSharingEnabled() const
{
    return m_isStaveSharingEnabled;
}

QString LayoutPanelTreeModel::addInstrumentsKeyboardShortcut() const
{
    muse::shortcuts::Shortcut shortcut = commandShortcutsRegister()->shortcut(INSTRUMENTS_SELECT_COMMAND);
    if (shortcut.sequences.empty()) {
        return {};
    }

    return muse::shortcuts::sequencesToNativeText({ shortcut.sequences[0] });
}

void LayoutPanelTreeModel::setIsRemovingAvailable(bool isRemovingAvailable)
{
    if (m_isRemovingAvailable == isRemovingAvailable) {
        return;
    }

    m_isRemovingAvailable = isRemovingAvailable;
    emit isRemovingAvailableChanged(m_isRemovingAvailable);
}

void LayoutPanelTreeModel::updateRearrangementAvailability()
{
    QModelIndexList selectedIndexList = m_selectionModel->selectedIndexes();

    if (selectedIndexList.isEmpty()) {
        updateMovingUpAvailability(false);
        updateMovingDownAvailability(false);
        return;
    }

    std::sort(selectedIndexList.begin(), selectedIndexList.end(), [](const QModelIndex& f, const QModelIndex& s) -> bool {
        return f.row() < s.row();
    });

    bool isRearrangementAvailable = true;

    QMutableListIterator<QModelIndex> it(selectedIndexList);
    LayoutPanelItemType::ItemType prevItemType = LayoutPanelItemType::UNDEFINED;

    while (it.hasNext() && selectedIndexList.count() > 1) {
        int nextRow = it.next().row();
        int previousRow = it.peekPrevious().row();

        isRearrangementAvailable = (nextRow - previousRow <= 1);

        if (isRearrangementAvailable) {
            const AbstractLayoutPanelTreeItem* item = modelIndexToItem(it.value());

            if (prevItemType != LayoutPanelItemType::UNDEFINED) {
                isRearrangementAvailable = item->type() == prevItemType;
            }

            prevItemType = item->type();
        }

        if (!isRearrangementAvailable) {
            updateMovingUpAvailability(isRearrangementAvailable);
            updateMovingDownAvailability(isRearrangementAvailable);
            return;
        }
    }

    updateMovingUpAvailability(isRearrangementAvailable, selectedIndexList.first());
    updateMovingDownAvailability(isRearrangementAvailable, selectedIndexList.last());
}

void LayoutPanelTreeModel::updateMovingUpAvailability(bool isSelectionMovable, const QModelIndex& firstSelectedRowIndex)
{
    const AbstractLayoutPanelTreeItem* parentItem = modelIndexToItem(firstSelectedRowIndex.parent());
    if (!parentItem) {
        parentItem = m_rootItem;
    }

    // can't go above the topmost system objects layer
    int minAllowedRow = 0;
    if (parentItem->childType(0) == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
        minAllowedRow = 1;
    }

    bool isRowInBoundaries = firstSelectedRowIndex.isValid() && firstSelectedRowIndex.row() > minAllowedRow;

    setIsMovingUpAvailable(isSelectionMovable && isRowInBoundaries);
}

void LayoutPanelTreeModel::updateMovingDownAvailability(bool isSelectionMovable, const QModelIndex& lastSelectedRowIndex)
{
    const AbstractLayoutPanelTreeItem* parentItem = modelIndexToItem(lastSelectedRowIndex.parent());
    if (!parentItem) {
        parentItem = m_rootItem;
    }

    // exclude the control item
    bool hasControlItem = parentItem->type() != LayoutPanelItemType::ROOT;
    const AbstractLayoutPanelTreeItem* curItem = modelIndexToItem(lastSelectedRowIndex);
    bool lastSelectedIsSystemObjectLayer = curItem && curItem->type() == LayoutPanelItemType::ItemType::SYSTEM_OBJECTS_LAYER;

    if (lastSelectedRowIndex.row() == 0 && lastSelectedIsSystemObjectLayer) {
        // Selecting/moving the top system object layer not allowed
        setIsMovingDownAvailable(false);
        return;
    }

    int lastItemRowIndex = parentItem->childCount() - 1 - (hasControlItem ? 1 : 0);

    bool isRowInBoundaries = lastSelectedRowIndex.isValid() && lastSelectedRowIndex.row() < lastItemRowIndex;

    setIsMovingDownAvailable(isSelectionMovable && isRowInBoundaries);
}

void LayoutPanelTreeModel::updateRemovingAvailability()
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();

    bool isRemovingAvailable = !selectedIndexes.empty();
    const AbstractLayoutPanelTreeItem* parent = nullptr;

    for (const QModelIndex& index : selectedIndexes) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        isRemovingAvailable = item && item->isRemovable();

        //! NOTE: all selected items must have the same parent
        if (isRemovingAvailable && parent) {
            if (parent != item->parentItem()) {
                isRemovingAvailable = false;
            }
        }

        if (!isRemovingAvailable) {
            break;
        }

        parent = item->parentItem();
    }

    //! NOTE: the user is allowed to remove all children only for the root item
    if (isRemovingAvailable && parent != m_rootItem) {
        int removableChildCount = 0;

        for (const AbstractLayoutPanelTreeItem* item : parent->childItems()) {
            if (item->isRemovable()) {
                removableChildCount++;
            }
        }

        isRemovingAvailable = selectedIndexes.size() < removableChildCount;
    }

    setIsRemovingAvailable(isRemovingAvailable);
}

void LayoutPanelTreeModel::updateSelectedItemsType()
{
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();

    LayoutPanelItemType::ItemType selectedItemsType = LayoutPanelItemType::ItemType::UNDEFINED;

    for (const QModelIndex& index : selectedIndexes) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        if (item) {
            selectedItemsType = item->type();
            break;
        }
    }

    if (m_selectedItemsType != selectedItemsType) {
        m_selectedItemsType = selectedItemsType;
        emit selectedItemsTypeChanged(static_cast<int>(m_selectedItemsType));
    }
}

void LayoutPanelTreeModel::updateIsAddingSystemMarkingsAvailable()
{
    bool addingSysMarkAvailable = false;

    if (!isAddingAvailable() || !isMasterScore()) {
        addingSysMarkAvailable = false;
    } else {
        int systemLayerCount = 0;
        for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
            if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                ++systemLayerCount;
            }
        }

        addingSysMarkAvailable = systemLayerCount < 0.5 * m_rootItem->childCount();
    }

    if (addingSysMarkAvailable != m_isAddingSystemMarkingsAvailable) {
        m_isAddingSystemMarkingsAvailable = addingSysMarkAvailable;
        emit isAddingSystemMarkingsAvailableChanged(m_isAddingSystemMarkingsAvailable);
    }
}

void LayoutPanelTreeModel::updateIsStaveSharingEnabled()
{
    bool enabled = m_notation->style()->styleValue(StyleId::enableStaveSharing).toBool();
    if (enabled == m_isStaveSharingEnabled) {
        return;
    }

    m_isStaveSharingEnabled = enabled;
    emit isStaveSharingEnabledChanged(enabled);
}

void LayoutPanelTreeModel::updateIsHideEmptyStavesEnabled()
{
    bool enabled = m_notation && m_notation->style()->styleValue(StyleId::hideEmptyStaves).toBool();
    if (enabled == m_isHideEmptyStavesEnabled) {
        return;
    }

    m_isHideEmptyStavesEnabled = enabled;
    emit isHideEmptyStavesEnabledChanged(enabled);
}

void LayoutPanelTreeModel::updateSharedGroupRoles()
{
    if (!m_rootItem) {
        return;
    }

    // Shared parts are inserted directly before their origin parts, so a group is
    // a SHARED_PART row followed by the consecutive PART rows that belong to it.
    const QList<AbstractLayoutPanelTreeItem*>& items = m_rootItem->childItems();
    const Part* currentSharedPart = nullptr;
    AbstractLayoutPanelTreeItem* lastMember = nullptr;

    auto closeGroup = [&]() {
        if (lastMember) {
            lastMember->setSharedGroupRole(LayoutPanelItemType::GROUP_LAST_MEMBER);
        }
        currentSharedPart = nullptr;
        lastMember = nullptr;
    };

    for (AbstractLayoutPanelTreeItem* item : items) {
        if (item->type() == LayoutPanelItemType::EMPTY_SHARED_GROUP) {
            // A group of its own, without instruments
            closeGroup();
            item->setSharedGroupRole(LayoutPanelItemType::GROUP_HEADER);
            item->setSharedGroupId(item->idStr());
            continue;
        }

        int role = LayoutPanelItemType::NOT_IN_GROUP;

        if (item->type() == LayoutPanelItemType::SHARED_PART) {
            closeGroup();
            currentSharedPart = static_cast<PartTreeItem*>(item)->part();
            role = LayoutPanelItemType::GROUP_HEADER;
        } else if (item->type() == LayoutPanelItemType::PART && currentSharedPart) {
            const Part* part = static_cast<PartTreeItem*>(item)->part();
            if (part && part->sharedPart() == currentSharedPart) {
                role = LayoutPanelItemType::GROUP_MEMBER;
                lastMember = item;
            } else {
                closeGroup();
            }
        } else {
            closeGroup();
        }

        const QString groupId = currentSharedPart ? currentSharedPart->id().toQString() : QString();

        item->setSharedGroupRole(role);
        item->setSharedGroupId(groupId);
        for (AbstractLayoutPanelTreeItem* child : item->childItems()) {
            child->setSharedGroupRole(role);
            child->setSharedGroupId(groupId);
        }
    }

    closeGroup();

    // Children of the last member inherit its role, so that the group background closes below them
    for (AbstractLayoutPanelTreeItem* item : items) {
        if (item->sharedGroupRole() == LayoutPanelItemType::GROUP_LAST_MEMBER) {
            for (AbstractLayoutPanelTreeItem* child : item->childItems()) {
                child->setSharedGroupRole(LayoutPanelItemType::GROUP_LAST_MEMBER);
            }
        }
    }
}

void LayoutPanelTreeModel::syncItemsSelection()
{
    if (!m_rootItem) {
        return;
    }

    // The selection deltas reported around row moves can refer to rows' old positions,
    // so derive each item's selected state from the selection model itself
    std::function<void(AbstractLayoutPanelTreeItem*)> sync = [&](AbstractLayoutPanelTreeItem* parent) {
        for (AbstractLayoutPanelTreeItem* item : parent->childItems()) {
            if (item->isSelectable()) {
                item->setIsSelected(m_selectionModel->isSelected(createIndex(item->row(), 0, item)));
            }
            sync(item);
        }
    };

    sync(m_rootItem);
}

void LayoutPanelTreeModel::setItemsSelected(const QModelIndexList& indexes, bool selected)
{
    for (const QModelIndex& index : indexes) {
        if (AbstractLayoutPanelTreeItem* item = modelIndexToItem(index)) {
            if (!item->isSelectable()) {
                continue;
            }
            item->setIsSelected(selected);
        }
    }
}

bool LayoutPanelTreeModel::needWarnOnRemoveRows(int row, int count)
{
    for (int i = row + count - 1; i >= row; --i) {
        if (m_rootItem->childType(i) != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            return true;
        }
    }

    return false;
}

bool LayoutPanelTreeModel::warnAboutRemovingInstrumentsIfNecessary(int count)
{
    // Only warn if excerpts are existent
    if (m_masterNotation->excerpts().empty()) {
        return true;
    }

    return interactive()->warningSync(
        muse::trc("layoutpanel", "Are you sure you want to delete the selected instrument(s)?", nullptr, count),
        muse::trc("layoutpanel", "This will remove the instrument(s) from the full score and all part scores.", nullptr, count),
        { IInteractive::Button::No, IInteractive::Button::Yes }
        )
           .standardButton() == IInteractive::Button::Yes;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildSharedPartItem(const Part* part)
{
    auto sharedPartItem = new SharedPartTreeItem(m_masterNotation, m_notation, m_rootItem);
    sharedPartItem->init(part);

    return sharedPartItem;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildMasterPartItem(const Part* masterPart)
{
    TRACEFUNC;

    auto partItem = new PartTreeItem(m_masterNotation, m_notation, m_rootItem);
    partItem->init(masterPart);

    for (const Staff* staff : m_masterNotation->parts()->staffList(partItem->id())) {
        auto staffItem = buildMasterStaffItem(staff, partItem);
        partItem->appendChild(staffItem);
    }

    auto addStaffControlItem = buildAddStaffControlItem(partItem->id(), partItem);
    partItem->appendChild(addStaffControlItem);

    setupStavesConnections(partItem->id());

    return partItem;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildMasterStaffItem(const Staff* masterStaff, QObject* parent)
{
    auto result = new StaffTreeItem(m_masterNotation, m_notation, parent);
    result->init(masterStaff);

    return result;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildSystemObjectsLayerItem(const mu::notation::Staff* masterStaff,
                                                                               const SystemObjectGroups& systemObjects)
{
    auto result = new SystemObjectsLayerTreeItem(m_masterNotation, m_notation, m_rootItem);
    result->init(masterStaff, systemObjects);

    return result;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::buildAddStaffControlItem(const muse::ID& partId, QObject* parent)
{
    auto result = new StaffControlTreeItem(m_masterNotation, m_notation, parent);
    result->init(partId);

    return result;
}

AbstractLayoutPanelTreeItem* LayoutPanelTreeModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<AbstractLayoutPanelTreeItem*>(index.internalPointer());
}

void LayoutPanelTreeModel::updateSystemObjectLayers()
{
    if (!m_masterNotation || !m_rootItem || !m_shouldUpdateSystemObjectLayers) {
        return;
    }

    if (!shouldShowSystemObjectLayers()) {
        return;
    }

    TRACEFUNC;

    m_shouldUpdateSystemObjectLayers = false;

    // Create copy, because we're going to modify them
    const INotationPartsPtr notationParts = m_masterNotation->notation()->parts();
    std::vector<Staff*> newSystemObjectStaves = notationParts->systemObjectStaves();
    QList<AbstractLayoutPanelTreeItem*> children = m_rootItem->childItems();

    // Remove old system object layers
    std::vector<const PartTreeItem*> partItems;
    std::vector<SystemObjectsLayerTreeItem*> existingSystemObjectLayers;

    for (AbstractLayoutPanelTreeItem* item : children) {
        if (item->type() != LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
            if (item->type() == LayoutPanelItemType::PART || item->type() == LayoutPanelItemType::SHARED_PART) {
                partItems.push_back(static_cast<const PartTreeItem*>(item));
            }
            continue;
        }

        auto layerItem = static_cast<SystemObjectsLayerTreeItem*>(item);
        if (muse::remove(newSystemObjectStaves, const_cast<Staff*>(layerItem->staff()))) {
            existingSystemObjectLayers.push_back(layerItem);
            continue;
        }

        int row = layerItem->row();
        beginRemoveRows(QModelIndex(), row, row);
        m_rootItem->removeChildren(row, 1, false);
        endRemoveRows();
    }

    SystemObjectGroupsByStaff systemObjects = collectSystemObjectGroups(notationParts->systemObjectStaves());

    // Update position of existing layers if changed
    for (SystemObjectsLayerTreeItem* layerItem : existingSystemObjectLayers) {
        const PartTreeItem* partItem = findPartItemByStaff(layerItem->staff());
        IF_ASSERT_FAILED(partItem) {
            continue;
        }

        const int partRow = partItem->row();
        const int layerRow = layerItem->row();
        const int correctLayerRow = layerItem->staff()->hasSystemObjectsBelowBottomStaff() ? partRow + 1 : partRow - 1;

        if (layerRow != correctLayerRow) {
            beginMoveRows(QModelIndex(), layerRow, layerRow, QModelIndex(), partRow);
            m_rootItem->moveChildren(layerRow, 1, m_rootItem, partRow, false /*updateNotation*/);
            endMoveRows();
        }

        layerItem->setSystemObjects(systemObjects[layerItem->staff()]);
    }

    // Create new system object layers
    for (const Staff* staff : newSystemObjectStaves) {
        for (const PartTreeItem* partItem : partItems) {
            if (staff->part() != partItem->part()) {
                continue;
            }

            AbstractLayoutPanelTreeItem* newItem = buildSystemObjectsLayerItem(staff, systemObjects[staff]);
            int row = staff->hasSystemObjectsBelowBottomStaff() ? partItem->row() + 1 : partItem->row();

            beginInsertRows(QModelIndex(), row, row);
            m_rootItem->insertChild(newItem, row);
            endInsertRows();

            if (row != 0 && m_systemStaffToSelect == staff->id()) {
                m_selectionModel->select(createIndex(row, 0, newItem));
                m_systemStaffToSelect = ID();
            }

            break;
        }
    }

    updateIsAddingSystemMarkingsAvailable();
}

const PartTreeItem* LayoutPanelTreeModel::findPartItemByStaff(const Staff* staff) const
{
    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (item->type() != LayoutPanelItemType::PART && item->type() != LayoutPanelItemType::SHARED_PART) {
            continue;
        }

        const PartTreeItem* partItem = static_cast<const PartTreeItem*>(item);
        if (muse::contains(partItem->part()->staves(), const_cast<Staff*>(staff))) {
            return partItem;
        }
    }

    return nullptr;
}

const Staff* LayoutPanelTreeModel::resolveNewSystemObjectStaff() const
{
    const std::vector<Staff*>& systemObjectStaves = m_notation->parts()->systemObjectStaves();

    const auto resolveStaff = [&](const AbstractLayoutPanelTreeItem* item) -> const Staff* {
        if (item->type() != LayoutPanelItemType::PART) {
            return nullptr;
        }

        const Part* part = static_cast<const PartTreeItem*>(item)->part();
        for (Staff* staff : part->staves()) {
            if (muse::contains(systemObjectStaves, staff)) {
                return nullptr;
            }
        }

        return part->staff(0);
    };

    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();
    for (const QModelIndex& index : selectedIndexes) {
        const AbstractLayoutPanelTreeItem* item = modelIndexToItem(index);
        if (const Staff* staff = resolveStaff(item)) {
            return staff;
        }
    }

    for (const AbstractLayoutPanelTreeItem* item : m_rootItem->childItems()) {
        if (const Staff* staff = resolveStaff(item)) {
            return staff;
        }
    }

    return nullptr;
}

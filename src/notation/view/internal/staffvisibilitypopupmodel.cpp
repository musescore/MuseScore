/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "staffvisibilitypopupmodel.h"

#include <algorithm>
#include <iterator>
#include <memory>

#include "containers.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/staffvisibilityindicator.h"
#include "engraving/dom/system.h"
#include "engraving/types/fraction.h"
#include "engraving/types/types.h"

using namespace mu::notation;

StaffVisibilityPopupModel::StaffVisibilityPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_STAFF_VISIBILITY, parent)
    , m_emptyStavesVisibilityModel(std::make_unique<EmptyStavesVisibilityModel>(this))
{
}

void StaffVisibilityPopupModel::classBegin()
{
    init();
}

void StaffVisibilityPopupModel::init()
{
    AbstractElementPopupModel::init();

    System* system = m_item && m_item->isStaffVisibilityIndicator()
                     ? toStaffVisibilityIndicator(m_item)->system()
                     : nullptr;
    if (!system) {
        return;
    }

    m_emptyStavesVisibilityModel->load(currentNotation(), system);

    m_systemIndex = muse::indexOf(system->score()->systems(), system);
    emit systemIndexChanged();
}

struct EmptyStavesVisibilityModel::Item {
    virtual ~Item() = default;

    muse::ID id;
    QString name;
    bool isVisible = false;
    bool canReset = false;
};

struct EmptyStavesVisibilityModel::PartItem : public EmptyStavesVisibilityModel::Item {
    std::vector<std::unique_ptr<EmptyStavesVisibilityModel::StaffItem> > staves;
};

struct EmptyStavesVisibilityModel::StaffItem : public EmptyStavesVisibilityModel::Item {
    PartItem* part = nullptr;
    staff_idx_t staffIndex = 0;
};

EmptyStavesVisibilityModel::EmptyStavesVisibilityModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

EmptyStavesVisibilityModel::~EmptyStavesVisibilityModel() = default;

void EmptyStavesVisibilityModel::load(INotationPtr notation, engraving::System* system)
{
    m_notation = notation;
    m_system = system;

    reload();
}

void EmptyStavesVisibilityModel::reload()
{
    engraving::Fraction tick = m_system->tick();

    beginResetModel();
    m_parts.clear();

    staff_idx_t staffIdx = 0;
    for (const Part* part : m_system->score()->parts()) {
        auto partItem = std::make_unique<PartItem>();
        partItem->id = part->id();
        partItem->name = part->instrument(tick)->nameAsPlainText();

        for (const Staff* staff : part->staves()) {
            auto staffItem = std::make_unique<StaffItem>();
            staffItem->id = staff->id();
            staffItem->name = staff->staffName();
            staffItem->part = partItem.get();
            staffItem->staffIndex = staffIdx;

            staffItem->isVisible = m_system->staff(staffIdx)->show();
            if (staffItem->isVisible) {
                // Part is visible if any of its staves is visible
                partItem->isVisible = true;
            }

            // Staff can reset visibility if any measure in the range contains an override
            for (const engraving::MeasureBase* mb : m_system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }
                if (engraving::toMeasure(mb)->hideStaffIfEmpty(staffIdx) != engraving::AutoOnOff::AUTO) {
                    staffItem->canReset = true;
                    break;
                }
            }
            if (staffItem->canReset) {
                // Part can reset visibility if any of its staves can reset
                partItem->canReset = true;
            }

            partItem->staves.emplace_back(std::move(staffItem));
            ++staffIdx;
        }

        m_parts.emplace_back(std::move(partItem));
    }

    endResetModel();

    emit canResetAllChanged();
}

QModelIndex EmptyStavesVisibilityModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (parent.isValid()) {
        const PartItem* part = static_cast<const PartItem*>(parent.internalPointer());
        assert(part);
        return createIndex(row, column, part->staves[row].get());
    }

    return createIndex(row, column, m_parts[row].get());
}

QModelIndex EmptyStavesVisibilityModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    const Item* childItem = static_cast<const Item*>(child.internalPointer());
    assert(childItem);

    if (const StaffItem* staff = dynamic_cast<const StaffItem*>(childItem)) {
        const PartItem* part = staff->part;
        assert(part);
        int row = partIndex(part);
        return createIndex(row, 0, part);
    }

    assert(dynamic_cast<const PartItem*>(childItem));
    return QModelIndex(); // Part has no parent
}

int EmptyStavesVisibilityModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    if (parent.isValid()) {
        const PartItem* part = static_cast<const PartItem*>(parent.internalPointer());
        assert(part);
        if (part->staves.size() > 1) {
            return static_cast<int>(part->staves.size());
        }
        return 0; // If part has only one staff, it is not displayed separately
    }

    return static_cast<int>(m_parts.size());
}

int EmptyStavesVisibilityModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant EmptyStavesVisibilityModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Item* item = static_cast<const Item*>(index.internalPointer());
    assert(item);

    switch (role) {
    case Name:
        return item->name;
    case IsVisible:
        return item->isVisible;
    case CanReset:
        return item->canReset;
    default:
        return QVariant();
    }
}

bool EmptyStavesVisibilityModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != IsVisible) {
        return false;
    }

    Item* item = static_cast<Item*>(index.internalPointer());
    assert(item);

    if (StaffItem* staff = dynamic_cast<StaffItem*>(item)) {
        setStaffVisibility(staff, value.toBool() ? engraving::AutoOnOff::ON : engraving::AutoOnOff::OFF);
    } else if (PartItem* part = dynamic_cast<PartItem*>(item)) {
        setPartVisibility(part, value.toBool() ? engraving::AutoOnOff::ON : engraving::AutoOnOff::OFF);
    } else {
        return false;
    }
    return true;
}

void EmptyStavesVisibilityModel::resetVisibility(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    Item* item = static_cast<Item*>(index.internalPointer());
    assert(item);

    if (StaffItem* staff = dynamic_cast<StaffItem*>(item)) {
        setStaffVisibility(staff, engraving::AutoOnOff::AUTO);
    } else if (PartItem* part = dynamic_cast<PartItem*>(item)) {
        setPartVisibility(part, engraving::AutoOnOff::AUTO);
    }
}

void EmptyStavesVisibilityModel::resetAllVisibility()
{
    m_notation->undoStack()->prepareChanges(
        muse::TranslatableString("notation/staffvisibilitypopup", "Reset all staff visibility"));

    engraving::Score* score = m_notation->elements()->msScore();
    assert(score);

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        score->cmdSetHideStaffIfEmptyOverride(staffIdx, m_system, engraving::AutoOnOff::AUTO);
    }

    m_notation->undoStack()->commitChanges();

    reload();
}

bool EmptyStavesVisibilityModel::canResetAll() const
{
    for (const auto& part : m_parts) {
        for (const auto& staff : part->staves) {
            if (staff->canReset) {
                return true;
            }
        }
    }
    return false;
}

QHash<int, QByteArray> EmptyStavesVisibilityModel::roleNames() const
{
    QHash<int, QByteArray> roles {
        { Name, "name" },
        { IsVisible, "isVisible" },
        { CanReset, "canReset" }
    };

    return roles;
}

void EmptyStavesVisibilityModel::setPartVisibility(PartItem* partItem, engraving::AutoOnOff value)
{
    m_notation->undoStack()->prepareChanges(
        muse::TranslatableString("notation/staffvisibilitypopup", "Change part visibility: %1").arg(partItem->name));

    engraving::Score* score = m_notation->elements()->msScore();
    assert(score);

    for (const auto& staffItem : partItem->staves) {
        score->cmdSetHideStaffIfEmptyOverride(staffItem->staffIndex, m_system, value);
    }

    m_notation->undoStack()->commitChanges();

    updateData(partItem);
}

void EmptyStavesVisibilityModel::setStaffVisibility(StaffItem* staffItem, engraving::AutoOnOff value)
{
    m_notation->undoStack()->prepareChanges(
        muse::TranslatableString("notation/staffvisibilitypopup", "Change staff visibility: %1").arg(staffItem->name));

    engraving::Score* score = m_notation->elements()->msScore();
    assert(score);

    score->cmdSetHideStaffIfEmptyOverride(staffItem->staffIndex, m_system, value);

    m_notation->undoStack()->commitChanges();

    updateData(staffItem->part);
}

void EmptyStavesVisibilityModel::updateData(PartItem* partItem)
{
    partItem->isVisible = false;
    partItem->canReset = false;

    for (const auto& staffItem : partItem->staves) {
        staffItem->isVisible = m_system->staff(staffItem->staffIndex)->show();
        if (staffItem->isVisible) {
            // Part is visible if any of its staves is visible
            partItem->isVisible = true;
        }

        // Staff can reset visibility if any measure in the range contains an override
        staffItem->canReset = false;
        for (const engraving::MeasureBase* mb : m_system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            if (engraving::toMeasure(mb)->hideStaffIfEmpty(staffItem->staffIndex) != engraving::AutoOnOff::AUTO) {
                staffItem->canReset = true;
                break;
            }
        }
        if (staffItem->canReset) {
            // Part can reset visibility if any of its staves can reset
            partItem->canReset = true;
        }
    }

    int row = partIndex(partItem);
    QModelIndex partModelIndex = index(row, 0, QModelIndex());
    emit dataChanged(partModelIndex, partModelIndex, { IsVisible, CanReset });

    if (partItem->staves.size() > 1) {
        emit dataChanged(index(0, 0, partModelIndex),
                         index(static_cast<int>(partItem->staves.size()) - 1, 0, partModelIndex),
                         { IsVisible, CanReset });
    }

    emit canResetAllChanged();
}

int EmptyStavesVisibilityModel::partIndex(const PartItem* partItem) const
{
    auto it = std::find_if(m_parts.begin(), m_parts.end(), [&](const std::unique_ptr<PartItem>& item) {
        return item.get() == partItem;
    });

    return it != m_parts.end() ? std::distance(m_parts.begin(), it) : -1;
}

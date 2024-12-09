/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "systemobjectslayertreeitem.h"

#include "layoutpanelutils.h"
#include "translation.h"
#include "log.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;

static QString formatLayerTitle(const SystemObjectGroups& groups)
{
    if (groups.empty()) {
        return muse::qtrc("instruments", "System objects");
    }

    QString title;

    const size_t lastIdx = groups.size() - 1;
    for (size_t i = 0; i <= lastIdx; ++i) {
        const SystemObjectsGroup& group = groups.at(i);
        if (!isSystemObjectsGroupVisible(group)) {
            continue;
        }

        if (title.isEmpty()) {
            title = translatedSystemObjectsGroupName(group);
            continue;
        }

        if (i == lastIdx) {
            title += " & ";
        } else {
            title += ", ";
        }

        title += translatedSystemObjectsGroupName(group).toLower();
    }

    if (title.isEmpty()) {
        title = muse::qtrc("instruments", "System objects hidden");
    }

    return title;
}

static bool isLayerVisible(const SystemObjectGroups& groups)
{
    for (const SystemObjectsGroup& group : groups) {
        if (isSystemObjectsGroupVisible(group)) {
            return true;
        }
    }

    return true;
}

SystemObjectsLayerTreeItem::SystemObjectsLayerTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractLayoutPanelTreeItem(LayoutPanelItemType::SYSTEM_OBJECTS_LAYER, masterNotation, notation, parent)
{
    setIsEditable(true);
    setIsExpandable(false);
}

void SystemObjectsLayerTreeItem::init(const Staff* staff, const SystemObjectGroups& systemObjects)
{
    m_systemObjectGroups = systemObjects;

    setStaff(staff);
    setTitle(formatLayerTitle(systemObjects));
    setIsVisible(isLayerVisible(systemObjects));

    bool isTopLayer = staff->score()->staff(0) == staff;
    setIsRemovable(!isTopLayer);
    setIsSelectable(!isTopLayer);

    listenUndoStackChanged();
    listenVisibleChanged();
}

const Staff* SystemObjectsLayerTreeItem::staff() const
{
    const_cast<SystemObjectsLayerTreeItem*>(this)->updateStaff();

    return m_staff;
}

void SystemObjectsLayerTreeItem::setStaff(const Staff* staff)
{
    m_staff = staff;

    if (staff) {
        setId(staff->id());
        m_staffIdx = staff->idx(); // optimization
    } else {
        setId(ID());
        m_staffIdx = muse::nidx;
    }
}

QString SystemObjectsLayerTreeItem::staffId() const
{
    const Staff* s = staff();
    return s ? s->id().toQString() : QString();
}

bool SystemObjectsLayerTreeItem::canAcceptDrop(const QVariant&) const
{
    return false;
}

void SystemObjectsLayerTreeItem::listenUndoStackChanged()
{
    notation()->undoStack()->changesChannel().onReceive(this, [this](const ChangesRange& changes) {
        if (muse::contains(changes.changedPropertyIdSet, Pid::TRACK)) {
            updateStaff();
        }

        if (changes.staffIdxFrom > m_staffIdx || changes.staffIdxTo < m_staffIdx) {
            return;
        }

        for (const EngravingItem* item : changes.changedItems) {
            if (!item->systemFlag() || item->staffIdx() != m_staffIdx) {
                continue;
            }

            // TODO: optimize
            m_systemObjectGroups = collectSystemObjectGroups(m_staff);
            setTitle(formatLayerTitle(m_systemObjectGroups));

            m_ignoreVisibilityChanges = true;
            setIsVisible(isLayerVisible(m_systemObjectGroups));
            m_ignoreVisibilityChanges = false;

            return;
        }
    });
}

void SystemObjectsLayerTreeItem::listenVisibleChanged()
{
    connect(this, &AbstractLayoutPanelTreeItem::isVisibleChanged, this, [this](bool isVisible) {
        if (m_ignoreVisibilityChanges || m_systemObjectGroups.empty()) {
            return;
        }

        const muse::TranslatableString actionName = isVisible
                                                    ? TranslatableString("undoableAction", "Make system object(s) visible")
                                                    : TranslatableString("undoableAction", "Make system object(s) invisible");

        notation()->undoStack()->prepareChanges(actionName);

        for (const SystemObjectsGroup& group : m_systemObjectGroups) {
            for (engraving::EngravingItem* item : group.items) {
                item->undoSetVisible(isVisible);
            }
        }

        notation()->undoStack()->commitChanges();
        notation()->notationChanged().notify();
    });
}

void SystemObjectsLayerTreeItem::updateStaff()
{
    if (!m_systemObjectGroups.empty()) {
        const SystemObjectsGroup& firstGroup = m_systemObjectGroups.front();
        if (!firstGroup.items.empty()) {
            setStaff(firstGroup.items.front()->staff());
        }
    }
}

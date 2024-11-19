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

static QString formatLayerTitle(const std::vector<SystemObjectsGroup>& groups)
{
    if (groups.empty()) {
        return muse::qtrc("instruments", "System objects");
    }

    QString title;

    const size_t lastIdx = groups.size() - 1;
    for (size_t i = 0; i <= lastIdx; ++i) {
        if (i == 0) {
            title = groups.at(i).name.translated();
        } else {
            if (i == lastIdx) {
                title += " & ";
            } else {
                title += ", ";
            }

            title += groups.at(i).name.translated().toLower();
        }
    }

    return title;
}

static bool isLayerVisible(const std::vector<SystemObjectsGroup>& groups)
{
    for (const SystemObjectsGroup& group : groups) {
        if (isSystemObjectsGroupVisible(group)) {
            return true;
        }
    }

    return true;
}

SystemObjectsLayerTreeItem::SystemObjectsLayerTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : AbstractLayoutPanelTreeItem(LayoutPanelItemType::ItemType::SYSTEM_OBJECTS_LAYER, masterNotation, notation, parent)
{
    setIsEditable(true);
    setIsExpandable(false);
}

void SystemObjectsLayerTreeItem::init(const Staff* staff, bool isTopLayer)
{
    m_staff = staff;
    std::vector<SystemObjectsGroup> groups = collectSystemObjectGroups(staff);

    setId(staff->id());
    setTitle(formatLayerTitle(groups));
    setIsVisible(isLayerVisible(groups));
    setIsRemovable(!isTopLayer);
    setIsSelectable(!isTopLayer);
}

const Staff* SystemObjectsLayerTreeItem::staff() const
{
    return m_staff;
}

QString SystemObjectsLayerTreeItem::staffId() const
{
    return m_staff ? m_staff->id().toQString() : QString();
}

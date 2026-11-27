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
#include "sharedparttreeitem.h"

#include <algorithm>

#include "engraving/dom/sharedpart.h"

#include "notation/inotation.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

SharedPartTreeItem::SharedPartTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : PartTreeItem(masterNotation, notation, parent, LayoutPanelItemType::ItemType::SHARED_PART)
{
    listenEnabledChanged();
}

void SharedPartTreeItem::init(const notation::Part* part)
{
    PartTreeItem::init(part);

    updateEnabled();

    m_ignoreEnabledChanged = false;
}

void SharedPartTreeItem::applyDrop(const AbstractLayoutPanelTreeItem* dragged)
{
    notation()->parts()->movePartsToSharedPart({ dragged->id() }, id());
}

bool SharedPartTreeItem::isSelectedInScore(const std::vector<mu::engraving::EngravingItem*>&) const
{
    return false;
}

void SharedPartTreeItem::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    updateVisibility();

    if (isTitleAffected(changes)) {
        setTitle(part()->partName().simplified());
    }

    m_ignoreEnabledChanged = true;
    updateEnabled();
    m_ignoreEnabledChanged = false;
}

void SharedPartTreeItem::updateEnabled()
{
    const bool enabled = engraving::toSharedPart(part())->enabled();
    setIsExpandable(enabled);
    setIsEnabled(enabled);
}

void SharedPartTreeItem::listenEnabledChanged()
{
    connect(this, &AbstractLayoutPanelTreeItem::isEnabledChanged, this, [this](bool enable) {
        if (m_ignoreEnabledChanged) {
            return;
        }

        INotationPartsPtr parts = notation()->parts();
        if (!parts) {
            return;
        }

        if (parts->partExists(id())) {
            parts->setSharedPartEnabled(id(), enable);
        }
    });
}

bool SharedPartTreeItem::isTitleAffected(const mu::engraving::ScoreChanges& changes) const
{
    const engraving::SharedPart* sharedPart = engraving::toSharedPart(part());
    const std::vector<engraving::Part*>& originParts = sharedPart->originParts();

    for (const auto& changed : changes.changedObjects) {
        const engraving::EngravingObject* object = changed.first;
        if (object == sharedPart || std::find(originParts.cbegin(), originParts.cend(), object) != originParts.cend()) {
            return true;
        }
    }

    return false;
}

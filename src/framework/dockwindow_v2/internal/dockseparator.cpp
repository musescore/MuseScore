/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "dockseparator.h"

#include "log.h"
#include "../docktypes.h"

#include "kddockwidgets/src/core/Separator.h"
#include "kddockwidgets/src/core/DockRegistry.h"
#include "kddockwidgets/src/core/Group.h"
#include "kddockwidgets/src/core/DockWidget.h"
#include "kddockwidgets/src/core/layouting/LayoutingSeparator_p.h"

// This include pulls in kdbindings/signal.h which is incompatible with
// Qt's `emit` macro being defined. Temporarily undefine it, then restore it.
#ifdef emit
#undef emit
#include "kddockwidgets/src/core/layouting/Item_p.h"
#define emit
#else
#include "kddockwidgets/src/core/layouting/Item_p.h"
#endif

#include <QTimer>

using namespace muse::dock;
using namespace KDDockWidgets;

namespace muse::dock {
static const QObject* findNearestDockView(const DockSeparator* separator)
{
    auto* coreSeparator = static_cast<Core::Separator*>(separator->controller());
    auto* layoutingSeparator = coreSeparator->asLayoutingSeparator();
    auto* container = layoutingSeparator->parentContainer();
    if (!container) {
        return nullptr;
    }

    int separatorPos = layoutingSeparator->position();
    Qt::Orientation orientation = layoutingSeparator->orientation();
    Core::Item::List children = container->visibleChildren();

    const Core::Item* nearestItem = nullptr;
    int minPosDiff = std::numeric_limits<int>::max();

    for (const Core::Item* child : children) {
        int childPos = child->pos(orientation);
        int diff = std::abs(childPos - separatorPos);
        if (diff < minPosDiff) {
            nearestItem = child;
            minPosDiff = diff;
        }
    }

    if (!nearestItem) {
        return nullptr;
    }

    auto* guest = nearestItem->guest();
    for (Core::Group* group : DockRegistry::self()->groups()) {
        if (group->asLayoutingGuest() == guest) {
            if (group->isEmpty()) {
                return nullptr;
            }

            return group->currentDockWidget();
        }
    }

    return nullptr;
}
}

DockSeparator::DockSeparator(Core::Separator* controller, QQuickItem* parent)
    : KDDockWidgets::QtQuick::Separator(controller, parent),
    m_isSeparatorVisible(true)
{
    QTimer::singleShot(0, this, [this]() {
        // Set inited before emitting showResizeCursorChanged so QML bindings
        // can safely call showResizeCursor() — minPosForSeparator_global() asserts
        // if called during construction before the separator is in the layout.
        m_inited = true;
        emit showResizeCursorChanged();
        initAvailability();
    });
}

void DockSeparator::initAvailability()
{
    const QObject* dock = findNearestDockView(this);
    DockProperties properties = readPropertiesFromObject(dock);

    if (properties.isValid()) {
        m_isSeparatorVisible = properties.separatorsVisible;
        emit isSeparatorVisibleChanged();
        emit showResizeCursorChanged();
    }
}

bool DockSeparator::isSeparatorVisible() const
{
    return m_isSeparatorVisible;
}

bool DockSeparator::showResizeCursor() const
{
    if (!m_inited) {
        // Called during QML item construction before the separator is in the layout.
        // minPosForSeparator_global() would assert — return false until inited.
        return false;
    }
    auto* coreSepa = static_cast<Core::Separator*>(controller());
    auto* layoutingSep = coreSepa->asLayoutingSeparator();
    auto* container = layoutingSep->parentContainer();
    return container
           && (container->minPosForSeparator_global(layoutingSep)
               != container->maxPosForSeparator_global(layoutingSep));
}

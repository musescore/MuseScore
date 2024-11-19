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

#include "dockseparator.h"

#include "log.h"
#include "../docktypes.h"

#include "thirdparty/KDDockWidgets/src/core/Frame_p.h"
#include "thirdparty/KDDockWidgets/src/qtquick/views/Rubberband.h"
#include "thirdparty/KDDockWidgets/src/core/DockWidget.h"
#include "thirdparty/KDDockWidgets/src/core/layouting/Item_p.h"
#include "thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"

#include <QTimer>

using namespace muse::dock;

namespace muse::dock {
static const KDDockWidgets::Core::DockWidget* findNearestDock(const DockSeparator* separator)
{
    const KDDockWidgets::Core::ItemBoxContainer* container = separator->parentContainer();
    if (!container) {
        return nullptr;
    }

    int separatorPos = separator->KDDockWidgets::Core::LayoutingSeparator::position();
    Qt::Orientation orientation = separator->orientation();
    KDDockWidgets::Core::Item::List children = container->visibleChildren();

    const KDDockWidgets::Core::Item* nearestItem = nullptr;
    int minPosDiff = std::numeric_limits<int>::max();

    for (const KDDockWidgets::Core::Item* child : children) {
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

    auto frame = dynamic_cast<KDDockWidgets::Frame*>(nearestItem->guestAsQObject());
    return frame && !frame->isEmpty() ? frame->currentDockWidget() : nullptr;
}
}

DockSeparator::DockSeparator(KDDockWidgets::Core::LayoutingHost* parent)
    : QQuickItem(qobject_cast<QQuickItem*>(parent->m_rootItem())),
    KDDockWidgets::Core::LayoutingSeparator(parent, isVertical() ? Qt::Vertical : Qt::Horizontal, m_parentContainer),
    KDDockWidgets::Core::ViewFactory(), m_isSeparatorVisible(true)
{
    createQQuickItem("qrc:/qml/Muse/Dock/DockSeparator.qml", this);

    // Only set on Separator::init(), so single-shot
    QTimer::singleShot(0, this, &DockSeparator::isVerticalChanged);
    QTimer::singleShot(0, this, &DockSeparator::showResizeCursorChanged);
    QTimer::singleShot(0, this, &DockSeparator::initAvailability);
}

void DockSeparator::initAvailability()
{
    const KDDockWidgets::Core::DockWidget* dock = findNearestDock(this);
    DockProperties properties = readPropertiesFromObject(dock);

    if (properties.isValid()) {
        m_isSeparatorVisible = properties.separatorsVisible;
        emit isSeparatorVisibleChanged();
        emit showResizeCursorChanged();
    }
}

bool DockSeparator::isVertical() const
{
    return KDDockWidgets::Core::LayoutingSeparator::isVertical();
}

bool DockSeparator::isSeparatorVisible() const
{
    return m_isSeparatorVisible;
}

bool DockSeparator::showResizeCursor() const
{
    return parentContainer()
           && (parentContainer()->minPosForSeparator_global(const_cast<DockSeparator*>(this))
               != parentContainer()->maxPosForSeparator_global(const_cast<DockSeparator*>(this)));
}

KDDockWidgets::Core::View* DockSeparator::createRubberBand(KDDockWidgets::Core::View* parent) const
{
    if (!parent) {
        LOGE() << "Parent is required";
        return nullptr;
    }

    return new KDDockWidgets::Core::View(new KDDockWidgets::QtQuick::RubberBand(parent));
}

void DockSeparator::onMousePressed()
{
    KDDockWidgets::Core::LayoutingSeparator::onMousePress();
}

void DockSeparator::onMouseMoved(QPointF localPos)
{
    const QPointF pos = QQuickItem::mapToItem(parentItem(), localPos);
    KDDockWidgets::Core::LayoutingSeparator::onMouseMove(pos.toPoint());
}

void DockSeparator::onMouseReleased()
{
    KDDockWidgets::Core::LayoutingSeparator::onMouseRelease();
}

void DockSeparator::onMouseDoubleClicked()
{
    // KDDockWidgets::Core::LayoutingSeparator::onMouseDoubleClick();
}

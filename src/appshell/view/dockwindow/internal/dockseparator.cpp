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

#include "thirdparty/KDDockWidgets/src/private/multisplitter/Rubberband_quick.h"

#include <QTimer>

using namespace mu::dock;

DockSeparator::DockSeparator(Layouting::Widget* parent)
    : QQuickItem(qobject_cast<QQuickItem*>(parent->asQObject())),
    Layouting::Separator(parent),
    Layouting::Widget_quick(this)
{
    createQQuickItem("qrc:/qml/dockwindow/DockSeparator.qml", this);

    // Only set on Separator::init(), so single-shot
    QTimer::singleShot(0, this, &DockSeparator::isVerticalChanged);
}

bool DockSeparator::isVertical() const
{
    return Layouting::Separator::isVertical();
}

Layouting::Widget* DockSeparator::createRubberBand(Layouting::Widget* parent)
{
    if (!parent) {
        LOGE() << "Parent is required";
        return nullptr;
    }

    return new Layouting::Widget_quick(new Layouting::RubberBand(parent));
}

Layouting::Widget* DockSeparator::asWidget()
{
    return this;
}

void DockSeparator::onMousePressed()
{
    Layouting::Separator::onMousePress();
}

void DockSeparator::onMouseMoved(QPointF localPos)
{
    const QPointF pos = QQuickItem::mapToItem(parentItem(), localPos);
    Layouting::Separator::onMouseMove(pos.toPoint());
}

void DockSeparator::onMouseReleased()
{
    Layouting::Separator::onMouseReleased();
}

void DockSeparator::onMouseDoubleClicked()
{
    Layouting::Separator::onMouseDoubleClick();
}

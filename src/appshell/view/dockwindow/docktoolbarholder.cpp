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

#include "docktoolbarholder.h"

using namespace mu::dock;

DockToolBarHolder::DockToolBarHolder(QQuickItem* parent)
    : DockToolBarView(parent)
{
    setVisible(false);
}

void DockToolBarHolder::componentComplete()
{
    DockToolBarView::componentComplete();

    switch (location()) {
    case DockLocation::Left:
    case DockLocation::Right:
        setWidth(MIN_SIDE_SIZE);
        setMinimumWidth(MIN_SIDE_SIZE);
        setMaximumWidth(MIN_SIDE_SIZE * 2);
        break;
    case DockLocation::Top:
    case DockLocation::Bottom:
        setHeight(MIN_SIDE_SIZE);
        setMinimumHeight(MIN_SIDE_SIZE);
        setMaximumHeight(MIN_SIDE_SIZE);
        break;
    case DockLocation::Center:
    case DockLocation::Undefined:
        break;
    }
}

DockType DockToolBarHolder::type() const
{
    return DockType::ToolBarDockingHolder;
}

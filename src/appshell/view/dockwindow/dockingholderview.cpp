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

#include "dockingholderview.h"

using namespace mu::dock;

constexpr int MIN_LENGTH = 36;

DockingHolderView::DockingHolderView(QQuickItem* parent)
    : DockBase(parent)
{
    setVisible(false);
}

void DockingHolderView::componentComplete()
{
    switch (location()) {
    case DockLocation::Left:
    case DockLocation::Right:
        setMinimumWidth(MIN_LENGTH);
        setWidth(MIN_LENGTH);
        break;
    case DockLocation::Top:
    case DockLocation::Bottom:
        setMinimumHeight(MIN_LENGTH);
        setHeight(MIN_LENGTH);
        break;
    case DockLocation::Center:
    case DockLocation::Undefined:
        break;
    }

    DockBase::componentComplete();
}

DockType DockingHolderView::type() const
{
    return DockType::DockingHolder;
}

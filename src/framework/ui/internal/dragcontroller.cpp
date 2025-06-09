/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "dragcontroller.h"

#include <QDropEvent>

using namespace muse::ui;

void DragController::setCurrentData(const DragDataPtr& data)
{
    m_dragData = data;
}

const DragDataPtr& DragController::dragData() const
{
    return m_dragData;
}

const QMimeData* DragController::mimeData(const QDropEvent* event) const
{
    const QMimeData* md = nullptr;
    if (configuration()->isSystemDragSupported() && event) {
        md = event->mimeData();
    }

    if (md) {
        return md;
    }

    if (m_dragData) {
        return &m_dragData->mimeData;
    }

    return nullptr;
}

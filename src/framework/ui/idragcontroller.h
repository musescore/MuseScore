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
#pragma once

#include <memory>
#include <QMimeData>

#include "global/modularity/imoduleinterface.h"

class QDropEvent;
class QMimeData;

namespace muse::ui {
struct DragData {
    QMimeData mimeData;
};

using DragDataPtr = std::shared_ptr<DragData>;

class IDragController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IDragController)

public:

    virtual ~IDragController() = default;

    virtual void setCurrentData(const DragDataPtr& data) = 0;

    virtual const DragDataPtr& dragData() const = 0;
    virtual const QMimeData* mimeData(const QDropEvent* event = nullptr) const = 0;
};
}

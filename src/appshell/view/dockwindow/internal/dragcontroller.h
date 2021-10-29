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

#ifndef MU_DOCK_DRAGCONTROLLER_H
#define MU_DOCK_DRAGCONTROLLER_H

#include "modularity/ioc.h"
#include "../idockwindowprovider.h"

#include "thirdparty/KDDockWidgets/src/private/DropIndicatorOverlayInterface_p.h"

namespace mu::dock {
class DragController : public KDDockWidgets::DropIndicatorOverlayInterface
{
    INJECT(dock, IDockWindowProvider, dockWindowProvider)

public:
    explicit DragController(KDDockWidgets::DropArea* dropArea);

    DropLocation hover_impl(QPoint globalPos) override;
    QPoint posForIndicator(DropLocation) const override;

private:
    void updateVisibility() override;

    IDockWindow* dockWindow() const;
};
}

#endif // MU_DOCK_DRAGCONTROLLER_H

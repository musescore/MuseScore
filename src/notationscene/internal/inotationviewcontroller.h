/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "notation/types/viewmode.h"

namespace mu::engraving {
class EngravingItem;
}

namespace mu::notation {
class INotationViewController
{
public:
    virtual ~INotationViewController() = default;

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void zoomToPageWidth() = 0;
    virtual void zoomToWholePage() = 0;
    virtual void zoomToTwoPages() = 0;
    virtual void setZoom(int zoomPercentage) = 0;

    virtual void setViewMode(ViewMode viewMode) = 0;

    virtual void nextScreen() = 0;
    virtual void previousScreen() = 0;
    virtual void nextPage() = 0;
    virtual void previousPage() = 0;
    virtual void startOfScore() = 0;
    virtual void endOfScore() = 0;

    virtual void openContextMenuOfSelection() = 0;

    virtual void togglePopupForItemIfSupports(const mu::engraving::EngravingItem* item) = 0;

    virtual void showSearch() = 0;

    // diagnostic
    virtual void redrawView() = 0;
};
}

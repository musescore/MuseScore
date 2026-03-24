/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "../inotationcontextconfiguration.h"
#include "global/modularity/ioc.h"
#include "ui/iuicontextconfiguration.h"

namespace mu::notation {
class NotationContextConfiguration : public INotationContextConfiguration, public muse::Contextable
{
    muse::ContextInject<muse::ui::IUiContextConfiguration> uiConfiguration = { this };

public:
    NotationContextConfiguration(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    double guiScaling() const override;
    double notationScaling() const override;

    qreal scalingFromZoomPercentage(int zoomPercentage) const override;
    int zoomPercentageFromScaling(qreal scaling) const override;
};
}

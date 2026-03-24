/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "../iuicontextconfiguration.h"

#include "global/modularity/ioc.h"
#include "../imainwindow.h"
#include "../iuiconfiguration.h"

class QScreen;

namespace muse::ui {
class UiContextConfiguration : public IUiContextConfiguration, public muse::Contextable
{
    GlobalInject<IUiConfiguration> configuration;
    ContextInject<IMainWindow> mainWindow = { this };

public:
    UiContextConfiguration(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    double guiScaling() const override;
    double physicalDpi() const override;
    double logicalDpi() const override;

private:
    QScreen* screen() const;
};
}

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and redistribute it and/or modify
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

#include "../iappshellstate.h"

#include "modularity/ioc.h"
#include "ui/iuistate.h"

namespace mu::appshell {
class AppShellState : public IAppShellState, public muse::Contextable
{
    muse::ContextInject<muse::ui::IUiState> uistate = { this };

public:
    AppShellState(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    bool isNotationNavigatorVisible() const override;
    void setIsNotationNavigatorVisible(bool visible) override;
    muse::async::Notification isNotationNavigatorVisibleChanged() const override;
};
}

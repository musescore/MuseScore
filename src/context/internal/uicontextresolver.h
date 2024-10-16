/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_CONTEXT_UICONTEXTRESOLVER_H
#define MU_CONTEXT_UICONTEXTRESOLVER_H

#include "../iuicontextresolver.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "playback/iplaybackcontroller.h"
#include "iinteractive.h"
#include "../iglobalcontext.h"
#include "ui/inavigationcontroller.h"

namespace mu::context {
class UiContextResolver : public IUiContextResolver, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<IGlobalContext> globalContext = { this };
    muse::Inject<muse::ui::INavigationController> navigationController = { this };

public:
    UiContextResolver(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void init();

    muse::ui::UiContext currentUiContext() const override;
    muse::async::Notification currentUiContextChanged() const override;

    bool match(const muse::ui::UiContext& currentCtx, const muse::ui::UiContext& actCtx) const override;
    bool matchWithCurrent(const muse::ui::UiContext& ctx) const override;

    bool isShortcutContextAllowed(const std::string& scContext) const override;

private:
    void notifyAboutContextChanged();

    muse::async::Notification m_currentUiContextChanged;
};
}

#endif // MU_CONTEXT_UICONTEXTRESOLVER_H

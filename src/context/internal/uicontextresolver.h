//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_CONTEXT_UICONTEXTRESOLVER_H
#define MU_CONTEXT_UICONTEXTRESOLVER_H

#include "../iuicontextresolver.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "playback/iplaybackcontroller.h"
#include "iinteractive.h"
#include "../iglobalcontext.h"

namespace mu::context {
class UiContextResolver : public IUiContextResolver, public async::Asyncable
{
    INJECT(context, framework::IInteractive, interactive)
    INJECT(context, playback::IPlaybackController, playbackController)
    INJECT(context, IGlobalContext, globalContext)

public:
    UiContextResolver() = default;

    void init();

    ui::UiContext currentUiContext() const override;
    async::Notification currentUiContextChanged() const override;

    bool match(const ui::UiContext& currentCtx, const ui::UiContext& actCtx) const override;
    bool matchWithCurrent(const ui::UiContext& ctx) const override;

private:
    async::Notification m_currentUiContextChanged;
};
}

#endif // MU_CONTEXT_UICONTEXTRESOLVER_H

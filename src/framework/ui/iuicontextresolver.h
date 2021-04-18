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
#ifndef MU_UI_IUICONTEXTRESOLVER_H
#define MU_UI_IUICONTEXTRESOLVER_H

#include <memory>
#include "modularity/imoduleexport.h"
#include "uitypes.h"
#include "async/notification.h"

namespace mu::ui {
class IUiContextResolver : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUiContextResolver)

public:
    virtual ~IUiContextResolver() = default;

    virtual ui::UiContext currentUiContext() const = 0;
    virtual async::Notification currentUiContextChanged() const = 0;

    virtual bool match(const ui::UiContext& currentCtx, const ui::UiContext& actCtx) const = 0;
    virtual bool matchWithCurrent(const ui::UiContext& ctx) const = 0;
};

using IUiContextResolverPtr = std::shared_ptr<IUiContextResolver>;
}

#endif // MU_UI_IUICONTEXTRESOLVER_H

//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_UI_IUIACTIONSREGISTER_H
#define MU_UI_IUIACTIONSREGISTER_H

#include <memory>
#include "modularity/imoduleexport.h"
#include "iuiactionsmodule.h"
#include "uitypes.h"
#include "async/channel.h"

namespace mu::ui {
class IUiActionsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUiActionsRegister)

public:
    virtual ~IUiActionsRegister() = default;

    virtual void reg(const IUiActionsModulePtr& actions) = 0;

    virtual const UiAction& action(const actions::ActionCode& code) const = 0;
    virtual UiActionState actionState(const actions::ActionCode& code) const = 0;
    virtual async::Channel<actions::ActionCodeList> actionStateChanged() const = 0;
};
}

#endif // MU_UI_IUIACTIONSREGISTER_H

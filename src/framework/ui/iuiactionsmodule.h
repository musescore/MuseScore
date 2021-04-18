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
#ifndef MU_UI_IUIACTIONSMODULE_H
#define MU_UI_IUIACTIONSMODULE_H

#include <memory>
#include "uitypes.h"
#include "async/channel.h"

namespace mu::ui {
class IUiActionsModule
{
public:
    virtual ~IUiActionsModule() = default;

    virtual const UiActionList& actionsList() const = 0;
    virtual bool actionEnabled(const UiAction& act) const = 0;
    virtual async::Channel<actions::ActionCodeList> actionEnabledChanged() const = 0;

    virtual bool actionChecked(const UiAction& act) const = 0;
    virtual async::Channel<actions::ActionCodeList> actionCheckedChanged() const = 0;
};
using IUiActionsModulePtr = std::shared_ptr<IUiActionsModule>;
}

#endif // MU_UI_IUIACTIONSMODULE_H

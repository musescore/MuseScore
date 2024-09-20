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
#ifndef MUSE_UI_IUIACTIONSMODULE_H
#define MUSE_UI_IUIACTIONSMODULE_H

#include <memory>

#include "global/async/channel.h"

#include "uiaction.h"

namespace muse::ui {
class IUiActionsModule
{
public:
    virtual ~IUiActionsModule() = default;

    virtual const UiActionList& actionsList() const = 0;
    virtual async::Channel<UiActionList> actionsChanged() const
    {
        //! NOTE Usually actions don't change,
        //! so let's add a default implementation here.
        static async::Channel<UiActionList> ch;
        return ch;
    }

    virtual bool actionEnabled(const UiAction& act) const = 0;
    virtual async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const = 0;

    virtual bool actionChecked(const UiAction& act) const = 0;
    virtual async::Channel<muse::actions::ActionCodeList> actionCheckedChanged() const = 0;
};
using IUiActionsModulePtr = std::shared_ptr<IUiActionsModule>;
}

#endif // MUSE_UI_IUIACTIONSMODULE_H

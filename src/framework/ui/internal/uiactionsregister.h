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
#ifndef MUSE_UI_UIACTIONSREGISTER_H
#define MUSE_UI_UIACTIONSREGISTER_H

#include <vector>
#include <unordered_map>

#include "../iuiactionsregister.h"
#include "modularity/ioc.h"
#include "shortcuts/ishortcutsregister.h"
#include "iuicontextresolver.h"
#include "async/asyncable.h"

namespace muse::ui {
class UiActionsRegister : public IUiActionsRegister, public Injectable, public async::Asyncable
{
    Inject<IUiContextResolver> uicontextResolver = { this };
    Inject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };

public:
    UiActionsRegister(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    void reg(const IUiActionsModulePtr& actions) override;

    std::vector<UiAction> actionList() const override;

    const UiAction& action(const actions::ActionCode& code) const override;
    async::Channel<UiActionList> actionsChanged() const override;

    UiActionState actionState(const actions::ActionCode& code) const override;
    async::Channel<actions::ActionCodeList> actionStateChanged() const override;

private:

    struct Info
    {
        IUiActionsModulePtr module;
        ui::UiAction action;
        ui::UiActionState state;

        bool isValid() const
        {
            return module != nullptr && action.isValid();
        }
    };

    Info& info(const actions::ActionCode& code);
    const Info& info(const actions::ActionCode& code) const;

    void updateShortcuts(const actions::ActionCodeList& codes);
    void updateShortcutsAll();

    void updateActions(const UiActionList& actions);

    void updateEnabled(const actions::ActionCodeList& codes);
    void updateEnabledAll();
    void requestUpdateEnabledAll();
    void doUpdateEnabled(Info& inf, const IUiContextResolverPtr& ctxResolver, const UiContext& currentCtx,
                         actions::ActionCodeList& changedList);

    void updateCheckedAll();
    void updateChecked(const actions::ActionCodeList& codes);

    std::unordered_map<actions::ActionCode, Info> m_actions;
    async::Channel<UiActionList> m_actionsChanged;
    async::Channel<actions::ActionCodeList> m_actionStateChanged;

    bool m_isUpdateEnabledAllRequested = false;
};
}

#endif // MUSE_UI_UIACTIONSREGISTER_H

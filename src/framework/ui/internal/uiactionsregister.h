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
#ifndef MU_UI_UIACTIONSREGISTER_H
#define MU_UI_UIACTIONSREGISTER_H

#include <vector>
#include <unordered_map>

#include "../iuiactionsregister.h"
#include "modularity/ioc.h"
#include "shortcuts/ishortcutsregister.h"
#include "iuicontextresolver.h"
#include "async/asyncable.h"

namespace mu::ui {
class UiActionsRegister : public IUiActionsRegister, public async::Asyncable
{
    INJECT(ui, IUiContextResolver, uicontextResolver)
    INJECT(ui, shortcuts::IShortcutsRegister, shortcutsRegister)
public:
    UiActionsRegister() = default;

    void init();

    void reg(const IUiActionsModulePtr& actions) override;
    const UiAction& action(const actions::ActionCode& code) const override;
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

    void updateShortcuts();

    void updateEnabled(const actions::ActionCodeList& codes);
    void updateEnabledAll();
    void doUpdateEnabled(Info& inf,const IUiContextResolverPtr& ctxResolver,const UiContext& currentCtx,
                         actions::ActionCodeList& changedList);

    void updateCheckedAll();
    void updateChecked(const actions::ActionCodeList& codes);

    std::unordered_map<actions::ActionCode, Info> m_actions;
    async::Channel<actions::ActionCodeList> m_actionStateChanged;
};
}

#endif // MU_UI_UIACTIONSREGISTER_H

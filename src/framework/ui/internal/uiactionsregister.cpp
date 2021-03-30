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
#include "uiactionsregister.h"

#include "log.h"

using namespace mu::ui;
using namespace mu::actions;

void UiActionsRegister::init()
{
    // init state
    //! NOTE updateEnabledAll is not needed because the UI context will change and then call updateEnabledAll

    updateCheckedAll();

    updateShortcuts();

    // listen
    uicontextResolver()->currentUiContextChanged().onNotify(this, [this]() {
        updateEnabledAll();
    });

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        updateShortcuts();
    });
}

void UiActionsRegister::reg(const IUiActionsModulePtr& module)
{
    const UiActionList& alist = module->actionsList();
    for (const UiAction& a : alist) {
        Info info;
        info.module = module;
        info.action = a;
        m_actions[a.code] = std::move(info);
    }

    module->actionEnabledChanged().onReceive(this, [this](const ActionCodeList& codes) {
        updateEnabled(codes);
        m_actionStateChanged.send(codes);
    });

    module->actionCheckedChanged().onReceive(this, [this](const ActionCodeList& codes) {
        updateChecked(codes);
        m_actionStateChanged.send(codes);
    });
}

UiActionsRegister::Info& UiActionsRegister::info(const actions::ActionCode& code)
{
    auto it = m_actions.find(code);
    if (it != m_actions.end()) {
        return it->second;
    }

    static Info null;
    return null;
}

const UiActionsRegister::Info& UiActionsRegister::info(const actions::ActionCode& code) const
{
    auto it = m_actions.find(code);
    if (it != m_actions.end()) {
        return it->second;
    }

    static Info null;
    return null;
}

const UiAction& UiActionsRegister::action(const ActionCode& code) const
{
    return info(code).action;
}

UiActionState UiActionsRegister::actionState(const ActionCode& code) const
{
    const Info& inf = info(code);
    if (!inf.action.isValid()) {
        LOGE() << "not found action with code: " << code;
        return UiActionState::make_disabled();
    }

    return inf.state;
}

void UiActionsRegister::updateShortcuts()
{
    TRACEFUNC;

    auto screg = shortcutsRegister();
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
        Info& inf = it->second;
        inf.action.shortcut = screg->shortcut(inf.action.code).sequence;
    }
}

void UiActionsRegister::doUpdateEnabled(Info& inf,
                                        const IUiContextResolverPtr& ctxResolver,
                                        const UiContext& currentCtx,
                                        ActionCodeList& changedList)
{
    bool oldEnabled = inf.state.enabled;
    if (!ctxResolver->match(currentCtx, inf.action.context)) {
        inf.state.enabled = false;
    } else {
        inf.state.enabled = inf.module->actionEnabled(inf.action);
    }

    if (oldEnabled != inf.state.enabled) {
        changedList.push_back(inf.action.code);
    }
}

void UiActionsRegister::updateEnabled(const actions::ActionCodeList& codes)
{
    TRACEFUNC;

    ActionCodeList changedList;
    auto ctxResolver = uicontextResolver();
    ui::UiContext currentCtx = ctxResolver->currentUiContext();
    for (const actions::ActionCode& code : codes) {
        Info& inf = info(code);
        if (!inf.isValid()) {
            continue;
        }

        doUpdateEnabled(inf, ctxResolver, currentCtx, changedList);
    }

    if (!changedList.empty()) {
        m_actionStateChanged.send(changedList);
    }
}

void UiActionsRegister::updateEnabledAll()
{
    TRACEFUNC;

    ActionCodeList changedList;
    auto ctxResolver = uicontextResolver();
    ui::UiContext currentCtx = ctxResolver->currentUiContext();
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
        Info& inf = it->second;
        doUpdateEnabled(inf, ctxResolver, currentCtx, changedList);
    }

    if (!changedList.empty()) {
        m_actionStateChanged.send(changedList);
    }
}

void UiActionsRegister::updateCheckedAll()
{
    TRACEFUNC;

    ActionCodeList changedList;
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
        Info& inf = it->second;

        bool oldChecked = inf.state.checked;
        inf.state.checked = inf.module->actionChecked(inf.action);

        if (oldChecked != inf.state.checked) {
            changedList.push_back(inf.action.code);
        }
    }

    if (!changedList.empty()) {
        m_actionStateChanged.send(changedList);
    }
}

void UiActionsRegister::updateChecked(const actions::ActionCodeList& codes)
{
    TRACEFUNC;

    ActionCodeList changedList;
    for (const actions::ActionCode& code : codes) {
        Info& inf = info(code);
        if (!inf.isValid()) {
            continue;
        }

        bool oldChecked = inf.state.checked;
        inf.state.checked = inf.module->actionChecked(inf.action);

        if (oldChecked != inf.state.checked) {
            changedList.push_back(inf.action.code);
        }
    }

    if (!changedList.empty()) {
        m_actionStateChanged.send(changedList);
    }
}

mu::async::Channel<ActionCodeList> UiActionsRegister::actionStateChanged() const
{
    return m_actionStateChanged;
}

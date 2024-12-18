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
#include "uiactionsregister.h"

#include "log.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

void UiActionsRegister::init()
{
    // init state
    updateCheckedAll();
    updateEnabledAll();

    updateShortcutsAll();

    // listen
    uicontextResolver()->currentUiContextChanged().onNotify(this, [this]() {
        updateEnabledAll();
    });

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        updateShortcutsAll();
    });
}

void UiActionsRegister::reg(const IUiActionsModulePtr& module)
{
    const UiActionList& alist = module->actionsList();
    ActionCodeList newActionCodeList;
    for (const UiAction& action : alist) {
        Info info;
        info.module = module;
        info.action = action;
        m_actions[action.code] = std::move(info);

        newActionCodeList.push_back(action.code);
    }

    updateEnabled(newActionCodeList);
    updateChecked(newActionCodeList);
    updateShortcuts(newActionCodeList);

    module->actionsChanged().onReceive(this, [this](const UiActionList& actions) {
        updateActions(actions);
    });

    module->actionEnabledChanged().onReceive(this, [this](const ActionCodeList& codes) {
        updateEnabled(codes);
        m_actionStateChanged.send(codes);
    });

    module->actionCheckedChanged().onReceive(this, [this](const ActionCodeList& codes) {
        updateChecked(codes);
        m_actionStateChanged.send(codes);
    });
}

UiActionsRegister::Info& UiActionsRegister::info(const ActionCode& code)
{
    auto it = m_actions.find(code);
    if (it != m_actions.end()) {
        return it->second;
    }

    static Info null;
    return null;
}

const UiActionsRegister::Info& UiActionsRegister::info(const ActionCode& code) const
{
    auto it = m_actions.find(code);
    if (it != m_actions.end()) {
        return it->second;
    }

    static Info null;
    return null;
}

std::vector<UiAction> UiActionsRegister::actionList() const
{
    std::vector<UiAction> allActions;

    for (auto it = m_actions.begin(); it != m_actions.end(); it++) {
        allActions.push_back(it->second.action);
    }

    return allActions;
}

const UiAction& UiActionsRegister::action(const ActionCode& code) const
{
    return info(code).action;
}

async::Channel<UiActionList> UiActionsRegister::actionsChanged() const
{
    return m_actionsChanged;
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

void UiActionsRegister::updateShortcuts(const ActionCodeList& codes)
{
    auto screg = shortcutsRegister();
    for (const ActionCode& code : codes) {
        Info& inf = info(code);
        if (!inf.isValid()) {
            continue;
        }

        inf.action.shortcuts = screg->shortcut(inf.action.code).sequences;
    }
}

void UiActionsRegister::updateShortcutsAll()
{
    TRACEFUNC;

    auto screg = shortcutsRegister();
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
        Info& inf = it->second;
        inf.action.shortcuts = screg->shortcut(inf.action.code).sequences;
    }
}

void UiActionsRegister::updateActions(const UiActionList& actions)
{
    ActionCodeList codes;
    for (const UiAction& act : actions) {
        Info& inf = info(act.code);
        IF_ASSERT_FAILED(inf.isValid()) {
            continue;
        }

        inf.action = act;

        codes.push_back(act.code);
    }

    m_actionsChanged.send(actions);

    updateEnabled(codes);
    updateChecked(codes);
}

void UiActionsRegister::doUpdateEnabled(Info& inf,
                                        const IUiContextResolverPtr& ctxResolver,
                                        const UiContext& currentCtx,
                                        ActionCodeList& changedList)
{
    bool oldEnabled = inf.state.enabled;
    inf.state.enabled = false;

    if (ctxResolver->match(currentCtx, inf.action.uiCtx)) {
        inf.state.enabled = inf.module->actionEnabled(inf.action);
    }

    if (oldEnabled != inf.state.enabled) {
        changedList.push_back(inf.action.code);
    }
}

void UiActionsRegister::updateEnabled(const ActionCodeList& codes)
{
    TRACEFUNC;

    ActionCodeList changedList;
    auto ctxResolver = uicontextResolver();
    ui::UiContext currentCtx = ctxResolver->currentUiContext();
    for (const ActionCode& code : codes) {
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
    LOGD() << "currentCtx: " << currentCtx.toString();
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

void UiActionsRegister::updateChecked(const ActionCodeList& codes)
{
    TRACEFUNC;

    ActionCodeList changedList;
    for (const ActionCode& code : codes) {
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

async::Channel<ActionCodeList> UiActionsRegister::actionStateChanged() const
{
    return m_actionStateChanged;
}

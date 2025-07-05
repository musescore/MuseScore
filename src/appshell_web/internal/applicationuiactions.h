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
#ifndef MU_APPSHELL_APPLICATIONUIACTIONS_H
#define MU_APPSHELL_APPLICATIONUIACTIONS_H

#include "ui/iuiactionsmodule.h"

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "braille/ibrailleconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "dockwindow/idockwindowprovider.h"
#include "../iappshellconfiguration.h"

#include "../appshelltypes.h"
#include "applicationactioncontroller.h"

namespace mu::appshell {
class ApplicationUiActions : public muse::ui::IUiActionsModule, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::ui::IMainWindow> mainWindow = { this };
    muse::Inject<muse::dock::IDockWindowProvider> dockWindowProvider = { this };
    muse::Inject<IAppShellConfiguration> configuration = { this };
    muse::Inject<braille::IBrailleConfiguration> brailleConfiguration = { this };
    muse::Inject<mu::notation::INotationConfiguration> notationConfiguration = { this };

public:
    ApplicationUiActions(std::shared_ptr<ApplicationActionController> controller, const muse::modularity::ContextPtr& iocCtx);

    void init();

    const muse::ui::UiActionList& actionsList() const override;

    bool actionEnabled(const muse::ui::UiAction& act) const override;
    muse::async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const muse::ui::UiAction& act) const override;
    muse::async::Channel<muse::actions::ActionCodeList> actionCheckedChanged() const override;

    static const QMap<muse::actions::ActionCode, DockName>& toggleDockActions();

private:
    void listenOpenedDocksChanged(muse::dock::IDockWindow* window);

    static const muse::ui::UiActionList m_actions;

    std::shared_ptr<ApplicationActionController> m_controller;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionEnabledChanged;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_APPSHELL_APPLICATIONUIACTIONS_H

/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include "icloseprojectscenario.h"

#include <set>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "async/promise.h"
#include "context/iglobalcontext.h"
#include "interactive/iinteractive.h"
#include "rcommand/icommanddispatcher.h"

#include "inotationproject.h"
#include "isaveprojectscenario.h"

namespace mu::project {
class CloseProjectScenario : public ICloseProjectScenario, public muse::Contextable, public muse::async::Asyncable
{
public:
    muse::ContextInject<ISaveProjectScenario> saveProjectScenario = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };

    CloseProjectScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    muse::async::Promise<muse::Ret> closeOpenedProject(bool goToHome = true) override;

    bool isBusy(BusyStatus status) const override;
    muse::async::Notification busyChanged() const override;

private:
    static muse::async::Promise<muse::Ret> resolvedPromise(const muse::Ret& ret);

    INotationProjectPtr currentNotationProject() const;

    void setBusy(BusyStatus status, bool isBusy);
    muse::async::Promise<muse::Ret> runIfNotBusy(BusyStatus status, const std::function<muse::async::Promise<muse::Ret>()>& flow);

    muse::async::Promise<muse::IInteractive::Result> askAboutSavingScore(const INotationProjectPtr& project);

    //! NOTE Lets go of the score, and of everything that was opened for it
    muse::async::Promise<muse::Ret> doCloseProject(bool goToHome);

    void openHomePageIfNeed();

    std::set<BusyStatus> m_busyStatuses;
    muse::async::Notification m_busyChanged;
};
}

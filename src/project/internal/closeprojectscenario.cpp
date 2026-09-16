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

#include "closeprojectscenario.h"

#include "translation.h"
#include "types/projecturis.h"

#include "log.h"

using namespace mu::project;
using namespace muse;
using muse::async::Promise;

Promise<Ret> CloseProjectScenario::resolvedPromise(const Ret& ret)
{
    return async::make_promise<Ret>([ret](auto resolve) {
        return resolve(ret);
    });
}

Promise<Ret> CloseProjectScenario::closeOpenedProject(bool goToHome)
{
    return runIfNotBusy(BusyStatus::Closing, [this, goToHome]() -> Promise<Ret> {
        INotationProjectPtr project = currentNotationProject();
        if (!project) {
            return resolvedPromise(make_ok());
        }

        if (globalContext()->playbackState()->isPlaying()) {
            commandDispatcher()->dispatch(rcommand::Command("command://playback/stop"));
        }

        if (!project->isNeedSave()) {
            return doCloseProject(goToHome);
        }

        return askAboutSavingScore(project)
               .then<Ret>(this, [this, goToHome](const IInteractive::Result& res, auto resolve) {
            IInteractive::Button btn = res.standardButton();

            if (btn == IInteractive::Button::Cancel) {
                return resolve(make_ret(Ret::Code::Cancel));
            }

            if (btn != IInteractive::Button::Save) {
                doCloseProject(goToHome).onResolve(this, [resolve](const Ret& ret) {
                    (void)resolve(ret);
                });

                return Promise<Ret>::dummy_result();
            }

            //! NOTE The score is only let go of once its changes are safely written
            saveProjectScenario()->saveProject().onResolve(this, [this, goToHome, resolve](const Ret& ret) {
                if (!ret) {
                    (void)resolve(ret);
                    return;
                }

                doCloseProject(goToHome).onResolve(this, [resolve](const Ret& closeRet) {
                    (void)resolve(closeRet);
                });
            });

            return Promise<Ret>::dummy_result();
        });
    });
}

bool CloseProjectScenario::isBusy(BusyStatus status) const
{
    return m_busyStatuses.contains(status);
}

muse::async::Notification CloseProjectScenario::busyChanged() const
{
    return m_busyChanged;
}

INotationProjectPtr CloseProjectScenario::currentNotationProject() const
{
    return globalContext()->currentProject();
}

void CloseProjectScenario::setBusy(BusyStatus status, bool isBusy)
{
    bool wasBusy = m_busyStatuses.contains(status);
    if (wasBusy == isBusy) {
        return;
    }

    if (isBusy) {
        m_busyStatuses.insert(status);
    } else {
        m_busyStatuses.erase(status);
    }

    m_busyChanged.notify();
}

Promise<Ret> CloseProjectScenario::runIfNotBusy(BusyStatus status, const std::function<Promise<Ret>()>& flow)
{
    if (isBusy(status)) {
        return resolvedPromise(make_ret(Ret::Code::Busy));
    }

    setBusy(status, true);

    return flow().then<Ret>(this, [this, status](const Ret& ret, auto resolve) {
        setBusy(status, false);
        return resolve(ret);
    });
}

Promise<IInteractive::Result> CloseProjectScenario::askAboutSavingScore(const INotationProjectPtr& project)
{
    std::string title = muse::qtrc("project", "Do you want to save changes to the score “%1” before closing?")
                        .arg(project->displayName()).toStdString();

    std::string body = muse::trc("project", "Your changes will be lost if you don’t save them.");

    return interactive()->warning(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save);
}

Promise<Ret> CloseProjectScenario::doCloseProject(bool goToHome)
{
    return interactive()->closeAllDialogs().then<Ret>(this, [this, goToHome](const Ret&, auto resolve) {
        globalContext()->setCurrentProject(nullptr);

        if (goToHome) {
            openHomePageIfNeed();
        }

        return resolve(make_ok());
    });
}

void CloseProjectScenario::openHomePageIfNeed()
{
    if (interactive()->isOpened(HOME_PAGE_URI).val) {
        return;
    }

    interactive()->open(HOME_PAGE_URI);
}
